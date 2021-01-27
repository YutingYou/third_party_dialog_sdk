/**
 ****************************************************************************************
 *
 * @file sys_rcx_calibrate.c
 *
 * @brief RCX calibration source file.
 *
 * Copyright (C) 2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#if (dg_configUSE_LP_CLK == LP_CLK_RCX && defined(OS_FREERTOS))

#include "hw_pdc.h"
#include "hw_rtc.h"
#include "snc_hw_gpadc.h"

#include "../sys_man/sys_rcx_calibrate_internal.h"
#include "../adapters/src/sys_platform_devices_internal.h"


_SNC_RETAINED static snc_cm33_mutex_t rcx_mutex;
_SNC_RETAINED static uint32_t rtc_pdc_evt_cnt_minute;
#if defined(dg_configRTC_PDC_EVENT_PERIOD)
_SNC_RETAINED static uint32_t rtc_pdc_evt_cnt;
#endif
_SNC_RETAINED static struct {
        uint32_t lower_bound[2];   // the lower bound used in uCode to notify CM33 if current value is lower than that
        uint32_t upper_bound[2];   // the upper bound used in uCode to notify CM33 if current value is higher than that
        uint32_t current_value[2]; // current value of the GPADC measurement
} sys_adc_ucode_env;

__RETAINED static uint32_t ucode_id;
__RETAINED static uint16_t drift[2];
_SNC_RETAINED static uint32_t xtal32m_pdc_entry;

static void ucode_rcx_init(void)
{
        snc_mutex_SNC_create(&rcx_mutex);
}

static void ucode_rcx_set_bounds(uint16_t bandgap, uint16_t vbat)
{
        snc_mutex_SNC_lock(&rcx_mutex);

        sys_adc_ucode_env.lower_bound[0] = bandgap - drift[0];
        sys_adc_ucode_env.upper_bound[0] = bandgap + drift[0];

        sys_adc_ucode_env.lower_bound[1] = vbat - drift[1];
        sys_adc_ucode_env.upper_bound[1] = vbat + drift[1];

        snc_mutex_SNC_unlock(&rcx_mutex);
}

static uint16_t ucode_rcx_get_value(uint16_t *vbat)
{
        uint16_t ret;
        snc_mutex_SNC_lock(&rcx_mutex);
        ret = (uint16_t)sys_adc_ucode_env.current_value[0];
        *vbat = (uint16_t)sys_adc_ucode_env.current_value[1];
        snc_mutex_SNC_unlock(&rcx_mutex);
        return ret;
}

void sys_rcx_calibrate_config(ad_snc_interrupt_cb _rcx_calibrate_cb)
{
        ad_snc_ucode_cfg_t cfg = { 0 };
        ad_gpadc_handle_t handle;
        uint16_t val1, val2;

        drift[0] = hw_gpadc_convert_temperature_to_raw_val(TEMP_SENSOR_BANDGAP_INTERNAL.drv, 0) -
                hw_gpadc_convert_temperature_to_raw_val(TEMP_SENSOR_BANDGAP_INTERNAL.drv, BANGAP_TEMP_DRIFT);
        // 1 LSB = 4.88mV
        // V = 4.88mV * (ADC_raw >> 6) --> 1mV = 64/4.88 ~ 13
        drift[1] = VBAT_VOLTAGE_DRIFT * 13;

        handle = ad_gpadc_open(&TEMP_SENSOR_BANDGAP_INTERNAL);
        ad_gpadc_read_raw(handle, &val1);
        ad_gpadc_close(handle, false);

        handle = ad_gpadc_open(&BATTERY_LEVEL_INTERNAL);
        ad_gpadc_read_raw(handle, &val2);
        ad_gpadc_close(handle, false);

        ucode_rcx_init();
        ucode_rcx_set_bounds(val1, val2);

        // Configure the PDC event and uCode priorities
        cfg.pdc_evt_pr = AD_SNC_PDC_EVT_PR_0;
        cfg.ucode_pr = AD_SNC_UCODE_PR_1;

        // Set the SNC-to-CM33 notification callback
        cfg.cb = _rcx_calibrate_cb;

        // The GPADC uCode is executed on RTC-to-PDC event interrupt
        cfg.pdc_entry = HW_PDC_LUT_ENTRY_VAL(HW_PDC_TRIG_SELECT_PERIPHERAL,
                HW_PDC_PERIPH_TRIG_ID_RTC_TIMER, HW_PDC_MASTER_SNC, HW_PDC_LUT_ENTRY_EN_PER);

        xtal32m_pdc_entry = hw_pdc_add_entry( HW_PDC_TRIGGER_FROM_MASTER( HW_PDC_MASTER_SNC,
                                                          HW_PDC_LUT_ENTRY_EN_XTAL ));
        ucode_id = ad_snc_ucode_register(&cfg,
                SNC_UCODE_CTX(ucode_rcx_calibrate_notify));

        ad_snc_ucode_enable(ucode_id);
}

uint16_t sys_rcx_calibrate_get_value(uint16_t *vbat)
{
        return ucode_rcx_get_value(vbat);
}

void sys_rcx_calibrate_set_bounds(uint16_t bandgap, uint16_t vbat)
{
        ucode_rcx_set_bounds(bandgap, vbat);
}

SNC_UCODE_BLOCK_DEF(ucode_rcx_calibrate_notify)
{
        // Clear the RTC PDC event
        SNC_hw_sys_clear_rtc_pdc_event();
#if defined(dg_configRTC_PDC_EVENT_PERIOD)
        SENIS_inc1(da(&rtc_pdc_evt_cnt));
        SENIS_if (da(&rtc_pdc_evt_cnt), LT, dg_configRCX_CAL_POLL_INT) {
                SENIS_return;
        }
        SENIS_wadva(da(&rtc_pdc_evt_cnt), 0);
#endif
        SENIS_labels(pt_xtal32m_set_pending, pt_wait_for_xtal32m_to_settle, pt_wait_for_running_at_xtal32m,
                pt_start_calibration, pt_wait_for_calibration_to_finish, pt_return);

        SENIS_inc1(da(&rtc_pdc_evt_cnt_minute));
        SENIS_if(da(&rtc_pdc_evt_cnt_minute), EQ, UNCOND_CAL_TIME_IN_SEC) {
                SENIS_wadva(da(&rtc_pdc_evt_cnt_minute), 0);
                SENIS_goto(l(pt_xtal32m_set_pending));
        }

        SNC_MUTEX_LOCK(&rcx_mutex);

        // Initialize - Acquire the selected GPADC source device
        SNC_gpadc_open(&TEMP_SENSOR_BANDGAP_INTERNAL);

        // Get adc measurement
        SNC_gpadc_read(&TEMP_SENSOR_BANDGAP_INTERNAL, da(&sys_adc_ucode_env.current_value[0]));

        // De-initialize - Release the GPADC
        SNC_gpadc_close(&TEMP_SENSOR_BANDGAP_INTERNAL);

        // Notify CM33 based on the lower and upper temperature bounds
        SENIS_if (da(&sys_adc_ucode_env.current_value[0]), LT, da(&sys_adc_ucode_env.lower_bound[0])) {
                SENIS_goto(l(pt_xtal32m_set_pending));
        }
        SENIS_if (da(&sys_adc_ucode_env.current_value[0]), GT, da(&sys_adc_ucode_env.upper_bound[0])) {
                SENIS_goto(l(pt_xtal32m_set_pending));
        }

        // Initialize - Acquire the selected GPADC source device
        SNC_gpadc_open(&BATTERY_LEVEL_INTERNAL);

        // Get adc measurement
        SNC_gpadc_read(&BATTERY_LEVEL_INTERNAL, da(&sys_adc_ucode_env.current_value[1]));

        // De-initialize - Release the GPADC
        SNC_gpadc_close(&BATTERY_LEVEL_INTERNAL);

        // Notify CM33 based on the lower and upper temperature bounds
        SENIS_if (da(&sys_adc_ucode_env.current_value[1]), LT, da(&sys_adc_ucode_env.lower_bound[1])) {
                SENIS_goto(l(pt_xtal32m_set_pending));
        }
        SENIS_if (da(&sys_adc_ucode_env.current_value[1]), GT, da(&sys_adc_ucode_env.upper_bound[1])) {
                SENIS_goto(l(pt_xtal32m_set_pending));
        }

        SENIS_goto(l(pt_return));

        SENIS_label(pt_xtal32m_set_pending);

        SENIS_wadva(da(&PDC->PDC_SET_PENDING_REG), xtal32m_pdc_entry);
        SNC_hw_sys_pdc_acknowledge(xtal32m_pdc_entry);

        _SNC_STATIC(uint32_t, const_xtalrdy_0count_minus_1, sizeof(uint32_t),
                ((uint32_t)1 << REG_POS(CRG_XTAL, XTALRDY_STAT_REG, XTALRDY_COUNT)) - 1
        );

        SENIS_label(pt_wait_for_xtal32m_to_settle);
        // ... wait for XTAL32M to settle and ...

        SENIS_rdcgr(da(&CRG_XTAL->XTALRDY_STAT_REG), da(const_xtalrdy_0count_minus_1));
        SENIS_cobr_gr(l(pt_wait_for_xtal32m_to_settle));

        // ... finally switch to XTAL32M.
        SENIS_wadva(da(&CRG_TOP->CLK_SWITCH2XTAL_REG), 0x1);

        // wait until the switch is done
        SENIS_label(pt_wait_for_running_at_xtal32m);

        SENIS_rdcbi(da(&CRG_TOP->CLK_CTRL_REG), REG_POS(CRG_TOP, CLK_CTRL_REG, RUNNING_AT_XTAL32M));
        SENIS_cobr_eq(l(pt_start_calibration));
        SENIS_goto(l(pt_wait_for_running_at_xtal32m));

        // start calibration
        SENIS_label(pt_start_calibration);
        SENIS_wadva(da(&ANAMISC_BIF->CLK_REF_CNT_REG), RCX_CALIBRATION_CYCLES_WUP);
        SENIS_wadva(da(&ANAMISC_BIF->CLK_REF_SEL_REG), 0xB);// DIVN as reference clock, RCX as clock input for
                                                        // calibration, start calibration

        // wait for calibration to finish
        SENIS_label(pt_wait_for_calibration_to_finish);
        SENIS_rdcbi(da(&ANAMISC_BIF->CLK_REF_SEL_REG), REG_POS(ANAMISC_BIF, CLK_REF_SEL_REG, REF_CAL_START));
        SENIS_cobr_eq(l(pt_wait_for_calibration_to_finish));

        SNC_CM33_NOTIFY();

        SENIS_label(pt_return);
        SNC_MUTEX_UNLOCK(&rcx_mutex);
}
#endif /* (dg_configUSE_LP_CLK == LP_CLK_RCX && defined(OS_FREERTOS))*/
