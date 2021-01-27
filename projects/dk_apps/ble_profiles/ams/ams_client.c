/**
 ****************************************************************************************
 *
 * @file ams_client.c
 *
 * @brief Apple Media Service client implementation
 *
 * Copyright (C) 2018-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdarg.h>
#include <string.h>
#include "osal.h"
#include "ble_bufops.h"
#include "ble_gattc.h"
#include "ble_gattc_util.h"
#include "ble_uuid.h"
#include "ams_client.h"

#define UUID_AMS                "89D3502B-0F36-433A-8EF4-C502AD55F8DC"
#define UUID_REMOTE_COMMAND     "9B3C81D8-57B1-4A8A-B8DF-0E56F7CA51C2"
#define UUID_ENTITY_UPDATE      "2F7CABCE-808D-411F-9A0C-BB92BA96C102"
#define UUID_ENTITY_ATTRIBUTE   "C6B2F38C-23AB-46D8-A6AB-A3A870BBD5D7"

typedef struct {
        ble_client_t client;

        // Application callbacks
        const ams_client_callbacks_t *cb;

        // Remote Command characteristic value handle
        uint16_t remote_command_val_h;
        // Remote Command CCC descriptor handle
        uint16_t remote_command_ccc_h;
        // Entity Update characteristic value handle
        uint16_t entity_update_val_h;
        // Entity Update CCC descriptor handle
        uint16_t entity_update_ccc_h;
        // Entity Attribute characteristic value handle
        uint16_t entity_attribute_val_h;
} ams_client_t;

/* Structured used for client serialization */
typedef __PACKED_STRUCT {
        uint16_t remote_command_val_h;
        uint16_t remote_command_ccc_h;
        uint16_t entity_update_val_h;
        uint16_t entity_update_ccc_h;
        uint16_t entity_attribute_val_h;
        uint16_t start_h;
        uint16_t end_h;
} ams_client_serialized_t;

typedef __PACKED_STRUCT {
        uint8_t entity_id;
        uint8_t attribute_id;
        uint8_t entity_update_flags;
        uint8_t value[0];
} entity_update_notification_t;

static void cleanup(ble_client_t *client)
{
        ams_client_t *ams_client = (ams_client_t *) client;

        OS_FREE(ams_client);
}

static void handle_disconnect_evt(ble_client_t *client, const ble_evt_gap_disconnected_t *evt)
{
        client->conn_idx = BLE_CONN_IDX_INVALID;

        ble_client_remove(client);
}

static void handle_ccc_read_completed(ams_client_t *ams_client, att_error_t status,
                                ams_client_event_t event, uint16_t length, const uint8_t *value)
{
        uint16_t ccc = 0;

        if (!ams_client->cb->get_event_state_completed) {
                return;
        }

        if (status != ATT_ERROR_OK) {
                goto done;
        }

        if (length != sizeof(ccc)) {
                status = ATT_ERROR_INVALID_VALUE_LENGTH;
                goto done;
        }

        ccc = get_u16(value);

done:
        ams_client->cb->get_event_state_completed(&ams_client->client, status, event,
                                                                ccc & GATT_CCC_NOTIFICATIONS);
}

static void handle_entity_attribute_read_completed(ams_client_t *ams_client, att_error_t status,
                                                        uint16_t length, const uint8_t *value)
{
        if (ams_client->cb->entity_attribute_read_completed) {
                ams_client->cb->entity_attribute_read_completed(&ams_client->client, status,
                                                                                length, value);
        }
}

static void handle_read_completed_evt(ble_client_t *client,
                                        const ble_evt_gattc_read_completed_t *evt)
{
        ams_client_t *ams_client = (ams_client_t *) client;
        uint16_t handle = evt->handle;

        if (handle == ams_client->remote_command_ccc_h) {
                handle_ccc_read_completed(ams_client, evt->status,
                                        AMS_CLIENT_EVENT_REMOTE_COMMAND_NOTIFY, evt->length,
                                        evt->value);
        } else if (handle == ams_client->entity_update_ccc_h) {
                handle_ccc_read_completed(ams_client, evt->status,
                                                AMS_CLIENT_EVENT_ENTITY_UPDATE_NOTIFY, evt->length,
                                                evt->value);
        } else if (handle == ams_client->entity_attribute_val_h) {
                handle_entity_attribute_read_completed(ams_client, evt->status, evt->length,
                                                                                evt->value);
        }
}

