/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup SDADC
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file hw_sdadc.c
 *
 * @brief Implementation of the SDADC Low Level Driver.
 *
 * Copyright (C) 2018-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#if dg_configUSE_HW_SDADC

#include <hw_sdadc.h>

#if (dg_configSYSTEMVIEW)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif

static hw_sdadc_interrupt_cb intr_cb = NULL;

/************************************ Help Functions ************************************/

/**
 * \brief Get trimmed values for gain and offset
 *
 * \param[in]  input_mode indicates differential or single ended
 * \param[out] gain pointer reference to gain correction value
 * \param[out] offs pointer reference to offset correction value
 */
__WEAK void hw_sdadc_get_trimmed_values(uint8_t mode, int16_t *gain, int16_t *offs);

/* Workaround for "Errata issue 279": ADC, gain error related to OSR */
uint16_t hw_sdadc_gain_recalculate(uint16_t raw_gain)
{
        HW_SDADC_OSR osr = hw_sdadc_get_oversampling();

        /* Do a sign extension */
        int16_t gain = (raw_gain & 0x0200) ? (raw_gain | 0xFC00) : (raw_gain & 0x01FF);

        /* Convert into "normal" */
        gain = gain + 32;

        /* Correct gain error with empirical values */
        switch (osr) {
        case HW_SDADC_OSR_128:
                gain = gain - 267;
                break;
        case HW_SDADC_OSR_256:
                gain = gain - 133;
                break;
        case HW_SDADC_OSR_512:
                gain = gain - 65;
                break;
        default:
                gain = gain - 32;
                break;
        }

        /* Check for 10bit limits */
        if (gain >  511) gain =  511;
        if (gain < -512) gain = -512;

        /* Select only the 10 LSBs so gain is ready to be store to REG */
        return (gain & 0x03FF);
 }

void hw_sdadc_get_trimmed_and_set_to_regs()
{
        int16_t gain_corr;
        int16_t offs_corr;

        /* retrieve gain and offset from TCS */
        hw_sdadc_get_trimmed_values(hw_sdadc_get_input_mode(), &gain_corr, &offs_corr);
        /* set the gain value to REG */
        hw_sdadc_set_gain_correction(gain_corr);
        /* set the corrected offset */
        hw_sdadc_set_offset_correction(offs_corr);
}

void hw_sdadc_measure(void)
{
        hw_sdadc_start();     // Start AD conversion

        while (hw_sdadc_in_progress());
        hw_sdadc_clear_interrupt();
}

/************************************ Getter Functions ************************************/

/**
 * \brief Get current positive input channel
 *
 * \return positive input channel
 *
 */
__STATIC_INLINE  HW_SDADC_INPUT hw_sdadc_get_inp(void)
{
        return (HW_SDADC_INPUT) REG_GETF(SDADC, SDADC_CTRL_REG, SDADC_INP_SEL);
}

/**
 * \brief Get current negative input channel
 *
 * \return negative input channel
 *
 */
__STATIC_INLINE  HW_SDADC_INPUT hw_sdadc_get_inn(void)
{
        return (HW_SDADC_INPUT) REG_GETF(SDADC, SDADC_CTRL_REG, SDADC_INN_SEL);
}

/**
 * \brief Get clock frequency
 *
 * \return clock frequency
 *
 */
__STATIC_INLINE  HW_SDADC_CLOCK_FREQ hw_sdadc_get_frequency(void)
{
        return (HW_SDADC_CLOCK_FREQ) REG_GETF(SDADC, SDADC_TEST_REG, SDADC_CLK_FREQ);
}

/************************************ Setter Functions ************************************/

void hw_sdadc_set_gain_correction(uint16_t gain)
{
        REG_SETF(SDADC, SDADC_GAIN_CORR_REG, SDADC_GAIN_CORR, hw_sdadc_gain_recalculate(gain));
}

void hw_sdadc_set_oversampling(HW_SDADC_OSR osr)
{
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_OSR, osr);
        /* oversampling rate affects the gain/offset values */
        hw_sdadc_get_trimmed_and_set_to_regs();
}

void hw_sdadc_set_input_mode(HW_SDADC_INPUT_MODE mode)
{
        bool switched = false;

        if (mode != hw_sdadc_get_input_mode()) {
                switched = true;
        }

        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_SE, mode);
        /* input mode affects the gain/offset values */
        hw_sdadc_get_trimmed_and_set_to_regs();

        /* SDADC "Errata issue 284": ADC, First Conversion is Erroneous after Switching SE <-> DIF */
        if (switched && hw_sdadc_is_enabled()) {
                hw_sdadc_disable();
                hw_sdadc_enable();
        }
}

 /**
  * \brief Set positive input channel
  *
  * \param [in] input positive input channel
  *
  */
