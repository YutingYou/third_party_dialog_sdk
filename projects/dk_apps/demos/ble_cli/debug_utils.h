/**
 ****************************************************************************************
 *
 * @file debug_utils.h
 *
 * @brief Debug utilities
 *
 * Copyright (C) 2016-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef DEBUG_UTILS_H_
#define DEBUG_UTILS_H_

#include <stdlib.h>
#include "ble_att.h"
#include "ble_common.h"
#include "ble_gap.h"

#define _debug_printf(type, fmt, args ...) printf(PRE_##type fmt POST_##type, ##args)

#define PRE_PARAMETER "\t\t"
#define POST_PARAMETER "\r\n"

#define print_parameter(fmt, args ...) _debug_printf(PARAMETER, fmt, ## args)

#define PRE_CATEGORY "\r\n\t"
#define POST_CATEGORY "\r\n"

#define print_category(fmt, args ...) _debug_printf(CATEGORY, fmt, ## args)

#define PRE_COMMAND "\r\n\r\n"
#define POST_COMMAND "\r\n"

#define print_command(fmt, args ...) _debug_printf(COMMAND, fmt, ## args)

#define PRE_STATUS "\r\n"
#define POST_STATUS "\r\n\r\n"

#define print_status(fmt, args ...) _debug_printf(STATUS, fmt, ## args)

#define PRE_EVENT "\r\n"
#define POST_EVENT "\r\n"

#define print_event(fmt, args ...) _debug_printf(EVENT, fmt, ## args)

#define PRE_NO_FORMAT ""
#define POST_NO_FORMAT "\r\n"

#define print(fmt, args ...) _debug_printf(NO_FORMAT, fmt, ## args)

/**
 * Debug handler callback. Returns true if called successfully, otherwise false - help message
 * will be printed out.
 */
typedef bool (* debug_callback_t) (int argc, const char **argv);

/**
 * Debug handler struct
 */
typedef struct {
        const char *command;
        const char *help;
        debug_callback_t callback;
} debug_handler_t;

/**
 * Helper comparing second argv with debug handler commands and calling callback
 */
void debug_handle_message(int argc, const char *argv[], const debug_handler_t *handlers);

void convert_str_to_bdaddr(const char *bd_addr_str, bd_address_t *addr);

void convert_str_to_own_bdaddr(const char *own_bd_addr_str, own_address_t *addr);

char *format_bd_address(const bd_address_t *addr);

char *format_own_address(const own_address_t *addr);

const char *format_uuid(const att_uuid_t *uuid);

const char *get_status(ble_error_t status);

const char *get_phy(ble_gap_phy_t phy);

const char *get_phy_pref(ble_gap_phy_pref_t phy_pref);

char *format_adv_data(const uint8_t *data, uint8_t data_length);

void print_data_parameter(const uint8_t *data, uint8_t data_length);

int str_to_hex(const char *str, uint8_t *buf, int buf_size);

bool is_bdaddr_param(const char *param_string);


#endif /* DEBUG_UTILS_H_ */
