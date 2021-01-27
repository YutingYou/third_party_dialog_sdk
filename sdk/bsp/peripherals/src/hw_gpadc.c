/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup GPADC
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file hw_gpadc.c
 *
 * @brief Implementation of the GPADC Low Level Driver.
 *
 * Copyright (C) 2015-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#if dg_configUSE_HW_GPADC


#include <stdio.h>
#include <string.h>
#include <hw_gpadc.h>

#if (dg_configSYSTEMVIEW)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif

static hw_gpadc_interrupt_cb intr_cb = NULL;

__RETAINED static int16_t hw_gpadc_differential_gain_error;
__RETAINED static int16_t hw_gpadc_single_ended_gain_error;
__RETAINED static int16_t hw_gpadc_differential_offset_error;
__RETAINED static int16_t hw_gpadc_single_ended_offset_error;

#define ADC_IRQ         GPADC_IRQn
#define GP_ADC_CTRL_SZ          12      // the CTRL registers size altogether

/**
 * \brief Calibration Data-Point
 * (Temperature, ADC value) & slope (x 1000)
 */
typedef struct {
        int32_t temp;
        uint32_t adc;
        int32_t millislope;
} hw_gpadc_calibration_values_t;

/**
 * \brief Default temperature calibration data.
 *
 * The calibration point coordinates and the temperature functions slope are empirical values
 * calculated via a range of test measurements between -40 and 80 degrees Celsius.
 * The arithmetic values are 32-bit left-aligned, following the format of the GP_ADC_RESULT_REG
 */
__RETAINED_CONST_INIT static const hw_gpadc_calibration_values_t temperature_calibration_data[HW_GPADC_TEMPSENSOR_MAX] = {
        {0, 0, 0},                      /* GND - no sensor */
        {0, 0, 0},                      /* No calibration function - Open circuit value Z */
        {0, 0, 0},                      /* No calibration function - V(ntc) */
        {0, 712 << 6, 2440 << 6},       /* temperature sensor near charger */
        {0, 0, 0},                      /* no sensor */
        {0, 641 << 6, -(1282 << 6)},    /* diode near radio */
        {0, 637 << 6, -(1300 << 6)},    /* diode near charger */
        {0, 638 << 6, -(1291 << 6)},    /* diode near bandgap */
};

void hw_gpadc_init(const gpadc_config *cfg)
{
        ASSERT_WARNING(REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_1V8P_ENABLE) ||
                REG_GETF(CRG_TOP, POWER_CTRL_REG, LDO_1V8P_RET_ENABLE_ACTIVE));
        GPADC->GP_ADC_CTRL_REG = 0;
        GPADC->GP_ADC_CTRL2_REG = 0;
        GPADC->GP_ADC_CTRL3_REG = 0x40;      // default value for GP_ADC_EN_DEL

        NVIC_DisableIRQ(ADC_IRQ);

        hw_gpadc_configure(cfg);
}

void hw_gpadc_reset(void)
{
        /* Implementing the following DS note:
         * Before making any changes to the ADC settings,
         * Continuous mode must be disabled by setting bit GP_ADC_CONT to 0,
         * while waiting until bit GP_ADC_START = 0.
         */
        hw_gpadc_set_continuous(false);
        while (hw_gpadc_in_progress());

        GPADC->GP_ADC_CTRL_REG = REG_MSK(GPADC, GP_ADC_CTRL_REG, GP_ADC_EN);
        GPADC->GP_ADC_CTRL2_REG = 0;
        GPADC->GP_ADC_CTRL3_REG = 0x40;      // default value for GP_ADC_EN_DEL

        NVIC_DisableIRQ(ADC_IRQ);
}

