/**
 * \addtogroup PLA_DRI_PER_ANALOG
 * \{
 * \addtogroup HW_SDADC SDADC Driver
 * \{
 * \brief Sigma Delta ADC
 */

/**
 ****************************************************************************************
 *
 * @file hw_sdadc.h
 *
 * @brief Definition of API for the SDADC Low Level Driver.
 *
 * Copyright (C) 2018-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_SDADC_H
#define HW_SDADC_H


#if dg_configUSE_HW_SDADC

#include <stdbool.h>
#include <stdint.h>
#include <sdk_defs.h>

/**
 * \brief SDADC input mode
 *
 */
typedef enum {
        HW_SDADC_INPUT_MODE_DIFFERENTIAL = 0,    /**< Differential mode (default) */
        HW_SDADC_INPUT_MODE_SINGLE_ENDED = 1     /**< Single ended mode. Input selection negative side is ignored */
} HW_SDADC_INPUT_MODE;

/**
 * \brief SDADC VREF selection
 *
 */
typedef enum {
        HW_SDADC_VREF_INTERNAL = 0,    /**< Internal bandgap reference (default) */
        HW_SDADC_VREF_EXTERNAL = 1     /**< External reference */
} HW_SDADC_VREF_SEL;

typedef enum {
        HW_SDADC_VREF_VOLTAGE_MIN      = 0,
        HW_SDADC_VREF_VOLTAGE_INTERNAL = 1200,
        HW_SDADC_VREF_VOLTAGE_MAX      = 3300
} HW_SDADC_VREF_VOLTAGE_RANGE;

#define HW_SDADC_VREF_IN_RANGE(voltage)    (((uint16_t)voltage >= HW_SDADC_VREF_VOLTAGE_MIN) && ((uint16_t)voltage <= HW_SDADC_VREF_VOLTAGE_MAX))

/**
 * \brief SDADC Clock Source
 *
 * \deprecated the clock source is fixed, hence there is no need for enumeration with the allowed values
 */
typedef uint8_t HW_SDADC_CLOCK;

/**
 * \brief SDADC input
 *
 * \p SDADC_IN_* values can be used in input selection of both negative and positive side
 * \p SDADC_INP_* values shall be used only in input selection of positive side
 *
 */
typedef enum {
        HW_SDADC_IN_ADC0_P1_09 = 0,    /**< GPIO P1_09 */
        HW_SDADC_IN_ADC1_P0_25 = 1,    /**< GPIO P0_25 */
        HW_SDADC_IN_ADC2_P0_08 = 2,    /**< GPIO P0_08 */
        HW_SDADC_IN_ADC3_P0_09 = 3,    /**< GPIO P0_09 */
        HW_SDADC_IN_ADC4_P1_14 = 4,    /**< GPIO P1_14 */
        HW_SDADC_IN_ADC5_P1_20 = 5,    /**< GPIO P1_20 */
        HW_SDADC_IN_ADC6_P1_21 = 6,    /**< GPIO P1_21 */
        HW_SDADC_IN_ADC7_P1_22 = 7,    /**< GPIO P1_22 */
        HW_SDADC_INP_VBAT = 8,         /**< VBAT via 4x attenuator, negative side (INN) connected to ground */
} HW_SDADC_INPUT;

/**
 * \brief SDADC oversample rate
 *
 */
typedef enum {
        HW_SDADC_OSR_128     = 0,
        HW_SDADC_OSR_256     = 1,
        HW_SDADC_OSR_512     = 2,
        HW_SDADC_OSR_1024    = 3,
} HW_SDADC_OSR;

/**
 * \brief SDADC clock frequency
 *
 */
typedef enum {
        HW_SDADC_CLOCK_FREQ_250K  = 0,           /**< 00: 250KHz */
        HW_SDADC_CLOCK_FREQ_500K  = 1,           /**< 01: 500KHz */
        HW_SDADC_CLOCK_FREQ_1M    = 2,           /**< 10: 1MHz (default for DA1469x-00) */
        HW_SDADC_CLOCK_FREQ_2M    = 3,           /**< 11: 2MHz */
} HW_SDADC_CLOCK_FREQ;

/**
 * \brief SDADC interrupt handler
 *
 */
typedef void (*hw_sdadc_interrupt_cb)(void);

/**
 * \brief SDADC configuration
 *
 */
