/**
 * \{
 * \addtogroup HW_SMOTOR
 * \{
 * \brief SMotor Controller (SMC)
 */

/**
 *****************************************************************************************
 *
 * @file hw_smotor.c
 *
 * @brief Implementation of the Motor Controller Low Level Driver.
 *
 * Copyright (C) 2018-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *****************************************************************************************
 */

#if dg_configUSE_HW_SMOTOR

#include "sdk_defs.h"
#include "hw_gpio.h"
#include "hw_smotor.h"

/* Struct containing SMotor interrupt registered callbacks*/
static hw_smotor_callback_t smotor_state = {
        .fifo_ovf_cb = NULL,
        .fifo_unr_cb = NULL,
        .fifo_threshold_cb = NULL,
        .genstart_cb = NULL,
        .smotor_current_wave_idx = 0
};
/* Index of next free entry of wave table*/
static uint8_t smotor_wave_idx = 0;

void hw_smotor_initialization(const hw_smotor_cfg_t *cfg)
{
        /* Enable SMotor Operation*/
        REG_SETF(CRG_PER, CLK_PER_REG, MC_CLK_EN, 0);
        REG_SETF(CRG_PER, CLK_PER_REG, MC_CLK_EN, 1);

        for (int pg_idx = 0; pg_idx < 5; pg_idx++) {
                /* configure only active PGs */
                if (cfg->pg_cfg[pg_idx] == NULL) {
                        continue;
                }

                volatile uint32 *hw_smotor_pg_reg = REG_GET_ADDR_INDEXED(SMOTOR, PG0_CTRL_REG, sizeof(SMOTOR->PG0_CTRL_REG), pg_idx);
                hw_smotor_pg_cfg_t* tmp = cfg->pg_cfg[pg_idx];

                /* pass configuration to pg control register*/
                *hw_smotor_pg_reg = tmp->genstart_irq_en |
                        tmp->genend_irq_en |
                        tmp->pg_start_mode |
                        tmp->pg_mode |
                        tmp->sig0_en |
                        tmp->sig1_en |
                        tmp->sig2_en |
                        tmp->sig3_en;

                hw_smotor_pg_signal_outputs(pg_idx, tmp->out0_sig, tmp->out1_sig, tmp->out2_sig,
                        tmp->out3_sig);
        }
        /* Configure SMotor Control Register */
        SMOTOR->SMOTOR_CTRL_REG = cfg->operation_mode |
                cfg->fifo_overflow_irq_en |
                cfg->fifo_underrun_irq_en |
                cfg->genstart_irq_en |
                cfg->genend_irq_en |
                cfg->rtc_trigger_en |
                cfg->sleep_clk_trigger_en |
                cfg->threshold_irq_en;

        hw_smotor_set_moi(cfg->moi);
        hw_smotor_set_irq_threshold(cfg->threshold);
        if (cfg->operation_mode == HW_SMOTOR_CYCLIC_FIFO_MODE) {
                hw_smotor_set_cyclic_size(cfg->cyclic_size);
        }
}

