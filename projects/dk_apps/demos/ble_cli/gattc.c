/**
 ****************************************************************************************
 *
 * @file gattc.c
 *
 * @brief BLE GATTC API implementation
 *
 * Copyright (C) 2016-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include "ble_common.h"
#include "ble_gap.h"
#include "ble_gattc.h"
#include "ble_uuid.h"
#include "cli_utils.h"
#include "debug_utils.h"
#include "gattc.h"

#define MAX_VALUE_LENGTH (512)

static bool gattc_browse(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t conn_idx;

        if (argc < 3) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        /* Parse optional UUID */
        if (argc > 3) {
                att_uuid_t uuid;

                if (!ble_uuid_from_string(argv[3], &uuid)) {
                        return false;
                }

                status = ble_gattc_browse(conn_idx, &uuid);
        } else {
                status = ble_gattc_browse(conn_idx, NULL);
        }

        print_command("ble_gattc_browse ( %d, %s )", conn_idx, argc > 3 ? argv[3] : "NULL");
        print_status("return: %s", get_status(status));

        return true;
}

static bool gattc_browse_range(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t conn_idx;
        uint16_t start_h;
        uint16_t end_h;

        if (argc < 5) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        if (!parse_u16(argv[3], &start_h)) {
                return false;
        }

        if (!parse_u16(argv[4], &end_h)) {
                return false;
        }

        /* Parse optional UUID */
        if (argc > 5) {
                att_uuid_t uuid;

                if (!ble_uuid_from_string(argv[5], &uuid)) {
                        return false;
                }

                status = ble_gattc_browse_range(conn_idx, start_h, end_h, &uuid);
        } else {
                status = ble_gattc_browse_range(conn_idx, start_h, end_h, NULL);
        }

        print_command("ble_gattc_browse_range ( %d, 0x%04x, 0x%04x, %s )", conn_idx, start_h,
                                                                end_h, argc > 5 ? argv[5] : "NULL");
        print_status("return: %s", get_status(status));

        return true;
}

static bool gattc_discover_svc(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t conn_idx;

        if (argc < 3) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        /* Parse optional UUID */
        if (argc > 3) {
                att_uuid_t uuid;

                if (!ble_uuid_from_string(argv[3], &uuid)) {
                        return false;
                }

                status = ble_gattc_discover_svc(conn_idx, &uuid);
        } else {
                status = ble_gattc_discover_svc(conn_idx, NULL);
        }

        print_command("ble_gattc_discover_svc ( %d, %s )", conn_idx, argc > 3 ? argv[3] : "NULL");
        print_status("return: %s", get_status(status));

        return true;
}

static bool gattc_discover_include(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t conn_idx, start_h, end_h;

        if (argc < 5) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        if (!parse_u16(argv[3], &start_h)) {
                return false;
        }

        if (!parse_u16(argv[4], &end_h)) {
                return false;
        }

        status = ble_gattc_discover_include(conn_idx, start_h, end_h);
        print_command("ble_gattc_discover_include ( %d, 0x%04X, 0x%04X )", conn_idx, start_h, end_h);
        print_status("return: %s", get_status(status));

        return true;
}

static bool gattc_discover_desc(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t conn_idx, start_h, end_h;

        if (argc < 5) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        if (!parse_u16(argv[3], &start_h)) {
                return false;
        }

        if (!parse_u16(argv[4], &end_h)) {
                return false;
        }

        status = ble_gattc_discover_desc(conn_idx, start_h, end_h);
        print_command("ble_gattc_discover_desc ( %d, 0x%04X, 0x%04X )", conn_idx, start_h, end_h);
        print_status("return: %s", get_status(status));

        return true;
}

