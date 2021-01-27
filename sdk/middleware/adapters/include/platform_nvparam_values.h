/**
 ****************************************************************************************
 *
 * @file platform_nvparam_values.h
 *
 * @brief Non-volatile parameters description for create_nvparam script
 *
 * Copyright (C) 2016-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 * \note
 * This file is not used in regular build. It is only used by create_nvparam script to create flash
 * image to populate parameters partition with default parameter values.
 * See utilities/nvparam for more information.
 *
 ****************************************************************************************
 */

#define U16(VALUE) (uint8_t)(VALUE), (uint8_t)(VALUE >> 8)

//                                                                        ,-- parameter value           'validity' flag --
//                                                                        |                                               |
//                                                                        V                                               V
NVPARAM_PARAM_VALUE( NVPARAM_BLE_PLATFORM_BD_ADDRESS,         uint8_t,  0x01, 0x00, 0xF4, 0x35, 0x23, 0x48,              0x00)
NVPARAM_PARAM_VALUE( NVPARAM_BLE_PLATFORM_LPCLK_DRIFT,        uint8_t,  U16(0xFFFF),                                     0xFF)
NVPARAM_PARAM_VALUE( NVPARAM_BLE_PLATFORM_EXT_WAKEUP_TIME,    uint8_t,  U16(0xFFFF),                                     0xFF)
NVPARAM_PARAM_VALUE( NVPARAM_BLE_PLATFORM_OSC_WAKEUP_TIME,    uint8_t,  U16(0xFFFF),                                     0xFF)
NVPARAM_PARAM_VALUE( NVPARAM_BLE_PLATFORM_RM_WAKEUP_TIME,     uint8_t,  U16(0xFFFF),                                     0xFF)
NVPARAM_PARAM_VALUE( NVPARAM_BLE_PLATFORM_SLEEP_ENABLE,       uint8_t,  0xFF,                                            0xFF)
NVPARAM_PARAM_VALUE( NVPARAM_BLE_PLATFORM_EXT_WAKEUP_ENABLE,  uint8_t,  0xFF,                                            0xFF)
NVPARAM_PARAM_VALUE( NVPARAM_BLE_PLATFORM_BLE_CA_TIMER_DUR,   uint8_t,  U16(0xFFFF),                                     0xFF)
NVPARAM_PARAM_VALUE( NVPARAM_BLE_PLATFORM_BLE_CRA_TIMER_DUR,  uint8_t,  0xFF,                                            0xFF)
NVPARAM_PARAM_VALUE( NVPARAM_BLE_PLATFORM_BLE_CA_MIN_RSSI,    uint8_t,  0xFF,                                            0xFF)
NVPARAM_PARAM_VALUE( NVPARAM_BLE_PLATFORM_BLE_CA_NB_PKT,      uint8_t,  0xFF,                                            0xFF)
NVPARAM_PARAM_VALUE( NVPARAM_BLE_PLATFORM_BLE_CA_NB_BAD_PKT,  uint8_t,  0xFF,                                            0xFF)
NVPARAM_PARAM_VALUE( NVPARAM_BLE_PLATFORM_IRK,                uint8_t,  0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01, \
                                                                        0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01,  0x00)
