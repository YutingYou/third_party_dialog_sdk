/**
 * \addtogroup MID_SYS_SERVICES
 * \{
 * \addtogroup SYS_TIMER System Timer
 *
 * \brief System timer
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file sys_timer.h
 *
 * @brief System timer header file.
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SYS_TIMER_H_
#define SYS_TIMER_H_


#include "sdk_defs.h"

/**
 * \brief Start the OS timer.
 *
 * \param period The timer period in LP clock cycles
 *
 * \note This function must be called from vPortSetupTimerInterrupt()
 *
 */
void sys_timer_start(uint32_t period);

/*
 * \brief Stop the OS timer.
 *
 */
void sys_timer_stop(void);

/**
 * \brief Set OS timer trigger
 *
 * \param[in] trigger The new timer trigger
 */
void sys_timer_set_trigger(uint32_t trigger);

/**
 * \brief Get OS timer offset in the tick period
 *
 * \return The offset in the tick period
 */
uint32_t sys_timer_get_tick_offset(void);


/**
 * \brief Get uptime ticks value.
 *
 * \return The uptime ticks. Each tick will be 1000000 / configSYSTICK_CLOCK_HZ
 *         (e.g. 30.5us if XTAL32K is set as LP clock)
 *
 * \warning This function is called only from OS Tasks.
 *
 */
uint64_t sys_timer_get_uptime_ticks(void);

/**
 * \brief Get uptime ticks value.
 *
 * \return The uptime ticks. Each tick will be 1000000 / configSYSTICK_CLOCK_HZ
 *         (e.g. 30.5us if XTAL32K is set as LP clock)
 *
 * \warning This function is called only when interrupts are disabled.
 *
 */
__RETAINED_CODE uint64_t sys_timer_get_uptime_ticks_fromISR(void);

/**
 * \brief Get uptime in usec.
 *
 * \return The uptime in usec.
 *
 * \warning This function is called only from OS Tasks.
 *
 */
uint64_t sys_timer_get_uptime_usec(void);

/**
 * \brief Get uptime in usec.
 *
 * \return The uptime in usec.
 *
 * \warning This function is called only when interrupts are disabled.
 *
 */
__RETAINED_CODE uint64_t sys_timer_get_uptime_usec_fromISR(void);

/**
 * \brief Get timestamp value.
 *
 * \return The current timestamp. This is expressed in ticks of the clock that clocks OS timer
 *         (e.g. XTAL32K). For example, if XTAL32K drives OS timer, each timestamp tick will be
 *         1000000 / 32768 = 30.5uS
 *
 * \warning This function is called only from OS Tasks.
 * \deprecated This function is deprecated. User shall call sys_timer_get_uptime_ticks() instead.
 */
DEPRECATED_MSG("API no longer supported, use sys_timer_get_uptime_ticks() instead.")
__STATIC_INLINE uint64_t sys_timer_get_timestamp(void)
{
        return sys_timer_get_uptime_ticks();
}
/**
 * \brief Get timestamp value.
 *
 * \return The current timestamp. This is expressed in ticks of the clock that clocks OS timer
 *         (e.g. XTAL32K). For, example, if XTAL32K drives OS timer, each timestamp tick will be
 *         1000000 / 32768 = 30.5uS
 *
 * \warning This function is called only when interrupts are disabled.
 * \deprecated This function is deprecated. User shall call sys_timer_get_uptime_ticks_fromISR() instead.
 */
DEPRECATED_MSG("API no longer supported, use sys_timer_get_uptime_ticks_fromISR() instead.")
__STATIC_INLINE uint64_t sys_timer_get_timestamp_fromISR(void)
{
        return sys_timer_get_uptime_ticks_fromISR();
}
#endif /* SYS_TIMER_H_ */

/**
\}
\}
*/
