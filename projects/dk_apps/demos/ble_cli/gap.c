/**
 ****************************************************************************************
 *
 * @file gap.c
 *
 * @brief BLE GAP API implementation
 *
 * Copyright (C) 2016-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include "ad_ble.h"
#include "ble_common.h"
#include "ble_config.h"
#include "ble_gap.h"
#include "cli_utils.h"
#include "debug_utils.h"
#include "ble_cli_config.h"
#include "gap.h"

static bool parse_address(int argc, const char **argv, int *idx, bd_address_t *addr, addr_type_t default_addr_type)
{
        addr_type_t type = default_addr_type;

        if (*idx == argc) {
                return false;
        }

        if (strcmp(argv[*idx], "public") == 0) {
                type = PUBLIC_ADDRESS;
                (*idx)++;
        } else if (strcmp(argv[*idx], "private") == 0) {
                type = PRIVATE_ADDRESS;
                (*idx)++;
        }

        if (*idx == argc || !is_bdaddr_param(argv[*idx])) {
                return false;
        }

        ble_address_from_string(argv[(*idx)++], type, addr);

        return true;
}

/*
 * Print GAP connection parameters
 */
static void print_gap_conn_params(const gap_conn_params_t *params)
{
        print_category("GAP connection parameters:");
        print_parameter("Minimum connection interval: %d", params->interval_min);
        print_parameter("Maximum connection interval: %d", params->interval_max);
        print_parameter("Slave latency: %d", params->slave_latency);
        print_parameter("Supervision timeout: %d", params->sup_timeout);
}

/*
 * Print GAP connected event information
 */
static void handle_ble_evt_gap_connected(const ble_evt_gap_connected_t *info)
{
        print_event("BLE_EVT_GAP_CONNECTED");
        print_parameter("Connection index: %d", info->conn_idx);
        print_parameter("Own device BD address: %s", format_bd_address(&info->own_addr));
        print_parameter("Peer device BD address: %s", format_bd_address(&info->peer_address));
        print_gap_conn_params(&info->conn_params);
}

/*
 * Print GAP disconnected event information
 */
static void handle_ble_evt_gap_disconnected(const ble_evt_gap_disconnected_t *info)
{
        print_event("BLE_EVT_GAP_DISCONNECTED");
        print_parameter("Connection index: %d", info->conn_idx);
        print_parameter("BD address of disconnected device: %s", format_bd_address(&info->address));
        print_parameter("Reason of disconnection: 0x%02x", info->reason);
}

/*
 * Print GAP disconnect failed event information
 */
static void handle_ble_evt_gap_disconnect_failed(const ble_evt_gap_disconnect_failed_t *info)
{
        print_event("BLE_EVT_GAP_DISCONNECT_FAILED");
        print_parameter("Connection index: %d", info->conn_idx);
        print_parameter("Status: 0x%02x", info->status);
}

/*
 * Print GAP scan completed event information
 */
static void handle_ble_evt_gap_scan_completed(const ble_evt_gap_scan_completed_t *info)
{
        print_event("BLE_EVT_GAP_SCAN_COMPLETED");
        print_parameter("Scan type: %s", info->scan_type == GAP_SCAN_ACTIVE ? "active" : "passive");
        print_parameter("Completion status: 0x%02x", info->status);
}

/*
 * Print GAP scan parameters
 */
static void print_gap_scan_params(const gap_scan_params_t *scan_params)
{
        print_category("GAP scan parameters:");
        print_parameter("Interval: %d", scan_params->interval);
        print_parameter("Window: %d", scan_params->window);
}

/*
 * Print GAP advertising completed event information
 */
static void handle_ble_evt_gap_adv_completed(const ble_evt_gap_adv_completed_t *info)
{
        print_event("BLE_EVT_GAP_ADV_COMPLETED");
        print_parameter("Advertising type: %d", info->adv_type);
        print_parameter("Completion status: 0x%02x", info->status);
}

/*
 * Print GAP advertising report event information
 */
static void handle_ble_evt_gap_adv_report(const ble_evt_gap_adv_report_t *info)
{
        print_event("BLE_EVT_GAP_ADV_REPORT");
        print_parameter("Type of advertising packet: %d", info->type);
        print_parameter("BD address of advertising device: %s", format_bd_address(&info->address));
        print_parameter("RSSI: %d", info->rssi);
        print_parameter("Length of advertising data: %d", info->length);
        print_data_parameter(info->data, info->length);
}

/*
 * Print GAP connection parameter update request event information
 */
static void handle_ble_evt_gap_conn_param_update_req(ble_evt_gap_conn_param_update_req_t *info)
{
        print_event("BLE_EVT_GAP_CONN_PARAM_UPDATE_REQ");
        print_parameter("Connection index: %d", info->conn_idx);
        print_gap_conn_params(&info->conn_params);

        #if (CFG_AUTO_CONN_PARAM_REPLY == 1)
        ble_error_t status = ble_gap_conn_param_update_reply(info->conn_idx, true);

        print_command("ble_gap_conn_param_update_reply: ( %d, %s )", info->conn_idx, "true");
        print_status("return: %s", get_status(status));
        #endif
}

/*
 * Print GAP connection parameter updated event information
 */
static void handle_ble_evt_gap_conn_param_updated(ble_evt_gap_conn_param_updated_t *info)
{
        print_event("BLE_EVT_GAP_CONN_PARAM_UPDATED");
        print_parameter("Connection index: %d", info->conn_idx);
        print_gap_conn_params(&info->conn_params);
}

/*
 * Print GAP pair request event information
 */
static void handle_ble_evt_gap_pair_req(ble_evt_gap_pair_req_t *info)
{
        print_event("BLE_EVT_GAP_PAIR_REQ");
        print_parameter("Connection index: %d", info->conn_idx);
        print_parameter("Bond: %s", info->bond ? "true" : "false");

        #if (CFG_AUTO_PAIR_REPLY == 1)
        ble_error_t status = ble_gap_pair_reply(info->conn_idx, true, info->bond);

        print_command("ble_gap_pair_reply ( %d, %s, %s )", info->conn_idx, "true",
                                                                     info->bond ? "true" : "false");
        print_status("return: %s", get_status(status));
        #endif
}

/*
 * Print GAP pair completed event information
 */
static void handle_ble_evt_gap_pair_completed(ble_evt_gap_pair_completed_t *info)
{
        print_event("BLE_EVT_GAP_PAIR_COMPLETED");
        print_parameter("Connection index: %d", info->conn_idx);
        print_parameter("Status: 0x%02x", info->status);
        if (info->status == 0 ) {
                print_parameter("Bond: %s", info->bond ? "true" : "false");
                print_parameter("MITM: %s", info->mitm ? "true" : "false");
        }
}

/*
 * Print GAP security request event information
 */
static void handle_ble_evt_gap_security_request(ble_evt_gap_security_request_t *info)
{
        print_event("BLE_EVT_GAP_SECURITY_REQUEST");
        print_parameter("Connection index: %d", info->conn_idx);
        print_parameter("Bond: %s", info->bond ? "true" : "false");
        print_parameter("MITM: %s", info->mitm ? "true" : "false");
}

/*
 * Print GAP passkey notification event information
 */
static void handle_ble_evt_gap_passkey_notify(ble_evt_gap_passkey_notify_t *info)
{
        print_event("BLE_EVT_GAP_PASSKEY_NOTIFY");
        print_parameter("Connection index: %d", info->conn_idx);
        print_parameter("Passkey: %06" PRIu32 "", info->passkey);
}

/*
 * Print GAP passkey request event information
 */
static void handle_ble_evt_gap_passkey_request(ble_evt_gap_passkey_request_t *info)
{
        print_event("BLE_EVT_GAP_PASSKEY_REQUEST");
        print_parameter("Connection index: %d", info->conn_idx);
}

#if (dg_configBLE_SECURE_CONNECTIONS == 1)
/*
 * Print GAP numeric request event information
 */
static void handle_ble_evt_gap_numeric_request(ble_evt_gap_numeric_request_t *info)
{
        print_event("BLE_EVT_GAP_NUMERIC_REQUEST");
        print_parameter("Connection index: %d", info->conn_idx);
        print_parameter("Numeric key: %" PRIu32 "", info->num_key);
}
#endif /* (dg_configBLE_SECURE_CONNECTIONS == 1) */

/*
 * Print GAP security level changed event information
 */
static void handle_ble_evt_gap_sec_level_changed(ble_evt_gap_sec_level_changed_t *info)
{
        print_event("BLE_EVT_GAP_SEC_LEVEL_CHANGED");
        print_parameter("Connection index: %d", info->conn_idx);
        print_parameter("Security level: %d", (info->level + 1));
}

/*
 * Print GAP address resolved event information
 */
static void handle_ble_evt_gap_address_resolved(ble_evt_gap_address_resolved_t *info)
{
        print_event("BLE_EVT_GAP_ADDRESS_RESOLVED");
        print_parameter("Connection index: %d", info->conn_idx);
        print_parameter("Static address: %s", format_bd_address(&info->address));
        print_parameter("Random address: %s", format_bd_address(&info->resolved_address));
}

