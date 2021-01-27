/**
 ****************************************************************************************
 *
 * @file gatts.c
 *
 * @brief BLE GATTS API implementation
 *
 * Copyright (C) 2016-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <inttypes.h>
#include <osal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ble_common.h"
#include "ble_gatts.h"
#include "ble_uuid.h"
#include "cli_utils.h"
#include "debug_utils.h"
#include "gatts.h"
#include "sdk_list.h"

#define VALUE_MAX_LEN 512

typedef enum {
        ATT_TYPE_INCLUDE,
        ATT_TYPE_CHARACTERISTIC,
        ATT_TYPE_DESCRIPTOR
} att_type_t;

struct att_list_elem {
        struct att_list_elem *next;
        att_type_t type;
};

typedef struct {
        struct att_list_elem hdr;
        uint16_t h_offset;
        uint16_t handle;
} include_t;

typedef struct {
        struct att_list_elem hdr;
        uint16_t h_offset;
        att_uuid_t uuid;
        gatt_prop_t prop;
        att_perm_t perm;
        uint16_t max_len;
        gatts_flag_t flags;
        uint16_t h_val_offset;
} characteristic_t;

typedef struct {
        struct att_list_elem hdr;
        uint16_t h_offset;
        att_uuid_t uuid;
        att_perm_t perm;
        gatts_flag_t flags;
        uint16_t max_len;
} descriptor_t;

__RETAINED static void *attributes_list;

static void print_attributes_list(uint16_t handle)
{
        int size = list_size(attributes_list);
        unsigned int i;

        print_parameter("Number of attributes: %d", size);

        for (i = 0; i < size; i++) {
                struct att_list_elem *element;

                element = (struct att_list_elem *) list_pop_back(&attributes_list);

                print_category("Attribute [%d]:", i);

                if (element->type == ATT_TYPE_INCLUDE) {
                        include_t *dptr = (include_t *) element;
                        print_parameter("Attribute type: Included Service");
                        print_parameter("Handle: 0x%04x", dptr->h_offset + handle);
                        print_parameter("Service Handle: 0x%04x", dptr->handle);
                } else if (element->type == ATT_TYPE_CHARACTERISTIC) {
                        characteristic_t *dptr = (characteristic_t *) element;
                        print_parameter("Attribute type: Characteristic");
                        print_parameter("Handle: 0x%04x", dptr->h_offset + handle);
                        print_parameter("UUID: %s", format_uuid(&(dptr->uuid)));
                        print_parameter("Properties: 0x%04x", dptr->prop);
                        print_parameter("Permissions: 0x%02x", dptr->perm);
                        print_parameter("Max length: %d", dptr->max_len);
                        print_parameter("Flags: 0x%02x",dptr->flags);
                        print_parameter("Value handle: 0x%04x", dptr->h_val_offset + handle);
                } else if (element->type == ATT_TYPE_DESCRIPTOR) {
                        descriptor_t *dptr = (descriptor_t *) element;
                        print_parameter("Attribute type: Descriptor");
                        print_parameter("Handle: 0x%04x", dptr->h_offset + handle);
                        print_parameter("UUID: %s", format_uuid(&(dptr->uuid)));
                        print_parameter("Permissions: 0x%02x", dptr->perm);
                        print_parameter("Max length: %d", dptr->max_len);
                        print_parameter("Flags: 0x%02x",dptr->flags);
                }

                OS_FREE(element);
        }
}

/*
 * Handler for gatts_add_service call.
 * Prints status returned by call.
 */
