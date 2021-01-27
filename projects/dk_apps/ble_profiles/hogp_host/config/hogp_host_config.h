/**
 ****************************************************************************************
 *
 * @file hogp_host_config.h
 *
 * @brief Application configuration
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HOGP_HOST_CONFIG_H_
#define HOGP_HOST_CONFIG_H_

/**
 * HOGP Host protocol mode
 *
 * Set to HIDS_CLIENT_PROTOCOL_MODE_BOOT to enable Boot protocol mode or
 * HIDS_CLIENT_PROTOCOL_MODE_REPORT to enable Report protocol mode
 */
#ifndef CFG_REPORT_MODE
#       define CFG_REPORT_MODE         HIDS_CLIENT_PROTOCOL_MODE_BOOT
#endif

/**
 * Connection parameters
 *
 * Connection parameters used to establish connection.
 */
#define CFG_CONN_PARAMS                                                 \
        {                                                               \
                .interval_min = BLE_CONN_INTERVAL_FROM_MS(50),          \
                .interval_max = BLE_CONN_INTERVAL_FROM_MS(70),          \
                .slave_latency = 0,                                     \
                .sup_timeout = BLE_SUPERVISION_TMO_FROM_MS(420),        \
        }

/**
 * Flag indicating if HOST should automatically enable all Input reports
 */
#ifndef CFG_AUTO_ENABLE_NOTIFICATIONS
#       define CFG_AUTO_ENABLE_NOTIFICATIONS (true)
#endif

/**
 * Default Scan Parameters value
 */
#define CFG_SCAN_PARAMS                                         \
        {                                                       \
                .interval = BLE_SCAN_INTERVAL_FROM_MS(30),      \
                .window = BLE_SCAN_WINDOW_FROM_MS(30)           \
        }

#endif /* HOGP_HOST_CONFIG_H_ */
