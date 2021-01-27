/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SNC_SYS
 *
 * \brief System LLD for SNC context
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file snc_hw_sys.h
 *
 * @brief SNC-Definition of Sensor Node Controller system Low Level Driver API
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SNC_HW_SYS_H_
#define SNC_HW_SYS_H_


#if dg_configUSE_HW_SENSOR_NODE

#include "snc_defs.h"
#include "hw_sys.h"
#include "sys_bsr.h"
#ifndef dg_configUSE_HW_SENSOR_NODE_EMU
#include "hw_snc.h"
#else
#include "snc_emu.h"
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */
#include "hw_pdc.h"
#include "hw_gpio.h"
#include "hw_timer.h"

/*
 * ENUMERATION, DATA TYPE AND STRUCTURE DEFINITIONS
 *****************************************************************************************
 */

#if (defined(OS_FREERTOS) && dg_configUSE_HW_TIMER)
/**
 * \brief System timer SNC uptime ticks resolution mask
 */
#define SNC_HW_SYS_UPTIME_TICKS_RES        TIMER_MAX_RELOAD_VAL

/**
 * \brief System timer SNC timestamp resolution mask
 * \deprecated This definition is deprecated. User shall use SNC_HW_SYS_UPTIME_TICKS_RES instead
 */
#define SNC_HW_SYS_TIMESTAMP_RES           SNC_HW_SYS_UPTIME_TICKS_RES

/**
 * \brief System timer SNC uptime ticks
 */
typedef struct {
        uint32_t current_time;          /**< The current time of the clock source stored in SNC context */
        uint32_t prev_time;             /**< The current time of the clock source stored in CM33 context */
        uint64_t rtc_time;              /**< The current Real Time Clock time of the system (in CM33 context) */
} snc_hw_sys_uptime_ticks_t;

/**
 * \brief System timer SNC timestamp
 * \deprecated This structure is deprecated. User shall use  snc_hw_sys_uptime_ticks_t instead
 */
DEPRECATED_MSG("API no longer supported, use snc_hw_sys_uptime_ticks_t instead.")
typedef snc_hw_sys_uptime_ticks_t snc_hw_sys_timestamp_t;

#endif /* defined(OS_FREERTOS) && dg_configUSE_HW_TIMER */

/*
 * MACRO DEPENDENCIES
 *****************************************************************************************
 */

#include "snc_hw_sys_macros.h"

/*
 * FUNCTION DECLARATIONS
 *****************************************************************************************
 */

//==================== SNC critical section definition =========================

/**
 * \brief Function used in SNC context to enter critical section
 *
 * Enter critical section, preventing other masters from accessing a shared resource.
 *
 * Example usage:
 * \code{.c}
 * uint32_t foo_var = 0;                        // Definition of the shared variable foo_var
 *
 * SNC_UCODE_BLOCK_DEF(foo_ucode)               // Definition of foo_ucode uCode-Block
 * {
 *         ...
 *         SNC_ENTER_CRITICAL_SECTION();        // Enter critical section in SNC context
 *         SENIS_assign(da(&foo_var), 0);       // Access foo_var in SNC context
 *         SNC_LEAVE_CRITICAL_SECTION();        // Leave critical section in SNC context
 *         ...
 * )
 *
 * void foo_func_in_CM33_context()              // Function called in CM33 context,
 *                                              // accessing shared variable foo_var
 * {
 *         ...
 *         snc_enter_SNC_critical_section();    // Enter critical section in CM33 context
 *         foo_var = 1;                         // Access foo_var in CM33 context
 *         snc_leave_SNC_critical_section();    // Leave critical section in CM33 context
 *         ...
 * }
 * \endcode
 *
 * \sa SNC_LEAVE_CRITICAL_SECTION
 * \sa snc_enter_SNC_critical_section
 * \sa snc_leave_SNC_critical_section
 *
 */
#define SNC_ENTER_CRITICAL_SECTION()                                                            \
        _SNC_ENTER_CRITICAL_SECTION()

