/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup Real Time Clock (RTC)
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file hw_rtc.c
 *
 * @brief Implementation of the Real Time Clock Low Level Driver.
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#if dg_configUSE_HW_RTC

#include <stdio.h>
#include <sdk_defs.h>
#include <hw_rtc.h>

#if (dg_configSYSTEMVIEW == 1)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif /* (dg_configSYSTEMVIEW == 1) */

static hw_rtc_interrupt_cb rtc_interrupt_cb;

/**
 * \brief Converts time from decimal to binary-coded decimal (BCD)
 *
 * \param[in] time pointer to RTC time in decimal number format
 *
 * \return time in BCD format
 */
static uint32_t time_to_bcd(const hw_rtc_time_t *time)
{
        uint32_t time_bcd;

        time_bcd = ((time->hsec % 10) << 0);
        time_bcd += ((time->hsec / 10) << 4);

        time_bcd += ((time->sec % 10) << 8);
        time_bcd += ((time->sec / 10) << 12);

        time_bcd += ((time->minute % 10) << 16);
        time_bcd += ((time->minute / 10) << 20);

        if (time->hour_mode == RTC_24H_CLK) {
                hw_rtc_set_hour_clk_mode(RTC_24H_CLK); //24hr mode
                time_bcd += ((time->hour % 10) << 24);
                time_bcd += (( time->hour / 10) << 28);
        } else if (time->hour_mode == RTC_12H_CLK) {
                hw_rtc_set_hour_clk_mode(RTC_12H_CLK); //12hr mode
                time_bcd += ((time->hour % 10 ) << 24);
                time_bcd += ((time->hour / 10) << 28);
                time_bcd += ((time->pm_flag) << 30);
        } else {
                ASSERT_WARNING(0);//Invalid argument
        }

        return time_bcd;
}

/**
 * \brief Converts alarm time from decimal to binary-coded decimal (BCD)
 *
 * \param[in] time pointer to alarm time in decimal number format
 *
 * \return time in BCD format
 */
static uint32_t alarm_time_to_bcd(const hw_rtc_time_t *time)
{
        uint32_t time_bcd;

        time_bcd = ((time->hsec % 10) << 0);
        time_bcd += ((time->hsec / 10) << 4);

        time_bcd += ((time->sec % 10) << 8);
        time_bcd += ((time->sec / 10) << 12);

        time_bcd += ((time->minute % 10) << 16);
        time_bcd += ((time->minute / 10) << 20);

        if (hw_rtc_get_hour_clk_mode() == RTC_24H_CLK) {
                time_bcd += ((time->hour % 10) << 24);
                time_bcd += (( time->hour / 10) << 28);
        } else if (hw_rtc_get_hour_clk_mode() == RTC_12H_CLK) {
                time_bcd += ((time->hour % 10 ) << 24);
                time_bcd += ((time->hour / 10) << 28);
                time_bcd += ((time->pm_flag) << 30);
        } else {
                ASSERT_WARNING(0);//Invalid argument
        }

        return time_bcd;
}

/**
 * \brief Converts Calendar date from decimal to binary-coded decimal (BCD)
 *
 * \param[in] clndr pointer to RTC calendar in decimal number format
 *
 * \return calendar date in BCD format
 */
static uint32_t calendar_to_bcd(const hw_rtc_calendar_t *clndr)
{
        uint32_t clndr_bcd;

        clndr_bcd = clndr->wday & 0x7;

        if (clndr->month > 9) {
                clndr_bcd += (0x80 + ((clndr->month-10) << 3));
        } else {
                clndr_bcd += (clndr->month << 3);
        }

        clndr_bcd += ((clndr->mday % 10) << 8);
        clndr_bcd += ((clndr->mday / 10) << 12);

        clndr_bcd += (((clndr->year % 100) % 10) << 16);
        clndr_bcd += (((clndr->year % 100) / 10) << 20);

        clndr_bcd += (((clndr->year/100) % 10) << 24);
        clndr_bcd += (((clndr->year/100) / 10) << 28);

        return clndr_bcd;
}