/*
 * Print GAP set security level failed event information
 */
static void handle_ble_evt_gap_set_sec_level_failed(ble_evt_gap_set_sec_level_failed_t *info)
{
        print_event("BLE_EVT_GAP_SET_SEC_LEVEL_FAILED");
        print_parameter("Connection index: %d", info->conn_idx);
        print_parameter("Status: 0x%02x", info->status);
}

/*
 * Print GAP connection parameter update completed event information
 */
static void handle_ble_evt_gap_conn_param_update_completed(ble_evt_gap_conn_param_update_completed_t * info)
{
        print_event("BLE_EVT_GAP_CONN_PARAM_UPDATE_COMPLETED");
        print_parameter("Connection index: %d", info->conn_idx);
        print_parameter("Status: 0x%02x", info->status);
}

/*
 * Print GAP data length changed event information
 */
static void handle_ble_evt_gap_data_length_changed(ble_evt_gap_data_length_changed_t * info)
{
        print_event("BLE_EVT_GAP_DATA_LENGTH_CHANGED");
        print_parameter("Connection index: %d", info->conn_idx);
        print_parameter("Maximum RX data length: %d", info->max_rx_length);
        print_parameter("Maximum RX time: %d", info->max_rx_time);
        print_parameter("Maximum TX data length: %d", info->max_tx_length);
        print_parameter("Maximum TX time: %d", info->max_tx_time);
}

/*
 * Print GAP data length set failed event information
 */
static void handle_ble_evt_gap_data_length_set_failed(ble_evt_gap_data_length_set_failed_t * info)
{
        print_event("BLE_EVT_GAP_DATA_LENGTH_SET_FAILED");
        print_parameter("Connection index: %d", info->conn_idx);
        print_parameter("Status: 0x%02x", info->status);
}

/*
 * Print GAP connection completed event information
 */
static void handle_ble_evt_gap_connection_completed(const ble_evt_gap_connection_completed_t *info)
{
        print_event("BLE_EVT_GAP_CONNECTION_COMPLETED");
        print_parameter("Status: 0x%02x", info->status);
}

static void handle_ble_evt_gap_peer_version(ble_evt_gap_peer_version_t *info)
{
        print_event("BLE_EVT_GAP_PEER_VERSION");
        print_parameter("Bluetooth Version: 0x%02x", info->lmp_version);
        print_parameter("Company ID: 0x%04x", info->company_id);
        print_parameter("Implementation Rev.: 0x%04x", info->lmp_subversion);
}

static void handle_ble_evt_gap_peer_features(ble_evt_gap_peer_features_t *info)
{
        print_event("BLE_EVT_GAP_PEER_FEATURES");
        print_data_parameter(info->le_features, LE_FEATS_LEN);
}

static void handle_ble_evt_gap_address_resolution_failed(const ble_evt_gap_address_resolution_failed_t *info)
{
        print_event("BLE_EVT_GAP_ADDRESS_RESOLUTION_FAILED");
        print_parameter("Status: 0x%02x", info->status);
}

static void handle_ble_evt_gap_ltk_missing(const ble_evt_gap_ltk_missing_t *info)
{
        print_event("BLE_EVT_GAP_LTK_MISSING");
        print_parameter("Connection index: %d", info->conn_idx);
}

#if (dg_configBLE_2MBIT_PHY == 1)
/*
 * Print GAP PHY set failed event information
 */
static void handle_ble_evt_gap_phy_set_completed(ble_evt_gap_phy_set_completed_t * info)
{
        print_event("BLE_EVT_GAP_PHY_SET_COMPLETED");
        print_parameter("Connection index: %d", info->conn_idx);
        print_parameter("Status: 0x%02x", info->status);
}

/*
 * Print GAP PHY changed event information
 */
static void handle_ble_evt_gap_phy_changed(ble_evt_gap_phy_changed_t * info)
{
        print_event("BLE_EVT_GAP_PHY_CHANGED");
        print_parameter("Connection index: %d", info->conn_idx);
        print_parameter("Transmitter PHY:  %s", get_phy(info->tx_phy));
        print_parameter("Receiver PHY:     %s", get_phy(info->rx_phy));
}
#endif /* (dg_configBLE_2MBIT_PHY == 1) */

static void handle_ble_evt_gap_local_tx_pwr(ble_evt_gap_local_tx_pwr_t * info)
{
        print_event("BLE_EVT_GAP_LOCAL_TX_PWR");
        print_parameter("Connection index: %d", info->conn_idx);
        print_parameter("Status: 0x%02x", info->status);
        print_parameter("PHY:  %s", get_phy(info->phy));
        print_parameter("Current transmit power level:  %d", info->curr_tx_pwr_lvl);
        print_parameter("Maximum transmit power level:  %d", info->max_tx_pwr_lvl);
}

static void handle_ble_evt_gap_tx_pwr_report(ble_evt_gap_tx_pwr_report_t * info)
{
        print_event("BLE_EVT_GAP_TX_PWR_REPORT");
        print_parameter("Connection index: %d", info->conn_idx);
        print_parameter("Status: 0x%02x", info->status);
        print_parameter("Reason:  %d", info->reason);
        print_parameter("PHY:  %s", get_phy(info->phy));
        print_parameter("Transmit power level:  %d", info->tx_pwr_lvl);
        print_parameter("Transmit power level flag :  %d", info->tx_pwr_lvl_flag);
        print_parameter("Delta :  %d", info->delta);
}

static void handle_ble_evt_gap_path_loss_thres(ble_evt_gap_path_loss_thres_t * info)
{
        print_event("BLE_EVT_GAP_PATH_LOSS_THRES");
        print_parameter("Connection index: %d", info->conn_idx);
        print_parameter("Current path loss:  %d", info->curr_path_loss);
        print_parameter("Zone entered:  %d", info->zone_enter);
}

#if BLE_SSP_DEBUG
void handle_ble_evt_gap_ltk(ble_evt_gap_ltk_t * info)
{
        print_event("BLE_EVT_GAP_LTK");
        print_parameter("Connection index: %d", info->conn_idx);
        print_data_parameter(info->ltk.key, sizeof(info->ltk.key));
}
#endif

