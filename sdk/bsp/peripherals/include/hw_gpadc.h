/**
 * \addtogroup PLA_DRI_PER_ANALOG
 * \{
 * \addtogroup HW_GPADC GPADC Driver
 * \{
 * \brief General Purpose ADC
 */

/**
 ****************************************************************************************
 *
 * @file hw_gpadc.h
 *
 * @brief Definition of API for the GPADC Low Level Driver.
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_GPADC_H
#define HW_GPADC_H


#if dg_configUSE_HW_GPADC

#include <stdbool.h>
#include <stdint.h>
#include <sdk_defs.h>

/**
 * \brief GPADC input voltages
 *
 */
typedef enum {
        HW_GPADC_INPUT_VOLTAGE_UP_TO_1V2 = 0,    /**< input voltages up to 1.2 V are allowed */
        HW_GPADC_INPUT_VOLTAGE_UP_TO_3V6 = 1     /**< input voltages up to 3.6 V are allowed */
} HW_GPADC_INPUT_VOLTAGE;

/**
 * \brief ADC input mode
 *
 */
typedef enum {
        HW_GPADC_INPUT_MODE_DIFFERENTIAL = 0,    /**< differential mode (default) */
        HW_GPADC_INPUT_MODE_SINGLE_ENDED = 1     /**< single ended mode */
} HW_GPADC_INPUT_MODE;

/**
 * \brief ADC clock source
 *
 */
typedef enum {
        HW_GPADC_CLOCK_INTERNAL = 0,       /**< internal high-speed clock (default) */
        HW_GPADC_CLOCK_DIGITAL = 1         /**< digital clock (16/96MHz) */
} HW_GPADC_CLOCK;

typedef void * HW_GPADC_ID;

/**
 * \brief ADC input
 *
 * \p GPADC_INPUT_SE_* values should be used only in single ended mode
 * \p GPADC_INPUT_DIFF_* values should be used only in differential mode
 *
 */
typedef enum {
        HW_GPADC_INPUT_SE_P1_09 = 0,            /**< GPIO P1_09 */
        HW_GPADC_INPUT_SE_P0_25 = 1,            /**< GPIO P0_25 */
        HW_GPADC_INPUT_SE_P0_08 = 2,            /**< GPIO P0_08 */
        HW_GPADC_INPUT_SE_P0_09 = 3,            /**< GPIO P0_09 */
        HW_GPADC_INPUT_SE_VDD   = 4,            /**< VDD supply of the ADC circuit */
        HW_GPADC_INPUT_SE_V30_1 = 5,            /**< V30 supply rail */
        HW_GPADC_INPUT_SE_V30_2 = 6,            /**< V30 supply rail */
        HW_GPADC_INPUT_SE_VBAT  = 8,            /**< Battery voltage, scaled from 5V to 1.2V */
        HW_GPADC_INPUT_SE_VSSA  = 9,            /**< ADC ground */
        HW_GPADC_INPUT_SE_P1_13 = 16,           /**< GPIO P1_13 */
        HW_GPADC_INPUT_SE_P1_12 = 17,           /**< GPIO P1_12 */
        HW_GPADC_INPUT_SE_P1_18 = 18,           /**< GPIO P1_18 */
        HW_GPADC_INPUT_SE_P1_19 = 19,           /**< GPIO P1_19 */
        HW_GPADC_INPUT_SE_TEMPSENS = 20,        /**< temperature sensor */
        HW_GPADC_INPUT_DIFF_P1_09_P0_25 = 0,    /**< GPIO P1_09 vs P0_25 */
        HW_GPADC_INPUT_DIFF_P0_08_P0_09 = 1,    /**< GPIO P0_08 vs P0_09 - all other values */
} HW_GPADC_INPUT;

/**
 * \brief On-chip temperature sensors
 *
 */