static void handle_ccc_write_completed(ams_client_t *ams_client, att_error_t status,
                                                                        ams_client_event_t event)
{
        if (ams_client->cb->set_event_state_completed) {
                ams_client->cb->set_event_state_completed(&ams_client->client, status, event);
        }
}

static void handle_remote_command_write_completed(ams_client_t *ams_client, att_error_t status)
{
        if (ams_client->cb->remote_command_completed) {
                ams_client->cb->remote_command_completed(&ams_client->client, status);
        }
}

static void handle_entity_update_write_completed(ams_client_t *ams_client, att_error_t status)
{
        if (ams_client->cb->entity_update_command_completed) {
                ams_client->cb->entity_update_command_completed(&ams_client->client, status);
        }
}

static void handle_entity_attribute_write_completed(ams_client_t *ams_client, att_error_t status)
{
        if (ams_client->cb->entity_attribute_write_completed) {
                ams_client->cb->entity_attribute_write_completed(&ams_client->client, status);
        }
}

static void handle_write_completed_evt(ble_client_t *client,
                                                const ble_evt_gattc_write_completed_t *evt)
{
        ams_client_t *ams_client = (ams_client_t *) client;
        uint16_t handle = evt->handle;

        if (handle == ams_client->remote_command_ccc_h) {
                handle_ccc_write_completed(ams_client, evt->status,
                                                        AMS_CLIENT_EVENT_REMOTE_COMMAND_NOTIFY);
        } else if (handle == ams_client->entity_update_ccc_h) {
                handle_ccc_write_completed(ams_client, evt->status,
                                                        AMS_CLIENT_EVENT_ENTITY_UPDATE_NOTIFY);
        } else if (handle == ams_client->remote_command_val_h) {
                handle_remote_command_write_completed(ams_client, evt->status);
        } else if (handle == ams_client->entity_update_val_h) {
                handle_entity_update_write_completed(ams_client, evt->status);
        } else if (handle == ams_client->entity_attribute_val_h) {
                handle_entity_attribute_write_completed(ams_client, evt->status);
        }
}

static void handle_remote_command_notification(ams_client_t *ams_client, uint16_t length,
                                                                        const uint8_t *value)
{
        if (ams_client->cb->remote_commands_update) {
                ams_client->cb->remote_commands_update(&ams_client->client, length, value);
        }
}

static void handle_entity_update_notification(ams_client_t *ams_client, uint16_t length,
                                                                        const uint8_t *value)
{
        const entity_update_notification_t *hdr;

        if (!ams_client->cb->entity_update || length < sizeof(*hdr)) {
                return;
        }

        hdr = (const entity_update_notification_t *) value;

        ams_client->cb->entity_update(&ams_client->client, hdr->entity_id, hdr->attribute_id,
                                                hdr->entity_update_flags, length - sizeof(*hdr),
                                                hdr->value);
}

static void handle_notification_evt(ble_client_t *client, const ble_evt_gattc_notification_t *evt)
{
        ams_client_t *ams_client = (ams_client_t *) client;
        uint16_t handle = evt->handle;

        if (handle == ams_client->remote_command_val_h) {
                handle_remote_command_notification(ams_client, evt->length, evt->value);
        } else if (handle == ams_client->entity_update_val_h) {
                handle_entity_update_notification(ams_client, evt->length, evt->value);
        }
}

static void serialize(ble_client_t *client, void *data, size_t *length)
{
        ams_client_t *ams_client = (ams_client_t *) client;
        ams_client_serialized_t *s_data = data;
        *length = sizeof(ams_client_serialized_t);

        if (!data) {
                return;
        }

        s_data->remote_command_val_h = ams_client->remote_command_val_h;
        s_data->remote_command_ccc_h = ams_client->remote_command_ccc_h;
        s_data->entity_update_val_h = ams_client->entity_update_val_h;
        s_data->entity_update_ccc_h = ams_client->entity_update_ccc_h;
        s_data->entity_attribute_val_h = ams_client->entity_attribute_val_h;
        s_data->start_h = ams_client->client.start_h;
        s_data->end_h = ams_client->client.end_h;
}