/**
 * \brief Function used in SNC context to leave critical section
 *
 * Restore access to a shared resource, allowing other masters to access it.
 *
 * Example usage:
 * \code{.c}
 * uint32_t foo_var = 0;                        // Definition of the shared variable foo_var
 *
 * SNC_UCODE_BLOCK_DEF(foo_ucode)               // Definition of foo_ucode uCode-Block
 * {
 *         ...
 *         SNC_ENTER_CRITICAL_SECTION();        // Enter critical section in SNC context
 *         SENIS_assign(da(&foo_var), 0);       // Access foo_var in SNC context
 *         SNC_LEAVE_CRITICAL_SECTION();        // Leave critical section in SNC context
 *         ...
 * )
 *
 * void foo_func_in_CM33_context()              // Function called in CM33 context,
 *                                              // accessing shared variable foo_var
 * {
 *         ...
 *         snc_enter_SNC_critical_section();    // Enter critical section in CM33 context
 *         foo_var = 1;                         // Access foo_var in CM33 context
 *         snc_leave_SNC_critical_section();    // Leave critical section in CM33 context
 *         ...
 * }
 * \endcode
 *
 * \sa SNC_ENTER_CRITICAL_SECTION
 * \sa snc_enter_SNC_critical_section
 * \sa snc_leave_SNC_critical_section
 *
 */
#define SNC_LEAVE_CRITICAL_SECTION()                                                            \
        _SNC_LEAVE_CRITICAL_SECTION()

//==================== SNC mutex manipulation macros ===========================

/**
 * \brief Macro used in SNC context to acquire a mutex
 *
 * When using this macro, an SNC/CM33 mutex is acquired, which has been previously created using
 * snc_mutex_SNC_create() function in SYSCPU (CM33) context
 *
 * \param [in] mutex    (snc_cm33_mutex_t*: build-time-only value)
 *                      pointer to the variable used as the mutex to lock
 *
 * \sa SNC_MUTEX_UNLOCK
 * \sa snc_mutex_SNC_create
 *
 */
#define SNC_MUTEX_LOCK(mutex)                                                                   \
        _SNC_MUTEX_LOCK(mutex)

/**
 * \brief Macro used in SNC context to release a mutex
 *
 * When using this macro, an SNC/CM33 mutex is released, which has been previously created using
 * snc_mutex_SNC_create() function in SYSCPU (CM33) context
 *
 * \param [in] mutex    (snc_cm33_mutex_t*: build-time-only value)
 *                      pointer to the variable used as the mutex to unlock
 *
 * \sa SNC_MUTEX_LOCK
 * \sa snc_mutex_SNC_create
 *
 */
#define SNC_MUTEX_UNLOCK(mutex)                                                                 \
        _SNC_MUTEX_UNLOCK(mutex)

//==================== SNC uCodes exchanging events with SYSCPU macros =========

/**
 * \brief Macro used in SNC context to send an event directly to SYSCPU (CM33)
 *
 * When using this macro, Sensor Node handler is triggered, and the ID of the corresponding
 * (source) uCode is updated in SNC context. Use snc_get_SNC_to_CM33_trigger() and
 * snc_clear_SNC_to_CM33_trigger() to read the status and clear the corresponding to the
 * uCode ID bits (uCode ID is equal to bit position)
 *
 * \sa snc_get_SNC_to_CM33_trigger
 * \sa snc_clear_SNC_to_CM33_trigger
 *
 */
#define SNC_CM33_NOTIFY()                                                                       \
        _SNC_CM33_NOTIFY()