typedef enum {
        /* Temperature selection for GP_ADC_DIFF_TEMP_EN = 0 follows this line */
        HW_GPADC_CHARGER_TEMPSENS_GND        = 0,    /**< Ground (no sensor) */
        HW_GPADC_CHARGER_TEMPSENS_Z          = 1,    /**< Z from charger */
        HW_GPADC_CHARGER_TEMPSENS_VNTC       = 2,    /**< V(ntc) from charger */
        HW_GPADC_CHARGER_TEMPSENS_VTEMP      = 3,    /**< V(temp) from charger */
        /* Temperature selection for GP_ADC_DIFF_TEMP_EN = 1 follows this line */
        HW_GPADC_NO_TEMP_SENSOR              = 4,    /**< No on-chip temperature sensor selected (default) */
        HW_GPADC_TEMP_SENSOR_NEAR_RADIO      = 5,    /**< Diode temperature sensor near radio */
        HW_GPADC_TEMP_SENSOR_NEAR_CHARGER    = 6,    /**< Diode temperature sensor near charger */
        HW_GPADC_TEMP_SENSOR_NEAR_BANDGAP    = 7,    /**< Diode temperature sensor near bandgap */
        HW_GPADC_TEMPSENSOR_MAX              = 8,    /**< Invalid */
} HW_GPADC_TEMP_SENSORS;


/**
 * \brief ADC interrupt handler
 *
 */
typedef void (*hw_gpadc_interrupt_cb)(void);

/**
 * \brief ADC configuration
 *
 */
typedef struct {
        HW_GPADC_CLOCK        clock;              /**< clock source */
        HW_GPADC_INPUT_MODE   input_mode;         /**< input mode */
        HW_GPADC_INPUT        input;              /**< ADC input */
        HW_GPADC_TEMP_SENSORS temp_sensor;        /**< Temperature sensor selection */
        uint8_t               sample_time;        /**< sample time */
        bool                  continuous;         /**< continuous mode state */
        uint8_t               interval;           /**< interval between conversions in continuous mode */
        bool                  input_attenuator;   /**< input attenuator state */
        bool                  chopping;           /**< chopping state */
        uint8_t               oversampling;       /**< oversampling value */
} gpadc_config;

/**
 * \brief Initialize ADC
 *
 * \p cfg can be NULL - no configuration is performed in such case.
 *
 * \param [in] cfg configuration
 *
 */
void hw_gpadc_init(const gpadc_config *cfg);

/**
 * \brief Configure ADC
 *
 * Shortcut to call appropriate configuration function. If \p cfg is NULL, this function does
 * nothing.
 *
 * \param [in] cfg configuration
 *
 */
void hw_gpadc_configure(const gpadc_config *cfg);

/**
 * \brief Reset ADC to its default values without disabling the LDO.
 *
 */
void hw_gpadc_reset(void);

/**
 * \brief Register interrupt handler
 *
 * Interrupt is enabled after calling this function. Application is responsible for clearing
 * interrupt using hw_gpadc_clear_interrupt(). If no callback is specified interrupt is cleared by
 * driver.
 *
 * \param [in] cb callback fired on interrupt
 *
 * \sa hw_gpadc_clear_interrupt
 *
 */
void hw_gpadc_register_interrupt(hw_gpadc_interrupt_cb cb);

/**
 * \brief Unregister interrupt handler
 *
 * Interrupt is disabled after calling this function.
 *
 */
void hw_gpadc_unregister_interrupt(void);

/**
 * \brief Clear interrupt
 *
 * Application should call this in interrupt handler to clear interrupt.
 *
 * \sa hw_gpadc_register_interrupt
 *
 */
__STATIC_INLINE void hw_gpadc_clear_interrupt(void)
{
        GPADC->GP_ADC_CLEAR_INT_REG = 1;
}

/**
 * \brief Enable ADC
 *
 * Sampling is started after calling this function, to start conversion application should call
 * hw_gpadc_start().
 *
 * \sa hw_gpadc_start
 *
 */
__STATIC_INLINE void hw_gpadc_enable(void)
{
        REG_SET_BIT(GPADC, GP_ADC_CTRL_REG, GP_ADC_EN);
}

