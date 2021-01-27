/**
 ****************************************************************************************
 *
 * @file debug.c
 *
 * @brief Debug utilities
 *
 * Copyright (C) 2015-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include "osal.h"
#include "ble_client.h"
#include "cli_utils.h"
#include "hids_client.h"
#include "scps_client.h"
#include "hogp_host_task.h"
#include "hogp_host_config.h"
#include "debug.h"

void hogp_connect_usage(void);

typedef void (* debug_callback_t) (hogp_client_t *client, int argc, const char **argv);

/**
 * Get Protocol debug command handler.
 */
static void hogp_get_protocol_cb(hogp_client_t *client, int argc, const char **argv)
{
        bool status;

        if (argc != 1) {
                printf("usage: get_protocol <hid_client_id>\r\n");
                return;
        }

        printf("Get protocol mode request\r\n");
        status = hids_client_get_protocol_mode(client->client);
        printf("\tStatus: %s\r\n", status ? "success" : "failure");
}

/**
 * Read Boot debug command handler.
 */
static void hogp_boot_read_cb(hogp_client_t *client, int argc, const char **argv)
{
        hids_client_boot_report_type type;
        bool status;

        if (argc != 2) {
                printf("usage: boot_read <hid_client_id> <boot_report_type>\r\n");
                return;
        }

        if (!parse_u8(argv[1], &type)) {
                printf("ERROR: invalid report type\r\n");
                return;
        }

        printf("Boot report read request\r\n");
        printf("\tReport type: %d\r\n", type);
        status = hids_client_boot_report_read(client->client, type);
        printf("\tRequest status: %s\r\n", status ? "success" : "failure");
}

/**
 * Enable Notifications for Boot Report debug command handler.
 */
static void hogp_boot_notif_cb(hogp_client_t *client, int argc, const char **argv)
{
        hids_client_boot_report_type type;
        bool status, enable;

        if (argc != 3) {
                printf("usage: boot_notif <hid_client_id> <boot_report_type> <enable flag>\r\n");
                return;
        }

        if (!parse_u8(argv[1], &type)) {
                printf("ERROR: invalid report type\r\n");
                return;
        }
        if (!parse_bool(argv[2], &enable)) {
                printf("ERROR: invalid enable argument\r\n");
                return;
        }

        printf("Boot report set notif state\r\n");
        printf("\tAction: %s notifications\r\n", enable ? "Register for" : "Unregister");
        printf("\tReport type provided: %d\r\n", type);
        status = hids_client_boot_report_set_notif_state(client->client, type, enable);
        printf("\tRequest status: %s\r\n", status ? "success" : "failure");
}

/**
 * Read CCC of Boot Report debug command handler.
 */
static void hogp_boot_read_ccc_cb(hogp_client_t *client, int argc, const char **argv)
{
        hids_client_boot_report_type type;
        bool status;

        if (argc != 2) {
                printf("usage: boot_read_ccc <hid_client_id> <boot_report_type>\r\n");
                return;
        }

        if (!parse_u8(argv[1], &type)) {
                printf("ERROR: invalid report type\r\n");
                return;
        }

        printf("Boot report get notif state\r\n");
        printf("\tReport type provided: %d\r\n", type);
        status = hids_client_boot_report_get_notif_state(client->client, type);
        printf("\tRequest status: %s\r\n", status ? "success" : "failure");
}

/**
 * Helper converting string in format "0ae34d..." to uint8_t array
 */
static int str_to_hex(const char *str, uint8_t *buf, int buf_size)
{
        int str_len;
        int i, j;
        char c;
        uint8_t b;

        str_len = strlen(str);

        if (str_len % 2)
                return -1;

        for (i = 0, j = 0; i < buf_size && j < str_len; i++, j++) {
                c = str[j];

                if (c >= 'a' && c <= 'f')
                        c += 'A' - 'a';

                if (c >= '0' && c <= '9')
                        b = c - '0';
                else if (c >= 'A' && c <= 'F')
                        b = 10 + c - 'A';
                else
                        return 0;

                j++;

                c = str[j];

                if (c >= 'a' && c <= 'f')
                        c += 'A' - 'a';

                if (c >= '0' && c <= '9')
                        b = b * 16 + c - '0';
                else if (c >= 'A' && c <= 'F')
                        b = b * 16 + 10 + c - 'A';
                else
                        return 0;

                buf[i] = b;
        }

        return i;
}

/**
 * Write Boot Report debug command handler.
 */
