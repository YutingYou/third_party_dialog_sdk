/**
 ****************************************************************************************
 *
 * @file hogp_host_task.c
 *
 * @brief HOG Profile / Host demo application
 *
 * Copyright (C) 2015-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdarg.h>
#include <stdio.h>
#include "osal.h"
#include "cli.h"
#include "hw_gpio.h"
#include "sys_watchdog.h"
#include "hw_gpio.h"
#include "ble_bufops.h"
#include "ble_client.h"
#include "ble_uuid.h"
#include "sdk_queue.h"
#include "gatt_client.h"
#include "hids_client.h"
#include "bas_client.h"
#include "dis_client.h"
#include "scps_client.h"
#include "hogp_host_config.h"
#include "hogp_host_task.h"
#include "debug.h"

/**
 * CLI notify mask
 */
#define CLI_NOTIF               (1 << 2)

/**
 * HOGP Host state enum
 */
typedef enum {
        HOGP_HOST_STATE_DISCONNECTED,
        HOGP_HOST_STATE_SCANNING,
        HOGP_HOST_STATE_CONNECTING,
        HOGP_HOST_STATE_CONNECTED,
        HOGP_HOST_STATE_DISCONNECTING,
} hogp_host_state_t;

/**
 * Data type used to determine scan device data presence
 */
typedef enum {
        SCAN_BUFFER_HAS_NO_DATA         = 0x00,
        SCAN_BUFFER_HAS_DEVICE_ADDRESS  = 0x01,
        SCAN_BUFFER_HAS_DEVICE_NAME     = 0x02,
        SCAN_BUFFER_HAS_FULL_DATA       = (SCAN_BUFFER_HAS_DEVICE_ADDRESS |
                                               SCAN_BUFFER_HAS_DEVICE_NAME),
} scan_bufer_status_t;

static const hids_client_config_t hids_config = {
        .mode = CFG_REPORT_MODE,
};

typedef struct {
        uint16_t conn_idx;
        bd_address_t address;

        queue_t clients;

        bool svc_changed;
        bool pending_sec;
        bool pending_browse;
        bool pending_init;
} peer_info_t;

/* Current peer information (only 1 peer can be connected) */
__RETAINED_RW static peer_info_t peer_info = {
        .conn_idx = BLE_CONN_IDX_INVALID,
};

typedef struct {
        bd_address_t addr;
        bool name_found;
} found_device_t;

/* Scanning state */
__RETAINED static struct {
        bool match_any;
        found_device_t devices[MAX_FOUND_DEVICES];
        size_t num_devices;
} scan_state;

/**
 *  Client ID counter
 */
__RETAINED static uint8_t client_id;

/**
 * HOGP Host state
 */
__RETAINED_RW static hogp_host_state_t hogp_host_state = HOGP_HOST_STATE_DISCONNECTED;

/**
 * TASK handle
 */
__RETAINED static OS_TASK current_task;

/* Address list of found devices matched to the index (human-like notation) */
static bd_address_t found_dev_addr[MAX_FOUND_DEVICES + 1];

/* Current index reload state */
static bool index_reload_state;

static bool match_client_by_ble_client(const void *data, const void *match_data)
{
        const ble_client_t *ble_client = match_data;
        const hogp_client_t *client = data;

        return client->client == ble_client;
}

static hogp_client_t *get_client_by_ble_client(ble_client_t *client)
{
        return queue_find(&peer_info.clients, match_client_by_ble_client, client);
}

static const char *get_client_name(client_type_t type)
{
        switch (type) {
        case CLIENT_TYPE_HIDS:
                return "HID Client";
        case CLIENT_TYPE_GATT:
                return "GATT Client";
        case CLIENT_TYPE_DIS:
                return "DIS Client";
        case CLIENT_TYPE_BAS:
                return "BAS Client";
        case CLIENT_TYPE_SCPS:
                return "SCPS Client";
        default:
                return "Unknown Client";
        }
}

static void print_client_message(ble_client_t *ble_client, const char *msg)
{
        hogp_client_t *client = get_client_by_ble_client(ble_client);

        if (client) {
                printf("%s (ID = %d) %s\r\n", get_client_name(client->type), client->id, msg);
        }
}

static void client_destroy(void *data)
{
        hogp_client_t *client = data;

        printf("\t%s (ID = %d) Removed\r\n", get_client_name(client->type), client->id);

        ble_client_remove(client->client);
        ble_client_cleanup(client->client);

        OS_FREE(client);
}

static void client_new(ble_client_t *ble_client, client_type_t type)
{
        hogp_client_t *client = OS_MALLOC(sizeof(*client));

        ble_client_add(ble_client);
        client->client = ble_client;
        client->type = type;
        client->id = client_id++;

        queue_push_back(&peer_info.clients, client);

        printf("\t%s (ID = %d) Initialized\r\n", get_client_name(client->type), client->id);
}

static void client_attach(void *data, void *user_data)
{
        hogp_client_t *client = data;
        uint16_t *conn_idx = user_data;

        printf("\t%s (ID = %d) Attached\r\n", get_client_name(client->type), client->id);

        ble_client_attach(client->client, *conn_idx);
}

