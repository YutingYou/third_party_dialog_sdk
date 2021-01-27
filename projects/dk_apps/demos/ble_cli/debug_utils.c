/**
 ****************************************************************************************
 *
 * @file debug_utils.c
 *
 * @brief Debug utilities
 *
 * Copyright (C) 2016-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "osal.h"
#include "ble_config.h"
#include "ble_common.h"
#include "ble_gap.h"
#include "debug_utils.h"

/**
 * Debug prints buffer size.
 */
#define BD_ADDRESS_STRING_SIZE (17)

#define ADV_DATA_STRING_SIZE (63)

#define MAX_ADV_DATA_LENGTH (31)

/**
* Convert string to address part of BD address structure and write to addr array
**/
void convert_str_to_bdaddr(const char *bd_addr_str, bd_address_t *addr)
{
        /* addr_p used because sscanf requires pointer to unsigned int as type specifier and
         * addr->addr elements type is uint8_t not supported by sscanf */
        unsigned int addr_p[6];
        int i;

        sscanf(bd_addr_str, "%02x:%02x:%02x:%02x:%02x:%02x", &addr_p[5], &addr_p[4],
                &addr_p[3], &addr_p[2], &addr_p[1], &addr_p[0]);

        for (i = 0; i < sizeof(addr->addr); i++) {
                addr->addr[i] = addr_p[i];
        }
}

/**
* Convert string to address part of own BD address structure and write to addr array
**/
void convert_str_to_own_bdaddr(const char *own_bd_addr_str, own_address_t *addr)
{
        /* addr_p used because sscanf requires pointer to unsigned int as type specifier and
         * addr->addr elements type is uint8_t not supported by sscanf */
        unsigned int addr_p[6];
        int i;

        sscanf(own_bd_addr_str, "%02x:%02x:%02x:%02x:%02x:%02x", &addr_p[5], &addr_p[4],
                &addr_p[3], &addr_p[2], &addr_p[1], &addr_p[0]);

        for (i = 0; i < sizeof(addr->addr); i++) {
                addr->addr[i] = addr_p[i];
        }
}

/**
 * Return static buffer with formatted address
 */
char *format_bd_address(const bd_address_t *addr)
{
        static char buf[27];

        sprintf(buf, "%s, %s", addr->addr_type == PRIVATE_ADDRESS ? "private" : "public",
                                                                ble_address_to_string(addr));

        return buf;
}

/**
 * Return static buffer with formatted own BD address
 */
char *format_own_address(const own_address_t *addr)
{
        static char buf[48];
        char addr_part[18];
        int i, addr_size;

        switch (addr->addr_type) {
        case PUBLIC_STATIC_ADDRESS:
                strcpy(buf, "PUBLIC STATIC ");
                break;
        case PRIVATE_STATIC_ADDRESS:
                strcpy(buf, "PRIVATE STATIC ");
                break;
        case PRIVATE_RANDOM_RESOLVABLE_ADDRESS:
                strcpy(buf, "PRIVATE RANDOM RESOLVABLE ");
                break;
        case PRIVATE_RANDOM_NONRESOLVABLE_ADDRESS:
                strcpy(buf, "PRIVATE RANDOM NONRESOLVABLE ");
                break;
#if (dg_configBLE_PRIVACY_1_2 == 1)
        case PRIVATE_CNTL:
                strcpy(buf, "PRIVATE CNTL ");
                break;
#endif
        default:
                break;
        }

        addr_size = sizeof(addr->addr);
        for (i = 0; i < addr_size; i++) {
                int idx;

                // for printout, address should be reversed
                idx = addr_size - i - 1;
                if (i != addr_size - 1) {
                        sprintf(&addr_part[i * 3], "%02X:", addr->addr[idx]);
                } else {
                        sprintf(&addr_part[i * 3], "%02X", addr->addr[idx]);
                }
        }
        addr_part[sizeof(addr_part) - 1] = '\0';

        strcat(buf, addr_part);

        return buf;
}

/**
 * Return static buffer with formatted UUID
 */
const char *format_uuid(const att_uuid_t *uuid)
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

/**
 * Return status code represented as string
 */
const char *get_status(ble_error_t status)
{
        static char unknown_code[29];

        switch (status) {
        case BLE_STATUS_OK:
                return "Success";
                break;
        case BLE_ERROR_FAILED:
                return "Generic failure";
                break;
        case BLE_ERROR_ALREADY_DONE:
                return "Already done";
                break;
        case BLE_ERROR_IN_PROGRESS:
                return "Operation already in progress";
                break;
        case BLE_ERROR_INVALID_PARAM:
                return "Invalid parameter";
                break;
        case BLE_ERROR_NOT_ALLOWED:
                return "Not allowed";
                break;
        case BLE_ERROR_NOT_CONNECTED:
                return "Not connected";
                break;
        case BLE_ERROR_NOT_SUPPORTED:
                return "Not supported";
                break;
        case BLE_ERROR_NOT_ACCEPTED:
                return "Not accepted";
                break;
        case BLE_ERROR_BUSY:
                return "Busy";
                break;
        case BLE_ERROR_TIMEOUT:
                return "Request timed out";
                break;
        case BLE_ERROR_NOT_SUPPORTED_BY_PEER:
                return "Not supported by peer";
                break;
        case BLE_ERROR_INS_RESOURCES:
                return "Insufficient resources";
                break;
        case BLE_ERROR_NOT_FOUND:
                return "Not found";
                break;
        case BLE_ERROR_L2CAP_NO_CREDITS:
                return "No credits available on L2CAP CoC";
                break;
        case BLE_ERROR_L2CAP_MTU_EXCEEDED:
                return "MTU exceeded on L2CAP CoC";
                break;
        default:
                strcpy(unknown_code, "Unknown status code (");
                sprintf(&unknown_code[21], "0x%02x", status);
                strcat(unknown_code, ")");
                unknown_code[sizeof(unknown_code) - 1] = '\0';
        }

        return unknown_code;
}

