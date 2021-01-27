/**
 * \addtogroup MID_SYS_ADAPTERS
 * \{
 * \addtogroup ADAPTER_CONFIGURATION
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file platform_nvparam.h
 *
 * @brief Configuration of non-volatile parameters on platform
 *
 * Copyright (C) 2016-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef PLATFORM_NVPARAM_H_
#define PLATFORM_NVPARAM_H_

#include "ad_nvparam_defs.h"

/**
 * Tags definition for 'ble_platform' area.
 *
 * \note These are the same as tags for RW NVDS to make translation easier.
 */
#define NVPARAM_BLE_PLATFORM_BD_ADDRESS             0x01 // 6 bytes array
#define NVPARAM_BLE_PLATFORM_LPCLK_DRIFT            0x07 // 16bit value
#define NVPARAM_BLE_PLATFORM_SLEEP_ENABLE           0x11 // byte
#define NVPARAM_BLE_PLATFORM_BLE_CA_TIMER_DUR       0x40 // 16bit value
#define NVPARAM_BLE_PLATFORM_BLE_CRA_TIMER_DUR      0x41 // byte
#define NVPARAM_BLE_PLATFORM_BLE_CA_MIN_RSSI        0x42 // byte
#define NVPARAM_BLE_PLATFORM_BLE_CA_NB_PKT          0x43 // 16bit value
#define NVPARAM_BLE_PLATFORM_BLE_CA_NB_BAD_PKT      0x44 // 16bit value
#define NVPARAM_BLE_PLATFORM_IRK                    0x80 // 16 bytes array

/**
 * Offset definition for 'ble_platform' area.
 */
#define NVPARAM_OFFSET_BLE_PLATFORM_BD_ADDRESS              0x0000
#define NVPARAM_OFFSET_BLE_PLATFORM_LPCLK_DRIFT             0x0007
#define NVPARAM_OFFSET_BLE_PLATFORM_SLEEP_ENABLE            0x0013
#define NVPARAM_OFFSET_BLE_PLATFORM_BLE_CA_TIMER_DUR        0x0017
#define NVPARAM_OFFSET_BLE_PLATFORM_BLE_CRA_TIMER_DUR       0x001A
#define NVPARAM_OFFSET_BLE_PLATFORM_BLE_CA_MIN_RSSI         0x001C
#define NVPARAM_OFFSET_BLE_PLATFORM_BLE_CA_NB_PKT           0x001E
#define NVPARAM_OFFSET_BLE_PLATFORM_BLE_CA_NB_BAD_PKT       0x0021
#define NVPARAM_OFFSET_BLE_PLATFORM_IRK                     0x0024
/**
 * 'ble_platform' area definition
 *
 * Each parameter value includes additional byte for validity data, i.e. 1st byte of value shall be
 * set to 0x00 for parameter value to be considered valid.
 */
NVPARAM_AREA(ble_platform, NVMS_PARAM_PART, 0x0000)
        NVPARAM_PARAM(NVPARAM_BLE_PLATFORM_BD_ADDRESS,              NVPARAM_OFFSET_BLE_PLATFORM_BD_ADDRESS, 7)          // uint8[6]
        NVPARAM_PARAM(NVPARAM_BLE_PLATFORM_LPCLK_DRIFT,             NVPARAM_OFFSET_BLE_PLATFORM_LPCLK_DRIFT, 3)         // uint16
        NVPARAM_PARAM(NVPARAM_BLE_PLATFORM_SLEEP_ENABLE,            NVPARAM_OFFSET_BLE_PLATFORM_SLEEP_ENABLE, 2)        // uint8
        NVPARAM_PARAM(NVPARAM_BLE_PLATFORM_BLE_CA_TIMER_DUR,        NVPARAM_OFFSET_BLE_PLATFORM_BLE_CA_TIMER_DUR, 3)    // uint16
        NVPARAM_PARAM(NVPARAM_BLE_PLATFORM_BLE_CRA_TIMER_DUR,       NVPARAM_OFFSET_BLE_PLATFORM_BLE_CRA_TIMER_DUR, 2)   // uint8
        NVPARAM_PARAM(NVPARAM_BLE_PLATFORM_BLE_CA_MIN_RSSI,         NVPARAM_OFFSET_BLE_PLATFORM_BLE_CA_MIN_RSSI, 2)     // uint8
        NVPARAM_PARAM(NVPARAM_BLE_PLATFORM_BLE_CA_NB_PKT,           NVPARAM_OFFSET_BLE_PLATFORM_BLE_CA_NB_PKT, 3)       // uint16
        NVPARAM_PARAM(NVPARAM_BLE_PLATFORM_BLE_CA_NB_BAD_PKT,       NVPARAM_OFFSET_BLE_PLATFORM_BLE_CA_NB_BAD_PKT, 3)   // uint16
        NVPARAM_PARAM(NVPARAM_BLE_PLATFORM_IRK,                     NVPARAM_OFFSET_BLE_PLATFORM_IRK, 17)                // uint8[16]
NVPARAM_AREA_END()

#endif /* PLATFORM_NVPARAM_H_ */

/**
 * \}
 * \}
 */