uint8_t hw_smotor_add_wave(const uint8_t *wave)
{
        uint8_t pun = wave[0];
        uint8_t count = 0, first_idx = 0, count_idx = 0, mod, smotor_cur_wav_idx=0xFF;
        uint32_t tmp, *wave_reg;

        /* Check if wave table has empty space to add the wave */
        if (((2 * pun) + 1 + smotor_wave_idx) > WAVETABLE_LEN) {
                return 0xFF;
        }
        smotor_cur_wav_idx=smotor_wave_idx;

        mod = smotor_wave_idx % sizeof(uint32_t);
        first_idx = smotor_wave_idx / sizeof(uint32_t);
        /* Patch first wave entries to a non-full wave table word (32-bit) */
        if (mod) {
                wave_reg = (uint32_t*)REG_GET_ADDR_INDEXED(SMOTOR, WAVETABLE_BASE, sizeof(SMOTOR->WAVETABLE_BASE), first_idx);
                tmp = *wave_reg;
                switch (mod)
                {
                case 1:
                        tmp |= (wave[2] << 24) | (wave[1] << 16) | (wave[0] << 8);
                        count += 3;
                        break;
                case 2:
                        tmp |= (wave[1] << 24) | (wave[0] << 16);
                        count += 2;
                        break;
                case 3:
                        tmp |= (wave[0] << 24);
                        count++;
                        break;
                default:
                        break;
                }

                *wave_reg = tmp;
                count_idx++;
        }
        /* Add wave entries */
        for (; count < (2 * pun) + 1; count += 4)
                {
                wave_reg = (uint32_t*)REG_GET_ADDR_INDEXED(SMOTOR, WAVETABLE_BASE, sizeof(SMOTOR->WAVETABLE_BASE), first_idx + count_idx);
                tmp = (((uint32_t)wave[count + 3]) << 24) | (((uint32_t)wave[count + 2]) << 16)
                        | (((uint32_t)wave[count + 1]) << 8) | (wave[count]);
                *wave_reg = tmp;
                count_idx++;
        }
        smotor_wave_idx += (2 * pun) + 2;
        return smotor_cur_wav_idx;
}

void hw_smotor_rst_wave_idx(void)
{
        smotor_wave_idx = 0;
}

uint8_t hw_smotor_get_wave_idx(void)
{
        return smotor_wave_idx;
}

void hw_smotor_register_intr(hw_smotor_interrupt_cb handler,
        HW_SMOTOR_INTERRUPT_HANDLE_ID handle_type)
{
        switch (handle_type) {
        case HW_SMOTOR_GENSTART_IRQ_HANDLE_ID:
                smotor_state.genstart_cb = handler;
                break;
        case HW_SMOTOR_GENEND_IRQ_HANDLE_ID:
                smotor_state.genend_cb = handler;
                break;
        case HW_SMOTOR_FIFO_OVF_IRQ_HANDLE_ID:
                smotor_state.fifo_ovf_cb = handler;
                break;
        case HW_SMOTOR_FIFO_UNR_IRQ_HANDLE_ID:
                smotor_state.fifo_unr_cb = handler;
                break;
        case HW_SMOTOR_THRESHOLD_IRQ_HANDLE_ID:
                smotor_state.fifo_threshold_cb = handler;
                break;
        default:
                break;
        }
        NVIC_EnableIRQ(MOTOR_CONTROLLER_IRQn);
}

void hw_smotor_unregister_intr(HW_SMOTOR_INTERRUPT_HANDLE_ID handle_type)
{
        switch (handle_type) {
        case HW_SMOTOR_GENSTART_IRQ_HANDLE_ID:
                smotor_state.genstart_cb = NULL;
                hw_smotor_clear_genstart_irq_status();
                break;
        case HW_SMOTOR_GENEND_IRQ_HANDLE_ID:
                smotor_state.genend_cb = NULL;
                hw_smotor_clear_genend_irq_status();
                break;
        case HW_SMOTOR_FIFO_OVF_IRQ_HANDLE_ID:
                smotor_state.fifo_ovf_cb = NULL;
                hw_smotor_clear_fifo_ovf_irq_status();
                break;
        case HW_SMOTOR_FIFO_UNR_IRQ_HANDLE_ID:
                smotor_state.fifo_unr_cb = NULL;
                hw_smotor_clear_fifo_urn_irq_status();
                break;
        case HW_SMOTOR_THRESHOLD_IRQ_HANDLE_ID:
                smotor_state.fifo_threshold_cb = NULL;
                hw_smotor_clear_threshold_irq_status();
                break;
        default:
                break;
        }
        if (smotor_state.genstart_cb == NULL && smotor_state.genend_cb == NULL && smotor_state.fifo_ovf_cb == NULL
                && smotor_state.fifo_unr_cb == NULL && smotor_state.fifo_threshold_cb == NULL) {
                NVIC_DisableIRQ(MOTOR_CONTROLLER_IRQn);
                NVIC_ClearPendingIRQ(MOTOR_CONTROLLER_IRQn);
        }
}
/**
 * \brief Capture SMotor Interrupt Handler. Calls user interrupt handler. Checks interrupt flag to
 * identify the interrupt source, then calls proper callback function.
 *
 */
