/**
 \addtogroup BSP
 \{
 \addtogroup SYSTEM
 \{
 \addtogroup POWER_MANAGER
 \{
 \addtogroup INTERNAL
 \{
 */

/**
 ****************************************************************************************
 *
 * @file sys_power_mgr_internal.h
 *
 * @brief Power Manager internal header file.
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#ifndef SYS_POWER_MGR_INTERNAL_H_
#define SYS_POWER_MGR_INTERNAL_H_

#ifdef OS_FREERTOS

#include <stdint.h>
#include <stdbool.h>
#include "osal.h"
#ifdef CONFIG_USE_BLE
#include "ble_stack_config.h"
#endif

typedef enum system_state_type {
        sys_active = 0,
        sys_idle,
        sys_powered_down,
} system_state_t;




/**
 * \brief Get system sleep state
 *
 * \return The system sleep state
 *
 */
system_state_t pm_get_system_sleep_state(void);

#endif /* OS_FREERTOS */

#endif /* SYS_POWER_MGR_INTERNAL_H_ */

/**
 \}
 \}
 \}
 \}
 */