typedef struct {
        HW_SDADC_CLOCK         clock;             /**< deprecated, the clock source is fixed */
        HW_SDADC_INPUT_MODE    input_mode;        /**< Input mode */
        HW_SDADC_INPUT         inn;               /**< ADC negative input */
        HW_SDADC_INPUT         inp;               /**< ADC positive input */
        bool                   continuous;        /**< Continuous mode state */
        HW_SDADC_OSR           over_sampling;     /**< Oversampling rate */
        HW_SDADC_VREF_SEL      vref_selection;    /**< VREF source selection (internal/external) */
        uint16_t               vref_voltage;      /**< Reference voltage (mV) - MUST be set to 1200 when vref source is internal */
        bool                   use_dma;           /**< DMA functionality enable/disable */
        bool                   mask_int;          /**< Enable/Disable (mask) SDADC interrupt */
        HW_SDADC_CLOCK_FREQ    freq;              /**< CLOCK_FREQ selection - MUST be set to 250K when measuring VBAT */
} sdadc_config;

/**
 * \brief Initialize SDADC
 *
 * Sets the SDADC control register to default values and then calls
 * the configuration function. It also disables and clears pending SDADC interrupts.
 *
 * \p cfg can be NULL - no configuration is performed in such case.
 *
 * \param [in] cfg configuration
 *
 * \sa hw_sdadc_configure
 *
 */
void hw_sdadc_init(const sdadc_config *cfg);

/**
 * \brief De-initialize SDADC
 *
 * Sets the SDADC control register to default values.
 * It also disables and clears pending SDADC interrupts.
 *
 */
void hw_sdadc_deinit();

/**
 * \brief Configure SDADC
 *
 * Shortcut to call appropriate configuration function. If \p cfg is NULL, this function does
 * nothing.
 *
 * \param [in] cfg configuration
 *
 */
void hw_sdadc_configure(const sdadc_config *cfg);

/**
 * \brief Reset SDADC to its default values without disabling the LDO.
 *
 */
void hw_sdadc_reset(void);

/**
 * \brief Register interrupt handler
 *
 * Interrupt is enabled after calling this function. Application is responsible for clearing
 * interrupt using hw_sdadc_clear_interrupt(). If no callback is specified interrupt is cleared by
 * driver.
 *
 * \param [in] cb callback fired on interrupt
 *
 * \sa hw_sdadc_clear_interrupt
 *
 */
void hw_sdadc_register_interrupt(hw_sdadc_interrupt_cb cb);

/**
 * \brief Unregister interrupt handler
 *
 * Interrupt is disabled after calling this function.
 *
 */
void hw_sdadc_unregister_interrupt(void);

/**
 * \brief Clear interrupt
 *
 * Application should call this in interrupt handler to clear interrupt.
 *
 * \sa hw_sdadc_register_interrupt
 *
 */
__STATIC_INLINE  void hw_sdadc_clear_interrupt(void)
{
        SDADC->SDADC_CLEAR_INT_REG = 1;
}

/**
 * \brief Enable SDADC
 *
 * This function enables the SDADC. LDO, bias currents and modulator are enabled.
 * To start a conversion, the application should call hw_sdadc_start().
 *
 * \sa hw_sdadc_start
 *
 */
__STATIC_INLINE  void hw_sdadc_enable(void)
{
        REG_SET_BIT(SDADC, SDADC_CTRL_REG, SDADC_EN);
        while (0==REG_GETF(SDADC, SDADC_CTRL_REG, SDADC_LDO_OK));      // Wait for LDO OK
}

/**
 * \brief Disable SDADC
 *
 * Application should wait for conversion to be completed before disabling SDADC. In case of
 * continuous mode, application should disable continuous mode and then wait for conversion to be
 * completed in order to have SDADC in defined state.
 *
 * \sa hw_sdadc_in_progress
 * \sa hw_sdadc_set_continuous
 *
 */
__STATIC_INLINE  void hw_sdadc_disable(void)
{
        REG_CLR_BIT(SDADC, SDADC_CTRL_REG, SDADC_EN);
}

/**
 * \brief Get the enable status of the SDADC
 *
 * \return SDADC enable status
 *
 */
__STATIC_INLINE  bool hw_sdadc_is_enabled(void)
{
        return REG_GETF(SDADC, SDADC_CTRL_REG, SDADC_EN);
}

/**
 * \brief Start conversion
 *
 * Application should not call this function while conversion is still in progress.
 *
 * \sa hw_sdadc_in_progress
 *
 */