static void print_hids_client_cap(hogp_client_t *client, hids_client_cap_t cap)
{
        printf("HID Service (ID = %d) Supported characteristics:\r\n", client->id);

        if (cap & HIDS_CLIENT_CAP_PROTOCOL_MODE) {
                printf("\tProtocol Mode characteristic\r\n");
        }

        if (cap & HIDS_CLIENT_CAP_BOOT_MOUSE_INPUT) {
                printf("\tBoot Mouse Input characteristic\r\n");
        }

        if (cap & HIDS_CLIENT_CAP_BOOT_KEYBOARD_INPUT) {
                printf("\tBoot Keyboard Input characteristic\r\n");
        }

        if (cap & HIDS_CLIENT_CAP_BOOT_KEYBOARD_OUTPUT) {
                printf("\tBoot Keyboard Output characteristic\r\n");
        }

        if (cap & HIDS_CLIENT_CAP_HID_INFO) {
                printf("\tHID Info characteristic\r\n");
        }

        if (cap & HIDS_CLIENT_CAP_HID_CONTROL_POINT) {
                printf("\tHID Control Point characteristic\r\n");
        }

        if (cap & HIDS_CLIENT_CAP_REPORT_MAP) {
                printf("\tReport Map characteristic\r\n");
        }

        printf("\r\n");

}

static void client_init(void *data, void *user_data)
{
        hogp_client_t *client = data;

        switch (client->type) {
        case CLIENT_TYPE_BAS:
                bas_client_get_event_state(client->client, BAS_CLIENT_EVENT_BATTERY_LEVEL_NOTIFY);
                bas_client_set_event_state(client->client, BAS_CLIENT_EVENT_BATTERY_LEVEL_NOTIFY,
                                                                                              true);
                bas_client_read_battery_level(client->client);
                break;
        case CLIENT_TYPE_DIS:
                dis_client_read(client->client, DIS_CLIENT_CAP_PNP_ID);
                break;
        case CLIENT_TYPE_GATT:
                gatt_client_set_event_state(client->client,
                                                  GATT_CLIENT_EVENT_SERVICE_CHANGED_INDICATE, true);
                break;
        case CLIENT_TYPE_HIDS:
        {
                hids_client_cap_t cap;

                cap = hids_client_get_capabilities(client->client);
                print_hids_client_cap(client, cap);

                if (cap & HIDS_CLIENT_CAP_PROTOCOL_MODE) {
                        hids_client_set_protocol_mode(client->client);
                }

                if (hids_config.mode == HIDS_CLIENT_PROTOCOL_MODE_REPORT) {
                        hids_client_read_hid_info(client->client);
                        hids_client_read_report_map(client->client);
                        hids_client_discover_external_reports(client->client);
                        hids_client_discover_reports(client->client);
                } else {
#if CFG_AUTO_ENABLE_NOTIFICATIONS
                        if (cap & HIDS_CLIENT_CAP_BOOT_MOUSE_INPUT) {
                                hids_client_boot_report_set_notif_state(client->client,
                                                        HIDS_CLIENT_BOOT_MOUSE_INPUT, true);
                        }

                        if (cap & HIDS_CLIENT_CAP_BOOT_KEYBOARD_INPUT) {
                                hids_client_boot_report_set_notif_state(client->client,
                                                        HIDS_CLIENT_BOOT_KEYBOARD_INPUT, true);
                        }
#endif
                }
                break;
        }
        case CLIENT_TYPE_SCPS:
                scps_client_set_event_state(client->client, SCPS_CLIENT_EVENT_REFRESH_NOTIF, true);
                break;
        case CLIENT_TYPE_NONE:
        default:
                break;
        }
}

static void hids_report_map_cb(ble_client_t *hids_client, att_error_t status, uint16_t length,
                                                                        const uint8_t *data)
{
        int i;

        print_client_message(hids_client, "Read report map completed");
        printf("\tStatus: 0x%02X\r\n", status);

        if (!status) {
                printf("\tData: ");

                for (i = 0; i < length; i++) {
                        printf("%02x ", data[i]);
                }

                printf("\r\n");
        }
}

static void hids_hid_info_cb(ble_client_t *hids_client, const hids_client_hid_info_t *info)
{
        print_client_message(hids_client, "Read HID info completed");

        printf("\tResource: HID Info characteristic\r\n");
        printf("\tbcdHID: 0x%04x\r\n", info->bcd_hid);
        printf("\tbCountryCode: 0x%02x\r\n", info->country_code);
        printf("\tFlags: 0x%02x\r\n", info->flags);
}

static void hids_report_cb(ble_client_t *hids_client, hids_client_report_type_t type,
                                                        uint8_t report_id, att_error_t status,
                                                        uint16_t length, const uint8_t *data)
{
        int i;

        print_client_message(hids_client, "Report callback");

        if (!status) {
                printf("\tReport type: ");

                switch (type) {
                case HIDS_CLIENT_REPORT_TYPE_INPUT:
                        printf("REPORT_TYPE_INPUT\r\n");
                        break;
                case HIDS_CLIENT_REPORT_TYPE_OUTPUT:
                        printf("REPORT_TYPE_OUTPUT\r\n");
                        break;
                case HIDS_CLIENT_REPORT_TYPE_FEATURE:
                        printf("REPORT_TYPE_FEATURE\r\n");
                        break;
                }

                printf("\tReport Id: %d\r\n", report_id);
                printf("\tData: ");

                for (i = 0; i < length; i++) {
                        printf("%02x ", data[i]);
                }

                printf("\r\n");
        }
}

