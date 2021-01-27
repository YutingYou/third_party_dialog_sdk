/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup HAPTIC
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file hw_haptic.c
 *
 * @brief Implementation of the Haptic Low Level Driver.
 *
 * Copyright (C) 2018-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if ((DEVICE_VARIANT == DA14697) || (DEVICE_VARIANT == DA14699))

#if (dg_configUSE_HW_ERM || dg_configUSE_HW_LRA)

#include "hw_haptic.h"
#include "hw_clk.h"


/**************** INTERNAL DATA TYPE DEFINITIONS ****************/


typedef struct {
        uint16_t duty_cycle_nom_max;
        uint16_t duty_cycle_abs_max;
        uint32_t inv_duty_cycle_nom_max;
        uint32_t inv_duty_cycle_abs_max;
        hw_haptic_interrupt_cb_t interrupt_cb;
} haptic_config_env_t;


/****************** INTERNAL GLOBAL VARIABLE DECLARATIONS ******************/


__RETAINED static haptic_config_env_t haptic_cfg;


/****************** PUBLIC FUNCTION DEFINITIONS ******************/


void hw_haptic_init(const haptic_config_t *cfg)
{
        /* Ensure the configuration struct is not NULL. */
        ASSERT_ERROR(cfg);

        /* Ensure the duty_cycle_nom/abs_max parameter values are valid. */
        ASSERT_ERROR((cfg->duty_cycle_nom_max > 0) &&
                     (cfg->duty_cycle_abs_max > 0) &&
                     (cfg->duty_cycle_abs_max <= HW_HAPTIC_DREF_MAX) &&
                     (cfg->duty_cycle_nom_max <= cfg->duty_cycle_abs_max));

        haptic_cfg.duty_cycle_nom_max = cfg->duty_cycle_nom_max;
        haptic_cfg.duty_cycle_abs_max = cfg->duty_cycle_abs_max;

        /*
         * Calculate and store inverse nominal/absolute duty cycle limits.
         * These exist as an optimization so that the divide needed to calculate the drive level from
         * duty cycle is replaced by a multiply. This avoids slow divides being done in ISR.
         */
        haptic_cfg.inv_duty_cycle_nom_max = (HW_HAPTIC_UFIX16_ONE << HW_HAPTIC_UFIX16_Q)
                / (int32_t) haptic_cfg.duty_cycle_nom_max;
        haptic_cfg.inv_duty_cycle_abs_max = (HW_HAPTIC_UFIX16_ONE << HW_HAPTIC_UFIX16_Q)
                / (int32_t) haptic_cfg.duty_cycle_abs_max;

        /* Enable the Haptic Driver/Controller clock. */
        GLOBAL_INT_DISABLE();
        REG_SETF(CRG_PER, CLK_PER_REG, LRA_CLK_EN, 1);
        GLOBAL_INT_RESTORE();

        /* Apply H-bridge initial settings. */
        LRA->LRA_BRD_LS_REG = 0x077F;
        LRA->LRA_BRD_HS_REG = 0x1407;

        /* Apply ADC trim settings. */
        *((uint32_t *) 0x50030A48) = 0x00020100;
        *((uint32_t *) 0x50030A4C) = 0x00020100;

        /* Enable LDO, H-bridge and ADC. */
        REG_SETF(LRA, LRA_CTRL1_REG, LDO_EN, 1);
        REG_SETF(LRA, LRA_CTRL1_REG, HBRIDGE_EN, 1);
        REG_SETF(LRA, LRA_CTRL1_REG, ADC_EN, 1);

        /*
         * Enable constant duty cycle mode. (Set loop filter coefficients to zero, since loop filter
         * is not used in this case.)
         */
        LRA->LRA_FLT_COEF1_REG = 0;
        LRA->LRA_FLT_COEF2_REG = 0;
        LRA->LRA_FLT_COEF3_REG = 0;

        REG_SETF(LRA, LRA_CTRL2_REG, AUTO_MODE, 1);

        /* Initialize half period. */
        if (cfg->half_period) {
                hw_haptic_set_half_period(cfg->half_period);
        }

        /* Initialize duty cycle. */
        REG_SETF(LRA, LRA_CTRL3_REG, DREF, 0);

#if !dg_configUSE_HW_LRA
        /* Apply a fixed polarity (for DC Drive) (ERM case). */
        REG_SETF(LRA, LRA_DFT_REG, SWM_SEL, 1);
        REG_SETF(LRA, LRA_DFT_REG, SWM_MAN, cfg->signal_out);

#endif

        /* Configure the Controller to generate interrupt requests after capturing the 7th i-sample
         * of every half cycle. */
        REG_SETF(LRA, LRA_CTRL1_REG, IRQ_IDX, 6);
        REG_SETF(LRA, LRA_CTRL1_REG, IRQ_DIV, 0);

        /* Select raw down-sampled data from the Haptic ADC (as the data to be processed by the
         * interrupt callback function). */
        REG_SETF(LRA, LRA_CTRL1_REG, SMP_SEL, 0);

        /*
         * At system startup keeping LOOP_EN = 0 ensures clock synchronization in the FSM
         * that controls the H-bridge. We need to wait approximately 2 PWM cycles before
         * enabling the FSM.
         */
        hw_clk_delay_usec(8);
        REG_SETF(LRA, LRA_CTRL1_REG, LOOP_EN, 1);

        haptic_cfg.interrupt_cb = cfg->interrupt_cb;
        if (haptic_cfg.interrupt_cb != NULL) {
                /* Enable Haptic Controller interrupt requests. */
                REG_SET_BIT(LRA, LRA_CTRL1_REG, IRQ_CTRL_EN);
                NVIC_ClearPendingIRQ(LRA_IRQn);
                NVIC_EnableIRQ(LRA_IRQn);
        }
}