__STATIC_INLINE  void hw_sdadc_start(void)
{
        REG_SET_BIT(SDADC, SDADC_CTRL_REG, SDADC_START);
}

/**
 * \brief Check if conversion is in progress
 *
 * \return conversion state
 *
 */
__STATIC_INLINE  bool hw_sdadc_in_progress(void)
{
        return REG_GETF(SDADC, SDADC_CTRL_REG, SDADC_START);
}

/**
 * \brief Set continuous mode
 *
 * With continuous mode enabled SDADC will automatically restart conversion once completed. It's still
 * required to start 1st conversion using hw_sdadc_start().
 *
 * \param [in] enabled continuous mode state
 *
 * \sa hw_sdadc_start
 *
 */
__STATIC_INLINE  void hw_sdadc_set_continuous(bool enabled)
{
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_CONT, !!enabled);
}

/**
 * \brief Get continuous mode state
 *
 * \return continuous mode state
 *
 */
__STATIC_INLINE  bool hw_sdadc_get_continuous(void)
{
        return REG_GETF(SDADC, SDADC_CTRL_REG, SDADC_CONT);
}

/**
 * \brief Enable/Disable DMA functionality
 *
 * \param [in] enabled When true, DMA functionality is enabled
 *
 */
__STATIC_INLINE  void hw_sdadc_set_dma_functionality(bool enabled)
{
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_DMA_EN, !!enabled);
}

/**
 * \brief Get DMA functionality state
 *
 * \return DMA functionality state
 *
 */
__STATIC_INLINE  bool hw_sdadc_get_dma_functionality(void)
{
        return REG_GETF(SDADC, SDADC_CTRL_REG, SDADC_DMA_EN);
}

/**
 * \brief Enable SDADC interrupt
 *
 */
__STATIC_INLINE  void hw_sdadc_enable_interrupt(void)
{
        REG_SET_BIT(SDADC, SDADC_CTRL_REG, SDADC_MINT);
}

/**
 * \brief Disable SDADC interrupt
 *
 */
__STATIC_INLINE  void hw_sdadc_disable_interrupt(void)
{
        REG_CLR_BIT(SDADC, SDADC_CTRL_REG, SDADC_MINT);
}

/**
 * \brief Get the status of the SDADC maskable interrupt (MINT) to the CPU
 *
 * \return SDADC maskable interrupt (MINT) status
 *
 */
__STATIC_INLINE  bool hw_sdadc_is_interrupt_enabled(void)
{
        return REG_GETF(SDADC, SDADC_CTRL_REG, SDADC_MINT);
}

/**
 * \brief Set input mode
 *
 * \param [in] mode input mode
 *
 */
void hw_sdadc_set_input_mode(HW_SDADC_INPUT_MODE mode);

/**
 * \brief Get current input mode
 *
 * return input mode
 *
 */
__STATIC_INLINE  HW_SDADC_INPUT_MODE hw_sdadc_get_input_mode(void)
{
        return (HW_SDADC_INPUT_MODE) REG_GETF(SDADC, SDADC_CTRL_REG, SDADC_SE);
}

/**
 * \brief Set clock source
 *
 * \param [in] clock clock source
 *
 * \deprecated The clock source is fixed, hence set and get functions are trivial
 */
DEPRECATED_MSG("The clock source is fixed, hence set and get functions are trivial")
__STATIC_INLINE void hw_sdadc_set_clock(HW_SDADC_CLOCK clock)
{
}

/**
 * \brief Get current clock source
 *
 * \return zero
 *
 * \deprecated The clock source is fixed
 */
DEPRECATED_MSG("The clock source is fixed, hence set and get functions are trivial")
__STATIC_INLINE HW_SDADC_CLOCK hw_sdadc_get_clock(void)
{
        return 0;
}

/**
 * \brief Set oversampling
 *
 * With oversampling enabled multiple successive conversions will be executed and results are added
 * together to increase effective number of bits in result.
 *
 * \param [in] osr oversampling rate
 *
 * \note SDADC input mode shall be set before setting the oversampling rate
 *
 * \sa hw_sdadc_get_raw_value
 * \sa hw_sdadc_set_input_mode
 *
 */
void hw_sdadc_set_oversampling(HW_SDADC_OSR osr);

/**
 * \brief Get current oversampling
 *
 * \return oversampling rate
 *
 * \sa hw_sdadc_set_oversampling
 *
 */