static bool gattc_discover_char(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t conn_idx, start_h, end_h;

        if (argc < 5) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        if (!parse_u16(argv[3], &start_h)) {
                return false;
        }

        if (!parse_u16(argv[4], &end_h)) {
                return false;
        }

        /* Parse optional UUID */
        if (argc > 5) {
                att_uuid_t uuid;

                if (!ble_uuid_from_string(argv[5], &uuid)) {
                        return false;
                }

                status = ble_gattc_discover_char(conn_idx, start_h, end_h, &uuid);
        } else {
                status = ble_gattc_discover_char(conn_idx, start_h, end_h, NULL);
        }

        print_command("ble_gattc_discover_char ( %d, 0x%04X, 0x%04X, %s )", conn_idx, start_h,
                                                        end_h, argc > 5 ? argv[5] : "NULL");
        print_status("return: %s", get_status(status));

        return true;
}

static bool gattc_read(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t conn_idx, handle, offset = 0;

        if (argc < 4) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        if (!parse_u16(argv[3], &handle)) {
                return false;
        }

        if (argc > 4) {
                if (!parse_u16(argv[4], &offset)) {
                        return false;
                }
        }

        status = ble_gattc_read(conn_idx, handle, offset);
        print_command("ble_gattc_read ( %d, 0x%04X, 0x%04X )", conn_idx, handle, offset);
        print_status("return: %s", get_status(status));

        return true;
}

typedef enum {
        WRITE_TYPE_REQ,
        WRITE_TYPE_NO_RESP,
        WRITE_TYPE_PREPARE,
} write_type_t;

static bool write(write_type_t type, int argc, const char **argv)
{
        static uint8_t data[MAX_VALUE_LENGTH];
        uint16_t conn_idx = BLE_CONN_IDX_INVALID;
        uint16_t handle, offset;
        ble_error_t status;
        int length = 0;

        if (argc < 5) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        if (!parse_u16(argv[3], &handle)) {
                return false;
        }

        if (!parse_u16(argv[4], &offset)) {
                return false;
        }

        if (argc > 5) {
                length = str_to_hex(argv[5], data, sizeof(data));
        }

        if (length < 0) {
                return false;
        }

        switch (type) {
        case WRITE_TYPE_REQ:
                status = ble_gattc_write(conn_idx, handle, offset, length,
                                                                        length > 0 ? data : NULL);
                print_command("ble_gattc_write ( %d, 0x%04X, 0x%04X, 0x%04X, %s )", conn_idx,
                                        handle, offset, length, length > 0 ? argv[5] : "NULL");
                break;
        case WRITE_TYPE_NO_RESP:
                status = ble_gattc_write_no_resp(conn_idx, handle, offset, length,
                                                                        length > 0 ? data : NULL);
                print_command("ble_gattc_write_no_resp ( %d, 0x%04X, 0x%04X, 0x%04X, %s )",
                                conn_idx, handle, offset, length, length > 0 ? argv[5] : "NULL");
                break;
        case WRITE_TYPE_PREPARE:
                status = ble_gattc_write_prepare(conn_idx, handle, offset, length,
                                                                        length > 0 ? data : NULL);
                print_command("ble_gattc_write_prepare ( %d, 0x%04X, 0x%04X, 0x%04X, %s )",
                                conn_idx, handle, offset, length, length > 0 ? argv[5] : "NULL");
                break;
        default:
                return false;
        }

        print_status("return: %s", get_status(status));

        return true;
}

static bool gattc_write(int argc, const char **argv)
{
        return write(WRITE_TYPE_REQ, argc, argv);
}

static bool gattc_write_no_resp(int argc, const char **argv)
{
        return write(WRITE_TYPE_NO_RESP, argc, argv);
}

static bool gattc_write_prepare(int argc, const char **argv)
{
        return write(WRITE_TYPE_PREPARE, argc, argv);
}

static bool gattc_write_execute(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t conn_idx;
        bool commit;

        if (argc < 4) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        if (!parse_bool(argv[3], &commit)) {
                return false;
        }

        status = ble_gattc_write_execute(conn_idx, commit);
        print_command("ble_gattc_write_execute ( %d, %s )", conn_idx, commit ? "true" : "false");
        print_status("return: %s", get_status(status));

        return true;
}