/**
 * \brief Disable ADC
 *
 * Application should wait for conversion to be completed before disabling ADC. In case of
 * continuous mode, application should disable continuous mode and then wait for conversion to be
 * completed in order to have ADC in defined state.
 *
 * \sa hw_gpadc_in_progress
 * \sa hw_gpadc_set_continuous
 *
 */
__STATIC_INLINE void hw_gpadc_disable(void)
{
        REG_CLR_BIT(GPADC, GP_ADC_CTRL_REG, GP_ADC_EN);
}

/**
 * \brief Set the delay required to enable the ADC_LDO.
 *
 * param [in] LDO enable delay
 *
 */
__STATIC_INLINE void hw_gpadc_set_ldo_delay(uint32_t delay)
{
        REG_SETF(GPADC, GP_ADC_CTRL3_REG, GP_ADC_EN_DEL, delay);
}

/** \brief Perform ADC calibration
 *
 * Calibration is performed.
 * LDO constant and dynamic currents may be on.
 * The current driver configuration is saved and re-applied after calibration.
 *
 */
void hw_gpadc_offset_calibrate(void);

/**
 * \brief Start conversion
 *
 * Application should not call this function while conversion is still in progress.
 *
 * \sa hw_gpadc_in_progress
 *
 */
__STATIC_INLINE void hw_gpadc_start(void)
{
        REG_SETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_START, 1);
}

/**
 * \brief Check if conversion is in progress
 *
 * \return conversion state
 *
 */
__STATIC_INLINE bool hw_gpadc_in_progress(void)
{
        return REG_GETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_START);
}

/**
 * \brief Set continuous mode
 *
 * With continuous mode enabled ADC will automatically restart conversion once completed. It's still
 * required to start 1st conversion using hw_gpadc_start(). Interval between subsequent conversions
 * can be adjusted using hw_gpadc_set_interval().
 *
 * \param [in] enabled continuous mode state
 *
 * \sa hw_gpadc_start
 * \sa hw_gpadc_set_interval
 *
 */
__STATIC_INLINE void hw_gpadc_set_continuous(bool enabled)
{
        REG_SETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_CONT, !!enabled);
}

/**
 * \brief Get continuous mode state
 *
 * \return continuous mode state
 *
 */
__STATIC_INLINE bool hw_gpadc_get_continuous(void)
{
        return REG_GETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_CONT);
}

/**
 * \brief Set input channel
 *
 * Application is responsible for using proper input symbols depending on whether single ended or
 * differential mode is used.
 *
 * \param [in] input input channel
 *
 * \sa hw_gpadc_set_input_mode
 * \sa GPADC_INPUT_MODE
 *
 */
__STATIC_INLINE void hw_gpadc_set_input(HW_GPADC_INPUT input)
{
        REG_SETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_SEL, input);
}

/**
 * \brief Get current input channel
 *
 * \return input channel
 *
 */
__STATIC_INLINE HW_GPADC_INPUT hw_gpadc_get_input(void)
{
        return (HW_GPADC_INPUT) REG_GETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_SEL);
}

/**
 * \brief Set input mode
 *
 * \param [in] mode input mode
 *
 */
__STATIC_INLINE void hw_gpadc_set_input_mode(HW_GPADC_INPUT_MODE mode)
{
        REG_SETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_SE, mode);
}

/**
 * \brief Get current input mode
 *
 * return input mode
 *
 */
__STATIC_INLINE HW_GPADC_INPUT_MODE hw_gpadc_get_input_mode(void)
{
        return (HW_GPADC_INPUT_MODE) REG_GETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_SE);
}

/**
 * \brief Set clock source
 *
 * \param [in] clock clock source
 *
 */
__STATIC_INLINE void hw_gpadc_set_clock(HW_GPADC_CLOCK clock)
{
        REG_SETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_CLK_SEL, clock);
}

/**
 * \brief Get current clock source
 *
 * \return clock source
 *
 */