/**
 * \brief Macro used in SNC context to check pending event notification from SYSCPU (CM33)
 *        to SNC uCode
 *
 * This macro is used for checking if an event notification from SYSCPU to SNC uCode is pending.
 * This is detected based on the value of CM33_to_SNC_triggered flag in the context of a
 * uCode-Block. The flag is cleared accordingly when using this macro. The flag can be set,
 * indicating an event to SNC, using snc_notify_SNC_uCode() function.
 *
 * \param [out] is_pending      (uint32_t*: use da() or ia())
 *                              pointer to a variable to store pending event indication
 *                              (>0: pending event; 0: no pending event)
 *
 * \sa snc_notify_SNC_ucode
 *
 */
#define SNC_CHECK_CM33_NOTIF_PENDING(is_pending)                                                \
        _SNC_CHECK_CM33_NOTIF_PENDING(is_pending)

//==================== SNC-main-uCode control functions ========================

/**
 * \brief Function used in SNC context to setup the microcontroller system
 *
 * When using this macro, the system is initialized.
 *
 */
#define SNC_hw_sys_init()                                                                       \
        _SNC_hw_sys_init()

/**
 * \brief Macro used in SNC context to prevent SNC from going to sleep
 *
 * When using this macro, preventing SNC from going to sleep is requested. SNC will finally go to
 * sleep, when SNC_ALLOW_SLEEP() macro is called the same number of times SNC_PREVENT_SLEEP() is
 * called after SNC waking-up.
 *
 * \sa SNC_ALLOW_SLEEP
 *
 */
#define SNC_PREVENT_SLEEP()                                                                     \
        _SNC_PREVENT_SLEEP()

/**
 * \brief Macro used in SNC context to allow SNC to go to sleep
 *
 * When using this macro, SNC is allowed to go to sleep provided that the particular macro has
 * been called the same number of times SNC_PREVENT_SLEEP() has been called after SNC waking-up.
 *
 * \sa SNC_PREVENT_SLEEP
 *
 */
#define SNC_ALLOW_SLEEP()                                                                       \
        _SNC_ALLOW_SLEEP()

//==================== Peripheral Acquisition functions ========================

/**
 * \brief Function used in SNC context to try to acquire exclusive access to a specific peripheral.
 *        If no access is granted the function returns (non-blocking).
 *
 * \param [in] perif_id         (uint32_t: build-time-only value)
 *                              the peripheral to acquire
 *                              (valid range is (0 - BSR_PERIPH_ID_MAX); check HW_SYS_BSR_PERIPH_ID)
 * \param [out] is_acquired     (uint32_t*: use da() or ia())
 *                              pointer to variable to store peripheral acquisition result
 *                              (1 = peripheral acquired)
 *
 */
#define SNC_hw_sys_bsr_try_acquire(perif_id, is_acquired)                                       \
        _SNC_hw_sys_bsr_try_acquire(perif_id, is_acquired)

/**
 * \brief Function used in SNC context to acquire exclusive access to a specific peripheral.
 *        This function will block until access is granted.
 *
 * \param [in] perif_id         (uint32_t: build-time-only value)
 *                              the peripheral to acquire
 *                              (valid range is (0 - BSR_PERIPH_ID_MAX); check HW_SYS_BSR_PERIPH_ID)
 *
 */
#define SNC_hw_sys_bsr_acquire(perif_id)                                                        \
        _SNC_hw_sys_bsr_acquire(perif_id)

/**
 * \brief Function used in SNC context to release exclusive access to a specific peripheral,
 *        so that it can be also used by other masters (SYSCPU or CMAC).
 *
 * \param [in] perif_id         (uint32_t: build-time-only value)
 *                              the peripheral to release
 *                              (valid range is (0 - BSR_PERIPH_ID_MAX); check HW_SYS_BSR_PERIPH_ID)
 *
 */
#define SNC_hw_sys_bsr_release(perif_id)                                                        \
        _SNC_hw_sys_bsr_release(perif_id)

//==================== PDC event entry acknowledgment functions ================

/**
 * \brief Function used in SNC context to acknowledge a PDC LUT event entry
 *
 * \param [in] idx      (uint32_t: use da() or ia() or build-time-only value)
 *                      the index of the LUT event entry to acknowledge
 *                      (valid range is (0 - (HW_PDC_LUT_SIZE-1)))
 *
 */