static void hids_boot_report_cb(ble_client_t *hids_client, hids_client_boot_report_type type,
                                        att_error_t status, uint16_t length, const uint8_t *data)
{
        int i;

        print_client_message(hids_client, "Boot report callback");
        printf("\tReport type: ");

        switch (type) {
        case HIDS_CLIENT_BOOT_KEYBOARD_INPUT:
                printf("BOOT_KEYBOARD_INPUT\r\n");
                break;
        case HIDS_CLIENT_BOOT_KEYBOARD_OUTPUT:
                printf("BOOT_KEYBOARD_OUTPUT\r\n");
                break;
        case HIDS_CLIENT_BOOT_MOUSE_INPUT:
                printf("BOOT_MOUSE_INPUT\r\n");
                break;
        }

        printf("\tData: ");

        for (i = 0; i < length; i++) {
                printf("%02x ", data[i]);
        }

        printf("\r\n");
}

static void hids_get_protocol_mode_cb(ble_client_t *hids_client, att_error_t status,
                                                                hids_client_protocol_mode_t mode)
{
        print_client_message(hids_client, "Get protocol mode callback");
        printf("\tStatus: 0x%02X\r\n", status);

        if (!status) {
                printf("\tMode: %s\r\n", mode == HIDS_CLIENT_PROTOCOL_MODE_BOOT ?
                        "HIDS_CLIENT_PROTOCOL_MODE_BOOT" : "HIDS_CLIENT_PROTOCOL_MODE_REPORT");
        }

}

static void hids_input_report_get_notif_state_cb(ble_client_t *hids_client, uint8_t report_id,
                                                                att_error_t status, bool enabled)
{
        print_client_message(hids_client, "Get report notif state callback");
        printf("\tStatus: 0x%02X\r\n", status);

        if (!status) {
                printf("\tReport Id: %d\r\n", report_id);
                printf("\tNotifications: %s\r\n", enabled ? "enabled" : "disabled");
        }
}

static void hids_boot_report_get_notif_state_cb(ble_client_t *hids_client,
                                                                hids_client_boot_report_type type,
                                                                att_error_t status, bool enabled)
{
        print_client_message(hids_client, "Get boot report notif state callback");
        printf("\tStatus: 0x%02X\r\n", status);

        if (!status) {
                printf("\tReport type: ");

                switch (type) {
                case HIDS_CLIENT_BOOT_KEYBOARD_INPUT:
                        printf("BOOT_KEYBOARD_INPUT\r\n");
                        break;
                case HIDS_CLIENT_BOOT_KEYBOARD_OUTPUT:
                        printf("BOOT_KEYBOARD_OUTPUT\r\n");
                        break;
                case HIDS_CLIENT_BOOT_MOUSE_INPUT:
                        printf("BOOT_MOUSE_INPUT\r\n");
                        break;
                }

                printf("\tNotifications: %s\r\n", enabled ? "enabled" : "disabled");
        }
}

static const char *format_uuid(const att_uuid_t *uuid)
{
        static char buf[37];

        if (uuid->type == ATT_UUID_16) {
                sprintf(buf, "0x%04x", uuid->uuid16);
        } else {
                int i;
                int idx = 0;

                for (i = ATT_UUID_LENGTH; i > 0; i--) {
                        if (i == 12 || i == 10 || i == 8 || i == 6) {
                                buf[idx++] = '-';
                        }

                        idx += sprintf(&buf[idx], "%02x", uuid->uuid128[i - 1]);
                }
        }

        return buf;
}

static void hids_external_report_found_cb(ble_client_t *hids_client, att_error_t status,
                                                                        const att_uuid_t *uuid)
{
        print_client_message(hids_client, "External report found");
        printf("\tStatus: 0x%02X\r\n", status);

        if (!status) {
                printf("\tUUID: %s\r\n", format_uuid(uuid));
        }
}

static void hids_discover_external_reports_complete_cb(ble_client_t *hids_client)
{
        print_client_message(hids_client, "External report discovery complete");
        printf("External reports discovery completed\r\n");
}

static void hids_client_report_found_cb(ble_client_t *hids_client, att_error_t status,
                                                hids_client_report_type_t type, uint8_t report_id)
{
        print_client_message(hids_client, "Report found");
        printf("\tStatus: 0x%02X\r\n", status);

        if (!status) {
                printf("\tReport type: %d\r\n", type);
                printf("\tReport Id: %d\r\n", report_id);
        }

#if CFG_AUTO_ENABLE_NOTIFICATIONS
        if (status == ATT_ERROR_OK && type == HIDS_CLIENT_REPORT_TYPE_INPUT) {
                hids_client_input_report_set_notif_state(hids_client, report_id, true);
        }
#endif
}

static void hids_client_discover_reports_complete_cb(ble_client_t *hids_client)
{
        print_client_message(hids_client, "Reports discovery completed");
}

/**
 * HIDS Client callbacks
 */
static const hids_client_callbacks_t cb = {
        .report_map = hids_report_map_cb,
        .hid_info = hids_hid_info_cb,
        .report = hids_report_cb,
        .boot_report = hids_boot_report_cb,
        .get_protocol_mode = hids_get_protocol_mode_cb,
        .input_report_get_notif_state = hids_input_report_get_notif_state_cb,
        .boot_report_get_notif_state = hids_boot_report_get_notif_state_cb,
        .external_report_found = hids_external_report_found_cb,
        .discover_external_reports_complete = hids_discover_external_reports_complete_cb,
        .report_found = hids_client_report_found_cb,
        .discover_reports_complete = hids_client_discover_reports_complete_cb,
};

