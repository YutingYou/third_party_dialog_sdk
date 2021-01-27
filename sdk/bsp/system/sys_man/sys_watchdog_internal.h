/**
 * \addtogroup BSP
 * \{
 * \addtogroup SYSTEM
 * \{
 * \addtogroup SYS_WATCHDOG
 * \{
 * \addtogroup INTERNAL
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file sys_watchdog_internal.h
 *
 * @brief Watchdog service internal header file.
 *
 * Copyright (C) 2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SYS_WATCHDOG_INTERNAL_H_
#define SYS_WATCHDOG_INTERNAL_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * Find out if the only task currently monitored is the IDLE task.
 *
 * \return true if only the IDLE task is currently monitored, else false. This function is used
 * from CPM in order to know if watchdog should be stopped during sleep.
 */
__RETAINED_CODE bool sys_watchdog_monitor_mask_empty(void);

/**
 * \brief Set positive reload value of the watchdog timer
 *
 * \param [in] value reload value
 *
 * \note Care must be taken not to mix calls to sys_watchdog_set_pos_val() and
 * sys_watchdog_get_val() with call to hw_watchdog_set_pos_val()
 */
__RETAINED_CODE void sys_watchdog_set_pos_val(uint16_t value);

/**
 * \brief Get the value of the watchdog timer
 *
 * \return The watchdog value
 *
 * \note Care must be taken not to mix calls to sys_watchdog_set_pos_val() and
 * sys_watchdog_get_val() with call to hw_watchdog_set_pos_val()
 */
uint16_t sys_watchdog_get_val(void);

#endif /* SYS_WATCHDOG_INTERNAL_H_ */

/**
\}
\}
\}
\}
*/