/**
 * \brief Converts alarm Calendar date from decimal to binary-coded decimal (BCD)
 *
 * \param[in] clndr pointer to RTC alarm calendar in decimal number format
 *
 * \return alarm calendar date in BCD format
 */
static uint32_t alarm_calendar_to_bcd(const hw_rtc_alarm_calendar_t *clndr)
{
        uint32_t clndr_bcd;

        if (clndr->month > 9) {
                clndr_bcd = (0x80 + ((clndr->month-10) << 3));
        } else {
                clndr_bcd = (clndr->month << 3);
        }

        clndr_bcd += ((clndr->mday % 10) << 8);
        clndr_bcd += ((clndr->mday / 10) << 12);

        return clndr_bcd;
}

/**
 * \brief Converts RTC time from binary-coded decimal (BCD) to decimal
 *
 * \param[in] time_bcd  time in BCD
 * \param[out] time pointer to RTC time in decimal number format
 *
 */
static void bdc_to_time(uint32_t time_bcd, hw_rtc_time_t *time)
{
        time->pm_flag = (time_bcd & RTC_RTC_TIME_REG_RTC_TIME_PM_Msk) >> 30;
        time->hour = (((time_bcd & RTC_RTC_TIME_REG_RTC_TIME_HR_T_Msk) >> 28) * 10) +  ((time_bcd & RTC_RTC_TIME_REG_RTC_TIME_HR_U_Msk) >> 24);
        time->minute = (((time_bcd & RTC_RTC_TIME_REG_RTC_TIME_M_T_Msk) >> 20) * 10) +  ((time_bcd & RTC_RTC_TIME_REG_RTC_TIME_M_U_Msk) >> 16);
        time->sec = (((time_bcd & RTC_RTC_TIME_REG_RTC_TIME_S_T_Msk) >> 12) * 10) +  ((time_bcd & RTC_RTC_TIME_REG_RTC_TIME_S_U_Msk) >> 8);
        time->hsec = (((time_bcd & RTC_RTC_TIME_REG_RTC_TIME_H_T_Msk) >> 4) * 10) +  (time_bcd & RTC_RTC_TIME_REG_RTC_TIME_H_U_Msk);
}

/**
 * \brief Converts Calendar date from binary-coded decimal (BCD) to decimal
 *
 * \param[in] date_bcd  calendar date in BCD
 * \param[out] clndr pointer to RTC calendar in decimal number format
 *
 */
static void bdc_to_clndr(uint32_t date_bcd, hw_rtc_calendar_t *clndr)
{
        clndr->year = (((date_bcd & RTC_RTC_CALENDAR_REG_RTC_CAL_C_T_Msk) >> 28) * 1000) +
                      (((date_bcd & RTC_RTC_CALENDAR_REG_RTC_CAL_C_U_Msk) >> 24) * 100) +
                      (((date_bcd & RTC_RTC_CALENDAR_REG_RTC_CAL_Y_T_Msk) >> 20) * 10) +
                      ((date_bcd & RTC_RTC_CALENDAR_REG_RTC_CAL_Y_U_Msk) >> 16);
        clndr->mday = (((date_bcd & RTC_RTC_CALENDAR_REG_RTC_CAL_D_T_Msk) >> 12) * 10) +  ((date_bcd & RTC_RTC_CALENDAR_REG_RTC_CAL_D_U_Msk) >> 8);
        clndr->month = (((date_bcd & RTC_RTC_CALENDAR_REG_RTC_CAL_M_T_Msk) >> 7) * 10) +  ((date_bcd & RTC_RTC_CALENDAR_REG_RTC_CAL_M_U_Msk) >> 3);
        clndr->wday = date_bcd & RTC_RTC_CALENDAR_REG_RTC_DAY_Msk;
}

