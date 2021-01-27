/**
 ****************************************************************************************
 *
 * @file ble_cli_config.h
 *
 * @brief Application configuration
 *
 * Copyright (C) 2016-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BLE_DEBUG_CONFIG_H_
#define BLE_DEBUG_CONFIG_H_

#define CFG_AUTO_CONN_PARAM_REPLY       ( 1 )

#define CFG_AUTO_PAIR_REPLY             ( 1 )

/**
 * Default connection parameters
 *
 * They will be used for gap calls, if no parameters entered in terminal.
 */
#define CFG_CONN_PARAMS                                         \
        {                                                       \
                .interval_min = 0x28,                           \
                .interval_max = 0x38,                           \
                .slave_latency = 0,                             \
                .sup_timeout = 0x2a,                            \
        }

/**
 * Default scan parameters
 *
 * They will be used for gap scan start call, if no parameters passed in command line.
 */
#define CFG_SCAN_TYPE           (GAP_SCAN_ACTIVE)
#define CFG_SCAN_MODE           (GAP_SCAN_GEN_DISC_MODE)
#define CFG_SCAN_INTERVAL       BLE_SCAN_INTERVAL_FROM_MS(0x64)
#define CFG_SCAN_WINDOW         BLE_SCAN_WINDOW_FROM_MS(0x32)
#define CFG_SCAN_FILT_WLIST     (false)
#define CFG_SCAN_FILT_DUPLT     (false)

/**
 * Default advertising data length
 */
#define ADVERTISE_DATA_LENGTH (17)

/**
 * Default advertising data
 *
 * They will be used for gap advertising data set call, if no parameters passed in command line.
 */
#define ADVERTISE_DATA                  { 0x10, GAP_DATA_TYPE_LOCAL_NAME, \
                                        'D', 'i', 'a', 'l', 'o', 'g', ' ', 'B', 'L', 'E', \
                                                                        ' ', 'D', 'e', 'm', 'o' }

/**
 * Default scan response data length
 */
#define SCAN_RESPONSE_DATA_LENGTH (17)

/**
 * Default scan response data
 *
 * They will be used for gap advertising data set call, if no parameters passed in command line.
 */
#define SCAN_RESPONSE_DATA              { 0x10, GAP_DATA_TYPE_LOCAL_NAME, \
                                        'D', 'i', 'a', 'l', 'o', 'g', ' ', 'B', 'L', 'E', \
                                                                        ' ', 'D', 'e', 'm', 'o' }

#endif /* BLE_DEBUG_CONFIG_H_ */