static void hogp_boot_write_cb(hogp_client_t *client, int argc, const char **argv)
{
        hids_client_boot_report_type type;
        uint16_t data_length;
        uint8_t data[32];
        bool status;

        if (argc != 3) {
                printf("usage: boot_write <hid_client_id> <boot_report_type> <data>\r\n");
                return;
        }

        if (!parse_u8(argv[1], &type)) {
                printf("ERROR: invalid report type\r\n");
                return;
        }
        data_length = str_to_hex(argv[2], data, sizeof(data));
        if (data_length == 0) {
                printf("No data to write\r\n");
                return;
        }

        printf("Boot report write\r\n");
        printf("\tReport type provided: %d\r\n", type);
        printf("\tRequire response: true\r\n");
        printf("\tValue length: %d\r\n", data_length);
        status = hids_client_boot_report_write(client->client, type, true, data_length, data);
        printf("\tRequest status: %s\r\n", status ? "success" : "failure");
}

/**
 * Read Report debug command handler.
 */
static void hogp_report_read_cb(hogp_client_t *client, int argc, const char **argv)
{
        hids_client_report_type_t type;
        uint8_t report_id;
        bool status;

        if (argc != 3) {
                printf("usage: report_read <hid_client_id> <report_type> <report_id>\r\n");
                return;
        }

        if (!parse_u8(argv[1], &type)) {
                printf("ERROR: invalid report type\r\n");
                return;
        }
        if (!parse_u8(argv[2], &report_id)) {
                printf("ERROR: invalid report id\r\n");
                return;
        }

        printf("Report read\r\n");
        printf("\tReport type provided: %d\r\n", type);
        printf("\tReport Id provided: %d\r\n", report_id);
        status = hids_client_report_read(client->client, type, report_id);
        printf("\tRequest status: %s\r\n", status ? "success" : "failure");
}

/**
 * Enable Input Report Notifications debug command handler.
 */
static void hogp_report_notif_cb(hogp_client_t *client, int argc, const char **argv)
{
        bool status, enable;
        uint8_t report_id;

        if (argc != 3) {
                printf("usage: report_notif <hid_client_id> <report_id> <enable>\r\n");
                return;
        }

        if (!parse_u8(argv[1], &report_id)) {
                printf("ERROR: invalid report type\r\n");
                return;
        }
        if (!parse_bool(argv[2], &enable)) {
                printf("ERROR: invalid enable argument\r\n");
                return;
        }

        printf("Report set notif state\r\n");
        printf("\tAction: %s notifications\r\n", enable ? "Register for" : "Unregister");
        printf("\tReport Id provided: %d\r\n", report_id);
        status = hids_client_input_report_set_notif_state(client->client, report_id, enable);
        printf("\tRequest status: %s\r\n", status ? "success" : "failure");
}

/**
 * Write Report debug command handler.
 */
static void hogp_report_write_cb(hogp_client_t *client, int argc, const char **argv)
{
        hids_client_report_type_t type;
        uint8_t report_id;
        bool status, response;
        uint16_t data_length;
        uint8_t data[32];

        if (argc != 5) {
                printf("usage: report_write <hid_client_id> <report_type> <report_id>"
                                                                   " <confirm_flag> <data>\r\n");
                return;
        }

        if (!parse_u8(argv[1], &type)) {
                printf("ERROR: invalid report type\r\n");
                return;
        }
        if (!parse_u8(argv[2], &report_id)) {
                printf("ERROR: invalid report id\r\n");
                return;
        }
        if (!parse_bool(argv[3], &response)) {
                printf("ERROR: invalid response\r\n");
                return;
        }
        data_length = str_to_hex(argv[4], data, sizeof(data));
        if (data_length == 0) {
                printf("No data to write\r\n");
                return;
        }

        printf("Report write\r\n");
        printf("\tReport type provided: %d\r\n", type);
        printf("\tReport Id provided: %d\r\n", report_id);
        printf("\tRequire response: %s\r\n", response ? "true" : "false");
        printf("\tReport length: %d\r\n", data_length);
        status = hids_client_report_write(client->client, type, report_id, response, data_length,
                                                                                        data);
        printf("\tRequest status: %s\r\n", status ? "success" : "failure");
}

/**
 * Write Control Point debug command handler.
 */
static void hogp_cp_command_cb(hogp_client_t *client, int argc, const char **argv)
{
        hids_client_cp_command_t command;
        bool status;

        if (argc != 2) {
                printf("usage: cp_command <hid_client_id> <command>\r\n");
                return;
        }

        if (!parse_u8(argv[1], &command)) {
                printf("ERROR: invalid command\r\n");
                return;
        }

        printf("Control command write\r\n");
        printf("\tCommand provided: %d\r\n", command);
        status = hids_client_cp_command(client->client, command);
        printf("\tRequest status: %s\r\n", status ? "success" : "failure");
}

/**
 * Read Input Report CCC descriptor.
 */