void hw_rtc_config_RTC_to_PDC_evt(const hw_rtc_config_pdc_evt_t *cfg)
{
        hw_rtc_pdc_event_disable();
        if (cfg->pdc_evt_en) {
                hw_rtc_set_pdc_event_period(cfg->pdc_evt_period);
                hw_rtc_pdc_event_enable();
        }
}

void hw_rtc_config_RTC_to_Motor_evt(const hw_rtc_config_motor_evt_t *cfg)
{
        hw_rtc_motor_event_disable();
        if (cfg->motor_evt_en) {
                hw_rtc_set_motor_event_period(cfg->motor_evt_period);
                hw_rtc_motor_event_enable();
        }
}

void hw_rtc_register_intr(hw_rtc_interrupt_cb handler, uint8_t mask)
{
        rtc_interrupt_cb = handler;
        hw_rtc_interrupt_enable(mask);
        NVIC_EnableIRQ(RTC_IRQn);
}

void hw_rtc_unregister_intr(void)
{
        rtc_interrupt_cb = NULL;
        hw_rtc_interrupt_disable(0xff);
        NVIC_ClearPendingIRQ(RTC_IRQn);
        NVIC_DisableIRQ(RTC_IRQn);
}

#if (dg_configRTC_CORRECTION == 1)

static __RETAINED void (*rtc_callback)(const hw_rtc_time_t *time);

void hw_rtc_register_cb(void (*cb)(const hw_rtc_time_t *time))
{
        rtc_callback = cb;
}

void hw_rtc_unregister_cb(void)
{
        rtc_callback = NULL;
}
#endif

HW_RTC_SET_REG_STATUS hw_rtc_set_time_clndr(const hw_rtc_time_t *time, const hw_rtc_calendar_t *clndr)
{
        uint8_t status;
        // stores the current RTC time. If the new time value causes an entry error, this time value will be re-written
        uint32_t time_cur_val;
        // stores the current RTC calendar. If the new calendar value causes an entry error, this time will be re-written
        uint32_t clndr_cur_val;
        uint8_t ret = HW_RTC_VALID_ENTRY;

        GLOBAL_INT_DISABLE();
        if ((time != NULL) && (clndr != NULL)) {
                // set both time and calendar. Stop and start counters at the same time
                hw_rtc_stop();
                time_cur_val = hw_rtc_get_time_bcd();
                clndr_cur_val = hw_rtc_get_clndr_bcd();
                hw_rtc_set_time_bcd(time_to_bcd(time));
                hw_rtc_set_clndr_bcd(calendar_to_bcd(clndr));
                status = hw_rtc_get_status();
                if ((status & (HW_RTC_VALID_TIME | HW_RTC_VALID_CLNDR)) == 0x0) {
                        hw_rtc_set_clndr_bcd(clndr_cur_val);
                        hw_rtc_set_time_bcd(time_cur_val);
                        ret = HW_RTC_INVALID_TIME_CLNDR;
                } else if ((status & HW_RTC_VALID_TIME) != HW_RTC_VALID_TIME) {
                        hw_rtc_set_time_bcd(time_cur_val);
                        ret = HW_RTC_INVALID_TIME;
                } else if ((status & HW_RTC_VALID_CLNDR) != HW_RTC_VALID_CLNDR) {
                        hw_rtc_set_clndr_bcd(clndr_cur_val);
                        ret = HW_RTC_INVALID_CLNDR;
                }
#if (dg_configRTC_CORRECTION == 1)
                if (rtc_callback) {
                        hw_rtc_time_t time_cur;
                        bdc_to_time(time_cur_val, &time_cur);
                        ((ret == HW_RTC_INVALID_TIME_CLNDR) || (ret == HW_RTC_INVALID_TIME)) ? rtc_callback(&time_cur) : rtc_callback(time);
                }
#endif
                hw_rtc_start();

        } else if (time != NULL) {
                hw_rtc_time_stop();
                time_cur_val = hw_rtc_get_time_bcd();
                hw_rtc_set_time_bcd(time_to_bcd(time));
                status = hw_rtc_get_status();
                if ((status & HW_RTC_VALID_TIME) != HW_RTC_VALID_TIME) {
                        hw_rtc_set_time_bcd(time_cur_val);
                        ret = HW_RTC_INVALID_TIME;
                }
#if (dg_configRTC_CORRECTION == 1)
                if (rtc_callback) {
                        hw_rtc_time_t time_cur;
                        bdc_to_time(time_cur_val, &time_cur);
                        (ret == HW_RTC_INVALID_TIME) ? rtc_callback(&time_cur) : rtc_callback(time);
                }
#endif
                hw_rtc_time_start();
        } else if (clndr != NULL) {
                hw_rtc_clndr_stop();
                clndr_cur_val = hw_rtc_get_clndr_bcd();
                hw_rtc_set_clndr_bcd(calendar_to_bcd(clndr));
                status = hw_rtc_get_status();
                if ((status & HW_RTC_VALID_CLNDR) != HW_RTC_VALID_CLNDR) {
                        hw_rtc_set_clndr_bcd(clndr_cur_val);
                        ret = HW_RTC_INVALID_CLNDR;
                }
                hw_rtc_clndr_start();
        }
        GLOBAL_INT_RESTORE();
        return ret;
}