__STATIC_INLINE HW_GPADC_CLOCK hw_gpadc_get_clock(void)
{
        return (HW_GPADC_CLOCK) REG_GETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_CLK_SEL);
}

/**
 * \brief Set oversampling
 *
 * With oversampling enabled multiple successive conversions will be executed and results are added
 * together to increase effective number of bits in result.
 *
 * Number of samples taken is 2<sup>\p n_samples</sup>. Valid values for \p n_samples are 0-7 thus
 * at most 128 samples can be taken. In this case, 17bits of result are generated with the least
 * significant bit being discarded.
 *
 * \param [in] n_samples number of samples to be taken
 *
 */
__STATIC_INLINE void hw_gpadc_set_oversampling(uint8_t n_samples)
{
        REG_SETF(GPADC, GP_ADC_CTRL2_REG, GP_ADC_CONV_NRS, n_samples);
}

/**
 * \brief Get current oversampling
 *
 * \return number of samples to be taken
 *
 * \sa hw_gpadc_set_oversampling
 *
 */
__STATIC_INLINE uint8_t hw_gpadc_get_oversampling(void)
{
        return REG_GETF(GPADC, GP_ADC_CTRL2_REG, GP_ADC_CONV_NRS);
}

/**
 * \brief Set sample time
 *
 * Sample time is \p mult x 32 clock cycles or 1 clock cycle when \p mult is 0. Valid values are
 * 0-15.
 *
 * \param [in] mult multiplier
 *
 */
__STATIC_INLINE void hw_gpadc_set_sample_time(uint8_t mult)
{
        REG_SETF(GPADC, GP_ADC_CTRL2_REG, GP_ADC_SMPL_TIME, mult);
}

/**
 * \brief Get current sample time
 *
 * \return multiplier
 *
 * \sa hw_gpadc_set_sample_time
 *
 */
__STATIC_INLINE uint8_t hw_gpadc_get_sample_time(void)
{
        return REG_GETF(GPADC, GP_ADC_CTRL2_REG, GP_ADC_SMPL_TIME);
}

/**
 * \brief Set state of input attenuator
 *
 * Enabling internal attenuator scales input voltage by factor of 3 thus increasing effective input
 * scale from 0-1.2V to 0-3.6V in single ended mode or from -1.2-1.2V to -3.6-3.6V in differential
 * mode.
 *
 * \param [in] enabled attenuator state
 *
 */
__STATIC_INLINE void hw_gpadc_set_input_attenuator_state(bool enabled)
{
        REG_SETF(GPADC, GP_ADC_CTRL2_REG, GP_ADC_ATTN3X, !!enabled);
}

/**
 * \brief Get current state of input attenuator
 *
 * \return attenuator state
 *
 */
__STATIC_INLINE bool hw_gpadc_get_input_attenuator_state(void)
{
        return REG_GETF(GPADC, GP_ADC_CTRL2_REG, GP_ADC_ATTN3X);
}

/**
 * \brief Set input mute state
 *
 * Once enabled, samples are taken at mid-scale to determine internal offset and/or notice of the
 * ADC with regards to VDD_REF.
 *
 * \param [in] enabled mute state
 *
 * \sa hw_gpadc_offset_calibrate
 *
 */
__STATIC_INLINE void hw_gpadc_set_mute(bool enabled)
{
        REG_SETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_MUTE, !!enabled);
}

/**
 * \brief Get current input mute state
 *
 * \return mute state
 *
 * \sa hw_gpadc_set_mute
 *
 */
__STATIC_INLINE bool hw_gpadc_get_mute(void)
{
        return REG_GETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_MUTE);
}

/**
 * \brief Set input and output sign change
 *
 * Once enabled, sign of ADC input and output is changed.
 *
 * \param [in] enabled sign change state
 *
 */
__STATIC_INLINE void hw_gpadc_set_sign_change(bool enabled)
{
        REG_SETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_SIGN, !!enabled);
}