static void hogp_report_read_ccc_cb(hogp_client_t *client, int argc, const char **argv)
{
        uint8_t report_id;
        bool status;

        if (argc != 2) {
                printf("usage: report_read_ccc <hid_client_id> <report_id>\r\n");
                return;
        }

        if (!parse_u8(argv[1], &report_id)) {
                printf("ERROR: invalid report id\r\n");
                return;
        }

        printf("Report get notif state\r\n");
        printf("\tReport Id provided: %d\r\n", report_id);
        status = hids_client_input_report_get_notif_state(client->client, report_id);
        printf("\tRequest status: %s\r\n", status ? "success" : "failure");
}

/**
 * Connect to peripheral device.
 */
static void hogp_connect_cb(int argc, const char **argv, void *user_data)
{
        bd_address_t address;
        size_t dev_index = 0;

        if (argc < 2 || argc > 3) {
                hogp_connect_usage();
                return;
        }

        if (!strcmp(argv[1], "cancel")) {
                if (argc != 2) {
                        hogp_connect_usage();
                        return;
                }
                hogp_connect_cancel();
                return;
        }

        address.addr_type = PUBLIC_ADDRESS;

        if (argc > 2) {
                if (strcmp(argv[2], "private") == 0) {
                        address.addr_type = PRIVATE_ADDRESS;
                } else if (strcmp(argv[2], "public") != 0) {
                        printf("ERROR: invalid address type\r\n");
                        return;
                }
        }

        if (strchr(argv[1], ':')) {
                if (!ble_address_from_string(argv[1], address.addr_type, &address)) {
                        printf("ERROR: invalid address format\r\n");
                        printf("correct format: 'xx:xx:xx:xx:xx:xx' where xx - 2 bytes number in hex\r\n");
                        return;
                }
        } else {
                if (!parse_size_t(argv[1], &dev_index)) {
                        printf("ERROR: invalid device index format\r\n");
                        return;
                }
        }

        hogp_connect(&address, dev_index);

}

/**
 * Disconnect first peripheral on list of connected devices.
 */
static void hogp_disconnect_cb(int argc, const char **argv, void *user_data)
{
        if (argc != 1) {
                printf("usage: disconnect\r\n");
                return;
        }
        hogp_disconnect();
}

static void clicmd_scan_usage(void)
{
        printf("usage: scan <start [any] | stop>\r\n");
        printf("\t\"any\" will disable filtering devices by HOGP UUID, only valid for \"scan start\"\r\n");
}

/**
 * Start and stop scanning procedure.
 */
static void hogp_scan_cb(int argc, const char **argv, void *user_data)
{
        bool scan_any = false;

        if (argc < 2 || argc > 3) {
                clicmd_scan_usage();
                return;
        }

        if (strcmp(argv[1], "start") == 0) {
                if (argc > 2) {
                        if (!strcmp(argv[2], "any")) {
                                scan_any = true;
                        } else {
                                clicmd_scan_usage();
                                return;
                        }
                }
                hogp_scan(true, scan_any);
        } else if (strcmp(argv[1], "stop") == 0) {
                if (argc != 2) {
                        clicmd_scan_usage();
                        return;
                }
                hogp_scan(false, scan_any);
        } else {
                clicmd_scan_usage();
        }
}

static void hids_client_cb(int argc, const char *argv[], void *user_data)
{
        debug_callback_t callback = user_data;
        uint8_t client_id;
        hogp_client_t *client;

        if (argc < 2) {
                printf("ERROR: invalid arguments count\r\n");
                return;
        }

        if (!parse_u8(argv[1], &client_id)) {
                printf("ERROR: invalid client id\r\n");
                return;
        }
        client = get_client(client_id, CLIENT_TYPE_HIDS);

        if (!client) {
                printf("ERROR: client with Id: %d not found\r\n", client_id);
                return;
        }

        callback(client, argc - 1, (argc == 1 ? NULL : &argv[1]));
}

static void scps_iw_write_cb(int argc, const char **argv, void *user_data)
{
        gap_scan_params_t scan_params = CFG_SCAN_PARAMS;
        uint8_t client_id;
        hogp_client_t *client;

        if (argc != 1) {
                printf("usage: scan_iw_write <hid_client_id>\r\n");
                return;
        }

        if (!parse_u8(argv[1], &client_id)) {
                printf("ERROR: invalid client id\r\n");
                return;
        }
        client = get_client(client_id, CLIENT_TYPE_SCPS);

        if (!client) {
                printf("ERROR: client with Id: %d not found\r\n", client_id);
                return;
        }

        scps_client_write_scan_interval_window(client->client, scan_params.interval,
                                                                        scan_params.window);
}