#define SNC_hw_sys_pdc_acknowledge(idx)                                                         \
        _SNC_hw_sys_pdc_acknowledge(idx)

/**
 * \brief Function used in SNC context to get all PDC LUT event entries pending for
 *        Sensor Node Controller
 *
 * \param [out] pending         (uint32_t*: use da() or ia())
 *                              pointer to variable to store all PDC LUT event entries value
 *
 */
#define SNC_hw_sys_get_pdc_pending(pending)                                                     \
        _SNC_hw_sys_get_pdc_pending(pending)

//==================== Clearing peripheral events functions ====================

/**
 * \brief Function used in SNC context to clear WakeUp Controller interrupt latch status
 *
 * \param [in] port     (HW_GPIO_PORT: build-time-only value)
 *                      GPIO port number
 * \param [in] status   (uint32_t: build-time-only value)
 *                      GPIO pin status bitmask
 *
 */
#define SNC_hw_sys_clear_wkup_status(port, status)                                              \
        _SNC_hw_sys_clear_wkup_status(port, status)

/**
 * \brief Function used in SNC context to clear capture time GPIO event
 *
 * \param [in] mask     (uint32_t: build-time-only value)
 *                      bitmask of capture time event GPIOs (set "1" to clear event)
 * \parblock
 *         Bit:          |   3   |   2   |   1   |   0   |
 *                       +-------+-------+-------+-------+
 *         event GPIO    | GPIO4 | GPIO3 | GPIO2 | GPIO1 |
 *                       +-------+-------+-------+-------+
 * \endparblock
 *
 */
#define SNC_hw_sys_clear_timer_gpio_event(mask)                                                 \
        _SNC_hw_sys_clear_timer_gpio_event(mask)

/**
 * \brief Function used in SNC context to clear timer interrupt
 *
 * \param [in] id       (HW_TIMER_ID: build-time-only value)
 *                      timer id
 *
 */
#define SNC_hw_sys_clear_timer_interrupt(id)                                                    \
        _SNC_hw_sys_clear_timer_interrupt(id)

/**
 * \brief Function used in SNC context to get and clear RTC event flags
 *
 * \param [out] flags   (uint32_t*: use da() or ia())
 *                      timer bitmask of event flags (i.e. HW_RTC_EVENT), where
 *                      "1" indicates that the event occurred since the last reset:
 * \parblock
 *         Bit:    |    6   |    5   |   4   |   3   |   2  |   1  |   0   |
 *                 +--------+--------+-------+-------+------+------+-------+
 *         Event:  |on alarm|on month|on mday|on hour|on min|on sec|on hsec|
 *                 +--------+--------+-------+-------+------+------+-------+
 * \endparblock
 *
 * \note Reading the event flags will also clear them
 *
 */
#define SNC_hw_sys_get_rtc_event_flags(flags)                                                   \
        _SNC_hw_sys_get_rtc_event_flags(flags)

/**
 * \brief Function used in SNC context to clear RTC to PDC event
 *
 */
#define SNC_hw_sys_clear_rtc_pdc_event()                                                        \
        _SNC_hw_sys_clear_rtc_pdc_event()

//==================== RTC Time/Calendar Getter functions ======================

/**
 * \brief Function used in SNC context to get RTC time
 *
 * \param [out] time    (uint32_t*: use da() or ia())
 *                      the obtained RTC time value in binary-coded decimal (BCD) format
 *
 */
#define SNC_hw_sys_rtc_get_time_bcd(time)                                                       \
        _SNC_hw_sys_rtc_get_time_bcd(time)

/**
 * \brief Function used in SNC context to get RTC calendar date
 *
 * \param [out] clndr   (uint32_t*: use da() or ia())
 *                      the obtained RTC calendar date value in binary-coded decimal (BCD) format
 *
 */
