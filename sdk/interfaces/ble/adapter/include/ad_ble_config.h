/**
 * \addtogroup MID_INT_BLE_ADAPTER
 * \{
 * \addtogroup BLE_ADAPTER_CONFIG Configuration
 *
 * \brief BLE Adapter configuration
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_ble_config.h
 *
 * @brief BLE Adapter configuration
 *
 * Copyright (C) 2015-2018 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef AD_BLE_CONFIG_H
#define AD_BLE_CONFIG_H

/**
 * \brief UP Queue size
 *
 * Defines the UP (adapter to manager) Queue length, in number of messages, if not defined
 *
 */
#ifndef AD_BLE_EVENT_QUEUE_LENGTH
#define AD_BLE_EVENT_QUEUE_LENGTH    8
#endif

/**
 * \brief DOWN Queue size
 *
 * Defines the DOWN (manager to adapter) Queue length, in number of messages, if not defined
 *
 */
#ifndef AD_BLE_COMMAND_QUEUE_LENGTH
#define AD_BLE_COMMAND_QUEUE_LENGTH  8
#endif


#endif /* AD_BLE_CONFIG_H */
/**
 \}
 \}
 */