void hw_rtc_get_time_clndr(hw_rtc_time_t *time, hw_rtc_calendar_t *clndr)
{
        GLOBAL_INT_DISABLE();
        if ((time != NULL) && (clndr != NULL)) {
                uint32_t val_bcd;

                // In order to obtain a coherent view of time and date, the counters must be stopped
                // while reading them. This avoids the situation where the date counter increments
                // between reading the time register and reading the calendar register.
                hw_rtc_stop();
                val_bcd = hw_rtc_get_time_bcd();
                bdc_to_time(val_bcd, time);
                time->hour_mode = hw_rtc_get_hour_clk_mode();
                val_bcd = hw_rtc_get_clndr_bcd();
                bdc_to_clndr(val_bcd, clndr);
                hw_rtc_start();
        } else if (time != NULL) {
                uint32_t time_bcd;

                time_bcd = hw_rtc_get_time_bcd();
                bdc_to_time(time_bcd, time);
                time->hour_mode = hw_rtc_get_hour_clk_mode();
        } else if (clndr != NULL) {
                uint32_t date_bcd;

                date_bcd = hw_rtc_get_clndr_bcd();
                bdc_to_clndr(date_bcd, clndr);
        }
        GLOBAL_INT_RESTORE();
}

HW_RTC_SET_REG_STATUS hw_rtc_set_alarm(const hw_rtc_time_t *time, const hw_rtc_alarm_calendar_t *clndr, const uint8_t mask)
{
        uint8_t status;
        uint32_t current_val;
        uint8_t tmp_msk;


        tmp_msk = hw_rtc_get_alarm_enable_msk();  // Get a copy of the alarm enable register to restore the value in case of invalid entry

        hw_rtc_interrupt_disable(HW_RTC_INT_ALRM);
        hw_rtc_alarm_enable(0x0); // disable alarm events



        if (time != NULL) {
                if (time->hour_mode != hw_rtc_get_hour_clk_mode()) {
                        hw_rtc_alarm_enable(tmp_msk);
                        hw_rtc_interrupt_enable(HW_RTC_INT_ALRM);
                        return HW_RTC_INVALID_TIME_HOUR_MODE_ALM; // Do not allow alarm with different hour clock mode from time
                }
                current_val = hw_rtc_get_alarm_time_bcd();
                hw_rtc_set_alarm_time_bcd(alarm_time_to_bcd(time));
                status = hw_rtc_get_status();
                if ((status & HW_RTC_VALID_TIME_ALM) != HW_RTC_VALID_TIME_ALM) {
                        hw_rtc_set_alarm_time_bcd(current_val);
                        hw_rtc_alarm_enable(tmp_msk);
                        hw_rtc_interrupt_enable(HW_RTC_INT_ALRM);
                        return HW_RTC_INVALID_TIME_ALM;
                }
        }

        if (clndr != NULL) {
                current_val = hw_rtc_get_alarm_clndr_bcd();
                hw_rtc_set_alarm_clndr_bcd(alarm_calendar_to_bcd(clndr));
                status = hw_rtc_get_status();
                if ((status & HW_RTC_VALID_CLNDR_ALM) != HW_RTC_VALID_CLNDR_ALM) {
                        hw_rtc_set_alarm_clndr_bcd(current_val);
                        hw_rtc_alarm_enable(tmp_msk);
                        hw_rtc_interrupt_enable(HW_RTC_INT_ALRM);
                        return HW_RTC_INVALID_CLNDR_ALM;
                }
        }

        hw_rtc_alarm_enable(mask);
        hw_rtc_interrupt_enable(HW_RTC_INT_ALRM);

        return HW_RTC_VALID_ENTRY;
}