#define SNC_hw_sys_rtc_get_clndr_bcd(clndr)                                                     \
        _SNC_hw_sys_rtc_get_clndr_bcd(clndr)

//==================== Timer Capture Getter functions ==========================

/**
 * \brief Function used in SNC context to get the capture time in a system timer for event on GPIO1
 *
 * \param [in] id       (HW_TIMER_ID: build-time-only value)
 *                      the timer id
 * \param [out] cap     (uint32_t*: use da() or ia())
 *                      the obtained capture value
 *
 */
#define SNC_hw_sys_timer_get_capture1(id, cap)                                                  \
        _SNC_hw_sys_timer_get_capture1(id, cap)

/**
 * \brief Function used in SNC context to get the capture time in a system timer for event on GPIO2
 *
 * \param [in] id       (HW_TIMER_ID: build-time-only value)
 *                      the timer id
 * \param [out] cap     (uint32_t*: use da() or ia())
 *                      the obtained capture value
 *
 */
#define SNC_hw_sys_timer_get_capture2(id, cap)                                                  \
        _SNC_hw_sys_timer_get_capture2(id, cap)

/**
 * \brief Function used in SNC context to get the capture time in Timer 1 (i.e. HW_TIMER)
 *        for event on GPIO3
 *
 * \param [out] cap     (uint32_t*: use da() or ia())
 *                      the obtained capture value
 *
 */
#define SNC_hw_sys_timer_get_capture3(cap)                                                      \
        _SNC_hw_sys_timer_get_capture3(cap)

/**
 * \brief Function used in SNC context to get the capture time in Timer 1 (i.e. HW_TIMER)
 *        for event on GPIO4
 *
 * \param [out] cap     (uint32_t*: use da() or ia())
 *                      the obtained capture value
 *
 */
#define SNC_hw_sys_timer_get_capture4(cap)                                                      \
        _SNC_hw_sys_timer_get_capture4(cap)

//==================== System Timer Getter functions ===========================

#if (defined(OS_FREERTOS) && dg_configUSE_HW_TIMER)

/**
 * \brief Function used in SNC context to get system timer uptime ticks.
 *
 * Two modes are supported based on the addressing mode of uptime ticks address being passed.
 * Direct addressing is used for passing the address to store the uptime ticks (i.e. da(uptime_ticks_addr)),
 * while indirect addressing is used for passing an indirect address to store the uptime ticks
 * (i.e. ia(uptime_ticks_ind_addr)) which is internally increased, so that it can point to the next
 * word to access.
 *
 * \param [out] uptime_ticks    (snc_hw_sys_uptime_ticks_t*: use da() or ia())
 *                              the obtained uptime ticks
 */
#define SNC_hw_sys_timer_get_uptime_ticks(uptime_ticks)                                         \
        _SNC_hw_sys_timer_get_uptime_ticks(uptime_ticks)

/**
 * \brief Function used in SNC context to get system timer timestamp value.
 *
 * Two modes are supported based on the addressing mode of timestamp address being passed.
 * Direct addressing is used for passing the address to store the timestamp (i.e. da(timestamp_addr)),
 * while indirect addressing is used for passing an indirect address to store the timestamp
 * (i.e. ia(timestamp_ind_addr)) which is internally increased, so that it can point to the next
 * word to access.
 *
 * \param [out] timestamp       (snc_hw_sys_timestamp_t*: use da() or ia())
 *                              the obtained timestamp value
 *
 * \deprecated This function is deprecated. User shall call SNC_hw_sys_timer_get_uptime_ticks() instead
 *
 */
#define SNC_hw_sys_timer_get_timestamp(timestamp)                                               \
        SNC_hw_sys_timer_get_uptime_ticks(timestamp)
#endif /* defined(OS_FREERTOS) && dg_configUSE_HW_TIMER */

#endif /* dg_configUSE_HW_SENSOR_NODE */


#endif /* SNC_HW_SYS_H_ */

/**
 * \}
 * \}
 */
