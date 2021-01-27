/**
 ****************************************************************************************
 *
 * @file gatts.h
 *
 * @brief BLE GATTS API
 *
 * Copyright (C) 2016-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef GATTS_H_
#define GATTS_H_

#include "ble_common.h"

void gatts_command(int argc, const char *argv[], void *user_data);

bool gatts_handle_event(ble_evt_hdr_t *event);

#endif /* GATTS_H_ */