/**
 * Return PHY code represented as string
 */
const char *get_phy(ble_gap_phy_t phy)
{
        static char unknown_code[19];

        switch (phy) {
        case BLE_GAP_PHY_1M:
                return "BLE_GAP_PHY_1M";
        case BLE_GAP_PHY_2M:
                return "BLE_GAP_PHY_2M";
        case BLE_GAP_PHY_CODED:
                return "BLE_GAP_PHY_CODED";
        default:
                strcpy(unknown_code, "Unknown PHY (");
                sprintf(&unknown_code[13], "0x%02x", (uint8_t) phy);
                strcat(unknown_code, ")");
                unknown_code[sizeof(unknown_code) - 1] = '\0';
                return unknown_code;
        }
}

/**
 * Return PHY preference code represented as string
 */
const char *get_phy_pref(uint8_t phy_pref)
{
        static char unknown_code[30];

        switch (phy_pref) {
        case BLE_GAP_PHY_PREF_AUTO:
                return "AUTO";
        case BLE_GAP_PHY_PREF_1M:
                return "1M";
        case BLE_GAP_PHY_PREF_2M:
                return "2M";
        case BLE_GAP_PHY_PREF_CODED:
                return "CODED";
        case ( BLE_GAP_PHY_PREF_1M | BLE_GAP_PHY_PREF_2M ):
                return "1M | 2M";
        case ( BLE_GAP_PHY_PREF_1M | BLE_GAP_PHY_PREF_CODED ):
                return "1M | CODED";
        case ( BLE_GAP_PHY_PREF_2M | BLE_GAP_PHY_PREF_CODED ):
                return "2M | CODED";
        case ( BLE_GAP_PHY_PREF_1M | BLE_GAP_PHY_PREF_2M | BLE_GAP_PHY_PREF_CODED ):
                return "1M | 2M | CODED";
        default:
                strcpy(unknown_code, "Unknown PHY preference (");
                sprintf(&unknown_code[13], "0x%02x", (uint8_t) phy_pref);
                strcat(unknown_code, ")");
                unknown_code[sizeof(unknown_code) - 1] = '\0';
                return unknown_code;
        }
}

/**
 * Return static buffer with formatted advertising data or scan response data
 */
char *format_adv_data(const uint8_t *data, uint8_t data_length)
{
        static char buf[ADV_DATA_STRING_SIZE];
        int i;

        if (data_length > MAX_ADV_DATA_LENGTH) {
                buf[0] = '\0';

                return buf;
        }

        for (i = 0; i < data_length; i++) {
                sprintf(&buf[i * 2], "%02x", data[i]);
        }

        buf[data_length * 2] = '\0';

        return buf;
}

/**
 * Print Raw data as parameter
 */
void print_data_parameter(const uint8_t *data, uint8_t data_length)
{
        int i;

        printf("\t\tData: ");
        for (i = 0; i < data_length; i++) {
                printf("%02x", data[i]);
        }

        printf("\r\n");
}

/**
 * Helper converting string in format "0ae34d..." to uint8_t array
 */
int str_to_hex(const char *str, uint8_t *buf, int buf_size)
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
 * Check if param is BD address
 */
bool is_bdaddr_param(const char *param_string)
{
        int i;
        uint16_t param_length;

        param_length = strlen(param_string);

        if (param_length != BD_ADDRESS_STRING_SIZE) {
                return false;
        }

        for (i = 1; i < param_length + 1; i++) {
                if (((i % 3) == 0) && (param_string[i - 1] != ':')) {
                        return false;
                }
                if (((i % 3) != 0) && (!isxdigit((int) param_string[i - 1]))) {
                        return false;
                }
        }
        return true;
}
static void print_debug_handler_help(const char *category, const debug_handler_t *debug_handler)
{
        print("\r\nUsage: %s %s %s", category, debug_handler->command, debug_handler->help);
}

/*
 * If counts maximum number of parameters of the command by checking how many spaces are included
 * in /p param_str.
 */
static uint8_t get_max_num_p(const char *param_str)
{
        int i;
        uint8_t param_num = 0;

        /* If there is no parameters then return 0 immediately */
        if (strcmp(param_str, "") == 0) {
                return param_num;
        }

        for (i = 0; i < strlen(param_str); i++) {
                if (param_str[i] == ' ') {
                        param_num += 1;
                }
        }

        return param_num + 1;   // need to add 1 because there is always one empty space less than
                                // number of parameters
}

void debug_handle_message(int argc, const char *argv[], const debug_handler_t *handlers)
{
        const debug_handler_t *handler;

        if (argc < 2) {
                goto help;
        }

        for (handler = handlers; handler && handler->command; handler++) {
                if (strcmp(handler->command, argv[1])) {
                       continue;
                }

                /*
                 * Maximum number of the command parameters including name of the command
                 */
                uint8_t cmd_max_num_p = get_max_num_p(handler->help) + 2;

                if ((argc > cmd_max_num_p) || ((argc > 2) && (strcmp("help", argv[2]) == 0)) ||
                                                                !handler->callback(argc, argv)) {
                        print_debug_handler_help(argv[0], handler);
                }

                return;
       }

        print("Unknown command: %s %s", argv[0], argv[1]);
help:
        print("%s commands:", argv[0]);
        for (handler = handlers; handler && handler->command; handler++) {
                print("\t%s %s", argv[0], handler->command);
        }
}