static ams_client_t *init(uint16_t conn_idx, const ams_client_callbacks_t *cb)
{
        ams_client_t *ams_client;

        /* AMS client initialization */
        ams_client = OS_MALLOC(sizeof(*ams_client));
        memset(ams_client, 0, sizeof(*ams_client));

        ams_client->client.conn_idx = conn_idx;
        ams_client->client.cleanup = cleanup;
        ams_client->client.disconnected_evt = handle_disconnect_evt;
        ams_client->client.read_completed_evt = handle_read_completed_evt;
        ams_client->client.write_completed_evt = handle_write_completed_evt;
        ams_client->client.notification_evt = handle_notification_evt;
        ams_client->client.serialize = serialize;
        ams_client->cb = cb;

        return ams_client;
}

ble_client_t *ams_client_init(const ams_client_callbacks_t *cb,
                                                        const ble_evt_gattc_browse_svc_t *evt)
{
        ams_client_t *ams_client;
        att_uuid_t uuid, uuid_ccc;
        const gattc_item_t *item;

        if (!cb || !evt) {
                return NULL;
        }

        ble_uuid_from_string(UUID_AMS, &uuid);
        if (!ble_uuid_equal(&uuid, &evt->uuid)) {
                return NULL;
        }

        ble_uuid_create16(UUID_GATT_CLIENT_CHAR_CONFIGURATION, &uuid_ccc);

        ams_client = init(evt->conn_idx, cb);
        ams_client->client.start_h = evt->start_h;
        ams_client->client.end_h = evt->end_h;

        ble_gattc_util_find_init(evt);

        /* Remote Command characteristic */
        ble_uuid_from_string(UUID_REMOTE_COMMAND, &uuid);
        item = ble_gattc_util_find_characteristic(&uuid);
        if (item && (item->c.properties & GATT_PROP_NOTIFY) &&
                                                        (item->c.properties & GATT_PROP_WRITE)) {
                ams_client->remote_command_val_h = item->c.value_handle;

                /* Remote Command CCC descriptor */
                item = ble_gattc_util_find_descriptor(&uuid_ccc);
                if (!item) {
                        goto failed;
                }
                ams_client->remote_command_ccc_h = item->handle;
        }

        /* Entity Update characteristic */
        ble_uuid_from_string(UUID_ENTITY_UPDATE, &uuid);
        item = ble_gattc_util_find_characteristic(&uuid);
        if (item && (item->c.properties & GATT_PROP_NOTIFY) &&
                                                        (item->c.properties & GATT_PROP_WRITE)) {
                ams_client->entity_update_val_h = item->c.value_handle;

                /* Entity Update CCC descriptor */
                item = ble_gattc_util_find_descriptor(&uuid_ccc);
                if (!item) {
                        goto failed;
                }
                ams_client->entity_update_ccc_h = item->handle;
        }

        /* Entity Attribute characteristic */
        ble_uuid_from_string(UUID_ENTITY_ATTRIBUTE, &uuid);
        item = ble_gattc_util_find_characteristic(&uuid);
        if (item && (item->c.properties & GATT_PROP_READ) &&
                                                        (item->c.properties & GATT_PROP_WRITE)) {
                ams_client->entity_attribute_val_h = item->c.value_handle;
        }

        return &ams_client->client;

failed:
        cleanup(&ams_client->client);
        return NULL;
}

ble_client_t *ams_client_init_from_data(uint16_t conn_idx, const ams_client_callbacks_t *cb,
                                                                const void *data, size_t length)
{
        const ams_client_serialized_t *s_data = (const ams_client_serialized_t *) data;
        ams_client_t *ams_client;

        if (!data || !cb || (length < sizeof(ams_client_serialized_t))) {
                return NULL;
        }

        ams_client = init(conn_idx, cb);

        ams_client->entity_attribute_val_h = s_data->entity_attribute_val_h;
        ams_client->entity_update_val_h = s_data->entity_update_val_h;
        ams_client->entity_update_ccc_h = s_data->entity_update_ccc_h;
        ams_client->remote_command_val_h = s_data->remote_command_val_h;
        ams_client->remote_command_ccc_h = s_data->remote_command_ccc_h;
        ams_client->client.start_h = s_data->start_h;
        ams_client->client.end_h = s_data->end_h;

        ble_client_attach(&ams_client->client, conn_idx);

        return &ams_client->client;
}