bool gap_handle_event(ble_evt_hdr_t *event)
{
        switch (event->evt_code) {
        case BLE_EVT_GAP_CONNECTED:
                handle_ble_evt_gap_connected((ble_evt_gap_connected_t *) event);
                break;
        case BLE_EVT_GAP_ADV_REPORT:
                handle_ble_evt_gap_adv_report((ble_evt_gap_adv_report_t *) event);
                break;
        case BLE_EVT_GAP_DISCONNECTED:
                handle_ble_evt_gap_disconnected((ble_evt_gap_disconnected_t *) event);
                break;
        case BLE_EVT_GAP_DISCONNECT_FAILED:
                handle_ble_evt_gap_disconnect_failed((ble_evt_gap_disconnect_failed_t *) event);
                break;
        case BLE_EVT_GAP_ADV_COMPLETED:
                handle_ble_evt_gap_adv_completed((ble_evt_gap_adv_completed_t *) event);
                break;
        case BLE_EVT_GAP_SCAN_COMPLETED:
                handle_ble_evt_gap_scan_completed((ble_evt_gap_scan_completed_t *) event);
                break;
        case BLE_EVT_GAP_CONN_PARAM_UPDATE_REQ:
                handle_ble_evt_gap_conn_param_update_req((ble_evt_gap_conn_param_update_req_t *) event);
                break;
        case BLE_EVT_GAP_CONN_PARAM_UPDATED:
                handle_ble_evt_gap_conn_param_updated((ble_evt_gap_conn_param_updated_t *) event);
                break;
        case BLE_EVT_GAP_PAIR_REQ:
                handle_ble_evt_gap_pair_req((ble_evt_gap_pair_req_t *) event);
                break;
        case BLE_EVT_GAP_PAIR_COMPLETED:
                handle_ble_evt_gap_pair_completed((ble_evt_gap_pair_completed_t *) event);
                break;
        case BLE_EVT_GAP_SECURITY_REQUEST:
                handle_ble_evt_gap_security_request((ble_evt_gap_security_request_t *) event);
                break;
        case BLE_EVT_GAP_PASSKEY_NOTIFY:
                handle_ble_evt_gap_passkey_notify((ble_evt_gap_passkey_notify_t *) event);
                break;
        case BLE_EVT_GAP_PASSKEY_REQUEST:
                handle_ble_evt_gap_passkey_request((ble_evt_gap_passkey_request_t *) event);
                break;
        case BLE_EVT_GAP_SEC_LEVEL_CHANGED:
                handle_ble_evt_gap_sec_level_changed((ble_evt_gap_sec_level_changed_t *) event);
                break;
        case BLE_EVT_GAP_ADDRESS_RESOLVED:
                handle_ble_evt_gap_address_resolved((ble_evt_gap_address_resolved_t *) event);
                break;
        case BLE_EVT_GAP_SET_SEC_LEVEL_FAILED:
                handle_ble_evt_gap_set_sec_level_failed((ble_evt_gap_set_sec_level_failed_t *) event);
                break;
        case BLE_EVT_GAP_CONN_PARAM_UPDATE_COMPLETED:
                handle_ble_evt_gap_conn_param_update_completed((ble_evt_gap_conn_param_update_completed_t *) event);
                break;
        case BLE_EVT_GAP_DATA_LENGTH_CHANGED:
                handle_ble_evt_gap_data_length_changed((ble_evt_gap_data_length_changed_t *) event);
                break;
        case BLE_EVT_GAP_DATA_LENGTH_SET_FAILED:
                handle_ble_evt_gap_data_length_set_failed((ble_evt_gap_data_length_set_failed_t *) event);
                break;
        case BLE_EVT_GAP_CONNECTION_COMPLETED:
                handle_ble_evt_gap_connection_completed((ble_evt_gap_connection_completed_t *) event);
                break;
        case BLE_EVT_GAP_PEER_VERSION:
                handle_ble_evt_gap_peer_version((ble_evt_gap_peer_version_t *) event);
                break;
        case BLE_EVT_GAP_PEER_FEATURES:
                handle_ble_evt_gap_peer_features((ble_evt_gap_peer_features_t *) event);
                break;
#if (dg_configBLE_SECURE_CONNECTIONS == 1)
        case BLE_EVT_GAP_NUMERIC_REQUEST:
                handle_ble_evt_gap_numeric_request((ble_evt_gap_numeric_request_t *) event);
                break;
#endif /* (dg_configBLE_SECURE_CONNECTIONS == 1) */
        case BLE_EVT_GAP_ADDRESS_RESOLUTION_FAILED:
                handle_ble_evt_gap_address_resolution_failed((ble_evt_gap_address_resolution_failed_t *) event);
                break;
        case BLE_EVT_GAP_LTK_MISSING:
                handle_ble_evt_gap_ltk_missing((ble_evt_gap_ltk_missing_t *) event);
                break;
#if (dg_configBLE_2MBIT_PHY == 1)
        case BLE_EVT_GAP_PHY_SET_COMPLETED:
                handle_ble_evt_gap_phy_set_completed((ble_evt_gap_phy_set_completed_t *) event);
                break;
        case BLE_EVT_GAP_PHY_CHANGED:
                handle_ble_evt_gap_phy_changed((ble_evt_gap_phy_changed_t *) event);
                break;
#endif /* (dg_configBLE_2MBIT_PHY == 1) */
        case BLE_EVT_GAP_LOCAL_TX_PWR:
                handle_ble_evt_gap_local_tx_pwr((ble_evt_gap_local_tx_pwr_t *) event);
                break;
        case BLE_EVT_GAP_TX_PWR_REPORT:
                handle_ble_evt_gap_tx_pwr_report((ble_evt_gap_tx_pwr_report_t *) event);
                break;
        case BLE_EVT_GAP_PATH_LOSS_THRES:
                handle_ble_evt_gap_path_loss_thres((ble_evt_gap_path_loss_thres_t *) event);
                break;
#if BLE_SSP_DEBUG
        case BLE_EVT_GAP_LTK:
                handle_ble_evt_gap_ltk((ble_evt_gap_ltk_t *) event);
                break;
#endif
        default:
                return false;
        }

        return true;
}

/*
 * Handler for ble_gap_address_get call.
 * Retrieves and prints currently set BD address of the device.
 * Prints status returned by call.
 */