static void gatt_set_event_state_completed_cb(ble_client_t *gatt_client, gatt_client_event_t event,
                                                                                att_error_t status)
{
        print_client_message(gatt_client, "Set event state completed");
        printf("\tEvent: 0x%02X\r\n", event);
        printf("\tStatus: 0x%02X\r\n", status);
}

static void gatt_get_event_state_completed_cb(ble_client_t *gatt_client, gatt_client_event_t event,
                                                                att_error_t status, bool enabled)
{
        print_client_message(gatt_client, "Get event state completed");
        printf("\tEvent: 0x%02X\r\n", event);
        printf("\tStatus: 0x%02X\r\n", status);
        printf("\tEnabled: %s\r\n", enabled ? "True" : "False");
}

static void gatt_service_changed_cb(ble_client_t *gatt_client, uint16_t start_handle,
                                                                        uint16_t end_handle)
{
        print_client_message(gatt_client, "Service changed callback");
        printf("\tStart handle: 0x%04X\r\n", start_handle);
        printf("\tEnd handle: 0x%04X\r\n", end_handle);

        if (!peer_info.pending_browse) {
                queue_remove_all(&peer_info.clients, client_destroy);
                peer_info.pending_browse = true;
                peer_info.pending_init = true;

                /**
                 * Start discovery
                 */
                ble_gattc_browse(peer_info.conn_idx, NULL);
        } else {
                peer_info.svc_changed = true;
        }
}

/**
 * GATT Client callbacks
 */
static const gatt_client_callbacks_t gatt_cb = {
        .set_event_state_completed = gatt_set_event_state_completed_cb,
        .get_event_state_completed = gatt_get_event_state_completed_cb,
        .service_changed = gatt_service_changed_cb,
};

static void bas_read_battery_level_cb(ble_client_t *bas_client, att_error_t status,
                                                                        uint8_t level)
{
        print_client_message(bas_client, "Read battery level");
        printf("\tStatus: 0x%02X\r\n", status);

        if (!status) {
                printf("\tLevel: 0x%02X\r\n", level);
        }
}

static void bas_get_event_state_cb(ble_client_t *bas_client, bas_client_event_t event,
                                                                att_error_t status, bool enabled)
{
        print_client_message(bas_client, "Get event state completed");
        printf("\tEvent: 0x%02X\r\n", event);
        printf("\tStatus: 0x%02X\r\n", status);

        if (!status) {
                printf("\tBattery Level notifications: %s\r\n", enabled ? "enabled" : "disabled");
        }
}

static void bas_battery_level_notif_cb(ble_client_t *bas_client, uint8_t level)
{
        print_client_message(bas_client, "Battery level notification");
        printf("\tLevel: %02d%%\r\n", level);
}

static void bas_set_event_state_cb(ble_client_t *bas_client, bas_client_event_t event,
                                                                        att_error_t status)
{
        print_client_message(bas_client, "Set event state completed");
        printf("\tEvent: 0x%02X\r\n", event);
        printf("\tStatus: 0x%02X\r\n", status);
}

/**
 * Battery Service Client callbacks
 */
static const bas_client_callbacks_t bas_cb = {
        .read_battery_level_completed = bas_read_battery_level_cb,
        .set_event_state_completed = bas_set_event_state_cb,
        .get_event_state_completed = bas_get_event_state_cb,
        .battery_level_notif = bas_battery_level_notif_cb,
};

static void dis_read_completed_cb(ble_client_t *dis_client, att_error_t status,
                                                                dis_client_cap_t capability,
                                                                uint16_t length,
                                                                const uint8_t *value)
{
        print_client_message(dis_client, "Read completed");
        printf("\tCapability: %d\r\n", capability);
        printf("\tStatus: 0x%02X\r\n", status);
        printf("\tLength: 0x%04X\r\n", length);

        if (capability == DIS_CLIENT_CAP_PNP_ID && length == sizeof(dis_client_pnp_id_t)) {
                const dis_client_pnp_id_t *pnp_id = (const dis_client_pnp_id_t *) value;

                printf("\tVendor ID Source: 0x%02X\r\n", pnp_id->vid_source);
                printf("\tVendor ID: 0x%04X\r\n", pnp_id->vid_source);
                printf("\tProduct ID: 0x%04X\r\n", pnp_id->pid);
                printf("\tProduct Version: 0x%04X\r\n", pnp_id->version);
        }
}

/**
 *
 */
static const dis_client_callbacks_t dis_cb = {
        .read_completed = dis_read_completed_cb,
};

static void scps_refresh_notif_cb(ble_client_t *scps_client)
{
        const gap_scan_params_t scan_params = CFG_SCAN_PARAMS;

        print_client_message(scps_client, "Refresh characteristic notification");

        scps_client_write_scan_interval_window(scps_client, scan_params.interval,
                                                                        scan_params.window);
}

static void scps_set_event_state_completed_cb(ble_client_t *scps_client, scps_client_event_t event,
                                                                                att_error_t status)
{

        print_client_message(scps_client, "Set event state completed");
        printf("\tEvent: 0x%02X\r\n", event);
        printf("\tStatus: 0x%02X\r\n", status);
}

static void scps_get_event_state_completed_cb(ble_client_t *scps_client, scps_client_event_t event,
                                                                att_error_t status, bool enabled)
{
        print_client_message(scps_client, "Get event state completed");
        printf("\tEvent: 0x%02X\r\n", event);
        printf("\tStatus: 0x%02X\r\n", status);
        printf("\tEnabled: %s\r\n", enabled ? "true" : "false");
}