ams_client_cap_t ams_client_get_capabilities(const ble_client_t *client)
{
        const ams_client_t *ams_client = (const ams_client_t *) client;
        ams_client_cap_t capabilities = 0;

        if (ams_client->remote_command_val_h) {
                capabilities |= AMS_CLIENT_CAP_REMOTE_COMMAND;
        }

        if (ams_client->entity_update_val_h) {
                capabilities |= AMS_CLIENT_CAP_ENTITY_UPDATE;
        }

        if (ams_client->entity_attribute_val_h) {
                capabilities |= AMS_CLIENT_CAP_ENTITY_ATTRIBUTE;
        }

        return capabilities;
}

static uint16_t get_ccc_handle_for_event(ams_client_t *ams_client, ams_client_event_t event)
{
        uint16_t handle;

        switch (event) {
        case AMS_CLIENT_EVENT_REMOTE_COMMAND_NOTIFY:
                handle = ams_client->remote_command_ccc_h;
                break;
        case AMS_CLIENT_EVENT_ENTITY_UPDATE_NOTIFY:
                handle = ams_client->entity_update_ccc_h;
                break;
        default:
                handle = 0;
                break;
        }

        return handle;
}

bool ams_client_set_event_state(const ble_client_t *client, ams_client_event_t event, bool enable)
{
        ams_client_t *ams_client = (ams_client_t *) client;
        ble_error_t status;
        uint16_t handle;

        handle = get_ccc_handle_for_event(ams_client, event);

        if (handle == 0) {
                return false;
        }

        status = ble_gattc_util_write_ccc(client->conn_idx, handle,
                                                enable ? GATT_CCC_NOTIFICATIONS : GATT_CCC_NONE);

        return (status == BLE_STATUS_OK);
}

bool ams_client_get_event_state(const ble_client_t *client, ams_client_event_t event)
{
        ams_client_t *ams_client = (ams_client_t *) client;
        ble_error_t status;
        uint16_t handle;

        handle = get_ccc_handle_for_event(ams_client, event);

        if (handle == 0) {
                return false;
        }

        status = ble_gattc_read(ams_client->client.conn_idx, handle, 0);

        return (status == BLE_STATUS_OK);
}

bool ams_client_remote_command(ble_client_t *client, ams_client_remote_command_t command)
{
        ams_client_t *ams_client = (ams_client_t *) client;
        uint8_t cmd = command;
        ble_error_t status;

        if (ams_client->remote_command_val_h == 0) {
                return false;
        }

        status = ble_gattc_write(client->conn_idx, ams_client->remote_command_val_h, 0,
                                                                        sizeof(cmd), &cmd);

        return (status == BLE_STATUS_OK);
}

bool ams_client_entity_update_command(ble_client_t *client, ams_client_entity_id_t entity_id,
                                        uint16_t num_of_attributes, const uint8_t *attribute_ids)
{
        ams_client_t *ams_client = (ams_client_t *) client;
        ble_error_t status;
        uint8_t *cmd;

        if (ams_client->entity_update_val_h == 0) {
                return false;
        }

        /* Allocate buffer for entity update command:
         * 1 octet - entity ID
         * 1 or more octets - attribute IDs
         */
        cmd = OS_MALLOC(num_of_attributes + sizeof(uint8_t));
        cmd[0] = entity_id;
        memcpy(&cmd[1], attribute_ids, num_of_attributes);

        status = ble_gattc_write(client->conn_idx, ams_client->entity_update_val_h, 0,
                                                        num_of_attributes + sizeof(uint8_t), cmd);

        OS_FREE(cmd);

        return (status == BLE_STATUS_OK);
}

bool ams_client_entity_attribute_write(ble_client_t *client, ams_client_entity_id_t entity_id,
                                                                        uint8_t attribute_id)
{
        ams_client_t *ams_client = (ams_client_t *) client;
        ble_error_t status;
        uint8_t cmd[2];

        if (ams_client->entity_attribute_val_h == 0) {
                return false;
        }

        cmd[0] = entity_id;
        cmd[1] = attribute_id;

        status = ble_gattc_write(client->conn_idx, ams_client->entity_attribute_val_h, 0,
                                                                                sizeof(cmd), cmd);

        return (status == BLE_STATUS_OK);
}

bool ams_client_entity_attribute_read(ble_client_t *client)
{
        ams_client_t *ams_client = (ams_client_t *) client;
        ble_error_t status;

        if (ams_client->entity_attribute_val_h == 0) {
                return false;
        }

        status = ble_gattc_read(client->conn_idx, ams_client->entity_attribute_val_h, 0);

        return (status == BLE_STATUS_OK);
}