static bool gattc_get_mtu(int argc, const char **argv)
{
        uint16_t conn_idx, mtu = 0;
        ble_error_t status;

        if (argc < 3) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        status = ble_gattc_get_mtu(conn_idx, &mtu);
        print_command("ble_gattc_get_mtu ( %d )", conn_idx);
        if (status == BLE_STATUS_OK) {
                print_parameter("MTU: %d", mtu);
        }
        print_status("return: %s", get_status(status));

        return true;
}

static bool gattc_exchange_mtu(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t conn_idx;

        if (argc < 3) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        status = ble_gattc_exchange_mtu(conn_idx);
        print_command("ble_gattc_exchange_mtu ( %d )", conn_idx);
        print_status("return: %s", get_status(status));

        return true;
}

static const debug_handler_t gattc_handlers[] = {
        { "browse", "<conn_idx> [uuid]", gattc_browse },
        { "browse_range", "<conn_idx> <start_h> <end_h> [uuid]", gattc_browse_range },
        { "discover_svc", "<conn_idx> [uuid]", gattc_discover_svc },
        { "discover_include", "<conn_idx> <start_h> <end_h>", gattc_discover_include },
        { "discover_desc", "<conn_idx> <start_h> <end_h>", gattc_discover_desc },
        { "discover_char", "<conn_idx> <start_h> <end_h> [uuid]", gattc_discover_char },
        { "read", "<conn_idx> <handle> [offset]", gattc_read },
        { "write", "<conn_idx> <handle> <offset> [data]", gattc_write },
        { "write_no_resp", "<conn_idx> <handle> <signed> [data]", gattc_write_no_resp },
        { "write_prepare", "<conn_idx> <handle> <offset> [data]", gattc_write_prepare },
        { "write_execute", "<conn_idx> <commit>", gattc_write_execute },
        { "get_mtu", "<conn_idx>", gattc_get_mtu },
        { "exchange_mtu", "<conn_idx>", gattc_exchange_mtu },
        { NULL },
};

void gattc_command(int argc, const char *argv[], void *user_data)
{
        debug_handle_message(argc, argv, gattc_handlers);
}

static void print_gattc_item(const gattc_item_t *item)
{
        switch (item->type) {
        case GATTC_ITEM_TYPE_INCLUDE:
                print_category("Item type: Included Service");
                print_parameter("Handle: 0x%04X", item->handle);
                print_parameter("UUID: %s", format_uuid(&item->uuid));
                print_parameter("Start handle: 0x%04X", item->i.start_h);
                print_parameter("End handle: 0x%04X", item->i.end_h);
                break;
        case GATTC_ITEM_TYPE_CHARACTERISTIC:
                print_category("Item type: Characteristic");
                print_parameter("Handle: 0x%04X", item->handle);
                print_parameter("UUID: %s", format_uuid(&item->uuid));
                print_parameter("Value handle: 0x%04X", item->c.value_handle);
                print_parameter("Properties: %d", item->c.properties);
                break;
        case GATTC_ITEM_TYPE_DESCRIPTOR:
                print_category("Item type: Descriptor");
                print_parameter("Handle: 0x%04X", item->handle);
                print_parameter("UUID: %s", format_uuid(&item->uuid));
                break;
        case GATTC_ITEM_TYPE_NONE:
        default:
                print_category("Item type: Unknown");
                break;
        }
}

static void handle_ble_evt_gattc_browse_svc(ble_evt_gattc_browse_svc_t *evt)
{
        uint16_t i;

        print_event("BLE_EVT_GATTC_BROWSE_SVC");
        print_parameter("Connection index: %d", evt->conn_idx);
        print_parameter("Service UUID: %s", format_uuid(&evt->uuid));
        print_parameter("Start handle: 0x%04X", evt->start_h);
        print_parameter("End handle: 0x%04X", evt->end_h);
        print_parameter("Number of items: %d", evt->num_items);

        for (i = 0; i < evt->num_items; i++) {
                print_gattc_item(&evt->items[i]);
        }
}