static const scps_client_callbacks_t scps_cb = {
        .refresh_notif = scps_refresh_notif_cb,
        .set_event_state_completed = scps_set_event_state_completed_cb,
        .get_event_state_completed = scps_get_event_state_completed_cb,
};

static void handle_disconnected(ble_evt_gap_disconnected_t *evt)
{
        printf("Device disconnected\r\n");
        printf("\tConnection index: %d\r\n", evt->conn_idx);
        printf("\tBD address of disconnected device: %s, %s\r\n",
                                evt->address.addr_type == PUBLIC_ADDRESS ? "public" : "private",
                                                          ble_address_to_string(&evt->address));
        printf("\tReason of disconnection: 0x%02x\r\n", evt->reason);

        if (peer_info.conn_idx == evt->conn_idx) {
                if (peer_info.pending_browse) {
                        /* List of services is incomplete, so remove clients */
                        queue_remove_all(&peer_info.clients, client_destroy);
                }

                hogp_host_state = HOGP_HOST_STATE_DISCONNECTED;

                peer_info.conn_idx = BLE_CONN_IDX_INVALID;
                peer_info.pending_browse = false;
                peer_info.pending_sec = false;
                peer_info.svc_changed = false;
        }
}

static void initialize_clients(void)
{
        if (peer_info.pending_sec) {
                return;
        }

        if (peer_info.pending_browse) {
                return;
        }

        // Init was previously performed
        if (!peer_info.pending_init) {
                return;
        }

        queue_foreach(&peer_info.clients, client_init, NULL);

        peer_info.pending_init = false;
}

static void handle_connected(ble_evt_gap_connected_t *evt)
{
        printf("Device connected\r\n");
        printf("\tConnection index: %d\r\n", evt->conn_idx);
        printf("\tAddress: %s\r\n", ble_address_to_string(&evt->peer_address));

        if (hogp_host_state == HOGP_HOST_STATE_CONNECTING && !ble_address_cmp(&evt->peer_address,
                                                                             &peer_info.address)) {
                bool bonded;

                peer_info.conn_idx = evt->conn_idx;
                hogp_host_state = HOGP_HOST_STATE_CONNECTED;

                ble_gap_is_bonded(peer_info.conn_idx, &bonded);
                if (bonded) {
                        ble_gap_set_sec_level(peer_info.conn_idx, GAP_SEC_LEVEL_2);
                } else {
                        ble_gap_pair(peer_info.conn_idx, true);
                }

                peer_info.pending_sec = true;

                if (queue_length(&peer_info.clients) == 0) {
                        /**
                         * Start discovery
                         */
                        ble_gattc_browse(peer_info.conn_idx, NULL);
                        peer_info.pending_browse = true;
                } else {
                        queue_foreach(&peer_info.clients, client_attach, &peer_info.conn_idx);
                }

                peer_info.pending_init = true;
        }
}

static void handle_evt_gap_connection_completed(const ble_evt_gap_connection_completed_t *evt)
{
        /* Successful connections are handled in separate event */
        if (evt->status == BLE_STATUS_OK) {
                return;
        }

        printf("Connection failed\r\n");
        printf("\tStatus: 0x%02x\r\n", evt->status);

        hogp_host_state = HOGP_HOST_STATE_DISCONNECTED;
}


static void handle_gattc_browse_svc(ble_evt_gattc_browse_svc_t *evt)
{
        ble_client_t *client = NULL;
        client_type_t type;
        att_uuid_t uuid;

        if (evt->conn_idx != peer_info.conn_idx || peer_info.svc_changed) {
                return;
        }

        printf("Service found during browsing\r\n");
        printf("\tStart handle: 0x%04X\r\n", evt->start_h);
        printf("\tEnd handle: 0x%04X\r\n", evt->end_h);

        ble_uuid_create16(UUID_SERVICE_HIDS, &uuid);
        if (ble_uuid_equal(&uuid, &evt->uuid)) {
                client = hids_client_init(&hids_config, &cb, evt);
                type = CLIENT_TYPE_HIDS;
                goto done;
        }

        ble_uuid_create16(UUID_SERVICE_GATT, &uuid);
        if (ble_uuid_equal(&uuid, &evt->uuid)) {
                client = gatt_client_init(&gatt_cb, evt);
                type = CLIENT_TYPE_GATT;
                goto done;
        }

        ble_uuid_create16(UUID_SERVICE_BAS, &uuid);
        if (ble_uuid_equal(&uuid, &evt->uuid)) {
                client = bas_client_init(&bas_cb, evt);
                type = CLIENT_TYPE_BAS;
                goto done;
        }

        ble_uuid_create16(UUID_SERVICE_DIS, &uuid);
        if (ble_uuid_equal(&uuid, &evt->uuid)) {
                client = dis_client_init(&dis_cb, evt);
                type = CLIENT_TYPE_DIS;
                goto done;
        }

        ble_uuid_create16(UUID_SERVICE_SCPS, &uuid);
        if (ble_uuid_equal(&uuid, &evt->uuid)) {
                client = scps_client_init(&scps_cb, evt);
                type = CLIENT_TYPE_SCPS;
                goto done;
        }

done:
        if (client) {
                client_new(client, type);
        }
}