void LRA_Handler(void)
{
        if (haptic_cfg.interrupt_cb != NULL) {
                uint8_t smp_idx, j;
                uint16_t half_period, drive_level, state;
                int32_t sample_regs[4] = {0};
                int32_t *sample_regs_p;

                /* Check current i-sample index. */
                smp_idx = REG_GETF(LRA, LRA_CTRL1_REG, SMP_IDX);
                if ((smp_idx & 7) != 7) {
                        /* We've started too late. */
                        return;
                }

                /* Read i-samples 1-8 or 9-16, depending on smp_idx. */
                sample_regs_p = (int32_t *) ((smp_idx < 8) ? &LRA->LRA_FLT_SMP1_REG : &LRA->LRA_FLT_SMP5_REG);
                for (j = 0; j < 4; j++) {
                        sample_regs[j] = *sample_regs_p++;
                }

                /* Read current half period, drive level and state. */
                half_period = hw_haptic_get_half_period();
                state = 0x0004 | (((uint16_t) hw_haptic_get_polarity()) << 1) | (smp_idx > 7);
                drive_level = hw_haptic_get_drive_level(HW_HAPTIC_DRIVE_LEVEL_REF_ABS_MAX);

                /* Compensate for adjustment for avoiding the automatic polarity flipping being disabled. */
                if (drive_level == 1) {
                        drive_level = 0;
                }

                /* Call haptic interrupt callback function to update half period, drive level and polarity. */
                haptic_cfg.interrupt_cb((int16_t *) sample_regs, &half_period, &drive_level, &state);

                /* Update half period, drive level and polarity. */
                hw_haptic_set_half_period(half_period);
                hw_haptic_set_drive_level(drive_level, HW_HAPTIC_DRIVE_LEVEL_REF_ABS_MAX);
                hw_haptic_set_polarity((state >> 1)  & 1);
        }
}

uint16_t hw_haptic_get_drive_level(HW_HAPTIC_DRIVE_LEVEL_REF ref)
{
        uint16_t drive_level;

        /* Calculate drive level by scaling duty cycle by drive level reference. */
        drive_level = (uint16_t) (((uint32_t) REG_GETF(LRA, LRA_CTRL3_REG, DREF) *
                (ref ? haptic_cfg.inv_duty_cycle_abs_max : haptic_cfg.inv_duty_cycle_nom_max) +
                HW_HAPTIC_UFIX16_RND) >> HW_HAPTIC_UFIX16_Q);

        /* Ensure drive level does not exceed 100%. */
        if (drive_level > HW_HAPTIC_UFIX16_ONE)
                drive_level = HW_HAPTIC_UFIX16_ONE;

        return drive_level;
}

void hw_haptic_set_drive_level(uint16_t drive_level, HW_HAPTIC_DRIVE_LEVEL_REF ref)
{
        uint16_t duty_cycle;

        /* Ensure drive level does not exceed 100%. */
        if (drive_level > HW_HAPTIC_UFIX16_ONE) {
                drive_level = HW_HAPTIC_UFIX16_ONE;
        }

        /* Calculate duty cycle by scaling drive level reference by drive level. */
        duty_cycle = (uint16_t) (((uint32_t) drive_level * (ref ?  haptic_cfg.duty_cycle_abs_max :
                haptic_cfg.duty_cycle_nom_max) + HW_HAPTIC_UFIX16_RND) >> HW_HAPTIC_UFIX16_Q);

#if HW_HAPTIC_DREF_MIN
        /* Ensure duty cycle is above the minimum supported limit. */
        if (duty_cycle < HW_HAPTIC_DREF_MIN) {
                duty_cycle = HW_HAPTIC_DREF_MIN;
        }
#else
        /*
         * Ensure duty cycle is always greater than zero, in order to avoid automatic polarity
         * flipping being disabled.
         */
        if (!duty_cycle) {
                duty_cycle = 1;
        }
#endif
        REG_SETF(LRA, LRA_CTRL3_REG, DREF, duty_cycle);
}

void hw_haptic_shutdown(void)
{
        hw_haptic_set_state(HW_HAPTIC_STATE_IDLE);
        /* Disable Haptic Controller interrupt requests. */
        REG_CLR_BIT(LRA, LRA_CTRL1_REG, IRQ_CTRL_EN);
        NVIC_DisableIRQ(LRA_IRQn);
        /* Disable Loop. */
        REG_SETF(LRA, LRA_CTRL1_REG, LOOP_EN, 0);
        /* Disable LDO. */
        REG_SETF(LRA, LRA_CTRL1_REG, LDO_EN, 0);
        /* Disable H-bridge. */
        REG_SETF(LRA, LRA_CTRL1_REG, HBRIDGE_EN, 0);
        /* Disable Haptic ADC. */
        REG_SETF(LRA, LRA_CTRL1_REG, ADC_EN, 0);
        /* Disable the Haptic Driver/Controller clock. */
        GLOBAL_INT_DISABLE();
        REG_SETF(CRG_PER, CLK_PER_REG, LRA_CLK_EN, 0);
        GLOBAL_INT_RESTORE();
}

#endif /* dg_configUSE_HW_ERM || dg_configUSE_HW_LRA */

#endif /* DEVICE_VARIANT */

/**
 * \}
 * \}
 * \}
 */