static void handle_ble_evt_gattc_browse_completed(ble_evt_gattc_browse_completed_t *evt)
{
        print_event("BLE_EVT_GATTC_BROWSE_COMPLETED");
        print_parameter("Connection index: %d", evt->conn_idx);
        print_parameter("Status: 0x%02X", evt->status);
}

static void handle_ble_evt_gattc_discover_svc(ble_evt_gattc_discover_svc_t *evt)
{
        print_event("BLE_EVT_GATTC_DISCOVER_SVC");
        print_parameter("Connection index: %d", evt->conn_idx);
        print_parameter("Service UUID: %s", format_uuid(&evt->uuid));
        print_parameter("Start handle: 0x%04X", evt->start_h);
        print_parameter("End handle: 0x%04X", evt->end_h);
}

static void handle_ble_evt_gattc_discover_completed(ble_evt_gattc_discover_completed_t *evt)
{
        print_event("BLE_EVT_GATTC_DISCOVER_COMPLETED");
        print_parameter("Connection index: %d", evt->conn_idx);
        print_parameter("Status: 0x%02X", evt->status);

        switch (evt->type) {
        case GATTC_DISCOVERY_TYPE_SVC:
                print_parameter("Discovery type: GATTC_DISCOVERY_TYPE_SVC");
                break;
        case GATTC_DISCOVERY_TYPE_INCLUDED:
                print_parameter("Discovery type: GATTC_DISCOVERY_TYPE_INCLUDED");
                break;
        case GATTC_DISCOVERY_TYPE_CHARACTERISTICS:
                print_parameter("Discovery type: GATTC_DISCOVERY_TYPE_CHARACTERISTICS");
                break;
        case GATTC_DISCOVERY_TYPE_DESCRIPTORS:
                print_parameter("Discovery type: GATTC_DISCOVERY_TYPE_DESCRIPTORS");
                break;
        default:
                break;
        }
}

static void handle_ble_evt_gattc_discover_include(ble_evt_gattc_discover_include_t *evt)
{
        print_event("BLE_EVT_GATTC_DISCOVER_INCLUDE");
        print_parameter("Connection index: %d", evt->conn_idx);
        print_parameter("Handle: 0x%04X", evt->handle);
        print_parameter("Service UUID: %s", format_uuid(&evt->uuid));
        print_parameter("Start handle: 0x%04X", evt->start_h);
        print_parameter("End handle: 0x%04X", evt->end_h);
}

static void handle_ble_evt_gattc_discover_desc(ble_evt_gattc_discover_desc_t *evt)
{
        print_event("BLE_EVT_GATTC_DISCOVER_DESC");
        print_parameter("Connection index: %d", evt->conn_idx);
        print_parameter("Handle: 0x%04X", evt->handle);
        print_parameter("UUID: %s", format_uuid(&evt->uuid));
}

static void handle_ble_evt_gattc_discover_char(ble_evt_gattc_discover_char_t *evt)
{
        print_event("BLE_EVT_GATTC_DISCOVER_CHAR");
        print_parameter("Connection index: %d", evt->conn_idx);
        print_parameter("UUID: %s", format_uuid(&evt->uuid));
        print_parameter("Handle: 0x%04X", evt->handle);
        print_parameter("Value handle: 0x%04X", evt->value_handle);
        print_parameter("Properties: 0x%02X", evt->properties);
}

static void handle_ble_evt_gattc_read_completed(ble_evt_gattc_read_completed_t * evt)
{
        print_event("BLE_EVT_GATTC_READ_COMPLETED");
        print_parameter("Connection index: %d", evt->conn_idx);
        print_parameter("Handle: 0x%04X", evt->handle);
        print_parameter("Status: 0x%02X", evt->status);
        print_parameter("Offset: 0x%04X", evt->offset);
        print_parameter("Length: 0x%04X", evt->length);
        print_data_parameter(evt->value, evt->length);
}