/**
 * \brief Set input and output sign change
 *
 * \return sign change state
 *
 */
__STATIC_INLINE bool hw_gpadc_get_sign_change(void)
{
        return REG_GETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_SIGN);
}

/**
 * \brief Set state of on-chip temperature sensors
 *
 * Once enabled, the diode temperature sensors can be selected.
 *
 * \param [in] enabled on-chip temperature sensors
 *
 * \sa hw_gpadc_select_diff_temp_sensor
 */
__STATIC_INLINE void hw_gpadc_set_diff_temp_sensors(bool enabled)
{
        REG_SETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_DIFF_TEMP_EN, !!enabled);
}

/**
 * \brief Get state of on-chip temperature sensors
 *
 * \return on-chip temperature sensors state
 *
 */
__STATIC_INLINE bool hw_gpadc_get_diff_temp_sensors(void)
{
        return REG_GETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_DIFF_TEMP_EN);
}

/**
 * \brief Selects on-chip temperature sensor
 *
 * \param [in] sensor on-chip temperature sensor
 *
 * \note When temperature sensors are enabled (GP_ADC_DIFF_TEMP_EN=1),
 * then: 0 = GND, 1 = sensor near radio, 2 = sensor near charger, 3 = sensor near bandgap.
 * When temperature sensors are disabled (GP_ADC_DIFF_TEMP_EN=0),
 * then: 0 = GND, 1 = Z, 2 = V(ntc) from charger, 3 = V(temp) from charger.
 * \note Users are advised NOT to use this API function, unless they know exactly what they are doing.
 * In the general case, setting the gpadc->temp_sensor and calling hw_gpadc_init() or hw_gpadc_configure() is enough.
 * \sa hw_gpadc_set_diff_temp_sensors \sa hw_gpadc_configure()
 */
__STATIC_INLINE void hw_gpadc_select_diff_temp_sensor(HW_GPADC_TEMP_SENSORS  sensor)
{
        REG_SETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_DIFF_TEMP_SEL, sensor & 0x03);
}


/**
 * \brief Set chopping state
 *
 * Once enabled, two samples with opposite polarity are taken to cancel offset.
 *
 * \param [in] enabled chopping state
 *
 */
__STATIC_INLINE void hw_gpadc_set_chopping(bool enabled)
{
        REG_SETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_CHOP, !!enabled);
}

/**
 * \brief Get current chopping state
 *
 * \return chopping state
 *
 */
__STATIC_INLINE bool hw_gpadc_get_chopping(void)
{
        return REG_GETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_CHOP);
}

/**
 * \brief Set state of constant 20uA load current on ADC LDO output
 *
 * Constant 20uA load current on LDO output can be enabled so that the current will not drop to 0.
 *
 * \param [in] enabled load current state
 *
 */
__STATIC_INLINE void hw_gpadc_set_ldo_constant_current(bool enabled)
{
        REG_SETF(GPADC, GP_ADC_CTRL2_REG, GP_ADC_I20U, !!enabled);
}

/**
 * \brief Get current state of constant 20uA load current on ADC LDO output
 *
 * \return load current current state
 *
 * \sa hw_gpadc_set_ldo_constant_current
 *
 */
__STATIC_INLINE bool hw_gpadc_get_ldo_constant_current(void)
{
        return REG_GETF(GPADC, GP_ADC_CTRL2_REG, GP_ADC_I20U);
}

/**
 * \brief Set state of dynamic 10uA load current on ADC LDO output
 *
 * 10uA load current on LDO output can be enabled during sample phase so that the load current
 * during sampling and conversion phase becomes approximately the same.
 *
 * \param [in] enabled load current state
 *
 */
__STATIC_INLINE void hw_gpadc_set_ldo_dynamic_current(bool enabled)
{
        REG_SETF(GPADC, GP_ADC_CTRL2_REG, GP_ADC_IDYN, !!enabled);
}

