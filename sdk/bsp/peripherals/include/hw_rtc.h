/**
 * \addtogroup PLA_DRI_PER_TIMERS
 * \{
 * \addtogroup HW_RTC Real_Time_Clock (RTC) Driver
 * \{
 * \brief Real Time Clock (RTC)
 */

/**
 *****************************************************************************************
 *
 * @file hw_rtc.h
 *
 * @brief Definition of API for the Real Time Clock Low Level Driver.
 *
 * Copyright (C) 2017-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *****************************************************************************************
 */

#ifndef HW_RTC_H_
#define HW_RTC_H_


#if dg_configUSE_HW_RTC

#include <stdbool.h>
#include <stdint.h>
#include <sdk_defs.h>

/**
 * \brief Get the value of a field of an RTC register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to read
 *
 * \return the value of the register field
 *
 */
#define HW_RTC_REG_GETF(reg, field) \
        ((RTC->reg & (RTC_##reg##_##field##_Msk)) >> (RTC_##reg##_##field##_Pos))

/**
 * \brief Set the value of a field of a RTC register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to write
 * \param [in] val is the value to write
 *
 */
#define HW_RTC_REG_SETF(reg, field, val) \
        RTC->reg = ((RTC->reg & ~(RTC_##reg##_##field##_Msk)) | \
        ((RTC_##reg##_##field##_Msk) & ((val) << (RTC_##reg##_##field##_Pos))))

/**
 * \brief Maximum value of RTC to Motor event period (12bits).
 *
 */
#define RTC_MOTOR_EVENT_PERIOD_MAX_VAL   0xfff

/**
 * \brief Maximum value of RTC to PDC event period (13bits).
 *
 */
#define RTC_PDC_EVENT_PERIOD_MAX_VAL   0x1fff

/**
 * \brief All entries are valid in the RTC status register
 *
 */
#define RTC_ALL_STATUS_VALID   0xf

/**
 * \brief Time and Calendar entries are valid in the RTC status register
 *
 */
#define RTC_TIME_CLDR_STATUS_VALID   0x3

/**
 * \brief Alarm Time and Calendar entries are valid in the RTC status register
 *
 */
#define RTC_ALARM_TIME_CLDR_STATUS_VALID   0xc

/**
 * \brief RTC lp_clk sources
 *
 */
typedef enum {
        RTC_LP_CLK_RC32K,
        RTC_LP_CLK_XTAL32K,
        RTC_LP_CLK_RCX,
} HW_RTC_LP_CLK;

/**
 * \brief Hour clock mode
 *
 */
typedef enum {
        RTC_24H_CLK,    /**< 24 hour clock format */
        RTC_12H_CLK,    /**< 12 hour clock format */
} HW_RTC_HOUR_MODE;

/**
 * \brief RTC denominator for the fractional division of the source clock
 *
 */
typedef enum {
        RTC_DIV_DENOM_1000 = 0x0,    /**< denominator for the fractional division is 1000 */
        RTC_DIV_DENOM_1024 = 0x1,    /**< denominator for the fractional division is 1024 */
}HW_RTC_DIV_DENOM;

/**
 * \brief RTC interrupt source
 *
 * Can be used as bitmask.
 *
 * \sa hw_rtc_interrupt_enable
 * \sa hw_rtc_interrupt_disable
 * \sa hw_rtc_get_interrupt_mask
 */
typedef enum {

        HW_RTC_INT_ALRM = RTC_RTC_INTERRUPT_MASK_REG_RTC_ALRM_INT_MSK_Msk,     /**< interrupt when alarm event occurs */
        HW_RTC_INT_MONTH = RTC_RTC_INTERRUPT_MASK_REG_RTC_MNTH_INT_MSK_Msk,    /**< interrupt when month event occurs */
        HW_RTC_INT_MDAY = RTC_RTC_INTERRUPT_MASK_REG_RTC_DATE_INT_MSK_Msk,     /**< interrupt when day of the month event occurs */
        HW_RTC_INT_HOUR = RTC_RTC_INTERRUPT_MASK_REG_RTC_HOUR_INT_MSK_Msk,     /**< interrupt when hour event occurs */
        HW_RTC_INT_MIN = RTC_RTC_INTERRUPT_MASK_REG_RTC_MIN_INT_MSK_Msk,       /**< interrupt when minute event occurs */
        HW_RTC_INT_SEC = RTC_RTC_INTERRUPT_MASK_REG_RTC_SEC_INT_MSK_Msk,       /**< interrupt when seconds event occurs */
        HW_RTC_INT_HSEC = RTC_RTC_INTERRUPT_MASK_REG_RTC_HOS_INT_MSK_Msk,      /**< interrupt when hundredth of a second event occurs */
} HW_RTC_INTR;

/**
 * \brief RTC Events
 *
 * Can be used as bitmask.
 *
 * \sa hw_rtc_get_event_flags
 *
 */
typedef enum {
        HW_RTC_EVENT_ALRM = RTC_RTC_EVENT_FLAGS_REG_RTC_EVENT_ALRM_Msk,     /**< alarm event  */
        HW_RTC_EVENT_MONTH = RTC_RTC_EVENT_FLAGS_REG_RTC_EVENT_MNTH_Msk,    /**< month rolls over event  */
        HW_RTC_EVENT_MDAY = RTC_RTC_EVENT_FLAGS_REG_RTC_EVENT_DATE_Msk,     /**< day of the month rolls over event */
        HW_RTC_EVENT_HOUR = RTC_RTC_EVENT_FLAGS_REG_RTC_EVENT_HOUR_Msk,     /**< hour rolls over event */
        HW_RTC_EVENT_MIN = RTC_RTC_EVENT_FLAGS_REG_RTC_EVENT_MIN_Msk,       /**< minute rolls over event */
        HW_RTC_EVENT_SEC = RTC_RTC_EVENT_FLAGS_REG_RTC_EVENT_SEC_Msk,       /**< second rolls over event  */
        HW_RTC_EVENT_HSEC = RTC_RTC_EVENT_FLAGS_REG_RTC_EVENT_HOS_Msk,      /**< hundredths of a second rolls over event */
} HW_RTC_EVENT;


/**
 * \brief RTC Alarm enable mask
 *
 * Can be used as bitmask.
 *
 * \sa hw_rtc_get_alarm_enable_msk
 * \sa hw_rtc_set_alarm
 * \sa hw_rtc_get_alarm
 *
 */
typedef enum {
        HW_RTC_ALARM_MONTH = RTC_RTC_ALARM_ENABLE_REG_RTC_ALARM_MNTH_EN_Msk,    /**< Enable to trigger alarm when the month in Calendar Alarm Register has been reached */
        HW_RTC_ALARM_MDAY = RTC_RTC_ALARM_ENABLE_REG_RTC_ALARM_DATE_EN_Msk,     /**< Enable to trigger alarm when the month day in Calendar Alarm Register has been reached */
        HW_RTC_ALARM_HOUR = RTC_RTC_ALARM_ENABLE_REG_RTC_ALARM_HOUR_EN_Msk,     /**< Enable to trigger alarm when the hour in Time Alarm Register has been reached */
        HW_RTC_ALARM_MIN = RTC_RTC_ALARM_ENABLE_REG_RTC_ALARM_MIN_EN_Msk,       /**< Enable to trigger alarm when the minute in Time Alarm Register has been reached*/
        HW_RTC_ALARM_SEC = RTC_RTC_ALARM_ENABLE_REG_RTC_ALARM_SEC_EN_Msk,       /**< Enable to trigger alarm when the sec in Time Alarm Register has been reached */
        HW_RTC_ALARM_HSEC = RTC_RTC_ALARM_ENABLE_REG_RTC_ALARM_HOS_EN_Msk,      /**< Enable to trigger alarm when the hsec in Time Alarm Register has been reached */
} HW_RTC_ALARM_EN;

/**
 * \brief RTC status
 *
 * Can be used as bitmask.
 *
 * \sa hw_rtc_get_status
 *
 */
typedef enum {
        HW_RTC_VALID_CLNDR_ALM = RTC_RTC_STATUS_REG_RTC_VALID_CAL_ALM_Msk,     /**< Valid Calendar Alarm. If cleared then indicates that invalid entry occurred when writing to Calendar Alarm Register */
        HW_RTC_VALID_TIME_ALM = RTC_RTC_STATUS_REG_RTC_VALID_TIME_ALM_Msk,    /**< Valid Time Alarm. If cleared then indicates that invalid entry occurred when writing to Time Alarm Register */
        HW_RTC_VALID_CLNDR = RTC_RTC_STATUS_REG_RTC_VALID_CAL_Msk,     /**< Valid Calendar. If cleared then indicates that invalid entry occurred when writing to Calendar Register */
        HW_RTC_VALID_TIME = RTC_RTC_STATUS_REG_RTC_VALID_TIME_Msk,     /**< Valid Time. If cleared then indicates that invalid entry occurred when writing to Time Register */
} HW_RTC_STATUS;

/**
 * \brief RTC status
 *
 * Can be used as bitmask.
 *
 * \sa hw_rtc_set_time_clndr
 * \sa hw_rtc_set_alarm
 *
 */
typedef enum {
        HW_RTC_VALID_ENTRY = 0x1,        /**< indicates a valid entry */
        HW_RTC_INVALID_TIME_HOUR_MODE_ALM = 0x3,  /**< indicates that invalid entry occurred when writing to Time Alarm Register hour clock mode.
                                                       For example, when time is in 24h mode, alarm can not be set in 12h mode. Note that this
                                                       is not flagged in the status register */
        HW_RTC_INVALID_CLNDR_ALM = 0x7,  /**< indicates that invalid entry occurred when writing to Calendar Alarm Register */
        HW_RTC_INVALID_TIME_ALM = 0xB,   /**< indicates that invalid entry occurred when writing to Time Alarm Register */
        HW_RTC_INVALID_CLNDR = 0xD,      /**< indicates that invalid entry occurred when writing to Calendar Register */
        HW_RTC_INVALID_TIME = 0xE,       /**< indicates that invalid entry occurred when writing to Time Register */
        HW_RTC_INVALID_TIME_CLNDR = 0xC, /**< indicates that invalid entry occurred to both time and calendar registers */
} HW_RTC_SET_REG_STATUS;

/**
 * \brief Callback that is fired on RTC events
 *
 * \param [in] event bitmask of HW_RTC_EVENT
 *
 */
typedef void (*hw_rtc_interrupt_cb)(uint8_t event);

DEPRECATED_MSG("API no longer supported, use hw_rtc_calendar_t instead.")
typedef struct {
} rtc_calendar;

DEPRECATED_MSG("API no longer supported, use hw_rtc_alarm_calendar_t instead.")
typedef struct {
} rtc_alarm_calendar;

DEPRECATED_MSG("API no longer supported, use hw_rtc_time_t instead.")
typedef struct {
} rtc_time;

DEPRECATED_MSG("API no longer supported, use hw_rtc_config_pdc_evt_t instead.")
typedef struct {
} rtc_config_pdc_evt;

DEPRECATED_MSG("API no longer supported, use hw_rtc_config_motor_evt_t instead.")
typedef struct {
} rtc_config_motor_evt;

/**
 * \brief Calendar configuration for RTC
 *
 */
typedef struct {
        uint16_t year;   /**< Year from 1900 to 2999 */
        uint8_t month;   /**< Month from 1 to 12 */
        uint8_t mday;    /**< Day of month from 1 to 28, 29, 30 or 31 (as a function of month and year) */
        uint8_t wday;    /**< Day of week from 1 to 7 (mapping is programmable) */
} hw_rtc_calendar_t;

/**
 * \brief Alarm Calendar configuration for RTC
 *
 */
typedef struct {
        uint8_t month;   /**< Month from 1 to 12 */
        uint8_t mday;    /**< Day of month from 1 to 28, 29, 30 or 31 (as a function of month and year) */
} hw_rtc_alarm_calendar_t;

/**
 * \brief Time configuration for RTC
 *
 */
typedef struct {
        uint8_t hour_mode;     /**< Hour mode: "0" for 24-hour clock or "1" for 12-hour clock */
        bool pm_flag;          /**< In 12 hour clock mode, indicates PM when set to "1" */
        uint8_t hour;          /**< Hour from 0 to 23 in 24-hour mode, or from 1 to 12 in 12-hour mode */
        uint8_t minute;        /**< Minutes after the hour, in the range 0 to 59 */
        uint8_t sec;           /**< Seconds after the minute, in the range 0 to 59 */
        uint8_t hsec;          /**< Hundredth of the second, in the range 0 to 99 */
} hw_rtc_time_t;

/**
 * \brief RTC to Power Domain Controller event configuration
 *
 */
typedef struct {
        bool      pdc_evt_en;      /**< 0 = Event to PDC is disabled. 1 = Even to PDC is enabled */
        uint16_t  pdc_evt_period;  /**< If event enabled, send an event to PDC every (pdc_event_period+1)*10ms */
} hw_rtc_config_pdc_evt_t;

/**
 * \brief RTC to Motor event configuration
 *
 */
typedef struct {

        bool      motor_evt_en;     /**< 0 = Event to Motor is disabled. 1 = Event to Motor is enabled */
        uint16_t  motor_evt_period; /**< If event enabled, send an event to motor every (pdc_motor_period+1)*10ms */
} hw_rtc_config_motor_evt_t;

/**
 * \brief RTC to PDC event configuration
 *
 * Configures and enables the RTC-to-PDC event.
 *
 * \note After configuration RTC is not running. Start RTC by calling hw_rtc_start().
 * \note When RF calibration is enabled and/or RCX is used as low power clock, this function must not be called.
 * In case application needs to use RTC event controller user shall define dg_configRTC_PDC_EVENT_PERIOD (< 1sec).
 *
 * \param [in] cfg configuration of the RTC event
 *
 */
void hw_rtc_config_RTC_to_PDC_evt(const hw_rtc_config_pdc_evt_t *cfg);

/**
 * \brief RTC to Motor Controller event configuration
 *
 * Configures and enables the RTC-to-Motor event.
 *
 * \note After configuration RTC is not running. Start RTC by calling hw_rtc_start().
 *
 * \param [in] cfg configuration of the RTC event
 *
 */
void hw_rtc_config_RTC_to_Motor_evt(const hw_rtc_config_motor_evt_t *cfg);

/**
 * \brief Register an interrupt handler
 *
 * \param [in] handler callback function to be called when RTC event occurs
 * \param [in] mask initial bitmask of requested interrupt events
 *
 * \sa hw_rtc_interrupt_enable
 * \sa hw_rtc_interrupt_disable
 *
 */
void hw_rtc_register_intr(hw_rtc_interrupt_cb handler, uint8_t mask);


/**
 * \brief Unregister the event handler and disable RTC interrupt (NVIC)
 *
 * This function disables all RTC interrupts by masking them. In addition
 * it clears any pending ones on the ARM core. The status of RAW_INTR_STAT_REG
 * remains unchanged.
 *
 * \sa hw_rtc_interrupt_disable
 * \sa hw_rtc_interrupt_enable
 *
 */
void hw_rtc_unregister_intr(void);

/**
 * \brief Set RTC time and/or calendar date
 *
 * \p time can be NULL if only calendar is set. Also, \p clndr can be NULL if only time is set.
 *
 * \param [in] time value to set the RTC time
 * \param [in] clndr value to set the RTC calendar date
 *
 * \return HW_RTC_VALID_ENTRY on success, otherwise the failure cause
 *
 * \note This function will stop the respective counter before setting the register
 *       and then it will start the counter again only if the entry was valid
 *
 */
HW_RTC_SET_REG_STATUS hw_rtc_set_time_clndr(const hw_rtc_time_t *time, const hw_rtc_calendar_t *clndr);

/**
 * \brief Set RTC time
 *
 * \param[in] time_bcd time in binary-coded decimal (BCD) format
 *
 * \note  Use wrapper function hw_rtc_set_time_clndr() to set time in easy to use decimal number format.
 *        If this function is used, then hw_rtc_get_status() should be called to check the status
 *
 * \sa hw_rtc_get_status
 *
 */
__STATIC_INLINE void hw_rtc_set_time_bcd(uint32_t time_bcd)
{
        RTC->RTC_TIME_REG = time_bcd;
}

/**
 * \brief Set RTC Calendar date
 *
 * \param[in] clndr_bcd Calendar date in binary-coded decimal (BCD) format
 *
 * \note  Use wrapper function hw_rtc_set_time_clndr() to set calendar in easy to use decimal number format.
 *        If this function is used, then hw_rtc_get_status() should be called to check the status
 *
 * \sa hw_rtc_get_status
 */
__STATIC_INLINE void hw_rtc_set_clndr_bcd(uint32_t clndr_bcd)
{
        RTC->RTC_CALENDAR_REG = clndr_bcd;
}

/**
 * \brief Get RTC time and/or calendar date
 *
 * \param [out] time returns the RTC time. Set NULL if there is no need to read time
 * \param [out] clndr returns the RTC calendar. Set NULL if there is no need to read calendar date
 *
 * \warning  When reading simultaneously time and date, in order to obtain a coherent view of time and date,
 *           the counters shall be stopped while reading them. This avoids the situation where the date
 *           counter increments between reading the time register and reading the date register.
 *           To avoid stopping the counters temporarily, call this function with \p clndr to NULL, to read time first
 *           and then call this function with \p time to NULL, to read the date.
 */
void hw_rtc_get_time_clndr(hw_rtc_time_t *time, hw_rtc_calendar_t *clndr);

/**
 * \brief Get RTC time
 *
 * \return time in binary-coded decimal (BCD) format
 *
 */
__STATIC_INLINE uint32_t hw_rtc_get_time_bcd(void)
{
        return RTC->RTC_TIME_REG;
}

/**
 * \brief Get RTC Calendar date
 *
 * \return Calendar date in binary-coded decimal (BCD) format
 *
 */
__STATIC_INLINE uint32_t hw_rtc_get_clndr_bcd(void)
{
        return RTC->RTC_CALENDAR_REG;
}

/**
 * \brief Start RTC time
 *
 */
__STATIC_INLINE void hw_rtc_time_start(void)
{
        HW_RTC_REG_SETF(RTC_CONTROL_REG, RTC_TIME_DISABLE, 0);
}

/**
 * \brief Stop RTC time
 *
 */
__STATIC_INLINE void hw_rtc_time_stop(void)
{
        HW_RTC_REG_SETF(RTC_CONTROL_REG, RTC_TIME_DISABLE, 1);
}

/**
 * \brief Start RTC Calendar
 *
 */
__STATIC_INLINE void hw_rtc_clndr_start(void)
{
        HW_RTC_REG_SETF(RTC_CONTROL_REG, RTC_CAL_DISABLE, 0);
}

/**
 * \brief Stop RTC Calendar
 *
 */
__STATIC_INLINE void hw_rtc_clndr_stop(void)
{
        HW_RTC_REG_SETF(RTC_CONTROL_REG, RTC_CAL_DISABLE, 1);
}

/**
 * \brief Start RTC. Starts both time and calendar
 *
 * \warning RTC is using the low power clock (lp_clk) as clock source, therefore
 *          firstly lp_clk must be configured and enabled and secondly the RTC clock 100Hz
 *          must be configured and enabled before calling this function.
 *
 * \sa hw_rtc_clk_config
 * \sa hw_rtc_clock_enable
 */

__STATIC_INLINE void hw_rtc_start(void)
{
        RTC->RTC_CONTROL_REG = 0x0;
}

/**
 * \brief Stop RTC. Stops both time and calendar
 *
 */
__STATIC_INLINE void hw_rtc_stop(void)
{
        RTC->RTC_CONTROL_REG = 0x3;
}

/**
 * \brief Set RTC alarm
 *
 * Set time and/or calendar alarms. Enable bitmask \p mask to select the alarms needed.
 * The alarm interrupt is enabled automatically, an interrupt will be generated when an alarm event occurs.
 *
 * \param [in] time time alarm. Can be NULL if only calendar alarm is set
 * \param [in] clndr calendar alarm. Can be NULL if only time alarm is set
 * \param [in] mask bitmask of HW_RTC_ALARM_EN. Set "1" to enable:
 * \parblock
 *         Bit:           |    5    |     4   |    3    |    2   |    1   |     0    |
 *                        +---------+---------+---------+--------+--------+----------+
 *         Alarm enable:  | on month| on mday | on hour | on min | on sec | on hsec  |
 *                        +---------+---------+---------+--------+--------+----------+
 * \endparblock
 *
 * \return HW_RTC_VALID_ENTRY on success, otherwise the failure cause
 *
 * \sa hw_rtc_interrupt_enable
 * \sa hw_rtc_alarm_enable
 *
 */
HW_RTC_SET_REG_STATUS hw_rtc_set_alarm(const hw_rtc_time_t *time, const hw_rtc_alarm_calendar_t *clndr, const uint8_t mask);

/**
 * \brief Set RTC alarm time
 *
 * \param[in] time_bcd time in binary-coded decimal (BCD) format
 *
 * \note  Use wrapper function hw_rtc_set_alarm() to set the alarm in easy to use decimal number format.
 *        If this function is used, then hw_rtc_get_status() should be called to check the status
 *
 */
__STATIC_INLINE void hw_rtc_set_alarm_time_bcd(uint32_t time_bcd)
{
        RTC->RTC_TIME_ALARM_REG = time_bcd;
}

/**
 * \brief Set RTC alarm Calendar
 *
 * \param[in] clndr_bcd Calendar date in binary-coded decimal (BCD) format
 *
 * \note  Use wrapper function hw_rtc_set_alarm() to set the alarm in easy to use decimal number format.
 *        If this function is used, then hw_rtc_get_status() should be called to check the status
 *
 */
__STATIC_INLINE void hw_rtc_set_alarm_clndr_bcd(uint32_t clndr_bcd)
{
        RTC->RTC_CALENDAR_ALARM_REG = clndr_bcd;
}

/**
 * \brief Get RTC time and/or calendar alarms
 *
 * \param [out] time returns time alarm. Set NULL if there is no need to read time alarm
 * \param [out] clndr returns calendar alarm. Set NULL if there is no need to read calendar alarm
 * \param [out] mask returns bitmask of HW_RTC_ALARM_EN. Set NULL if there is no need to read bitmask.
 *              "1" means the alarm is enabled:
 * \parblock
 *         Bit:           |    5    |     4   |    3    |    2   |    1   |     0    |
 *                        +---------+---------+---------+--------+--------+----------+
 *         Alarm enable:  | on month| on mday | on hour | on min | on sec | on hsec  |
 *                        +---------+---------+---------+--------+--------+----------+
 * \endparblock
 *
 */
void hw_rtc_get_alarm(hw_rtc_time_t *time, hw_rtc_alarm_calendar_t *clndr, uint8_t *mask);

/**
 * \brief Get RTC alarm time
 *
 * \return time in binary-coded decimal (BCD) format
 *
 */
__STATIC_INLINE uint32_t hw_rtc_get_alarm_time_bcd(void)
{
        return RTC->RTC_TIME_ALARM_REG;
}

/**
 * \brief Get RTC alarm calendar
 *
 * \return Calendar date in binary-coded decimal (BCD) format
 *
 */
__STATIC_INLINE uint32_t hw_rtc_get_alarm_clndr_bcd(void)
{
        return RTC->RTC_CALENDAR_ALARM_REG;
}


/**
 * \brief RTC alarm enable
 *
 * \param[in] mask bitmask of HW_RTC_ALARM_EN. Set "1" to enable:
 * \parblock
 *         Bit:           |    5    |     4   |    3    |    2   |    1   |     0    |
 *                        +---------+---------+---------+--------+--------+----------+
 *         Alarm enable:  | on month| on mday | on hour | on min | on sec | on hsec  |
 *                        +---------+---------+---------+--------+--------+----------+
 * \endparblock
 *
 */
__STATIC_INLINE void hw_rtc_alarm_enable(const uint8_t mask)
{
        RTC->RTC_ALARM_ENABLE_REG = mask;
}

/**
 * \brief Get RTC alarm enable bitmask
 *
 * \return bitmask of HW_RTC_ALARM_EN. "1" indicate the alarm is enabled:
 * \parblock
 *         Bit:           |    5    |     4   |    3    |    2   |    1   |     0    |
 *                        +---------+---------+---------+--------+--------+----------+
 *         Alarm enable:  | on month| on mday | on hour | on min | on sec | on hsec  |
 *                        +---------+---------+---------+--------+--------+----------+
 * \endparblock
 *
 */
__STATIC_INLINE uint8_t hw_rtc_get_alarm_enable_msk(void)
{
        return RTC->RTC_ALARM_ENABLE_REG;
}

/**
 * \brief Enable RTC interrupt(s)
 *
 * \param [in] mask bitmask of available interrupts (HW_RTC_INTR). Set "1" to enable:
 * \parblock
 *         Bit:         |  6     |    5   |   4   |   3   |  2   |  1   |   0   |
 *                      +--------+--------+-------+-------+------+------+-------+
 *         Enable irq:  |on alarm|on month|on mday|on hour|on min|on sec|on hsec|
 *                      +--------+--------+-------+-------+------+------+-------+
 * \endparblock
 */
__STATIC_INLINE void hw_rtc_interrupt_enable(const uint8_t mask)
{
        RTC->RTC_INTERRUPT_ENABLE_REG = mask;
}

/**
 * \brief Disable RTC interrupt(s)
 *
 * \param [in] mask bitmask of available interrupts (HW_RTC_INTR). Set "1" to disable:
 * \parblock
 *         Bit:         |  6     |    5   |   4   |   3   |  2   |  1   |   0   |
 *                      +--------+--------+-------+-------+------+------+-------+
 *         Disable irq: |on alarm|on month|on mday|on hour|on min|on sec|on hsec|
 *                      +--------+--------+-------+-------+------+------+-------+
 * \endparblock
 */
__STATIC_INLINE void hw_rtc_interrupt_disable(const uint8_t mask)
{
        RTC->RTC_INTERRUPT_DISABLE_REG = mask;
}

/**
 * \brief Get RTC event flags
 *
 * \return bitmask of event flags (HW_RTC_EVENT). "1" indicates that the event occurred since the last reset:
 * \parblock
 *         Bit:    |  6     |    5   |   4   |   3   |  2   |  1   |   0   |
 *                 +--------+--------+-------+-------+------+------+-------+
 *         Event:  |on alarm|on month|on mday|on hour|on min|on sec|on hsec|
 *                 +--------+--------+-------+-------+------+------+-------+
 * \endparblock
 *
 * \note Reading the event flag register will clear the register
 *
 */
__STATIC_INLINE uint8_t hw_rtc_get_event_flags(void)
{
        return RTC->RTC_EVENT_FLAGS_REG;
}

/**
 * \brief Get RTC interrupt mask
 *
 * \return interrupt bitmask (HW_RTC_INTR)
 * \parblock
 *         Bit:      |  6     |    5   |   4   |   3   |  2   |  1   |   0   |
 *                   +--------+--------+-------+-------+------+------+-------+
 *         Intr msk: | alarm  | month  | mday  | hour  | min  | sec  | hsec  |
 *                   +--------+--------+-------+-------+------+------+-------+
 * \endparblock
 *
 *  \note bitmask can be cleared by enabling the corresponding interrupt and
 *        it can be set by disabling the corresponding interrupt.
 *
 * \sa hw_rtc_interrupt_enable
 * \sa hw_rtc_interrupt_disable
 *
 */
__STATIC_INLINE uint8_t hw_rtc_get_interrupt_mask(void)
{
        return RTC->RTC_INTERRUPT_MASK_REG;
}

/**
 * \brief Set RTC hour clock mode
 *
 * \param [in] mode 12-hour or 24-hour clock format
 *
 */
__STATIC_INLINE void hw_rtc_set_hour_clk_mode(HW_RTC_HOUR_MODE mode)
{
        RTC->RTC_HOUR_MODE_REG = (mode == RTC_12H_CLK) ? 1 : 0;
}

/**
 * \brief Get RTC hour clock mode
 *
 * \return 12-hour or 24-hour clock format
 *
 */
__STATIC_INLINE HW_RTC_HOUR_MODE hw_rtc_get_hour_clk_mode(void)
{
        return ((HW_RTC_HOUR_MODE)RTC->RTC_HOUR_MODE_REG);
}

/**
 * \brief Get RTC status
 *
 * When setting the time/calendar or the alarm the entry can be invalid. The status
 * bitmask indicates if the entry was valid or not.
 *
 * \return status bitmask. "1" means valid entry and "0" invalid entry
 * \parblock
 *         Bit:      |   3            |  2         |  1       |   0  |
 *                   +----------------+------------+----------+------+
 *         Status    | calendar alarm | time alarm | calendar | time |
 *                   +----------------+------------+----------+------+
 * \endparblock
 *
 *  \sa hw_rtc_set_time_clndr
 *  \sa hw_rtc_set_alarm
 *
 */
__STATIC_INLINE uint8_t hw_rtc_get_status(void)
{
        return RTC->RTC_STATUS_REG;
}

/**
 * \brief Configure RTC to keep or reset its registers after reset
 *
 * \param [in] keep when true, the time and calendar registers and any other registers which directly affect
 *             or are affected by the time and calendar registers are NOT reset when software reset is applied.
 *             When false, the software reset will reset every register except the keep RTC and control registers.
 *
 */
__STATIC_INLINE void hw_rtc_set_keep_reg_on_reset(bool keep)
{
        RTC->RTC_KEEP_RTC_REG = keep;
}

/**
 * \brief Get RTC keep register status
 *
 * \return keep when true, the time and calendar registers and any other registers which directly affect
 *             or are affected by the time and calendar registers are NOT reset when software reset is applied.
 *             When false, the software reset will reset every register except the keep RTC and control registers.
 *
 */
__STATIC_INLINE bool hw_rtc_get_keep_reg_on_reset(void)
{
        return RTC->RTC_KEEP_RTC_REG;
}

/**
 * \brief Enable RTC to Power Domains Controller (PDC) event
 *
 */
__STATIC_INLINE void hw_rtc_pdc_event_enable(void)
{
        HW_RTC_REG_SETF( RTC_EVENT_CTRL_REG, RTC_PDC_EVENT_EN, 1);
}

/**
 * \brief Disable RTC to Power Domains Controller event
 *
 */
__STATIC_INLINE void hw_rtc_pdc_event_disable(void)
{
        HW_RTC_REG_SETF( RTC_EVENT_CTRL_REG, RTC_PDC_EVENT_EN, 0);
}

/**
 * \brief Get status of control of RTC to Power Domains Controller event
 *
 * \return if true, RTC to PDC event is enabled
 */
__STATIC_INLINE bool hw_rtc_get_pdc_event_cntrl(void)
{
        return HW_RTC_REG_GETF(RTC_EVENT_CTRL_REG, RTC_PDC_EVENT_EN);
}

/**
 * \brief Enable RTC to Motor Controller event
 *
 */
__STATIC_INLINE void hw_rtc_motor_event_enable(void)
{
        HW_RTC_REG_SETF( RTC_EVENT_CTRL_REG, RTC_MOTOR_EVENT_EN, 1);
}

/**
 * \brief Disable RTC to Motor Controller event
 *
 */
__STATIC_INLINE void hw_rtc_motor_event_disable(void)
{
        HW_RTC_REG_SETF( RTC_EVENT_CTRL_REG, RTC_MOTOR_EVENT_EN, 0);
}

/**
 * \brief Get status of control of RTC to Motor Controller event
 *
 * \return if true, RTC to Motor Controller event is enabled
 */
__STATIC_INLINE bool hw_rtc_get_motor_event_cntrl(void)
{
        return HW_RTC_REG_GETF(RTC_EVENT_CTRL_REG, RTC_MOTOR_EVENT_EN);
}

/**
 * \brief Set RTC to Motor controller event period
 *
 * \param[in] period RTC will send an event to Motor Controller, if RTC
 *                   to Motor Controller event is enabled, every (period+1)*10ms
 *
 * \sa hw_rtc_motor_event_enable
 * \sa hw_rtc_motor_event_disable
 *
 */
__STATIC_INLINE void hw_rtc_set_motor_event_period(uint16_t period)
{
        ASSERT_WARNING(RTC_MOTOR_EVENT_PERIOD_MAX_VAL >= period);
        RTC->RTC_MOTOR_EVENT_PERIOD_REG = period;
}

/**
 * \brief Get RTC to Motor controller event period
 *
 * \return period of RTC to Motor Controller event. Event is sent every (period+1)*10ms
 *
 * \sa hw_rtc_motor_event_enable
 * \sa hw_rtc_motor_event_disable
 *
 */
__STATIC_INLINE uint16_t hw_rtc_get_motor_event_period(void)
{
        return RTC->RTC_MOTOR_EVENT_PERIOD_REG;
}

/**
 * \brief Set RTC to PDC event period
 *
 * \param[in] period RTC will send an event to PDC, if RTC
 *                   to PDC event is enabled, every (period+1)*10ms
 *
 * \sa hw_rtc_pdc_event_enable
 * \sa hw_rtc_pdc_event_disable
 *
 */
__STATIC_INLINE void hw_rtc_set_pdc_event_period(uint16_t period)
{
        ASSERT_WARNING(RTC_PDC_EVENT_PERIOD_MAX_VAL >= period);
        RTC->RTC_PDC_EVENT_PERIOD_REG = period;
}

/**
 * \brief Get RTC to PDC event period
 *
 * \return period of RTC to PDC event. Event is sent every (period+1)*10ms
 *
 * \sa hw_rtc_pdc_event_enable
 * \sa hw_rtc_pdc_event_disable
 *
 */
__STATIC_INLINE uint16_t hw_rtc_get_pdc_event_period(void)
{
        return RTC->RTC_PDC_EVENT_PERIOD_REG;
}

/**
 * \brief Clear RTC to PDC event
 *
 * \return  Value is irrelevant. On read, PDC event is cleared
 *
 */
__STATIC_INLINE bool hw_rtc_pdc_event_clear(void)
{
        return RTC->RTC_PDC_EVENT_CLEAR_REG;
}

/**
 * \brief Get Motor Controller event count
 *
 * \return It gives the current value of the Motor event counter (0 to RTC_MOTOR_EVENT_PERIOD)
 *
 * \sa hw_rtc_set_motor_event_period
 *
 */
__STATIC_INLINE uint16_t hw_rtc_get_motor_event_cnt(void)
{
        return RTC->RTC_MOTOR_EVENT_CNT_REG;
}

/**
 * \brief Get PDC event count
 *
 * \return It gives the current value of the PDC event counter (0 to RTC_PDC_EVENT_PERIOD)
 *
 * \sa hw_rtc_set_pdc_event_period
 *
 */
__STATIC_INLINE uint16_t hw_rtc_get_pdc_event_cnt(void)
{
        return RTC->RTC_PDC_EVENT_CNT_REG;
}

/**
 * \brief Reset RTC module
 *
 *  Software/hardware reset will reset every register except the keep RTC and control registers.
 *  If Keep RTC is high, the time and calendar registers and any other registers which directly affect
 *  or are affected by the time and calendar registers are NOT reset when software/hardware reset is applied.
 *  Calling this function will reset the time and calendar registers, the keep RTC register as well as the
 *  PDC and Motor event period registers. Application should not call this function
 *
 * \note Reset pulse width it is not important since the reset is asynchronous
 *
 * \sa hw_rtc_set_keep_reg_on_reset
 */
__STATIC_INLINE void hw_rtc_reset(void)
{
        uint8_t temp_reg;
        temp_reg = RTC->RTC_INTERRUPT_MASK_REG;
        RTC->RTC_INTERRUPT_DISABLE_REG = 0xff;  // Mask all interrupts to avoid trigger due to reset
        REG_SETF(CRG_TOP, CLK_RTCDIV_REG, RTC_RESET_REQ, 1);
        REG_SETF(CRG_TOP, CLK_RTCDIV_REG, RTC_RESET_REQ, 0);
        RTC->RTC_INTERRUPT_ENABLE_REG = ~temp_reg & 0x7f;  // restore interrupt mask
}

/**
 * \brief Set RTC reset high.
 *
 *  The software reset will reset every register except the keep RTC and control registers.
 *  If Keep RTC is high, the time and calendar registers and any other registers which directly affect
 *  or are affected by the time and calendar registers are NOT reset when software reset is applied.
 *  Calling this function will reset the time and calendar registers, the keep RTC register as well as the
 *  PDC and Motor event period registers. Application should not call this function
 *
 * \note RTC reset shall be cleared before starting RTC module
 *
 * \sa hw_rtc_reset_clear
 * \sa hw_rtc_set_keep_reg_on_reset
 *
 */
__STATIC_INLINE void hw_rtc_reset_set(void)
{
        REG_SETF(CRG_TOP, CLK_RTCDIV_REG, RTC_RESET_REQ, 1);
}

/**
 * \brief Clear RTC reset.
 *
 * \sa hw_rtc_reset_set
 *
 */
__STATIC_INLINE void hw_rtc_reset_clear(void)
{
        REG_SETF(CRG_TOP, CLK_RTCDIV_REG, RTC_RESET_REQ, 0);
}

/**
 * \brief Configure RTC clock
 *
 * if div_denom = 1, div_frac out of 1024 cycles will divide by (div_int+1), the rest is div_int.
 * If div_denom = 0, div_frac out of 1000 cycles will divide by (div_int+1), the rest is div_int.
 *
 * \param [in] div_denom denominator for the fractional division
 * \param [in] div_int integer divisor part for RTC 100Hz generation
 * \param [in] div_frac Fractional divisor part for RTC 100Hz generation
 *
 * \note RTC is using the low power clock (lp_clk) as clock source, therefore
 *       lp_clk must be configured and enabled before calling this function.
 */
__STATIC_INLINE void hw_rtc_clk_config(HW_RTC_DIV_DENOM div_denom, uint16_t div_int, uint16_t div_frac )
{
        REG_SETF(CRG_TOP, CLK_RTCDIV_REG, RTC_DIV_DENOM, div_denom);
        REG_SETF(CRG_TOP, CLK_RTCDIV_REG, RTC_DIV_INT, div_int);
        REG_SETF(CRG_TOP, CLK_RTCDIV_REG, RTC_DIV_FRAC, div_frac);
}

/**
 * \brief Enable for the 100 Hz generation for the RTC block
 *
 * \sa hw_rtc_clk_config
 */
__STATIC_INLINE void hw_rtc_clock_enable(void)
{
        REG_SETF(CRG_TOP, CLK_RTCDIV_REG, RTC_DIV_ENABLE, 1);
}

#endif /* dg_configUSE_HW_RTC */
#endif /* HW_RTC_H_ */

/**
 * \}
 * \}
 */