static void clicmd_show_usage(void)
{
        printf("usage: show <connected|bonded>\r\n");
}

static void hogp_show_cb(int argc, const char *argv[], void *user_data)
{
        if (argc != 2) {
                clicmd_show_usage();
                return;
        }

        if (!strcmp("connected", argv[1])) {
                hogp_show_devices(GAP_DEVICE_FILTER_CONNECTED);
        } else if (!strcmp("bonded", argv[1])) {
                hogp_show_devices(GAP_DEVICE_FILTER_BONDED);
        } else {
                clicmd_show_usage();
        }
}

static void clicmd_unbond_usage(void)
{
        printf("usage: unbond <address [public|private] | all>\r\n");
        printf("\taddress   address of bonded device\r\n");
        printf("\tpublic    set address type public\r\n");
        printf("\tprivate   set address type private\r\n");
        printf("\tall       unbond all bonded devices\r\n");
}

static void hogp_unbond_cb(int argc, const char *argv[], void *user_data)
{
        bd_address_t addr;

        if (argc < 2 || argc > 3) {
                clicmd_unbond_usage();
                return;
        }

        if (!strcmp("all", argv[1])) {
                if (argc == 2) {
                        hogp_unbond_all();
                        return;
                } else {
                        clicmd_unbond_usage();
                        return;
                }
        }

        addr.addr_type = PUBLIC_ADDRESS;

        if (argc > 2) {
                if (strcmp(argv[2], "private") == 0) {
                        addr.addr_type = PRIVATE_ADDRESS;
                } else if (strcmp(argv[2], "public") != 0) {
                        printf("ERROR: invalid address type\r\n");
                        return;
                }
        }

        if (!ble_address_from_string(argv[1], addr.addr_type, &addr)) {
                printf("ERROR: invalid address format\r\n");
                printf("correct format: 'xx:xx:xx:xx:xx:xx' where xx - 2 bytes number in hex\r\n");
                return;
        }

        hogp_unbond_by_address(&addr);
}

__RETAINED_RW static cli_command_t debug_handlers[] = {
        { "get_protocol",       hids_client_cb,         hogp_get_protocol_cb },
        { "boot_read",          hids_client_cb,         hogp_boot_read_cb },
        { "boot_notif",         hids_client_cb,         hogp_boot_notif_cb },
        { "boot_read_ccc",      hids_client_cb,         hogp_boot_read_ccc_cb },
        { "boot_write",         hids_client_cb,         hogp_boot_write_cb },
        { "report_read",        hids_client_cb,         hogp_report_read_cb },
        { "report_notif",       hids_client_cb,         hogp_report_notif_cb },
        { "report_read_ccc",    hids_client_cb,         hogp_report_read_ccc_cb },
        { "report_write",       hids_client_cb,         hogp_report_write_cb },
        { "cp_command",         hids_client_cb,         hogp_cp_command_cb },
        { "scan",               hogp_scan_cb,           NULL },
        { "connect",            hogp_connect_cb,        NULL },
        { "disconnect",         hogp_disconnect_cb,     NULL },
        { "scan_iw_write",      scps_iw_write_cb,       NULL },
        { "show",               hogp_show_cb,           NULL },
        { "unbond",             hogp_unbond_cb,         NULL },
        { NULL },
};

static void default_handler(int argc, const char **argv, void *user_data)
{
        printf("Valid commands:\r\n");
        printf("\tget_protocol <hid_client_id>\r\n");
        printf("\tcp_command <hid_client_id> <command>\r\n");
        printf("\tboot_read <hid_client_id> <boot_report_type>\r\n");
        printf("\tboot_write <hid_client_id> <boot_report_type> <data>\r\n");
        printf("\tboot_notif <hid_client_id> <boot_report_type> <enable flag>\r\n");
        printf("\tboot_read_ccc <hid_client_id> <boot_report_type>\r\n");
        printf("\treport_read <hid_client_id> <report_type> <report_id>\r\n");
        printf("\treport_write <hid_client_id> <report_type> <report_id> <confirm_flag> <data>\r\n");
        printf("\treport_notif <hid_client_id> <report_id> <enable>\r\n");
        printf("\treport_read_ccc <hid_client_id> <report_id>\r\n");
        printf("\tscan <start [any] | stop>\r\n");
        printf("\tscan_iw_write <hid_client_id>\r\n");
        printf("\tconnect <address> [public|private]\r\n");
        printf("\tconnect cancel\r\n");
        printf("\tdisconnect\r\n");
        printf("\tshow <connected|bonded>\r\n");
        printf("\tunbond <address [public|private] | all>\r\n");
}

cli_t register_debug(uint32_t notif_mask)
{
        return cli_register(notif_mask, debug_handlers, default_handler);
}
