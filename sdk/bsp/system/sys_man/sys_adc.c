/**
 ****************************************************************************************
 *
 * @file sys_adc.c
 *
 * @brief sys_adc source file.
 *
 * Copyright (C) 2018-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if (dg_configUSE_SYS_ADC == 1)

#include "hw_pdc.h"
#include "hw_rtc.h"
#include "sys_adc.h"
#include "snc_hw_gpadc.h"

_SNC_RETAINED static snc_cm33_mutex_t gpadc_mutex;
#if defined(dg_configRTC_PDC_EVENT_PERIOD)
_SNC_RETAINED static uint32_t rtc_pdc_evt_cnt;
#endif
_SNC_RETAINED static struct {
        uint32_t notif_cm33;    // CM33 notification indication
        uint32_t lower_bound;   // the lower bound used in uCode to notify CM33 if current value is lower than that
        uint32_t upper_bound;   // the upper bound used in uCode to notify CM33 if current value is higher than that
        uint32_t current_value; // current value of the GPADC measurement
} sys_adc_ucode_env;

__RETAINED static uint32_t ucode_id;
__RETAINED static uint16_t drift;

static void ucode_gpadc_init(void)
{
        snc_mutex_SNC_create(&gpadc_mutex);
}

static void ucode_gpadc_set_bounds(uint16_t value)
{
        snc_mutex_SNC_lock(&gpadc_mutex);

        sys_adc_ucode_env.lower_bound = value - drift;
        sys_adc_ucode_env.upper_bound = value + drift;

        snc_mutex_SNC_unlock(&gpadc_mutex);
}

static uint32_t ucode_gpadc_get_value()
{
        uint32_t ret;
        snc_mutex_SNC_lock(&gpadc_mutex);
        ret = sys_adc_ucode_env.current_value;
        snc_mutex_SNC_unlock(&gpadc_mutex);
        return ret;
}

void sys_adc_config(const ad_gpadc_controller_conf_t * conf, ad_snc_interrupt_cb _gpadc_cb,
        sys_adc_monitor_par_t* monitor_par)
{
        ad_snc_ucode_cfg_t cfg = { 0 };
        ad_gpadc_handle_t handle;
        uint16_t val;


        drift = hw_gpadc_convert_temperature_to_raw_val(TEMP_SENSOR_RADIO_INTERNAL.drv, 0) -
                hw_gpadc_convert_temperature_to_raw_val(TEMP_SENSOR_RADIO_INTERNAL.drv, monitor_par->drift);

        handle = ad_gpadc_open(conf);
        ad_gpadc_read_raw(handle, &val);
        ad_gpadc_close(handle, false);

        // Configure the PDC event and uCode priorities
        cfg.pdc_evt_pr = AD_SNC_PDC_EVT_PR_0;
        cfg.ucode_pr = AD_SNC_UCODE_PR_1;

        // Set the SNC-to-CM33 notification callback
        cfg.cb = _gpadc_cb;

        // The GPADC uCode is executed on RTC-to-PDC event interrupt
        cfg.pdc_entry = HW_PDC_LUT_ENTRY_VAL(HW_PDC_TRIG_SELECT_PERIPHERAL,
                HW_PDC_PERIPH_TRIG_ID_RTC_TIMER, HW_PDC_MASTER_SNC, HW_PDC_LUT_ENTRY_EN_PER);

        ucode_gpadc_init();
        ucode_gpadc_set_bounds(val);

        ucode_id = ad_snc_ucode_register(&cfg,
                SNC_UCODE_CTX(ucode_gpadc_rf_temp_notify));

#if !defined(dg_configRTC_PDC_EVENT_PERIOD)
        hw_rtc_config_pdc_evt_t rtc_cfg = { 0 };

        // Enable the RTC PDC event
        rtc_cfg.pdc_evt_en = true;

        // Set the RTC PDC event period
        rtc_cfg.pdc_evt_period = (monitor_par->pollIntervalms / 10) - 1;

        // Configure the RTC event controller
        hw_rtc_config_RTC_to_PDC_evt(&rtc_cfg);

        // Enable the RTC peripheral clock
        hw_rtc_clock_enable();

        // Start the RTC
        hw_rtc_time_start();
#endif
}

void sys_adc_enable()
{
        ad_snc_ucode_enable(ucode_id);
}

void sys_adc_disable()
{
        ad_snc_ucode_disable(ucode_id);
}

uint32_t sys_adc_get_value()
{
        return ucode_gpadc_get_value();
}

void sys_adc_set_bounds(uint16_t value)
{
        ucode_gpadc_set_bounds(value);
}

SNC_UCODE_BLOCK_DEF(ucode_gpadc_rf_temp_notify)
{
        // Clear the RTC PDC event
        SNC_hw_sys_clear_rtc_pdc_event();
#if defined(dg_configRTC_PDC_EVENT_PERIOD)
        SENIS_inc1(da(&rtc_pdc_evt_cnt));
        SENIS_if (da(&rtc_pdc_evt_cnt), LT, dg_configRF_CAL_POLL_INT) {
                SENIS_return;
        }
        SENIS_wadva(da(&rtc_pdc_evt_cnt), 0);
#endif
        SNC_MUTEX_LOCK(&gpadc_mutex);

        // Initialize - Acquire the selected GPADC source device
        SNC_gpadc_open(&TEMP_SENSOR_RADIO_INTERNAL);

        // Get adc measurement
        SNC_gpadc_read(&TEMP_SENSOR_RADIO_INTERNAL, da(&sys_adc_ucode_env.current_value));

        // De-initialize - Release the GPADC
        SNC_gpadc_close(&TEMP_SENSOR_RADIO_INTERNAL);

        SENIS_assign(da(&sys_adc_ucode_env.notif_cm33), 0);

        // Notify CM33 based on the lower and upper temperature bounds
        SENIS_if (da(&sys_adc_ucode_env.current_value), LT, da(&sys_adc_ucode_env.lower_bound)) {
                SENIS_assign(da(&sys_adc_ucode_env.notif_cm33), 1);
        }
        SENIS_if (da(&sys_adc_ucode_env.current_value), GT, da(&sys_adc_ucode_env.upper_bound)) {
                SENIS_assign(da(&sys_adc_ucode_env.notif_cm33), 1);
        }

        SNC_MUTEX_UNLOCK(&gpadc_mutex);

        SENIS_if (da(&sys_adc_ucode_env.notif_cm33)) {
                SNC_CM33_NOTIFY();
        }
}

#endif /* (dg_configUSE_SYS_ADC == 1) */