void hw_gpadc_configure(const gpadc_config *cfg)
{
        /* Implementing the following DS note:
         * Before making any changes to the ADC settings,
         * Continuous mode must be disabled by setting bit GP_ADC_CONT to 0,
         * while waiting until bit GP_ADC_START = 0.
         */
        hw_gpadc_set_continuous(false);
        while (hw_gpadc_in_progress());

        if (cfg) {
                hw_gpadc_set_input_mode(cfg->input_mode);
                hw_gpadc_set_clock(cfg->clock);
                hw_gpadc_set_input(cfg->input);
                hw_gpadc_set_sample_time(cfg->sample_time);
                hw_gpadc_set_continuous(cfg->continuous);
                hw_gpadc_set_interval(cfg->interval);
                hw_gpadc_set_input_attenuator_state(cfg->input_attenuator);
                hw_gpadc_set_chopping(cfg->chopping);
                hw_gpadc_set_oversampling(cfg->oversampling);
                if (hw_gpadc_get_input() == HW_GPADC_INPUT_SE_TEMPSENS) {
                        ASSERT_ERROR(cfg->temp_sensor < HW_GPADC_TEMPSENSOR_MAX);
                        ASSERT_WARNING(cfg->temp_sensor != HW_GPADC_NO_TEMP_SENSOR);
                        ASSERT_WARNING(cfg->temp_sensor != HW_GPADC_CHARGER_TEMPSENS_GND);
                        /* Switches on/off the GP_ADC_DIFF_TEMP_EN bit, according to cfg->temp_sensor value.
                         * This field drives the TEMPSENS input circuit (diodes or charger tempsens).
                         */
                        hw_gpadc_set_diff_temp_sensors(cfg->temp_sensor > HW_GPADC_CHARGER_TEMPSENS_VTEMP);
                        hw_gpadc_select_diff_temp_sensor(cfg->temp_sensor);
                }
        }
}

void hw_gpadc_register_interrupt(hw_gpadc_interrupt_cb cb)
{
        intr_cb = cb;

        REG_SETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_MINT, 1);

        NVIC_EnableIRQ(ADC_IRQ);
}

void hw_gpadc_unregister_interrupt(void)
{
        NVIC_DisableIRQ(ADC_IRQ);

        REG_SETF(GPADC, GP_ADC_CTRL_REG, GP_ADC_MINT, 0);

        intr_cb = NULL;
}

void hw_gpadc_adc_measure(void)
{
        hw_gpadc_start();
        while (hw_gpadc_in_progress());
        hw_gpadc_clear_interrupt();
}

void ADC_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        if (intr_cb) {
                intr_cb();
        } else {
                hw_gpadc_clear_interrupt();
        }

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

void hw_gpadc_offset_calibrate(void)
{
        uint16_t adc_off_p, adc_off_n, verify, deviation;
        uint8_t old_conf[GP_ADC_CTRL_SZ];
        HW_GPADC_INPUT_MODE mode;
        int factor;

        memcpy(old_conf, (void *)GPADC_BASE, GP_ADC_CTRL_SZ);
        mode = hw_gpadc_get_input_mode();

        hw_gpadc_reset();
        hw_gpadc_set_input_mode(mode);
        hw_gpadc_set_oversampling(4);
        hw_gpadc_set_sample_time(3);
        hw_gpadc_set_mute(true);
        hw_gpadc_set_offset_positive(0x200);
        hw_gpadc_set_offset_negative(0x200);

        /* formula differs for SE and DIFF modes by this factor */
        factor = (mode == HW_GPADC_INPUT_MODE_SINGLE_ENDED) ? 2 : 1;

        /* Up to five calibration tries */
        for (int i = 0; i < 5; i++) {
                hw_gpadc_adc_measure();
                adc_off_p = (hw_gpadc_get_raw_value() >> 6) - 0x200;

                hw_gpadc_set_sign_change(true);
                hw_gpadc_adc_measure();
                adc_off_n = (hw_gpadc_get_raw_value() >> 6) - 0x200;

                hw_gpadc_set_offset_positive(0x200 - factor * adc_off_p);
                hw_gpadc_set_offset_negative(0x200 - factor * adc_off_n);

                /* Verification - Is result on mute close to 0x200 ? */
                hw_gpadc_set_sign_change(false);
                hw_gpadc_adc_measure();
                verify = hw_gpadc_get_raw_value() >> 6;
                deviation = (verify < 0x200)? (0x200 - verify) : (verify - 0x200);

                /* Calibration converges */
                if (deviation < 0x8) {
                        break;
                }

                /* Reset OFFSET registers if calibration does not converge */
                if (i == 4) {
                        ASSERT_WARNING(0);
                        hw_gpadc_set_offset_positive(0x200);
                        hw_gpadc_set_offset_negative(0x200);
                }
        }

        hw_gpadc_reset();
        hw_gpadc_disable();
        memcpy((void *)GPADC_BASE, old_conf, GP_ADC_CTRL_SZ);
}