void hw_rtc_get_alarm(hw_rtc_time_t *time, hw_rtc_alarm_calendar_t *clndr, uint8_t *mask)
{
        if (time != NULL) {
                uint32_t time_bcd;

                time_bcd = hw_rtc_get_alarm_time_bcd();

                time->pm_flag = (time_bcd & RTC_RTC_TIME_ALARM_REG_RTC_TIME_PM_Msk) >> 30;
                time->hour = (((time_bcd & RTC_RTC_TIME_ALARM_REG_RTC_TIME_HR_T_Msk) >> 28) * 10) +  ((time_bcd & RTC_RTC_TIME_ALARM_REG_RTC_TIME_HR_U_Msk) >> 24);
                time->minute = (((time_bcd & RTC_RTC_TIME_ALARM_REG_RTC_TIME_M_T_Msk) >> 20) * 10) +  ((time_bcd & RTC_RTC_TIME_ALARM_REG_RTC_TIME_M_U_Msk) >> 16);
                time->sec = (((time_bcd & RTC_RTC_TIME_ALARM_REG_RTC_TIME_S_T_Msk) >> 12) * 10) +  ((time_bcd & RTC_RTC_TIME_ALARM_REG_RTC_TIME_S_U_Msk) >> 8);
                time->hsec = (((time_bcd & RTC_RTC_TIME_ALARM_REG_RTC_TIME_H_T_Msk) >> 4) * 10) +  (time_bcd & RTC_RTC_TIME_ALARM_REG_RTC_TIME_H_U_Msk);
                time->hour_mode = hw_rtc_get_hour_clk_mode();
        }

        if (clndr != NULL) {
                uint32_t date_bcd;

                date_bcd = hw_rtc_get_alarm_clndr_bcd();

                clndr->mday = (((date_bcd & RTC_RTC_CALENDAR_ALARM_REG_RTC_CAL_D_T_Msk) >> 12) * 10) +  ((date_bcd & RTC_RTC_CALENDAR_ALARM_REG_RTC_CAL_D_U_Msk) >> 8);
                clndr->month = (((date_bcd & RTC_RTC_CALENDAR_ALARM_REG_RTC_CAL_M_T_Msk) >> 7) * 10) +  ((date_bcd & RTC_RTC_CALENDAR_ALARM_REG_RTC_CAL_M_U_Msk) >> 3);
        }

        if (mask != NULL) {
                *mask = hw_rtc_get_alarm_enable_msk();
        }

}

void RTC_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        if (rtc_interrupt_cb != NULL) {
                uint8_t event;
                event = hw_rtc_get_event_flags();
                rtc_interrupt_cb(event);
        }

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

#endif /* dg_configUSE_HW_RTC */
/**
 * \}
 * \}
 * \}
 */