/**
 * \brief Get current state of dynamic 10uA load current on ADC LDO output
 *
 * \return load current current state
 *
 * \sa hw_gpadc_set_ldo_dynamic_current
 *
 */
__STATIC_INLINE bool hw_gpadc_get_ldo_dynamic_current(void)
{
        return REG_GETF(GPADC, GP_ADC_CTRL2_REG, GP_ADC_IDYN);
}

/**
 * \brief Set interval between conversions in continuous mode
 *
 * Interval time is \p mult x 1.024ms. Valid values are 0-255.
 *
 * \param [in] mult multiplier
 *
 * \sa hw_gpadc_set_continuous
 *
 */
__STATIC_INLINE void hw_gpadc_set_interval(uint8_t mult)
{
        REG_SETF(GPADC, GP_ADC_CTRL3_REG, GP_ADC_INTERVAL, mult);
}

/**
 * \brief Get current interval between conversions in continuous mode
 *
 * \return multiplier
 *
 * \sa hw_gpadc_set_interval
 *
 */
__STATIC_INLINE uint8_t hw_gpadc_get_interval(void)
{
        return REG_GETF(GPADC, GP_ADC_CTRL3_REG, GP_ADC_INTERVAL);
}

/**
 * \brief Set offset adjustment for positive ADC array
 *
 * \param [in] offset offset value
 *
 * \sa hw_gpadc_offset_calibrate
 * \sa hw_gpadc_set_negative_offset
 *
 */
__STATIC_INLINE void hw_gpadc_set_offset_positive(uint16_t offset)
{
        GPADC->GP_ADC_OFFP_REG = offset & REG_MSK(GPADC, GP_ADC_OFFP_REG, GP_ADC_OFFP);
}

/**
 * \brief Get current offset adjustment for positive ADC array
 *
 * \return offset value
 *
 */
__STATIC_INLINE uint16_t hw_gpadc_get_offset_positive(void)
{
        return GPADC->GP_ADC_OFFP_REG & REG_MSK(GPADC, GP_ADC_OFFP_REG, GP_ADC_OFFP);
}

/**
 * \brief Set offset adjustment for negative ADC array
 *
 * \param [in] offset offset value
 *
 * \sa hw_gpadc_offset_calibrate
 * \sa hw_gpadc_set_positive_offset
 *
 */
__STATIC_INLINE void hw_gpadc_set_offset_negative(uint16_t offset)
{
        GPADC->GP_ADC_OFFN_REG = offset & REG_MSK(GPADC, GP_ADC_OFFN_REG, GP_ADC_OFFN);
}


/**
 * \brief Get current offset adjustment for negative ADC array
 *
 * \return offset value
 *
 */
__STATIC_INLINE uint16_t hw_gpadc_get_offset_negative(void)
{
        return GPADC->GP_ADC_OFFN_REG & REG_MSK(GPADC, GP_ADC_OFFN_REG, GP_ADC_OFFN);
}

/**
 * \brief Store Single Ended ADC Gain Error
 *
 * \param [in] single ADC Single Ended Gain Error
 *
 */
void hw_gpadc_store_se_gain_error(int16_t single);

/**
 * \brief Store Differential ADC Gain Error
 *
 * \param [in] diff ADC Differential Gain Error
 *
 */
void hw_gpadc_store_diff_gain_error(int16_t diff);


/**
 * \brief Store Single Ended ADC Offset Error
 *
 * \param [in] single ADC Single Ended Offset Error
 *
 */
void hw_gpadc_store_se_offset_error(int16_t single);

/**
 * \brief Store Differential ADC Offset Error
 *
 * \param [in] diff ADC Differential Offset Error
 *
 */
void hw_gpadc_store_diff_offset_error(int16_t diff);

/**
 * \brief Calculate Single Ended ADC Gain Error from two points
 *
 * \param [in] low  measurement at the low end of the full scale
 * \param [in] high measurement at the high end of the full scale
 *
 * \return Single Ended ADC Gain Error
 *
 * \note Valid return range (-2048, 2048)
 *
 */
