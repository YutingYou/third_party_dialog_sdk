/**
 * \addtogroup MID_INT_BLE
 * \{
 * \addtogroup BLE_STACK_CONFIG Configuration Options for BLE Stack
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ble_stack_config.h
 *
 * @brief BLE stack configuration options
 *
 * Copyright (C) 2015-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifndef _BLE_STACK_CONFIG_H_
#define _BLE_STACK_CONFIG_H_

#include "ble_config.h"


#define RAM_BUILD

/*
 * Stack definitions
 */
#include "da14690_config_host.h"


/* macro used to silence warnings about unused parameters/variables/function */
#define __UNUSED__      __attribute__((unused))

// Assertions showing a critical error that could require a full system reset
#define ASSERT_ERR(cond)                        ASSERT_ERROR(cond)


/**
 * \brief Default BLE stack database heap size in bytes
 */
#ifndef dg_configBLE_STACK_DB_HEAP_SIZE
#define dg_configBLE_STACK_DB_HEAP_SIZE    3072
#endif

/*
 * Ble pti values
 */
#define BLE_PTI_CONNECT_REQ_RESPONSE      0
#define BLE_PTI_LLCP_PACKETS              1
#define BLE_PTI_DATA_CHANNEL_TX           2
#define BLE_PTI_INITIATING_SCAN           3
#define BLE_PTI_ACTIVE_SCAN_MODE          4
#define BLE_PTI_CONNECTABLE_ADV_MODE      5
#define BLE_PTI_NON_CONNECTABLE_ADV_MODE  6

#include "bsp_debug.h"

#endif /* _BLE_STACK_CONFIG_H_ */

/**
 * \}
 * \}
 */