static void handle_gattc_browse_cmpl(ble_evt_gattc_browse_completed_t *evt)
{
        if (evt->conn_idx != peer_info.conn_idx) {
                return;
        }

        printf("Browse completed\r\n");
        printf("\tConnection index: %u\r\n", evt->conn_idx);
        printf("\tStatus: 0x%02u\r\n", evt->status);

        if (peer_info.svc_changed) {
                peer_info.svc_changed = false;

                /**
                 * Remove hids clients and start discovery
                 */
                queue_remove_all(&peer_info.clients, client_destroy);
                peer_info.pending_browse = true;

                /**
                 * Restart discovery
                 */
                ble_gattc_browse(peer_info.conn_idx, NULL);
        } else {
                peer_info.pending_browse = false;
        }

        initialize_clients();
}

static void security_done(uint16_t conn_idx)
{
        if (conn_idx != peer_info.conn_idx) {
                return;
        }

        peer_info.pending_sec = false;

        initialize_clients();
}

static void handle_gap_sec_level_changed(ble_evt_gap_sec_level_changed_t *evt)
{
        security_done(evt->conn_idx);
}

static void handle_gap_set_sec_level_failed(ble_evt_gap_set_sec_level_failed_t *evt)
{
        security_done(evt->conn_idx);
}

static void handle_gap_pair_completed(ble_evt_gap_pair_completed_t *evt)
{
        security_done(evt->conn_idx);
}

static void handle_security_request(ble_evt_gap_security_request_t *evt)
{
        /**
         * Send pairing request to remote device or enable encryption with existing keys
         */
        if (ble_gap_pair(evt->conn_idx, evt->bond) == BLE_ERROR_ALREADY_DONE) {
                ble_gap_set_sec_level(evt->conn_idx, GAP_SEC_LEVEL_2);
        }
}

static found_device_t *get_found_device(const bd_address_t *addr, size_t *index)
{
        size_t i;

        for (i = 0; i < scan_state.num_devices; i++) {
                found_device_t *dev = &scan_state.devices[i];

                if (ble_address_cmp(&dev->addr, addr)) {
                        *index = i + 1;
                        return dev;
                }
        }

        return NULL;
}

static inline found_device_t *add_found_device(const bd_address_t *addr, size_t *index)
{
        found_device_t *dev;

        if (scan_state.num_devices >= MAX_FOUND_DEVICES) {
                scan_state.num_devices = 0;
                index_reload_state = true;
        }

        dev = &scan_state.devices[scan_state.num_devices++];
        dev->addr = *addr;
        dev->name_found = false;

        *index = scan_state.num_devices;
        found_dev_addr[*index] = dev->addr;

        return dev;
}

static inline void clear_found_device(void)
{
        found_device_t *dev;

        size_t i;
        for (i = 1; i <= scan_state.num_devices; i++) {
                dev = &scan_state.devices[i];
                dev->name_found = false;
                size_t j;
                for (j = 0; j < sizeof(dev->addr.addr) / sizeof(dev->addr.addr[0]); j++) {
                        dev->addr.addr[j] = 0;
                }
        }
}

static void handle_evt_gap_scan_complt(const ble_evt_gap_scan_completed_t* evt)
{

        bd_address_t* addr;
        size_t num_found_dev;
        num_found_dev = scan_state.num_devices;

        printf("Scan stopped\r\n");
        if (!index_reload_state) {
                /* clear indexes not matched to any found device during the last scanning procedure */
                for (size_t i = num_found_dev + 1; i <= MAX_FOUND_DEVICES; i++) {
                        addr = &(found_dev_addr[i]);
                        for (size_t j = 0; j < sizeof(addr->addr); j++) {
                                addr->addr[j] = 0;
                        }
                }
        }

        index_reload_state = false;
        clear_found_device();
}

static void handle_evt_gap_adv_report(const ble_evt_gap_adv_report_t *evt)
{
        found_device_t *dev;
        size_t dev_index = 0;
        const uint8_t *p;
        uint8_t ad_len, ad_type;
        bool new_device = false;
        const char *dev_name = NULL;
        size_t dev_name_len = 0;

        dev = get_found_device(&evt->address, &dev_index);
        if (dev && dev->name_found) {
                return;
        }

        /* Add device if 'any' was specified as scan argument */
        if (!dev && scan_state.match_any) {
                new_device = true;
                dev = add_found_device(&evt->address, &dev_index);
        }

        for (p = evt->data; p < evt->data + evt->length; p += (ad_len - 1)) {
                ad_len = *p++;
                ad_type = *p++;

                /* Device not found so we look for UUID */
                if (!dev && (ad_type == GAP_DATA_TYPE_UUID16_LIST ||
                                                        ad_type == GAP_DATA_TYPE_UUID16_LIST_INC)) {
                        size_t idx;

                        for (idx = 0; idx < ad_len; idx += sizeof(uint16_t)) {
                                if (get_u16(p + idx) == UUID_SERVICE_HIDS) {
                                        new_device = true;
                                        dev = add_found_device(&evt->address, &dev_index);
                                        break;
                                }
                        }

                        continue;
                }

                /* Look for name and store it to use later, if proper UUID is found */
                if (ad_type == GAP_DATA_TYPE_SHORT_LOCAL_NAME ||
                                                        ad_type == GAP_DATA_TYPE_LOCAL_NAME) {
                        dev_name = (const char *) p;
                        dev_name_len = ad_len;

                        if (dev) {
                                /* Already have device, no need to look further */
                                break;
                        }
                }
        }

        /*
         * If we have both device and device name, print as new device found with name.
         * For new device and no name, just print address for now.
         */
        if (dev && dev_name) {
                dev->name_found = true;
                printf("[%02d] Device found: %s %s (%.*s)\r\n", dev_index,
                                evt->address.addr_type == PUBLIC_ADDRESS ? "public" : "private",
                                ble_address_to_string(&evt->address),
                                dev_name_len, dev_name);
        } else if (new_device) {
                printf("[%02d] Device found: %s %s\r\n", dev_index,
                                evt->address.addr_type == PUBLIC_ADDRESS ? "public" : "private",
                                ble_address_to_string(&evt->address));
        }
}