static bool gatts_add_service(int argc, const char **argv)
{
        ble_error_t status;
        att_uuid_t uuid;
        gatt_service_t type;
        uint16_t num_attrs;

        if (argc < 5) {
                return false;
        }

        if (!ble_uuid_from_string(argv[2], &uuid)) {
                return false;
        }

        if (strcmp(argv[3], "primary") == 0) {
                type = GATT_SERVICE_PRIMARY;
        } else if (strcmp(argv[3], "secondary") == 0) {
                type = GATT_SERVICE_SECONDARY;
        } else {
                return false;
        }

        if (!parse_u16(argv[4], &num_attrs)) {
                return false;
        }

        status = ble_gatts_add_service(&uuid, type, num_attrs);

        print_command("ble_gatts_add_service ( %s, %s, %d )", argv[2], argv[3], num_attrs);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for gatts_add_include call.
 * Prints status returned by call
 */
static bool gatts_add_include(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t handle;
        uint16_t h_offset;

        if (argc < 3) {
                return false;
        }

        if (!parse_u16(argv[2], &handle)) {
                return false;
        }

        status = ble_gatts_add_include(handle, &h_offset);

        print_command("ble_gatts_add_include ( 0x%04x )", handle);
        if (status == BLE_STATUS_OK) {
                include_t *dptr = OS_MALLOC(sizeof(*dptr));

                dptr->hdr.type = ATT_TYPE_INCLUDE;
                dptr->handle = handle;
                dptr->h_offset = h_offset;
                list_add(&attributes_list, (struct att_list_elem *) dptr);

                print_parameter("Handle offset: 0x%04x", h_offset);
        }
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for gatts_add_characteristic call.
 * Prints status returned by call
 */
static bool gatts_add_characteristic(int argc, const char **argv)
{
        ble_error_t status;
        att_uuid_t uuid;
        gatt_prop_t prop;
        att_perm_t perm;
        uint16_t max_len;
        gatts_flag_t flags;
        uint16_t h_offset;
        uint16_t h_val_offset;

        if (argc < 7) {
                return false;
        }

        if (!ble_uuid_from_string(argv[2], &uuid)) {
                return false;
        }


        if (!parse_u16(argv[3], &prop)) {
                return false;
        }

        if (!parse_u8(argv[4], &perm)) {
                return false;
        }

        if (!parse_u16(argv[5], &max_len)) {
                return false;
        }

        if (!parse_u8(argv[6], &flags)) {
                return false;
        }

        status = ble_gatts_add_characteristic(&uuid, prop, perm, max_len, flags, &h_offset,
                                                                                &h_val_offset);

        print_command("ble_gatts_add_characteristic ( %s, 0x%04x, 0x%02x, %d, 0x%02x )", argv[2],
                                                                     prop, perm, max_len, flags);
        if (status == BLE_STATUS_OK) {
                characteristic_t *dptr = OS_MALLOC(sizeof(*dptr));

                dptr->hdr.type = ATT_TYPE_CHARACTERISTIC;
                dptr->h_offset = h_offset;
                dptr->uuid = uuid;
                dptr->prop = prop;
                dptr->perm = perm;
                dptr->max_len = max_len;
                dptr->flags = flags;
                dptr->h_val_offset = h_val_offset;
                list_add(&attributes_list, (struct att_list_elem *) dptr);

                print_parameter("Handle offset: 0x%04x", h_offset);
                print_parameter("Value offset: 0x%04x", h_val_offset);
        }
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for gatts_add_descriptor call.
 * Prints status returned by call
 */
static bool gatts_add_descriptor(int argc, const char **argv)
{
        ble_error_t status;
        att_uuid_t uuid;
        att_perm_t perm;
        uint16_t max_len;
        gatts_flag_t flags;
        uint16_t h_offset;
        uint8_t buf;

        if (argc < 6) {
                return false;
        }

        if (!ble_uuid_from_string(argv[2], &uuid)) {
                return false;
        }

        if (!parse_u8(argv[3], &buf)) {
                return false;
        }

        perm = buf;

        if (!parse_u16(argv[4], &max_len)) {
                return false;
        }

        if (!parse_u8(argv[5], &buf)) {
                return false;
        }

        flags = buf;

        status = ble_gatts_add_descriptor(&uuid, perm, max_len, flags, &h_offset);

        print_command("ble_gatts_add_descriptor ( %s, 0x%02x, %d, 0x%02x )", argv[2], perm, max_len,
                                                                                        flags);
        if (status == BLE_STATUS_OK) {
                descriptor_t *dptr = OS_MALLOC(sizeof(*dptr));

                dptr->hdr.type = ATT_TYPE_DESCRIPTOR;
                dptr->h_offset = h_offset;
                dptr->uuid = uuid;
                dptr->perm = perm;
                dptr->max_len = max_len;
                dptr->flags = flags;
                list_add(&attributes_list, (struct att_list_elem *) dptr);

                print_parameter("Handle offset: 0x%04x", h_offset);
        }
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for gatts_register_service call.
 * Prints status returned by call
 */
static bool gatts_register_service(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t handle;

        status = ble_gatts_register_service(&handle);

        print_command("ble_gatts_register_service ( )");
        if (status == BLE_STATUS_OK) {
                print_parameter("Service handle: 0x%04x", handle);
                print_attributes_list(handle);
        }
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for gatts_enable_service call.
 * Prints status returned by call
 */
static bool gatts_enable_service(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t handle;

        if (argc < 3) {
                return false;
        }

        if (!parse_u16(argv[2], &handle)) {
                return false;
        }

        status = ble_gatts_enable_service(handle);

        print_command("ble_gatts_enable_service ( 0x%04x )", handle);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for gatts_disable_service call.
 * Prints status returned by call
 */
static bool gatts_disable_service(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t handle;

        if (argc < 3) {
                return false;
        }

        if (!parse_u16(argv[2], &handle)) {
                return false;
        }

        status = ble_gatts_disable_service(handle);

        print_command("ble_gatts_disable_service ( 0x%04x )", handle);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for gatts_get_characteristic_prop.
 * Prints status returned by call
 */
static bool gatts_get_characteristic_prop(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t handle;
        gatt_prop_t prop;
        att_perm_t perm;

        if (argc < 3) {
                return false;
        }

        if (!parse_u16(argv[2], &handle)) {
                return false;
        }

        status = ble_gatts_get_characteristic_prop(handle, &prop, &perm);

        print_command("ble_gatts_get_characteristic_prop ( 0x%04x )", handle);
        if (status == BLE_STATUS_OK) {
                print_parameter("Properties: 0x%04x", prop);
                print_parameter("Permissions: 0x%02x", perm);
        }
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for gatts_set_characteristic_prop.
 * Prints status returned by call
 */
static bool gatts_set_characteristic_prop(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t handle;
        gatt_prop_t prop;
        att_perm_t perm;

        if (argc < 5) {
                return false;
        }

        if (!parse_u16(argv[2], &handle)) {
                return false;
        }

        if (!parse_u16(argv[3], &prop)) {
                return false;
        }

        if (!parse_u8(argv[4], &perm)) {
                return false;
        }

        status = ble_gatts_set_characteristic_prop(handle, prop, perm);

        print_command("ble_gatts_set_characteristic_prop ( 0x%04x, 0x%04x, 0x%02x )", handle, prop,
                                                                                        perm);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for gatts_get_value.
 * Prints status returned by call
 */
static bool gatts_get_value(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t handle;
        uint8_t value_buffer[VALUE_MAX_LEN];
        uint16_t length;

        if (argc < 3) {
                return false;
        }

        if (!parse_u16(argv[2], &handle)) {
                return false;
        }

        length = VALUE_MAX_LEN;

        status = ble_gatts_get_value(handle, &length, value_buffer);

        print_command("ble_gatts_get_value ( 0x%04x )", handle);
        if (status == BLE_STATUS_OK) {
                print_parameter("Length: %d", length);
                print_data_parameter(value_buffer, length);
        }
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for gatts_set_value.
 * Prints status returned by call
 */
static bool gatts_set_value(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t handle;
        uint8_t value_buffer[VALUE_MAX_LEN];
        int length;

        if (argc < 4) {
                return false;
        }

        if (!parse_u16(argv[2], &handle)) {
                return false;
        }

        length = str_to_hex(argv[3], value_buffer, VALUE_MAX_LEN);

        if (length == -1) {
                return false;
        }

        status = ble_gatts_set_value(handle, length, value_buffer);

        print_command("ble_gatts_set_value ( 0x%04x, %d, %s )", handle, length, argv[3]);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for gatts_read_cfm.
 * Prints status returned by call
 */
static bool gatts_read_cfm(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t conn_idx;
        uint16_t handle;
        att_error_t att_status;
        int length;
        uint8_t value_buffer[VALUE_MAX_LEN];
        uint8_t buf;

        if (argc < 6) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        if (!parse_u16(argv[3], &handle)) {
                return false;
        }

        if (!parse_u8(argv[4], &buf)) {
                return false;
        }

        att_status = buf;

        length = str_to_hex(argv[5], value_buffer, VALUE_MAX_LEN);

        if (length == -1) {
                return false;
        }

        status = ble_gatts_read_cfm(conn_idx, handle, att_status, length, value_buffer);

        print_command("ble_gatts_read_cfm ( %d, 0x%04x, 0x%02x, %d, %s )", conn_idx, handle,
                                                                att_status, length, argv[5]);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for gatts_write_cfm.
 * Prints status returned by call
 */
static bool gatts_write_cfm(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t conn_idx;
        uint16_t handle;
        att_error_t att_status;
        uint8_t buf;

        if (argc < 5) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        if (!parse_u16(argv[3], &handle)) {
                return false;
        }

        if (!parse_u8(argv[4], &buf)) {
                return false;
        }

        att_status = buf;

        status = ble_gatts_write_cfm(conn_idx, handle, att_status);

        print_command("ble_gatts_write_cfm ( %d, 0x%04x, 0x%02x )", conn_idx, handle, att_status);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for gatts_prepare_write_cfm.
 * Prints status returned by call
 */
static bool gatts_prepare_write_cfm(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t conn_idx;
        uint16_t handle;
        uint16_t length;
        att_error_t att_status;
        uint8_t buf;

        if (argc < 6) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        if (!parse_u16(argv[3], &handle)) {
                return false;
        }

        if (!parse_u16(argv[4], &length)) {
                return false;
        }

        if (!parse_u8(argv[5], &buf)) {
                return false;
        }

        att_status = buf;

        status = ble_gatts_prepare_write_cfm(conn_idx, handle, length, att_status);

        print_command("ble_gatts_prepare_write_cfm ( %d, 0x%04x, %d, 0x%02x )", conn_idx, handle,
                                                                                length, att_status);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for gatts_send_event.
 * Prints status returned by call
 */
static bool gatts_send_event(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t conn_idx;
        uint16_t handle;
        gatt_event_t type;
        int length;
        uint8_t value_buffer[VALUE_MAX_LEN];

        if (argc < 6) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        if (!parse_u16(argv[3], &handle)) {
                return false;
        }

        if (strcmp(argv[4], "notification") == 0) {
                type = GATT_EVENT_NOTIFICATION;
        } else if (strcmp(argv[4], "indication") == 0) {
                type = GATT_EVENT_INDICATION;
        } else {
                return false;
        }

        length = str_to_hex(argv[5], value_buffer, VALUE_MAX_LEN);

        if (length == -1) {
                return false;
        }

        status = ble_gatts_send_event(conn_idx, handle, type, length, value_buffer);

        print_command("ble_gatts_send_event ( %d, 0x%04x, %s, %d, %s )", conn_idx, handle, argv[4], length, argv[5]);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for gatts_service_changed_ind.
 * Prints status returned by call
 */
static bool gatts_service_changed_ind(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t conn_idx;
        uint16_t start_handle;
        uint16_t end_handle;

        if (argc < 5) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        if (!parse_u16(argv[3], &start_handle)) {
                return false;
        }

        if (!parse_u16(argv[4], &end_handle)) {
                return false;
        }

        status = ble_gatts_service_changed_ind(conn_idx, start_handle, end_handle);

        print_command("ble_gatts_service_changed_ind ( %d, 0x%04x, 0x%04x )", conn_idx,
                                                                        start_handle, end_handle);
        print_status("return: %s", get_status(status));

        return true;
}

static const debug_handler_t gatts_handlers[] = {
        { "add_service", "<uuid> <primary|secondary> <attr_number>", gatts_add_service },
        { "add_include", "<handle>", gatts_add_include },
        { "add_characteristic", "<uuid> <properties> <permissions> <max_len> <flags>",
                                                                        gatts_add_characteristic },
        { "add_descriptor", "<uuid> <permissions> <max_len> <flags>", gatts_add_descriptor },
        { "register_service", "", gatts_register_service },
        { "enable_service", "<handle>", gatts_enable_service },
        { "disable_service", "<handle>", gatts_disable_service },
        { "get_characteristic_prop", "<handle>", gatts_get_characteristic_prop },
        { "set_characteristic_prop", "<handle> <properties> <permissions>",
                                                                gatts_set_characteristic_prop },
        { "get_value", "<handle>", gatts_get_value },
        { "set_value", "<handle> <value>", gatts_set_value },
        { "read_cfm", "<conn_idx> <handle> <status> <value>", gatts_read_cfm },
        { "write_cfm", "<conn_idx> <handle> <status>", gatts_write_cfm },
        { "prepare_write_cfm", "<conn_idx> <handle> <length> <status>", gatts_prepare_write_cfm },
        { "send_event", "<conn_idx> <handle> <notification|indication> <value>", gatts_send_event },
        { "service_changed_ind", "<conn_idx> <start_handle> <end_handle>",
                                                                        gatts_service_changed_ind },
        { NULL }
};

void gatts_command(int argc, const char *argv[], void *user_data)
{
        debug_handle_message(argc, argv, gatts_handlers);
}

static void handle_ble_evt_gatts_read_req(ble_evt_gatts_read_req_t *info)
{
        print_event("BLE_EVT_GATTS_READ_REQ");
        print_parameter("Connection index: %d", info->conn_idx);
        print_parameter("Attribute handle: 0x%04x", info->handle);
        print_parameter("Attribute value offset: 0x%04x", info->offset);
}

static void handle_ble_evt_gatts_write_req(ble_evt_gatts_write_req_t *info)
{
        print_event("BLE_EVT_GATTS_WRITE_REQ");
        print_parameter("Connection index: %d", info->conn_idx);
        print_parameter("Attribute handle: 0x%04x", info->handle);
        print_parameter("Attribute value offset: 0x%04x", info->offset);
        print_parameter("Attribute value length: %d", info->length);
        print_data_parameter(info->value, info->length);
}

static void handle_ble_evt_gatts_prepare_write_req(ble_evt_gatts_prepare_write_req_t *info)
{
        print_event("BLE_EVT_GATTS_PREPARE_WRITE_REQ");
        print_parameter("Connection index: %d", info->conn_idx);
        print_parameter("Attribute handle: 0x%04x", info->handle);
}

static void handle_ble_evt_gatts_event_sent(ble_evt_gatts_event_sent_t *info)
{
        print_event("BLE_EVT_GATTS_EVENT_SENT");
        print_parameter("Connection index: %d", info->conn_idx);
        print_parameter("Attribute handle: 0x%04x", info->handle);
        print_parameter("Event type: %s", info->type == GATT_EVENT_INDICATION ? "indication" :
                                                                                "notification");
        print_parameter("Event status: %d", info->status);
}

bool gatts_handle_event(ble_evt_hdr_t *event)
{
        switch (event->evt_code) {
        case BLE_EVT_GATTS_READ_REQ:
                handle_ble_evt_gatts_read_req((ble_evt_gatts_read_req_t *) event);
                break;
        case BLE_EVT_GATTS_WRITE_REQ:
                handle_ble_evt_gatts_write_req((ble_evt_gatts_write_req_t *) event);
                break;
        case BLE_EVT_GATTS_PREPARE_WRITE_REQ:
                handle_ble_evt_gatts_prepare_write_req((ble_evt_gatts_prepare_write_req_t *) event);
                break;
        case BLE_EVT_GATTS_EVENT_SENT:
                handle_ble_evt_gatts_event_sent((ble_evt_gatts_event_sent_t *) event);
                break;
        default:
                return false;
        }

        return true;
}