__STATIC_INLINE  uint8_t hw_sdadc_get_oversampling(void)
{
        return REG_GETF(SDADC, SDADC_CTRL_REG, SDADC_OSR);
}

/**
 * \brief Select voltage reference source
 *
 * \param [in] vref_sel source to get the reference voltage
 */
__STATIC_INLINE  void hw_sdadc_set_vref_sel(HW_SDADC_VREF_SEL vref_sel)
{
        REG_SETF(SDADC, SDADC_CTRL_REG, SDADC_VREF_SEL, vref_sel);
}

/**
 * \brief Get voltage reference source
 *
 * \return voltage reference source (internal/external)
 *
 */
__STATIC_INLINE  HW_SDADC_VREF_SEL hw_sdadc_get_vref_sel(void)
{
        return (HW_SDADC_VREF_SEL) REG_GETF(SDADC, SDADC_CTRL_REG, SDADC_VREF_SEL);
}

/**
 * \brief Set gain correction. The value-to-be-set is first re-calculated and then set
 *
 * \param [in] gain gain value
 *
 */
void hw_sdadc_set_gain_correction(uint16_t gain);

/**
 * \brief Get current gain value
 *
 * \return gain value
 *
 */
__STATIC_INLINE  uint16_t hw_sdadc_get_gain_correction(void)
{

        return REG_GETF (SDADC, SDADC_GAIN_CORR_REG, SDADC_GAIN_CORR);
}

/**
 * \brief Set offset correction
 *
 * \param [in] offset offset value
 *
 */
__STATIC_INLINE  void hw_sdadc_set_offset_correction(uint16_t offset)
{
        REG_SETF(SDADC, SDADC_OFFS_CORR_REG, SDADC_OFFS_CORR, offset);
}

/**
 * \brief Get current offset value
 *
 * \return offset value
 *
 */
__STATIC_INLINE  uint16_t hw_sdadc_get_offset_correction(void)
{
        return REG_GETF(SDADC, SDADC_OFFS_CORR_REG, SDADC_OFFS_CORR);
}

/**
 * \brief Get the result register value.
 *
 * \return result value
 *
 */
__STATIC_INLINE  uint16_t hw_sdadc_read_result_register(void)
{
        return (SDADC->SDADC_RESULT_REG);
}

/**
 * \brief Get raw value
 *
 * \return raw value
 *
 * \sa hw_sdadc_read_result_register
 *
 */
__STATIC_INLINE uint16_t hw_sdadc_get_raw_value(void)
{
        hw_sdadc_start(); // Start AD conversion

        while (hw_sdadc_in_progress());
        hw_sdadc_clear_interrupt();

        return hw_sdadc_read_result_register();
}

/**
 * \brief Get voltage in mV
 *
 * \param [in] *cfg sdadc configuration
 *
 * \return Directly get ADC from register, then convert to mV
 *
 */
__STATIC_INLINE int32_t hw_sdadc_result_reg_to_millivolt(const sdadc_config *cfg)
{
        int32_t converted;
        uint16_t max;
        uint8_t attenuator = 0x01;

        if (HW_SDADC_INP_VBAT == cfg->inp) {
                attenuator = 0x04;
        }

        switch (cfg->input_mode) {
        case HW_SDADC_INPUT_MODE_SINGLE_ENDED:
                converted = (uint16_t) hw_sdadc_read_result_register();
                max = UINT16_MAX;
                break;
        case HW_SDADC_INPUT_MODE_DIFFERENTIAL:
                converted = (int16_t) hw_sdadc_read_result_register();
                max = UINT16_MAX >> 1;
                break;
        default:
                ASSERT_ERROR(0);
                return 0; // Should never reach this line
        }
        converted = converted * attenuator * cfg->vref_voltage;
        return converted / max;

}

/**
 * \brief Get voltage in mV
 *
 * \param [in] *cfg sdadc configuration
 *
 * \return Start and get measure of ADC, then convert to mV
 *
 * \sa hw_sdadc_read_result_register
 *
 */
__STATIC_INLINE int32_t hw_sdadc_get_voltage(const sdadc_config *cfg)
{
        hw_sdadc_start(); // Start AD conversion

        while (hw_sdadc_in_progress());
        hw_sdadc_clear_interrupt();

        return hw_sdadc_result_reg_to_millivolt(cfg);
}

#endif /* dg_configUSE_HW_SDADC */


#endif /* HW_SDADC_H_ */

/**
 * \}
 * \}
 */