bool hw_gpadc_pre_check_for_gain_error(void)
{
        if (dg_configUSE_ADC_GAIN_ERROR_CORRECTION == 1) {
                return (hw_gpadc_single_ended_gain_error && hw_gpadc_differential_gain_error);
        }

        return false;
}

int16_t hw_gpadc_get_single_ended_gain_error(void)
{
        return hw_gpadc_single_ended_gain_error;
}

void hw_gpadc_store_se_gain_error(int16_t single)
{
        hw_gpadc_single_ended_gain_error = single;
}

void hw_gpadc_store_diff_gain_error(int16_t diff)
{
        hw_gpadc_differential_gain_error = diff;
}


void hw_gpadc_store_se_offset_error(int16_t single)
{
        hw_gpadc_single_ended_offset_error = single;
}

void hw_gpadc_store_diff_offset_error(int16_t diff)
{
        hw_gpadc_differential_offset_error = diff;
}

static uint16_t hw_gpadc_apply_correction(uint16_t raw)
{
        int64_t res;
        uint8_t mode;

        if (!hw_gpadc_pre_check_for_gain_error()) {
                return raw;
        }

        res = raw;
        mode = hw_gpadc_get_input_mode();

        /* Offset Correction */
        if (mode == HW_GPADC_INPUT_MODE_SINGLE_ENDED) {
                res -=  hw_gpadc_single_ended_offset_error;
        } else {
                res -=  hw_gpadc_differential_offset_error;
        }
        /* Boundary check for lower limit */
        if (res <= 0) {
                return 0;
        }
        /* Gain Correction */
        if (mode == HW_GPADC_INPUT_MODE_SINGLE_ENDED) {
                res = (UINT16_MAX * res) / (UINT16_MAX + hw_gpadc_single_ended_gain_error);
                /* Boundary check for upper limit */
                if (res >= UINT16_MAX) {
                        return UINT16_MAX;
                }
                return res;
        } else {
                res = (int16_t)(res ^ 0x8000);
                res = (UINT16_MAX * res) / (UINT16_MAX + hw_gpadc_differential_gain_error);
                /* Boundary check for lower limit */
                if (res < INT16_MIN) {
                        return 0;
                }
                /* Boundary check for upper limit */
                if (res > INT16_MAX) {
                        return UINT16_MAX;
                }
                return res ^ 0x8000;
        }
}

int16_t hw_gpadc_convert_to_temperature(const gpadc_config *cfg, uint16_t val)
{
        int32_t calib_temp;
        uint32_t calib_adc;
        int32_t slope_x_1000;
        uint32_t val32 = val << (6 - MIN(6, cfg->oversampling));

        /* Check if shifting due to oversampling is correct */
        ASSERT_ERROR((val32 & 0xFFFF0000) == 0);

        ASSERT_ERROR(cfg->temp_sensor < HW_GPADC_TEMPSENSOR_MAX);
        calib_temp = temperature_calibration_data[cfg->temp_sensor].temp;
        calib_adc = temperature_calibration_data[cfg->temp_sensor].adc;
        slope_x_1000 = temperature_calibration_data[cfg->temp_sensor].millislope;
        return calib_temp + ((int32_t)(val32 - calib_adc) * 1000) / slope_x_1000;
}

uint16_t hw_gpadc_convert_temperature_to_raw_val(const gpadc_config *cfg, int16_t temp)
{
        int32_t calib_temp;
        uint32_t calib_adc;
        int32_t slope_x_1000;
        uint32_t result32;

        ASSERT_ERROR(cfg->temp_sensor < HW_GPADC_TEMPSENSOR_MAX);
        calib_temp = temperature_calibration_data[cfg->temp_sensor].temp;
        calib_adc = temperature_calibration_data[cfg->temp_sensor].adc;
        slope_x_1000 = temperature_calibration_data[cfg->temp_sensor].millislope;
        result32 = calib_adc + ((int32_t)temp - calib_temp) * slope_x_1000 / 1000;
        ASSERT_ERROR((result32 & 0xFFFF0000) == 0);
        return (uint16_t) result32;
}


uint16_t hw_gpadc_get_value(void)
{
        uint16_t adc_raw_res = hw_gpadc_get_raw_value();
        return hw_gpadc_apply_correction(adc_raw_res) >> (6 - MIN(6, hw_gpadc_get_oversampling()));
}
#endif /* dg_configUSE_HW_GPADC */
/**
 * \}
 * \}
 * \}
 */