__STATIC_INLINE int16_t hw_gpadc_calculate_single_ended_gain_error(int16_t low, int16_t high)
{
        return ((high - low + ((uint16_t)(high - low) >> 2 )) - UINT16_MAX);
}

/**
 * \brief Calculate Single Ended ADC Offset Error from two points
 *
 * \param [in] low  measurement at the low end of the full scale
 * \param [in] high measurement at the high end of the full scale
 *
 * \return Single Ended ADC Offset Error
 *
 * \note Valid return range (-512, 512)
 *
 */
__STATIC_INLINE int16_t hw_gpadc_calculate_single_ended_offset_error(int16_t low, int16_t high)
{
        return ((int16_t)((9 * low) - high) >> 3);
}

/**
 * \brief Calculate Differential ADC Gain Error from two points
 *
 * \param [in] low  measurement at the low end of the full scale
 * \param [in] high measurement at the high end of the full scale
 *
 * \return Differential ADC Gain Error
 *
 * \note Valid return range (-2048, 2048)
 *
 */
__STATIC_INLINE int16_t hw_gpadc_calculate_differential_gain_error(int16_t low, int16_t high)
{
        return (((high - low) + ((uint16_t)(high - low) >> 2)) - UINT16_MAX);
}

/**
 * \brief Calculate Differential ADC Offset Error from two points
 *
 * \param [in] low  measurement at the low end of the full scale
 * \param [in] high measurement at the high end of the full scale
 *
 * \return Differential ADC Offset Error
 *
 * \note Valid return range (-512, 512)
 *
 */
__STATIC_INLINE int16_t hw_gpadc_calculate_differential_offset_error(int16_t low, int16_t high)
{
        return (low + high) >> 1;
}



/**
 * \brief Check the availability of ADC Gain Error
 *
 * \return ADC Gain Error availability
 *
 */
bool hw_gpadc_pre_check_for_gain_error(void);


/**
 * \brief Get raw ADC value.
 *
 * \return raw ADC value.
 *
 * \note Neither any correction nor any conversion does it take place.
 */
__STATIC_INLINE uint16_t hw_gpadc_get_raw_value(void)
{
        return GPADC->GP_ADC_RESULT_REG;
}

/**
 * \brief Convert the GPADC digital value to temperature
 *
 * \param [in] cfg      GPADC configuration
 * \param [in] val      digital GPADC value
 *
 * \return temperature in Celsius degrees
 *
 */
int16_t hw_gpadc_convert_to_temperature(const gpadc_config *cfg, uint16_t val);

/**
 * \brief Convert a temperature value to raw GPADC value, as it would be in the GP_ADC_RESULT_REG
 *
 * \param [in] cfg      GPADC configuration
 * \param [in] temp     temperature
 *
 * \return 16-bit left-aligned ADC value (raw)
 *
 */
uint16_t hw_gpadc_convert_temperature_to_raw_val(const gpadc_config *cfg, int16_t temp);

/*
 * \brief Initialize temperature sensor safely
 */
void hw_gpadc_safe_tempsens_initialize(void);

/*
 * \brief Finalize temperature sensor safely
 */
void hw_gpadc_safe_tempsens_finalize(void);

/**
 * \brief Get conversion result value with gain compensation and over sampling
 *
 * Invalid bits are discarded from result, i.e. oversampling is taken into account when calculating
 * value.
 *
 * \return conversion result value
 *
 * \sa hw_gpadc_get_raw_value
 * \sa hw_gpadc_set_oversampling
 *
 */
uint16_t hw_gpadc_get_value(void);

/**
 * \brief Start a measurement and wait for the result.
 *
 * \sa hw_gpadc_start
 * \sa hw_gpadc_in_progress
 * \sa hw_gpadc_clear_interrupt
 *
 */
void hw_gpadc_adc_measure(void);

#endif /* dg_configUSE_HW_GPADC */
#endif /* HW_GPADC_H */

/**
 * \}
 * \}
 */