struct client_id_type {
        client_type_t type;
        uint8_t id;
};

static bool client_match_id_type(const void *data, const void *match_data)
{
        const struct client_id_type *client_data = match_data;
        const hogp_client_t *client = data;

        if (client->id != client_data->id) {
                return false;
        }

        return (client->type == client_data->type);
}

hogp_client_t *get_client(uint8_t id, client_type_t type)
{
        struct client_id_type data;

        data.type = type;
        data.id = id;

        return queue_find(&peer_info.clients, client_match_id_type, &data);
}

void hogp_connect_usage(void)
{
        printf("usage: connect <address [public|private] | index>\r\n");
        printf("       connect cancel\r\n");
        printf("\tinstead of address, index of found device can be passed\r\n");
        printf("\tif not specified, public address is assumed\r\n");
        printf("\tuse 'connect cancel' to cancel any outgoing connection attempt\r\n");
}

void hogp_connect(bd_address_t *address, size_t dev_index)
{
        ble_error_t status;
        const gap_conn_params_t cp = CFG_CONN_PARAMS;

        if (dev_index) {
                if (dev_index < 1 || dev_index > scan_state.num_devices) {
                        printf("ERROR: device index out of range ([%02d])\r\n", dev_index);
                        return;
                }
                for (int i = 0; i < sizeof(address->addr); i++) {
                        address->addr[i] = found_dev_addr[dev_index].addr[i];
                }
        }

        if (hogp_host_state != HOGP_HOST_STATE_DISCONNECTED) {
                printf("ERROR: connection failed, device needs to be in disconnected state "
                                                                                "to connect\r\n");
                return;
        }

        status = ble_gap_connect(address, &cp);

        if (status != BLE_STATUS_OK) {
                printf("ERROR: connection failed\r\n");
                printf("\tStatus: 0x%02x\r\n", status);
                return;
        }

        printf("Connecting to %s ...\r\n", ble_address_to_string(address));
        hogp_host_state = HOGP_HOST_STATE_CONNECTING;
}

void hogp_connect_cancel(void)
{
        ble_error_t status;

        if (hogp_host_state != HOGP_HOST_STATE_CONNECTING) {
                printf("ERROR: device needs to be in connecting state to stop "
                                                                        "connecting\r\n");
                return;
        }

        status = ble_gap_connect_cancel();

        if (status != BLE_STATUS_OK) {
                printf("ERROR: connect can not be canceled (0x%02X)\r\n", status);
                return;
        }
}

void hogp_disconnect(void)
{
        switch (hogp_host_state) {
        case HOGP_HOST_STATE_CONNECTED:
                printf("Disconnecting...\r\n");
                ble_gap_disconnect(peer_info.conn_idx, BLE_HCI_ERROR_REMOTE_USER_TERM_CON);
                hogp_host_state = HOGP_HOST_STATE_DISCONNECTING;
                break;
        case HOGP_HOST_STATE_DISCONNECTING:
                printf("Disconnect in progress\r\n");
                break;
        default:
                printf("ERROR: device has to be connected to disconnect\r\n");
        }
}

void hogp_scan(bool start, bool scan_any)
{
        if (start) {
                scan_state.match_any = false;
                if (scan_any) {
                        scan_state.match_any = true;
                }

                if (hogp_host_state == HOGP_HOST_STATE_DISCONNECTED) {
                        ble_gap_scan_start(GAP_SCAN_ACTIVE, GAP_SCAN_OBSERVER_MODE, 48, 48, false,
                                                                                        false);
                        printf("Scanning...\r\n");
                        hogp_host_state = HOGP_HOST_STATE_SCANNING;
                        scan_state.num_devices = 0;
                } else {
                        printf("ERROR: device needs to be in idle state to start scanning\r\n");
                }
        } else {
                if (hogp_host_state == HOGP_HOST_STATE_SCANNING) {
                        ble_gap_scan_stop();
                        hogp_host_state = HOGP_HOST_STATE_DISCONNECTED;
                        printf("Scan stopping...\r\n");
                } else {
                        printf("ERROR: device needs to be in scanning state to stop "
                                                                                "scanning\r\n");
                }
        }
}

void hogp_show_devices(gap_device_filter_t filter)
{
        size_t i, length = BLE_GAP_MAX_CONNECTED;
        gap_device_t devices[BLE_GAP_MAX_CONNECTED];

        ble_gap_get_devices(filter, NULL, &length, devices);

        printf("%s devices (%u)\r\n", GAP_DEVICE_FILTER_BONDED == filter ? "Bonded" : "Connected",
                                                                                        length);

        for (i = 0; i < length; i++) {
                if (filter == GAP_DEVICE_FILTER_BONDED) {
                        printf("\tAddress: %s %s\r\n",
                                devices[i].address.addr_type == PUBLIC_ADDRESS ?
                                "public" : "private",
                                ble_address_to_string(&devices[i].address));
                } else {
                        printf("\tAddress: %s %s conn_idx: %d\r\n",
                                devices[i].address.addr_type == PUBLIC_ADDRESS ?
                                "public" : "private",
                                ble_address_to_string(&devices[i].address), devices[i].conn_idx);
                }
        }

}

