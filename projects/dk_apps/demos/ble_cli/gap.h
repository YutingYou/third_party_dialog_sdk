/**
 ****************************************************************************************
 *
 * @file gap.h
 *
 * @brief BLE GAP API
 *
 * Copyright (C) 2016-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef GAP_H_
#define GAP_H_

#include "ble_gap.h"

void gap_command(int argc, const char *argv[], void *user_data);

bool gap_handle_event(ble_evt_hdr_t *event);

#endif /* GAP_H_ */
