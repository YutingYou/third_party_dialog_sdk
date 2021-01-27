/**
 * \addtogroup MID_INT_BLE_CLIENTS
 * {
 * \addtogroup BLE_CLI_GATT GATT client
 *
 * \brief Generic Attribute Service Client
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file gatt_client.h
 *
 * @brief Generic Attribute Service Client header file
 *
 * Copyright (C) 2015-2018 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef GATT_CLIENT_H
#define GATT_CLIENT_H

/**
 * Characteristics containing CCC descriptors - may be configured for notifications or
 * indications
 */
typedef enum {
        /** Service Changed indications */
        GATT_CLIENT_EVENT_SERVICE_CHANGED_INDICATE = 0x01,
} gatt_client_event_t;

/**
 * Gatt Client capabilities
 */
typedef enum {
        /** Service Changed characteristics */
        GATT_CLIENT_CAP_SERVICE_CHANGED = 0x01,
} gatt_client_cap_t;

typedef void (* gatt_client_set_event_state_completed_cb_t) (ble_client_t *gatt_client,
                                                                        gatt_client_event_t event,
                                                                        att_error_t status);
typedef void (* gatt_client_get_event_state_completed_cb_t) (ble_client_t *gatt_client,
                                                                gatt_client_event_t event,
                                                                att_error_t status, bool enabled);
typedef void (* gatt_client_service_changed_cb_t) (ble_client_t *gatt_client,
                                                                        uint16_t start_handle,
                                                                        uint16_t end_handle);

typedef struct {
        /** Called once client set event state */
        gatt_client_set_event_state_completed_cb_t      set_event_state_completed;
        /** Called once client completed reading CCC descriptor */
        gatt_client_get_event_state_completed_cb_t      get_event_state_completed;
        /** Called once client received service changed indication */
        gatt_client_service_changed_cb_t                service_changed;
} gatt_client_callbacks_t;

/**
 * \brief Register GATT Client instance
 *
 * Function registers GATT Client
 *
 * \param [in] cb       application callbacks
 * \param [in] evt      browse svc event with Generic Attribute Service details
 *
 * \return client instance
 *
 */
ble_client_t *gatt_client_init(const gatt_client_callbacks_t *cb,
                                                        const ble_evt_gattc_browse_svc_t *evt);
/**
 * \brief Initialize and register GATT Client instance from data buffer
 *
 * Function initializes GATT Client from data buffer.
 *
 * \param [in] conn_idx         connection index
 * \param [in] cb               application callbacks
 * \param [in] data             data buffer
 * \param [in] length           data buffer's length
 *
 * \return client instance when initialized properly, NULL otherwise
 */
ble_client_t *gatt_client_init_from_data(uint16_t conn_idx, const gatt_client_callbacks_t *cb,
                                                                const void *data, size_t length);

/**
 * \brief Get Gatt client capabilities
 *
 * Function returns bit mask with Gatt client capabilities
 *
 * \param [in] gatt_client      client instance
 *
 * \return bit mask with Gatt client capabilities
 *
 */
gatt_client_cap_t gatt_client_get_capabilites(ble_client_t *gatt_client);

/**
 * \brief Set event state
 *
 * Function enable/disable indications/notifications for given characteristic
 *
 * \param [in] gatt_client      client instance
 * \param [in] event            event type
 * \param [in] enable           enable/disable flag
 *
 * \return true if write request to CCC descriptor has been sent successfully, false otherwise.
 */
bool gatt_client_set_event_state(ble_client_t *gatt_client, gatt_client_event_t event,
                                                                                bool enable);

/**
 * \brief Get event state
 *
 * Functions reads CCC descriptor of Service Changed characteristic.
 *
 * \param [in] gatt_client      client instance
 * \param [in] event            event type
 *
 * \return true if read request to CCC descriptor has been sent successfully, false otherwise.
 */
bool gatt_client_get_event_state(ble_client_t *gatt_client, gatt_client_event_t event);

#endif /* GATT_CLIENT_H */

/**
 \}
 \}
 */