static bool gap_address_get(int argc, const char **argv)
{
        ble_error_t status;
        own_address_t addr;

        status = ble_gap_address_get(&addr);

        print_command("ble_gap_address_get ( )");
        if (status == BLE_STATUS_OK) {
                print_parameter("Own address: %s", format_own_address(&addr));
        }
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_address_resolve call.
 * Initiates an address resolution procedure on a specified BD address.
 * Prints status returned by call.
 */
static bool gap_address_resolve(int argc, const char **argv)
{
        ble_error_t status;
        bd_address_t addr;
        int idx;

        idx = 2;

        if (!parse_address(argc, argv, &idx, &addr, PRIVATE_ADDRESS)) {
                return false;
        }

        status = ble_gap_address_resolve(addr);

        print_command("ble_gap_address_resolve ( { %s } )", format_bd_address(&addr));
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_address_set call.
 * Sets the address of the device and prints status returned by call
 */
static bool gap_address_set(int argc, const char **argv)
{
        ble_error_t status;
        own_address_t addr;
        uint16_t renew_dur;
        uint8_t rev;

        addr.addr_type = PRIVATE_STATIC_ADDRESS;
        renew_dur = defaultBLE_ADDRESS_RENEW_DURATION;

        if ((argc == 3) && !is_bdaddr_param(argv[2])) {
                return false;
        }

        rev = 0;

        if ((argc > 2) && !is_bdaddr_param(argv[2])) {
                if (!is_bdaddr_param(argv[3])) {
                        return false;
                } else {
                        rev = 1;
                        if (strcmp(argv[2], "pub") == 0) {
                                addr.addr_type = PUBLIC_STATIC_ADDRESS;
                        } else if (strcmp(argv[2], "priv") == 0) {
                                addr.addr_type = PRIVATE_STATIC_ADDRESS;
                        } else if (strcmp(argv[2], "priv-res") == 0) {
                                addr.addr_type = PRIVATE_RANDOM_RESOLVABLE_ADDRESS;
                        } else if (strcmp(argv[2], "priv-nonres") == 0) {
                                addr.addr_type = PRIVATE_RANDOM_NONRESOLVABLE_ADDRESS;
#if (dg_configBLE_PRIVACY_1_2 == 1)
                        } else if (strcmp(argv[2], "priv-cntl") == 0) {
                                addr.addr_type = PRIVATE_CNTL;
#endif
                        } else {
                                return false;
                        }
                }
        }

        if (argc > (3 + rev)) {
                if (!parse_u16(argv[3 + rev], &renew_dur)) {
                        return false;
                }
                if (argc > (4 + rev)) {
                        return false;
                }
        }

        if (argc > (2 + rev)) {
                convert_str_to_own_bdaddr(argv[2 + rev], &addr);
        } else {
                return false;
        }

        status = ble_gap_address_set(&addr, renew_dur);

        print_command("ble_gap_address_set ( %s, %d )", format_own_address(&addr), renew_dur);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_device_name_set call.
 * Sets the device name used for GAP service and prints status returned by call
 */
static bool gap_device_name_set(int argc, const char **argv)
{
        ble_error_t status;
        att_perm_t perm = ATT_PERM_READ | ATT_PERM_WRITE;

        if (argc < 3) {
                return false;
        }

        if (argc > 3) {
                if (!parse_u8(argv[3], &perm)) {
                        return false;
                }
        }

        // argv[2] is holding pointer to name
        status = ble_gap_device_name_set(argv[2], perm);

        print_command("ble_gap_device_name_set ( %s, 0x%02x )", argv[2], perm);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_device_name_get call.
 * Retrieves device name used for GAP service. Prints device name and its length.
 * Prints status returned by call
 */
static bool gap_device_name_get(int argc, const char **argv)
{
        ble_error_t status;
        char name[64];
        uint8_t length = sizeof(name);

        status = ble_gap_device_name_get(name, &length);

        print_command("ble_gap_device_name_get ( )");
        if (status == BLE_STATUS_OK) {
                print_parameter("Device name: %s", name);
                print_parameter("Device name length: %d", length);
        }
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_appearance_set call.
 * Sets the appearance used for GAP service and prints status returned by call
 */
static bool gap_appearance_set(int argc, const char **argv)
{
        ble_error_t status;
        gap_appearance_t appearance = defaultBLE_APPEARANCE;
        att_perm_t perm = ATT_PERM_READ | ATT_PERM_WRITE;

        if (argc > 3) {
                if (!parse_u8(argv[3], &perm)) {
                        return false;
                }
        }

        if (argc > 2) {
                if (!parse_u16(argv[2], &appearance)) {
                        return false;
                }
        }

        status = ble_gap_appearance_set(appearance, perm);

        print_command("ble_gap_appearance_set ( %d, 0x%02x )", appearance, perm);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_appearance_get call.
 * Gets the appearance value used for GAP service and prints status returned by call
 */
static bool gap_appearance_get(int argc, const char **argv)
{
        ble_error_t status;
        gap_appearance_t appearance;

        status = ble_gap_appearance_get(&appearance);

        print_command("ble_gap_appearance_get ( )");
        if (status == BLE_STATUS_OK) {
                print_parameter("Appearance: %d", appearance);
        }
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_per_pref_conn_params_set call.
 * Sets the peripheral preferred connection parameters used for GAP service.
 * Prints connection parameters and status returned by call
 */
static bool gap_per_pref_conn_params_set(int argc, const char **argv)
{
        ble_error_t status;
        gap_conn_params_t params = CFG_CONN_PARAMS;

        if (argc > 5) {
                if (!parse_u16(argv[5], &(params.sup_timeout))) {
                        return false;
                }
        }

        if (argc > 4) {
                if (!parse_u16(argv[4], &(params.slave_latency))) {
                        return false;
                }
        }

        if (argc > 3) {
                if (!parse_u16(argv[3], &(params.interval_max))) {
                        return false;
                }
        }

        if (argc > 2) {
                if (!parse_u16(argv[2], &(params.interval_min))) {
                        return false;
                }
        }

        status = ble_gap_per_pref_conn_params_set(&params);

        print_command("ble_gap_per_pref_conn_params_set ( { %d, %d, %d, %d } )",
                                params.interval_min, params.interval_max, params.slave_latency,
                                params.sup_timeout);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_per_pref_conn_params_get call.
 * Gets the peripheral preferred connection parameters currently set for GAP service.
 * Prints connection parameters and status returned by call
 */
static bool gap_per_pref_conn_params_get(int argc, const char **argv)
{
        ble_error_t status;
        gap_conn_params_t params;

        status = ble_gap_per_pref_conn_params_get(&params);

        print_command("ble_gap_per_pref_conn_params_get ( )");
        if (status == BLE_STATUS_OK) {
                print_gap_conn_params(&params);
        }
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_adv_start call.
 * Starts an advertising air operation. Prints status returned by call
 */
static bool gap_adv_start(int argc, const char **argv)
{
        ble_error_t status;
        gap_conn_mode_t adv_type;

        adv_type = GAP_CONN_MODE_UNDIRECTED;

        if (argc > 2) {
                if (!parse_u8(argv[2], &adv_type)) {
                        return false;
                }
        }

        status = ble_gap_adv_start(adv_type);

        print_command("ble_gap_adv_start ( %d ) ", adv_type);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_adv_stop call.
 * Stops a previously started advertising air operation.
 * Prints status returned by call
 */
static bool gap_adv_stop(int argc, const char **argv)
{
        ble_error_t status;

        status = ble_gap_adv_stop();

        print_command("ble_gap_adv_stop ( )");
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_adv_data_get call.
 * Gets Advertising Data and Scan Response Data used.
 * Prints call parameters and status returned by call.
 * If necessary, the given length is truncated to MAX_LEN.
 */
static bool gap_adv_data_get(int argc, const char **argv)
{
        ble_error_t status;
        uint8_t adv_data[BLE_ADV_DATA_LEN_MAX] = ADVERTISE_DATA;
        uint8_t scan_rsp_data[BLE_SCAN_RSP_LEN_MAX] = SCAN_RESPONSE_DATA;
        char adv_data_str[(BLE_ADV_DATA_LEN_MAX * 2) + 1];
        char scan_rsp_data_str[(BLE_SCAN_RSP_LEN_MAX * 2) + 1];
        uint8_t adv_len_in = 0;
        uint8_t adv_len_out = BLE_ADV_DATA_LEN_MAX;
        uint8_t scan_rsp_len_in = 0;
        uint8_t scan_rsp_len_out = BLE_SCAN_RSP_LEN_MAX;

        if (argc > 3) {
                if (parse_u8(argv[3], &scan_rsp_len_in)) {
                        if (scan_rsp_len_in > BLE_SCAN_RSP_LEN_MAX) {
                                scan_rsp_len_in = BLE_SCAN_RSP_LEN_MAX;
                        }
                }
        }

        if (argc > 2) {
                if (parse_u8(argv[2], &adv_len_in)) {
                        if (adv_len_in > BLE_ADV_DATA_LEN_MAX) {
                                adv_len_in = BLE_ADV_DATA_LEN_MAX;
                        }
                }
        }

        scan_rsp_len_out = scan_rsp_len_in;
        adv_len_out = adv_len_in;
        status = ble_gap_adv_data_get(&adv_len_out, adv_data, &scan_rsp_len_out, scan_rsp_data);

        /* Create printable strings */
        memcpy(adv_data_str, format_adv_data(adv_data, adv_len_out), (adv_len_out * 2) + 1);
        memcpy(scan_rsp_data_str, format_adv_data(scan_rsp_data, scan_rsp_len_out), (scan_rsp_len_out * 2) + 1);

        print_command("ble_gap_adv_data_get ( %d, %d, %s, %d, %d, %s )", adv_len_in, adv_len_out,
                adv_data_str, scan_rsp_len_in, scan_rsp_len_out, scan_rsp_data_str);

        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_adv_data_set call.
 * Sets Advertising Data and Scan Response Data used.
 * Prints call parameters and status returned by call
 */
static bool gap_adv_data_set(int argc, const char **argv)
{
        ble_error_t status;
        uint8_t adv_data[BLE_ADV_DATA_LEN_MAX] = ADVERTISE_DATA;
        uint8_t scan_rsp_data[BLE_SCAN_RSP_LEN_MAX] = SCAN_RESPONSE_DATA;
        char adv_data_str[(BLE_ADV_DATA_LEN_MAX * 2) + 1];
        char scan_rsp_data_str[(BLE_SCAN_RSP_LEN_MAX * 2) + 1];
        int adv_len = ADVERTISE_DATA_LENGTH;
        int rsp_len = SCAN_RESPONSE_DATA_LENGTH;

        if (argc > 3) {
                rsp_len = strlen(argv[3]) / 2;
                if (rsp_len > BLE_SCAN_RSP_LEN_MAX) {
                        return false;
                }
                if (str_to_hex(argv[3], scan_rsp_data, rsp_len) < 1) {
                        return false;
                }
        }

        if (argc > 2) {
                adv_len = strlen(argv[2]) / 2;
                if (adv_len > BLE_ADV_DATA_LEN_MAX) {
                        return false;
                }
                if (str_to_hex(argv[2], adv_data, adv_len) < 1) {
                        return false;
                }
        }

        status = ble_gap_adv_data_set(adv_len, adv_data, rsp_len, scan_rsp_data);

        /* Create printable strings */
        memcpy(adv_data_str, format_adv_data(adv_data, adv_len), (adv_len * 2) + 1);
        adv_data_str[adv_len * 2] = '\0';
        memcpy(scan_rsp_data_str, format_adv_data(scan_rsp_data, rsp_len), (rsp_len * 2) + 1);
        scan_rsp_data_str[rsp_len * 2] = '\0';

        print_command("ble_gap_adv_data_set ( %d, %s, %d, %s )",
                adv_len, adv_data_str, rsp_len, scan_rsp_data_str);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_adv_intv_get call.
 * Gets the minimum and maximum intervals to be used for advertising.
 * Prints status returned by call
 */
static bool gap_adv_intv_get(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t adv_intv_min;
        uint16_t adv_intv_max;

        status = ble_gap_adv_intv_get(&adv_intv_min, &adv_intv_max);

        print_command("ble_gap_adv_intv_get( %d, %d )", adv_intv_min, adv_intv_max);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_adv_intv_set call.
 * Sets the minimum and maximum intervals to be used for advertising.
 * Prints status returned by call
 */
static bool gap_adv_intv_set(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t adv_intv_min;
        uint16_t adv_intv_max;

        if (argc < 4) {
                return false;
        }

        if (!parse_u16(argv[3], &adv_intv_max)) {
                return false;
        }

        if (!parse_u16(argv[2], &adv_intv_min)) {
                return false;
        }

        status = ble_gap_adv_intv_set(adv_intv_min, adv_intv_max);

        print_command("ble_gap_adv_intv_set( %d, %d )", adv_intv_min, adv_intv_max);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_adv_chnl_map_get call.
 * Gets advertising channel map to be used for advertising and prints status returned by call
 */
static bool gap_adv_chnl_map_get(int argc, const char **argv)
{
        ble_error_t status;
        uint8_t chnl_map;

        status = ble_gap_adv_chnl_map_get(&chnl_map);

        print_command("ble_gap_adv_chnl_map_get ( %d )", chnl_map);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_adv_chnl_map_set call.
 * Sets advertising channel map to be used for advertising and prints status returned by call
 */
static bool gap_adv_chnl_map_set(int argc, const char **argv)
{
        ble_error_t status;
        uint8_t chnl_map;

        if (argc < 3) {
                return false;
        }

        if (!parse_u8(argv[2], &chnl_map)) {
                return false;
        }

        status = ble_gap_adv_chnl_map_set(chnl_map);

        print_command("ble_gap_adv_chnl_map_set ( %d )", chnl_map);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_adv_mode_get call.
 * Gets the discoverability mode used for advertising and prints status returned by call
 */
static bool gap_adv_mode_get(int argc, const char **argv)
{
        ble_error_t status;
        gap_disc_mode_t adv_mode;

        status = ble_gap_adv_mode_get(&adv_mode);

        print_command("ble_gap_adv_mode_get ( %d )", adv_mode);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_adv_mode_set call.
 * Sets the discoverability mode used for advertising and prints status returned by call
 */
static bool gap_adv_mode_set(int argc, const char **argv)
{
        ble_error_t status;
        gap_disc_mode_t adv_mode;

        adv_mode = GAP_DISC_MODE_NON_DISCOVERABLE;

        if (argc > 2) {
                if (!parse_u8(argv[2], &adv_mode)) {
                        return false;
                }
        }

        status = ble_gap_adv_mode_set(adv_mode);

        print_command("ble_gap_adv_mode_set ( %d )", adv_mode);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler ble_gap_adv_filt_policy_get call.
 * Gets the filtering policy used for advertising and prints status returned by call
 */
static bool gap_adv_filt_policy_get(int argc, const char **argv)
{
        ble_error_t status;
        adv_filt_pol_t filt_policy;

        status = ble_gap_adv_filt_policy_get(&filt_policy);

        print_command("ble_gap_adv_filt_policy_get ( %d )", filt_policy);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler ble_gap_adv_filt_policy_set call.
 * Sets the filtering policy used for advertising and prints status returned by call
 */
static bool gap_adv_filt_policy_set(int argc, const char **argv)
{
        ble_error_t status;
        adv_filt_pol_t filt_policy;

        filt_policy = ADV_ALLOW_SCAN_ANY_CONN_ANY;

        if (argc > 2) {
               if (!parse_u8(argv[2], &filt_policy)) {
                       return false;
               }
        }

        status = ble_gap_adv_filt_policy_set(filt_policy);

        print_command("ble_gap_adv_filt_policy_set ( %d )", filt_policy);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_adv_direct_address_get call.
 * Gets the peer address used for directed advertising and prints status returned by call
 */
static bool gap_adv_direct_address_get(int argc, const char **argv)
{
        ble_error_t status;
        bd_address_t addr;

        status = ble_gap_adv_direct_address_get(&addr);

        print_command("ble_gap_adv_direct_address_get ( { %s } )", format_bd_address(&addr));
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_adv_direct_address_set call.
 * Sets the peer address used for directed advertising and prints status returned by call
 */
static bool gap_adv_direct_address_set(int argc, const char **argv)
{
        ble_error_t status;
        bd_address_t addr;
        int idx;

        idx = 2;

        if (!parse_address(argc, argv, &idx, &addr, PUBLIC_ADDRESS)) {
                return false;
        } else if (argc > idx) {
                return false;
        }

        status = ble_gap_adv_direct_address_set(&addr);

        print_command("ble_gap_adv_direct_address_set ( { %s } )", format_bd_address(&addr));
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_scan_start call.
 * Initiates a scan procedure.
 * Prints scan parameters and status returned by call
 */
static bool gap_scan_start(int argc, const char **argv)
{
        ble_error_t status;
        gap_scan_params_t scan_params;
        gap_scan_type_t type = CFG_SCAN_TYPE;
        gap_scan_mode_t mode = CFG_SCAN_MODE;
        bool filt_dup = CFG_SCAN_FILT_DUPLT;
        bool wlist = CFG_SCAN_FILT_WLIST;
        uint16_t interval, window;

        ble_gap_scan_params_get(&scan_params);

        window = scan_params.window;
        interval = scan_params.interval;

        if (argc > 7) {
                if (!parse_bool(argv[7], &filt_dup)) {
                        return false;
                }
        }

        if (argc > 6) {
                if (!parse_bool(argv[6], &wlist)) {
                        return false;
                }
        }

        if (argc > 5) {
                if (!parse_u16(argv[5], &window)) {
                        return false;
                }
        }

        if (argc > 4) {
                if (!parse_u16(argv[4], &interval)) {
                        return false;
                }
        }

        if (argc > 3) {
                if (!parse_u8(argv[3], &mode)) {

                }
        }

        if (argc > 2) {
                if (strcmp(argv[2], "active") == 0) {
                        type = GAP_SCAN_ACTIVE;
                } else if (strcmp(argv[2], "passive") == 0) {
                        type = GAP_SCAN_PASSIVE;
                } else {
                        return false;
                }
        }

        status = ble_gap_scan_start(type, mode, interval, window, wlist, filt_dup);

        print_command("ble_gap_scan_start ( %s, %d, %d, %d, %s, %s )",
                                        type == GAP_SCAN_ACTIVE ? "active" : "passive", mode,
                                        interval, window, wlist ? "true" : "false",
                                        filt_dup ? "true" : "false");
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_scan_stop call.
 * Stops scan procedure previously started by ble_gap_scan_start().
 * Prints status returned by call
 */
static bool gap_scan_stop(int argc, const char **argv)
{
        ble_error_t status;

        status = ble_gap_scan_stop();

        print_command("ble_gap_scan_stop ( )");
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_scan_params_get call.
 * Retrieves the scan parameters used when a connection is initiated.
 * Prints retrieved scan parameters and status returned by call
 */
static bool gap_scan_params_get(int argc, const char **argv)
{
        ble_error_t status;
        gap_scan_params_t scan_params;

        status = ble_gap_scan_params_get(&scan_params);

        print_command("ble_gap_scan_params_get ( )");
        if (status == BLE_STATUS_OK) {
                print_gap_scan_params(&scan_params);
        }
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_scan_params_set call.
 * Sets the scan parameters used for initiated connections.
 * Prints scan parameters and status returned by call
 */
static bool gap_scan_params_set(int argc, const char **argv)
{
        ble_error_t status;
        gap_scan_params_t scan_params;

        if (argc < 4) {
                return false;
        }

        if (!parse_u16(argv[3], &(scan_params.window))) {
                return false;
        }

        if (!parse_u16(argv[2], &(scan_params.interval))) {
                return false;
        }

        status = ble_gap_scan_params_set(&scan_params);

        print_command("ble_gap_scan_params_set ( { %d, %d } )", scan_params.interval,
                                                                        scan_params.window);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_connect call.
 * Initiates a direct connection procedure to a specified peer device.
 * Prints device address, connection parameters and status returned by call
 */
static bool gap_connect(int argc, const char **argv)
{
        gap_conn_params_t params = CFG_CONN_PARAMS;
        ble_error_t status;
        bd_address_t addr;
        int idx;

        // Set first index of argv
        idx = 2;

        if (!parse_address(argc, argv, &idx, &addr, PUBLIC_ADDRESS)) {
                return false;
        }

        if (idx < argc) {
                if (!parse_u16(argv[idx], &(params.interval_min))) {
                        return false;
                }
                idx++;
        }

        if (idx < argc) {
                if (!parse_u16(argv[idx], &(params.interval_max))) {
                        return false;
                }
                idx++;
        }

        if (idx < argc) {
                if (!parse_u16(argv[idx], &(params.slave_latency))) {
                        return false;
                }
                idx++;
        }

        if (idx < argc) {
                if (!parse_u16(argv[idx], &(params.sup_timeout))) {
                        return false;
                }
                idx++;
                if (argc > idx) {
                        return false;
                }
        }

        status = ble_gap_connect(&addr, &params);

        print_command("ble_gap_connect ( { %s }, { %d, %d, %d, %d } )", format_bd_address(&addr),
                                params.interval_min, params.interval_max, params.slave_latency,
                                params.sup_timeout);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_connect_ce call.
 * Initiates a direct connection procedure to a specified peer device, also setting
 * ce_len_min and ce_len_max parameters.
 * Prints device address, connection parameters and status returned by call
 */
static bool gap_connect_ce(int argc, const char **argv)
{
        gap_conn_params_t params = CFG_CONN_PARAMS;
        uint16_t ce_len_min = 0;
        uint16_t ce_len_max = 0;
        ble_error_t status;
        bd_address_t addr;
        int idx;

        if (argc < 9) {
                return false;
        }

        // Set first index of argv
        idx = 2;

        if (!parse_address(argc, argv, &idx, &addr, PUBLIC_ADDRESS)) {
                return false;
        }

        if ((argc < 10) && (idx == 4)) {
                return false;
        }

        if (!parse_u16(argv[idx++], &(params.interval_min))) {
                return false;
        }

        if (!parse_u16(argv[idx++], &(params.interval_max))) {
                return false;
        }

        if (!parse_u16(argv[idx++], &(params.slave_latency))) {
                return false;
        }

        if (!parse_u16(argv[idx++], &(params.sup_timeout))) {
                return false;
        }

        if (!parse_u16(argv[idx++], &ce_len_min)) {
                return false;
        }

        if (!parse_u16(argv[idx++], &ce_len_max)) {
                return false;
        }
        if (argc > idx) {
                return false;
        }

        status = ble_gap_connect_ce(&addr, &params, ce_len_min, ce_len_max);

        print_command("ble_gap_connect_ce ( { %s }, { %d, %d, %d, %d, %d, %d } )",
                                format_bd_address(&addr), params.interval_min, params.interval_max,
                                params.slave_latency, params.sup_timeout, ce_len_min, ce_len_max);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_connect_cancel call.
 * Cancels a previously started connection procedure and prints status returned by call
 */
static bool gap_connect_cancel(int argc, const char **argv)
{
        ble_error_t status;

        status = ble_gap_connect_cancel();

        print_command("ble_gap_connect_cancel ( )");
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_disconnect call.
 * Initiates a disconnection procedure on an established link and prints status returned by call
 */
static bool gap_disconnect(int argc, const char **argv)
{
        ble_error_t status;
        ble_hci_error_t reason;
        uint16_t conn_idx = BLE_CONN_IDX_INVALID;

        reason = BLE_HCI_ERROR_REMOTE_USER_TERM_CON;

        if (argc < 3) {
                return false;
        }

        if (argc > 3) {
                if (!parse_u8(argv[3], &reason)) {
                        return false;
                }
        }

        if (argc > 2) {
                if (!parse_u16(argv[2], &conn_idx)) {
                        return false;
                }
        }

        status = ble_gap_disconnect(conn_idx, reason);

        print_command("ble_gap_disconnect ( %d, 0x%02x )", conn_idx, reason);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_peer_version_get call.
 * Initiates a Version Exchange and prints status returned by call.
 */
static bool gap_peer_version_get(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t conn_idx;

        if (argc < 3) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        status = ble_gap_peer_version_get(conn_idx);

        print_command("ble_gap_peer_version_get ( %d )", conn_idx);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_peer_features_get call.
 * Initiates a Feature Exchange and prints status returned by call.
 */
static bool gap_peer_features_get(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t conn_idx;

        if (argc < 3) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        status = ble_gap_peer_features_get(conn_idx);

        print_command("ble_gap_peer_features_get ( %d )", conn_idx);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_disconnect call.
 * Retrieves the RSSI of an active connection and prints status returned by call
 */
static bool gap_conn_rssi_get(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t conn_idx;
        int8_t conn_rssi;

        if (argc < 3) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        status = ble_gap_conn_rssi_get(conn_idx, &conn_rssi);

        print_command("ble_gap_conn_rssi_get ( %d )", conn_idx);
        if (status == BLE_STATUS_OK) {
                print_parameter("Connection RSSI: %d", conn_rssi);
        }
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_role_get call.
 * Gets the GAP role of the device and prints status returned by call
 */
static bool gap_role_get(int argc, const char **argv)
{
        ble_error_t status;
        gap_role_t role;

        status = ble_gap_role_get(&role);

        print_command("ble_gap_role_get ( 0x%02x )", role);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_role_set call.
 * Sets the GAP role of the device and prints status returned by call
 */
static bool gap_role_set(int argc, const char **argv)
{
        ble_error_t status;
        gap_role_t role;

        if (argc < 3) {
                return false;
        }

        if (!parse_u8(argv[2], &role)) {
                return false;
        }

        status = ble_gap_role_set(role);

        print_command("ble_gap_role_set ( 0x%02x )", role);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_mtu_size_get call.
 * Retrieves the MTU size that is used in exchange MTU transactions.
 * Prints status returned by call
 */
static bool gap_mtu_size_get(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t mtu_size;

        status = ble_gap_mtu_size_get(&mtu_size);

        print_command("ble_gap_mtu_size_get ( )");
        if (status == BLE_STATUS_OK) {
                print_parameter("MTU size: %d", mtu_size);
        }
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_mtu_size_set call.
 * Sets the MTU size and prints status returned by call
 */
static bool gap_mtu_size_set(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t mtu_size;

        if (argc < 3) {
                return false;
        }

        if (!parse_u16(argv[2], &mtu_size)) {
                return false;
        }

        status = ble_gap_mtu_size_set(mtu_size);

        print_command("ble_gap_mtu_size_set ( %d )", mtu_size);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_channel_map_get call.
 * Gets the currently set channel map of the device and prints status returned by call
 */
static bool gap_channel_map_get(int argc, const char **argv)
{
        ble_error_t status;
        uint64_t chnl_map;

        status = ble_gap_channel_map_get(&chnl_map);

        print_command("ble_gap_channel_map_get ( )");
        if (status == BLE_STATUS_OK) {
                print_parameter("Channel map: %" PRIu64 " ", chnl_map);
        }
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_channel_map_set call.
 * Sets the channel map of the device and prints status returned by call
 */
static bool gap_channel_map_set(int argc, const char **argv)
{
        ble_error_t status;
        uint64_t chnl_map;

        if (argc < 3) {
                return false;
        }

        if (!parse_u64(argv[2], &chnl_map)) {
                return false;
        }

        status = ble_gap_channel_map_set(chnl_map);

        print_command("ble_gap_channel_map_set ( %" PRIu64 " )", chnl_map);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_conn_param_update call.
 * Initiates a connection parameter update. Prints call parameters and status returned by call
 */
static bool gap_conn_param_update(int argc, const char **argv)
{
        gap_conn_params_t params;
        ble_error_t status;
        uint16_t conn_idx = 0;

        if (argc < 7) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        if (!parse_u16(argv[3], &(params.interval_min))) {
                return false;
        }

        if (!parse_u16(argv[4], &(params.interval_max))) {
                return false;
        }

        if (!parse_u16(argv[5], &(params.slave_latency))) {
                return false;
        }

        if (!parse_u16(argv[6], &(params.sup_timeout))) {
                return false;
        }

        status = ble_gap_conn_param_update(conn_idx, &params);

        print_command("ble_gap_conn_param_update: ( %d, { %d, %d, %d, %d })", conn_idx,
                                params.interval_min, params.interval_max, params.slave_latency,
                                params.sup_timeout);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_conn_param_update_reply.
 * Sends reply to a connection parameter update request and prints status returned by call
 */
static bool gap_conn_param_update_reply(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t conn_idx;
        bool accept;

        if (argc < 4) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        if (!parse_bool(argv[3], &accept)) {
                return false;
        }

        status = ble_gap_conn_param_update_reply(conn_idx, accept);

        print_command("ble_gap_conn_param_update_reply: ( %d, %s )", conn_idx,
                                                                        accept ? "true" : "false");
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_pair.
 * Starts pairing or bonding procedure and prints status returned by call
 */
static bool gap_pair(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t conn_idx;
        bool bond;

        if (argc < 4) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        if (!parse_bool(argv[3], &bond)) {
                return false;
        }

        status = ble_gap_pair(conn_idx, bond);

        print_command("ble_gap_pair ( %d, %s )", conn_idx, bond ? "true" : "false");
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_pair_reply.
 * Sends reply to pair request and prints status returned by call
 */
static bool gap_pair_reply(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t conn_idx;
        bool accept;
        bool bond = false;

        if (argc < 4) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        if (!parse_bool(argv[3], &accept)) {
                return false;
        }

        if (argc > 4 && !parse_bool(argv[4], &bond)) {
                return false;
        }

        status = ble_gap_pair_reply(conn_idx, accept, bond);

        print_command("ble_gap_pair_reply ( %d, %s, %s )", conn_idx, accept ? "true" : "false",
                                                                        bond ? "true" : "false");
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_get_connected call.
 * Gets connected devices list and prints status returned by call
 */
static bool gap_get_connected(int argc, const char **argv)
{
        ble_error_t status;
        uint8_t length;
        uint16_t *conn_idx;
        int i;

        status = ble_gap_get_connected(&length, &conn_idx);

        print_command("ble_gap_get_connected ( )");
        if (status == BLE_STATUS_OK) {
                print_category("Number of connected devices: %d", length);

                for (i = 0; i < length; ++i) {
                        print_parameter("Connection index [%d]: %d", i, conn_idx[i]);
                }
        }
        print_status("return: %s", get_status(status));

        OS_FREE(conn_idx);

        return true;
}

/*
 * Handler for ble_gap_get_bonded call.
 * Gets bonded devices list and prints status returned by call
 */
static bool gap_get_bonded(int argc, const char **argv)
{
        ble_error_t status;
        uint8_t length;
        bd_address_t *addr;
        int i;

        status = ble_gap_get_bonded(&length, &addr);

        print_command("ble_gap_get_bonded ( )");
        if (status == BLE_STATUS_OK) {
                print_category("Number of bonded devices: %d", length);

                for (i = 0; i < length; ++i) {
                        print_parameter("Device[%d] address: %s", i, format_bd_address(&addr[i]));
                }
        }
        print_status("return: %s", get_status(status));

        OS_FREE(addr);

        return true;
}

/*
 * Handler for ble_gap_get_io_cap call.
 * Gets adapter IO Capabilities and prints status returned by call
 */
static bool gap_get_io_cap(int argc, const char **argv)
{
        ble_error_t status;
        gap_io_cap_t io_cap;

        status = ble_gap_get_io_cap(&io_cap);

        print_command("ble_gap_get_io_cap ( 0x%02x )", io_cap);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_set_io_cap call.
 * Sets adapter IO Capabilities and prints status returned by call
 */
static bool gap_set_io_cap(int argc, const char **argv)
{
        ble_error_t status;
        gap_io_cap_t io_cap;

        if (argc < 3) {
                return false;
        }

        if (!parse_u8(argv[2], &io_cap)) {
                return false;
        }

        status = ble_gap_set_io_cap(io_cap);

        print_command("ble_gap_set_io_cap ( 0x%02x )", io_cap);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler ble_gap_passkey_reply.
 * Sends passkey reply to passkey request and prints status returned by call
 */
static bool gap_passkey_reply(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t conn_idx;
        bool accept;
        uint32_t passkey = 0;
        char *check_ptr;

        if (argc < 4) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        if (!parse_bool(argv[3], &accept)) {
                return false;
        }

        if (argc > 4) {
                passkey = (uint32_t)strtoul(argv[4], &check_ptr, 10);

                if (*argv[4] == '\0' || *check_ptr != '\0') {
                        return false;
                }
        }

        status = ble_gap_passkey_reply(conn_idx, accept, passkey);

        print_command("ble_gap_passkey_reply ( %d, %s, %" PRIu32 " )", conn_idx,
                                                                accept ? "true" : "false", passkey);
        print_status("return: %s", get_status(status));

        return true;
}

#if (dg_configBLE_SECURE_CONNECTIONS == 1)
/*
 * Handler ble_gap_numeric_reply.
 * Sends numeric comparison reply to numeric comparison request and prints status returned by call
 */
static bool gap_numeric_reply(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t conn_idx;
        bool accept;

        if (argc < 3) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        if (!parse_bool(argv[3], &accept)) {
                return false;
        }

        status = ble_gap_numeric_reply(conn_idx, accept);

        print_command("ble_gap_numeric_reply ( %d, %s )", conn_idx,
                                                                accept ? "true" : "false");
        print_status("return: %s", get_status(status));

        return true;
}
#endif /* (dg_configBLE_SECURE_CONNECTIONS == 1) */

/*
 * Handler for ble_gap_get_sec_level.
 * Gets connection security level and prints status returned by call
 */
static bool gap_get_sec_level(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t conn_idx;
        gap_sec_level_t level;

        if (argc < 3) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        status = ble_gap_get_sec_level(conn_idx, &level);

        print_command("ble_gap_get_sec_level ( %d )", conn_idx);
        if (status == BLE_STATUS_OK) {
                print_parameter("Connection security level: %d", (level + 1));
        }
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_unpair call.
 * Unpairs remote device and prints status returned by call
 */
static bool gap_unpair(int argc, const char **argv)
{
        ble_error_t status;
        bd_address_t addr;
        int idx;

        idx = 2;

        if (!parse_address(argc, argv, &idx, &addr, PUBLIC_ADDRESS)) {
                return false;
        } else if (argc > idx) {
                return false;
        }

        status = ble_gap_unpair(&addr);

        print_command("ble_gap_unpair ( %s )", format_bd_address(&addr));
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_set_sec_level.
 * Sets connection security level and prints status returned by call
 */
static bool gap_set_sec_level(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t conn_idx;
        gap_sec_level_t level;

        if (argc < 4) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        if (!parse_u8(argv[3], &level)) {
                return false;
        }

        status = ble_gap_set_sec_level(conn_idx, (level - 1));

        print_command("ble_gap_set_sec_level ( %d, %d )", conn_idx, level);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_data_length_set call.
 * Sets the data length used for TX. Prints call parameters and status returned by call
 */
static bool gap_data_length_set(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t conn_idx = BLE_CONN_IDX_INVALID;
        uint16_t tx_length;
        uint16_t tx_time;

        if (argc < 3) {
                return false;
        }

        if (!parse_u16(argv[2], &tx_length)) {
                return false;
        }

        if (argc > 3) {
                if (!parse_u16(argv[2], &conn_idx)) {
                        return false;
                }

                if (!parse_u16(argv[3], &tx_length)) {
                        return false;
                }
        }

        tx_time = BLE_DATA_LENGTH_TO_TIME(tx_length);

        if (argc > 4) {
                if (!parse_u16(argv[4], &tx_time)) {
                        return false;
                }
                if (argc > 5) {
                        return false;
                }
        }

        status = ble_gap_data_length_set(conn_idx, tx_length, tx_time);

        print_command("ble_gap_data_length_set: ( %d, %d, %d )", conn_idx, tx_length, tx_time);
        print_status("return: %s", get_status(status));

        return true;
}

#if (dg_configBLE_2MBIT_PHY == 1)
/*
 * Handler for ble_gap_phy_get call.
 * Gets the PHY used for TX and RX for a given connection.
 * Prints call parameter, status returned by call, TX PHY and RX PHY used.
 */
static bool gap_phy_get(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t conn_idx;
        ble_gap_phy_t tx_phy;
        ble_gap_phy_t rx_phy;

        if (argc < 3) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        status = ble_gap_phy_get(conn_idx, &tx_phy, &rx_phy);

        print_command("ble_gap_phy_get ( %d )", conn_idx);
        if (status == BLE_STATUS_OK) {
                print_parameter("Transmitter PHY: %s", ( conn_idx == BLE_CONN_IDX_INVALID ) ?
                                                         get_phy_pref(tx_phy) : get_phy(tx_phy) );
                print_parameter("Receiver PHY:    %s", ( conn_idx == BLE_CONN_IDX_INVALID ) ?
                                                         get_phy_pref(rx_phy) : get_phy(rx_phy) );
        }
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_phy_set call.
 * Sets the RX and TX PHY used for new connections or for a given connection.
 * Prints call parameters and status returned by call.
 */
static bool gap_phy_set(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t conn_idx;
        ble_gap_phy_pref_t tx_phy_pref;
        ble_gap_phy_pref_t rx_phy_pref;

        if (argc < 5) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        if (!parse_u8(argv[3], &tx_phy_pref)) {
                return false;
        }

        if (!parse_u8(argv[4], &rx_phy_pref)) {
                return false;
        }

        status = ble_gap_phy_set(conn_idx, tx_phy_pref, rx_phy_pref);

        print_command("ble_gap_phy_set( %d, %d, %d )", conn_idx, tx_phy_pref, rx_phy_pref);
        print_status("return: %s", get_status(status));

        return true;
}
#endif /* (dg_configBLE_2MBIT_PHY == 1) */

/*
 * Handler for ble_gap_tx_power_set.
 * Sets air operations TX power and prints status returned by call
 */
static bool gap_tx_power_set(int argc, const char **argv)
{
        ble_error_t status;
        uint8_t air_operation;
        gap_tx_power_t tx_power;

        if (argc < 4) {
                return false;
        }

        if (!parse_u8(argv[2], &air_operation)) {
                return false;
        }

        if (!parse_u8(argv[3], &tx_power)) {
                return false;
        }

        status = ble_gap_tx_power_set(air_operation, tx_power);

        print_command("ble_gap_tx_power_set ( %d, %d )", air_operation, tx_power);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_conn_tx_power_set.
 * Sets connection TX power level and prints status returned by call
 */
static bool gap_conn_tx_power_set(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t conn_idx;
        gap_tx_power_t tx_power;

        if (argc < 4) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        if (!parse_u8(argv[3], &tx_power)) {
                return false;
        }

        status = ble_gap_conn_tx_power_set(conn_idx, tx_power);

        print_command("ble_gap_conn_tx_power_set ( %d, %d )", conn_idx, tx_power);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_adv_set_permutation.
 * Sets advertising channel permutation and prints status returned by call
 */
static bool gap_adv_set_permutation(int argc, const char **argv)
{
        ble_error_t status;
        uint8_t pid;

        if (argc != 3) {
                return false;
        }

        if (!parse_u8(argv[2], &pid)) {
                return false;
        }

        status = ble_gap_adv_set_permutation(pid);

        print_command("ble_gap_adv_set_permutation ( %d )", pid);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_local_tx_power_get.
 * Gets the current and maximum Tx power levels of the local Controller
 * and prints status returned by call.
 */
static bool gap_local_tx_power_get(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t conn_idx;
        ble_gap_phy_t phy;

        if (argc < 4) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        if (!parse_u8(argv[3], &phy)) {
                return false;
        }

        status = ble_gap_local_tx_power_get(conn_idx, phy);

        print_command("ble_gap_local_tx_power_get ( %d, %d )", conn_idx, phy);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_remote_tx_power_get.
 * Gets Tx power levels of the remote Controller and prints status returned by call.
 */
static bool gap_remote_tx_power_get(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t conn_idx;
        ble_gap_phy_t phy;

        if (argc < 4) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        if (!parse_u8(argv[3], &phy)) {
                return false;
        }

        status = ble_gap_remote_tx_power_get(conn_idx, phy);

        print_command("ble_gap_remote_tx_power_get ( %d, %d )", conn_idx, phy);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_path_loss_report_params_set.
 * Gets Tx power levels of the remote Controller and prints status returned by call.
 */
static bool gap_path_loss_report_params_set(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t conn_idx;
        uint8_t high_thres;
        uint8_t high_hyst;
        uint8_t low_thres;
        uint8_t low_hyst;
        uint16_t min_time_spent;


        if (argc < 8) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        if (!parse_u8(argv[3], &high_thres)) {
                return false;
        }

        if (!parse_u8(argv[4], &high_hyst)) {
                return false;
        }

        if (!parse_u8(argv[5], &low_thres)) {
                return false;
        }

        if (!parse_u8(argv[6], &low_hyst)) {
                return false;
        }

        if (!parse_u16(argv[7], &min_time_spent)) {
                return false;
        }

        status = ble_gap_path_loss_report_params_set(conn_idx, high_thres, high_hyst, low_thres, low_hyst, min_time_spent);

        print_command("ble_gap_path_loss_report_params_set ( %d, %d, %d, %d, %d, %d)", conn_idx, high_thres, high_hyst, low_thres, low_hyst, min_time_spent);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_path_loss_report_en.
 * Enable or disable path loss reporting.
 */
static bool gap_path_loss_report_en(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t conn_idx;
        bool enable;

        if (argc < 4) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        if (!parse_bool(argv[3], &enable)) {
                return false;
        }

        status = ble_gap_path_loss_report_en(conn_idx, enable);

        print_command("ble_gap_path_loss_report_en ( %d, %d )", conn_idx, enable);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_tx_power_report_en​.
 * Enable or disable reporting of TX power for the local and remote Controller
 */
static bool gap_tx_power_report_en(int argc, const char **argv)
{
        ble_error_t status;
        uint16_t conn_idx;
        bool local_enable;
        bool remote_enable;

        if (argc < 5) {
                return false;
        }

        if (!parse_u16(argv[2], &conn_idx)) {
                return false;
        }

        if (!parse_bool(argv[3], &local_enable)) {
                return false;
        }

        if (!parse_bool(argv[4], &remote_enable)) {
                return false;
        }

        status = ble_gap_tx_power_report_en(conn_idx, local_enable, remote_enable);

        print_command("ble_gap_tx_power_report_en ( %d, %d, %d)", conn_idx, local_enable, remote_enable);
        print_status("return: %s", get_status(status));

        return true;
}

/*
 * Handler for ble_gap_rf_path_compensation_set.
 * Sets the RF TX/RX path compensation values and prints status returned by call
 */
static bool gap_rf_path_compensation_set(int argc, const char **argv)
{
        ble_error_t status;
        int16_t rf_tx_path_compensation;
        int16_t rf_rx_path_compensation;

        if (argc < 4) {
                return false;
        }

        if (!parse_16(argv[2], &rf_tx_path_compensation)) {
                return false;
        }

        if (!parse_16(argv[3], &rf_rx_path_compensation)) {
                return false;
        }

        status = ble_gap_rf_path_compensation_set(rf_tx_path_compensation, rf_rx_path_compensation);

        print_command("ble_gap_rf_path_compensation_set ( %d, %d )", rf_tx_path_compensation, rf_rx_path_compensation);
        print_status("return: %s", get_status(status));

        return true;
}

static const debug_handler_t gap_handlers[] = {
        { "address_get", "", gap_address_get },
        { "address_resolve", "[public|private] <bd_addr>", gap_address_resolve },
#if dg_configBLE_PRIVACY_1_2
        { "address_set", "[pub|priv|priv-res|priv-nonres|priv-cntl] <bd_addr> [renew_dur]",
                                                                                gap_address_set },
#else
        { "address_set", "[pub|priv|priv-res|priv-nonres] <bd_addr> [renew_dur]", gap_address_set },
#endif
        { "dev_name_set", "<dev_name> [permissions]", gap_device_name_set },
        { "dev_name_get", "", gap_device_name_get },
        { "appearance_set", "[appearance [permissions]]", gap_appearance_set },
        { "appearance_get", "", gap_appearance_get },
        { "per_pref_conn_params_set", "[interval_min [interval_max [slave_latency [sup_timeout]]]]",
                                                                gap_per_pref_conn_params_set },
        { "per_pref_conn_params_get", "", gap_per_pref_conn_params_get },
        { "adv_start", "[adv_type]", gap_adv_start },
        { "adv_stop", "", gap_adv_stop },
        { "adv_data_get", "[adv_data_len [scan_rsp_data_len]]", gap_adv_data_get },
        { "adv_data_set", "[adv_data [scan_rsp_data]]", gap_adv_data_set },
        { "adv_intv_get", "", gap_adv_intv_get },
        { "adv_intv_set", "<adv_intv_min> <adv_intv_max>", gap_adv_intv_set },
        { "adv_chnl_map_get", "", gap_adv_chnl_map_get },
        { "adv_chnl_map_set", "<chnl_map>", gap_adv_chnl_map_set },
        { "adv_mode_get", "", gap_adv_mode_get },
        { "adv_mode_set", "[mode]", gap_adv_mode_set },
        { "adv_filt_policy_get", "", gap_adv_filt_policy_get },
        { "adv_filt_policy_set", "[filt_policy]", gap_adv_filt_policy_set },
        { "adv_direct_address_get", "", gap_adv_direct_address_get },
        { "adv_direct_address_set", "[public|private] <bd_addr>", gap_adv_direct_address_set },
        { "scan_start", "[active|passive [scan_mode [interval [window [wlist [dupl]]]]]]",
                                                                                gap_scan_start },
        { "scan_stop", "", gap_scan_stop },
        { "scan_params_get", "", gap_scan_params_get },
        { "scan_params_set", "<interval> <window>", gap_scan_params_set },
        { "connect", "[public|private] <bd_addr> [interval_min [interval_max [slave_latency"
                                                                " [sup_timeout]]]]", gap_connect },
        { "connect_ce", "[public|private] <bd_addr> <interval_min> <interval_max> <slave_latency>"
                                    " <sup_timeout> <ce_len_min> <ce_len_max>", gap_connect_ce },
        { "connect_cancel", "", gap_connect_cancel },
        { "disconnect", "<conn_idx> [reason]", gap_disconnect },
        { "peer_version_get", "<conn_idx>", gap_peer_version_get },
        { "peer_features_get", "<conn_idx>", gap_peer_features_get },
        { "conn_rssi_get", "<conn_idx>", gap_conn_rssi_get },
        { "role_get", "", gap_role_get },
        { "role_set", "<role>", gap_role_set },
        { "mtu_size_get", "", gap_mtu_size_get },
        { "mtu_size_set", "<mtu_size>", gap_mtu_size_set },
        { "channel_map_get", "", gap_channel_map_get },
        { "channel_map_set", "<chnl_map>", gap_channel_map_set },
        { "conn_param_update", "<conn_idx> <interval_min> <interval_max> <slave_latency>"
                                                        " <sup_timeout>", gap_conn_param_update },
        { "conn_param_update_reply", "<conn_idx> <accept>", gap_conn_param_update_reply },
        { "pair", "<conn_idx> <bond>", gap_pair },
        { "pair_reply", "<conn_idx> <accept> [bond]", gap_pair_reply },
        { "get_connected", "", gap_get_connected },
        { "get_bonded", "", gap_get_bonded },
        { "get_io_cap", "", gap_get_io_cap },
        { "set_io_cap", "<io_cap>", gap_set_io_cap },
        { "passkey_reply", "<conn_idx> <accept> [passkey]", gap_passkey_reply },
#if (dg_configBLE_SECURE_CONNECTIONS == 1)
        { "numeric_reply", "<conn_idx> <accept>", gap_numeric_reply },
#endif /* (dg_configBLE_SECURE_CONNECTIONS == 1) */
        { "get_sec_level", "<conn_idx>", gap_get_sec_level },
        { "unpair", "[public|private] <bd_addr>", gap_unpair },
        { "set_sec_level", "<conn_idx> <sec_level>", gap_set_sec_level },
        { "data_length_set", "[conn_idx] <tx_length> [tx_time]", gap_data_length_set },
#if (dg_configBLE_2MBIT_PHY == 1)
        { "phy_get", "<conn_idx>", gap_phy_get },
        { "phy_set", "<conn_idx> <tx_phy> <rx_phy>", gap_phy_set },
#endif /* (dg_configBLE_2MBIT_PHY == 1) */
        { "tx_power_set", "<air_operation> <tx_power>", gap_tx_power_set },
        { "conn_tx_power_set", "<conn_idx> <tx_power>", gap_conn_tx_power_set },
        { "adv_set_permutation", "<permutation_id>", gap_adv_set_permutation },
        { "local_tx_power_get", "<conn_idx> <phy>", gap_local_tx_power_get },
        { "remote_tx_power_get", "<conn_idx> <phy>", gap_remote_tx_power_get },
        { "path_loss_report_params_set", "<conn_idx> <high_thres> <high_hyst> <low_thres> <low_hyst> <min_time_spent>", gap_path_loss_report_params_set },
        { "path_loss_report_en", "<conn_idx> <enable>", gap_path_loss_report_en },
        { "tx_power_report_en", "<conn_idx> <local_enable> <remote_enable>", gap_tx_power_report_en },
        { "rf_path_compensation_set", "<rf_tx_path_compensation> <rf_rx_path_compensation>", gap_rf_path_compensation_set },
        { NULL },
};

void gap_command(int argc, const char *argv[], void *user_data)
{
        debug_handle_message(argc, argv, gap_handlers);
}