void Motor_Controller_Handler(void)
{
        if (hw_smotor_get_status_reg() & HW_SMOTOR_GENSTART_IRQ_FIRED) {
                if (smotor_state.genstart_cb != NULL) {
                        smotor_state.genstart_cb(0);
                }
                hw_smotor_clear_genstart_irq_status();
        }
        if (hw_smotor_get_status_reg() & HW_SMOTOR_GENEND_IRQ_FIRED) {
                if (smotor_state.genend_cb != NULL) {
                        smotor_state.genend_cb(0);
                }
                hw_smotor_clear_genend_irq_status();
        }
        if (hw_smotor_get_status_reg() & HW_SMOTOR_FIFO_OVF_IRQ_FIRED) {
                if (smotor_state.fifo_ovf_cb != NULL) {
                        smotor_state.fifo_ovf_cb(0);
                }
                hw_smotor_clear_fifo_ovf_irq_status();
        }
        if (hw_smotor_get_status_reg() & HW_SMOTOR_FIFO_UNR_IRQ_FIRED) {
                if (smotor_state.fifo_unr_cb != NULL) {
                        smotor_state.fifo_unr_cb(0);
                }
                hw_smotor_clear_fifo_urn_irq_status();
        }
        if (hw_smotor_get_status_reg() & HW_SMOTOR_THRESHOLD_IRQ_FIRED) {
                if (smotor_state.fifo_threshold_cb != NULL) {
                        smotor_state.fifo_threshold_cb(0);
                }
                hw_smotor_clear_threshold_irq_status();
        }
}

bool hw_smotor_get_pg_busy_status(HW_SMOTOR_PG_IDX pg_id)
{
        uint32_t temp = 0x0;
        switch (pg_id) {
        case PG0_IDX:
                temp = REG_GETF(SMOTOR, SMOTOR_STATUS_REG, PG0_BUSY);
                break;
        case PG1_IDX:
                temp = REG_GETF(SMOTOR, SMOTOR_STATUS_REG, PG1_BUSY);
                break;
        case PG2_IDX:
                temp = REG_GETF(SMOTOR, SMOTOR_STATUS_REG, PG2_BUSY);
                break;
        case PG3_IDX:
                temp = REG_GETF(SMOTOR, SMOTOR_STATUS_REG, PG3_BUSY);
                break;
        case PG4_IDX:
                temp = REG_GETF(SMOTOR, SMOTOR_STATUS_REG, PG4_BUSY);
                break;
        default:
                ASSERT_ERROR(0);
        }

        return temp > 0;
}

void hw_smotor_fifo_pointers_reset(void)
{
        HW_SMOTOR_OPERATION_MODE current_mode = hw_smotor_get_command_fifo_mode();
        switch (current_mode) {
        case HW_SMOTOR_NORMAL_FIFO_MODE:
                hw_smotor_set_command_fifo_mode(HW_SMOTOR_CYCLIC_FIFO_MODE);
                hw_smotor_set_command_fifo_mode(HW_SMOTOR_NORMAL_FIFO_MODE);
                break;
        case HW_SMOTOR_CYCLIC_FIFO_MODE:
                hw_smotor_set_command_fifo_mode(HW_SMOTOR_NORMAL_FIFO_MODE);
                hw_smotor_set_command_fifo_mode(HW_SMOTOR_CYCLIC_FIFO_MODE);
                break;
        default:
                break;
        }
}

#endif /* dg_configUSE_HW_SMOTOR */


/**
 * \}
 */