static void handle_ble_evt_gattc_write_completed(ble_evt_gattc_write_completed_t * evt)
{
        print_event("BLE_EVT_GATTC_WRITE_COMPLETED");
        print_parameter("Connection index: %d", evt->conn_idx);
        print_parameter("Handle: 0x%04X", evt->handle);
        print_parameter("Status: 0x%02X", evt->status);
        print_parameter("Operation: 0x%02X", evt->operation);
}

static void handle_ble_evt_gattc_mtu_changed(ble_evt_gattc_mtu_changed_t * evt)
{
        print_event("BLE_EVT_GATTC_MTU_CHANGED");
        print_parameter("Connection index: %d", evt->conn_idx);
        print_parameter("MTU: %d", evt->mtu);
}

static void handle_ble_evt_gattc_notification(ble_evt_gattc_notification_t * evt)
{
        print_event("BLE_EVT_GATTC_NOTIFICATION");
        print_parameter("Connection index: %d", evt->conn_idx);
        print_parameter("Handle: 0x%04X", evt->handle);
        print_parameter("Length: 0x%04X", evt->length);
        print_data_parameter(evt->value, evt->length);
}

static void handle_ble_evt_gattc_indication(ble_evt_gattc_indication_t * evt)
{
        print_event("BLE_EVT_GATTC_INDICATION");
        print_parameter("Connection index: %d", evt->conn_idx);
        print_parameter("Handle: 0x%04X", evt->handle);
        print_parameter("Length: 0x%04X", evt->length);
        print_data_parameter(evt->value, evt->length);
}

bool gattc_handle_event(ble_evt_hdr_t *event)
{
        switch (event->evt_code) {
        case BLE_EVT_GATTC_BROWSE_SVC:
                handle_ble_evt_gattc_browse_svc((ble_evt_gattc_browse_svc_t *) event);
                break;
        case BLE_EVT_GATTC_BROWSE_COMPLETED:
                handle_ble_evt_gattc_browse_completed((ble_evt_gattc_browse_completed_t *) event);
                break;
        case BLE_EVT_GATTC_DISCOVER_SVC:
                handle_ble_evt_gattc_discover_svc((ble_evt_gattc_discover_svc_t *) event);
                break;
        case BLE_EVT_GATTC_DISCOVER_COMPLETED:
                handle_ble_evt_gattc_discover_completed(
                                                (ble_evt_gattc_discover_completed_t *) event);
                break;
        case BLE_EVT_GATTC_DISCOVER_INCLUDE:
                handle_ble_evt_gattc_discover_include((ble_evt_gattc_discover_include_t *) event);
                break;
        case BLE_EVT_GATTC_DISCOVER_DESC:
                handle_ble_evt_gattc_discover_desc((ble_evt_gattc_discover_desc_t *) event);
                break;
        case BLE_EVT_GATTC_DISCOVER_CHAR:
                handle_ble_evt_gattc_discover_char((ble_evt_gattc_discover_char_t *) event);
                break;
        case BLE_EVT_GATTC_READ_COMPLETED:
                handle_ble_evt_gattc_read_completed((ble_evt_gattc_read_completed_t *) event);
                break;
        case BLE_EVT_GATTC_WRITE_COMPLETED:
                handle_ble_evt_gattc_write_completed((ble_evt_gattc_write_completed_t *) event);
                break;
        case BLE_EVT_GATTC_MTU_CHANGED:
                handle_ble_evt_gattc_mtu_changed((ble_evt_gattc_mtu_changed_t *) event);
                break;
        case BLE_EVT_GATTC_NOTIFICATION:
                handle_ble_evt_gattc_notification((ble_evt_gattc_notification_t *) event);
                break;
        case BLE_EVT_GATTC_INDICATION:
                handle_ble_evt_gattc_indication((ble_evt_gattc_indication_t *) event);
                break;
        default:
                return false;
        }

        return true;
}