static void print_unbond_info(ble_error_t status, bd_address_t *address)
{
        printf("Unbond device\r\n");
        printf("\tStatus: 0x%02x\r\n", status);
        printf("\tAddress: %s\r\n", address ? ble_address_to_string(address) : "not found");
}

void hogp_unbond_all(void)
{
        uint8_t i, length;
        bd_address_t *bonded_devices;

        ble_gap_get_bonded(&length, &bonded_devices);

        if (!length) {
                printf("Bonded devices not found!\r\n");
        }

        for (i = 0; i < length; i++) {
                hogp_unbond_by_address(&bonded_devices[i]);
        }

        OS_FREE(bonded_devices);
}

void hogp_unbond_by_address(bd_address_t *address)
{
        ble_error_t status;

        status = ble_gap_unpair(address);
        print_unbond_info(status, address);
}

void hogp_host_task(void *params)
{
        int8_t wdog_id;
        const gap_scan_params_t scan_params = CFG_SCAN_PARAMS;
        cli_t cli;

        /* register hogp_host task to be monitored by watchdog */
        wdog_id = sys_watchdog_register(false);

        ble_enable();
        ble_gap_role_set(GAP_CENTRAL_ROLE);
        ble_register_app();

        queue_init(&peer_info.clients);
        current_task = OS_GET_CURRENT_TASK();

        ble_gap_appearance_set(BLE_GAP_APPEARANCE_GENERIC_HID, ATT_PERM_READ);
        ble_gap_device_name_set("Dialog HOGP Host", ATT_PERM_READ);
        ble_gap_scan_params_set(&scan_params);

        cli = register_debug(CLI_NOTIF);

        printf("HOGP Host application started\r\n");

        for (;;) {
                OS_BASE_TYPE ret;
                uint32_t notif;

                /* notify watchdog on each loop */
                sys_watchdog_notify(wdog_id);

                /* suspend watchdog while blocking on OS_TASK_NOTIFY_WAIT() */
                sys_watchdog_suspend(wdog_id);

                /*
                 * Wait on any of the notification bits, then clear them all
                 */
                ret = OS_TASK_NOTIFY_WAIT(0, (uint32_t) -1, &notif, portMAX_DELAY);
                /* Guaranteed to succeed since we're waiting forever for a notification */
                OS_ASSERT(ret == OS_TASK_NOTIFY_SUCCESS);

                /* resume watchdog */
                sys_watchdog_notify_and_resume(wdog_id);

                /* notified from BLE manager, can get event */
                if (notif & BLE_APP_NOTIFY_MASK) {
                        ble_evt_hdr_t *hdr;

                        /*
                         * no need to wait for event, should be already there since we were notified
                         * from manager
                         */
                        hdr = ble_get_event(false);

                        if (!hdr) {
                                goto no_event;
                        }

                        ble_client_handle_event(hdr);

                        switch (hdr->evt_code) {
                        case BLE_EVT_GAP_CONNECTED:
                                handle_connected((ble_evt_gap_connected_t *) hdr);
                                break;
                        case BLE_EVT_GAP_CONNECTION_COMPLETED:
                                handle_evt_gap_connection_completed((ble_evt_gap_connection_completed_t *) hdr);
                                break;
                        case BLE_EVT_GAP_DISCONNECTED:
                                handle_disconnected((ble_evt_gap_disconnected_t *) hdr);
                                break;
                        case BLE_EVT_GAP_SECURITY_REQUEST:
                                handle_security_request((ble_evt_gap_security_request_t *) hdr);
                                break;
                        case BLE_EVT_GAP_SCAN_COMPLETED:
                                handle_evt_gap_scan_complt((ble_evt_gap_scan_completed_t*) hdr);
                                break;
                        case BLE_EVT_GAP_ADV_REPORT:
                                handle_evt_gap_adv_report((ble_evt_gap_adv_report_t *) hdr);
                                break;
                        case BLE_EVT_GATTC_BROWSE_SVC:
                                handle_gattc_browse_svc((ble_evt_gattc_browse_svc_t *) hdr);
                                break;
                        case BLE_EVT_GATTC_BROWSE_COMPLETED:
                                handle_gattc_browse_cmpl((ble_evt_gattc_browse_completed_t *) hdr);
                                break;
                        case BLE_EVT_GAP_SEC_LEVEL_CHANGED:
                                handle_gap_sec_level_changed((ble_evt_gap_sec_level_changed_t *) hdr);
                                break;
                        case BLE_EVT_GAP_SET_SEC_LEVEL_FAILED:
                                handle_gap_set_sec_level_failed((ble_evt_gap_set_sec_level_failed_t *) hdr);
                                break;
                        case BLE_EVT_GAP_PAIR_COMPLETED:
                                handle_gap_pair_completed((ble_evt_gap_pair_completed_t *) hdr);
                                break;
                        default:
                                ble_handle_event_default(hdr);
                                break;
                        }

                        /* Free event buffer (it's not needed anymore) */
                        OS_FREE(hdr);

no_event:
                        /*
                         * If there are more events waiting in queue, application should process
                         * them now.
                         */
                        if (ble_has_event()) {
                                OS_TASK_NOTIFY(OS_GET_CURRENT_TASK(), BLE_APP_NOTIFY_MASK, eSetBits);
                        }
                }

                if (notif & CLI_NOTIF) {
                        cli_handle_notified(cli);
                }
        }
}