__STATIC_INLINE void hw_sdadc_set_inp(HW_SDADC_INPUT input)
{
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_INP_SEL, input);
}

/**
 * \brief Set negative input channel
 *
 * Only valid for differential mode. Negative side is ignored in single ended mode
 *
 * \param [in] input negative input channel
 *
 */
__STATIC_INLINE void hw_sdadc_set_inn(HW_SDADC_INPUT input)
{
        ASSERT_WARNING(HW_SDADC_IN_ADC7_P1_22 >= input);
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_INN_SEL, input);
}

/**
 * \brief Set clock frequency
 *
 * \param [in] freq clock frequenc
 *
 */
__STATIC_INLINE void hw_sdadc_set_frequency(HW_SDADC_CLOCK_FREQ freq)
{
        REG_SETF(SDADC, SDADC_TEST_REG, SDADC_CLK_FREQ, freq);
}

/************************************ API Function Bodies ************************************/

void hw_sdadc_init(const sdadc_config *cfg)
{
        SDADC->SDADC_CTRL_REG = 0;

        NVIC_DisableIRQ(SDADC_IRQn);
        NVIC_ClearPendingIRQ(SDADC_IRQn);

        hw_sdadc_configure(cfg);
}

void hw_sdadc_deinit()
{
        hw_sdadc_unregister_interrupt();
}

void hw_sdadc_reset(void)
{
        // Set control register to default value "0" but keep enable bit set
        SDADC->SDADC_CTRL_REG = REG_MSK(SDADC, SDADC_CTRL_REG, SDADC_EN);

        NVIC_DisableIRQ(SDADC_IRQn);
        NVIC_ClearPendingIRQ(SDADC_IRQn);
}

void hw_sdadc_configure(const sdadc_config *cfg)
{
        if (!cfg) {
                return;
        }
        if (cfg->inp == HW_SDADC_INP_VBAT) {
                if (cfg->freq != HW_SDADC_CLOCK_FREQ_250K) {
                        /* Workaround for "Errata issue 296": ADC Insufficient setting time when VBAT scaler is used */
                        /*
                         * 250K frequency is mandatory for accurate vbat measurement
                         */
                        ASSERT_WARNING(0);
                        return;
                }
        }
        if (HW_SDADC_VREF_INTERNAL == cfg->vref_selection) {
                if (HW_SDADC_VREF_VOLTAGE_INTERNAL != cfg->vref_voltage) {
                        /*
                         * Internal reference voltage is set by system to 1.2V
                         * A different value will yield erroneous conversion
                         */
                        ASSERT_WARNING(0);
                        return;
                }
        }
        if (!(HW_SDADC_VREF_IN_RANGE(cfg->vref_voltage))) {
                /*
                 * Reference voltage must not exceed HW limits
                 */
                ASSERT_WARNING(0);
                return;
        }

        hw_sdadc_set_input_mode(cfg->input_mode);
        hw_sdadc_set_inp(cfg->inp);
        hw_sdadc_set_inn(cfg->inn);
        hw_sdadc_set_continuous(cfg->continuous);
        hw_sdadc_set_oversampling(cfg->over_sampling);
        hw_sdadc_set_vref_sel(cfg->vref_selection);
        hw_sdadc_set_dma_functionality(cfg->use_dma);
        if (cfg->mask_int) {
                hw_sdadc_enable_interrupt();
        } else {
                hw_sdadc_disable_interrupt();
        }
        hw_sdadc_set_frequency(cfg->freq);
}

void hw_sdadc_register_interrupt(hw_sdadc_interrupt_cb cb)
{
        intr_cb = cb;

        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_MINT, 1);

        NVIC_ClearPendingIRQ(SDADC_IRQn);
        NVIC_EnableIRQ(SDADC_IRQn);
}

void hw_sdadc_unregister_interrupt(void)
{
        NVIC_DisableIRQ(SDADC_IRQn);
        NVIC_ClearPendingIRQ(SDADC_IRQn);
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_MINT, 0);

        intr_cb = NULL;
}

/************************************ Misc Functions ************************************/

void SDADC_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        if (intr_cb) {
                intr_cb();
        } else {
                hw_sdadc_clear_interrupt();
        }

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

#endif /* dg_configUSE_HW_SDADC */

/**
 * \}
 * \}
 * \}
 */
