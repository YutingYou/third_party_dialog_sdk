/**
 ****************************************************************************************
 *
 * @file gattc.h
 *
 * @brief BLE GATTC API
 *
 * Copyright (C) 2016-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef GATTC_H_
#define GATTC_H_

#include "ble_common.h"

void gattc_command(int argc, const char *argv[], void *user_data);

bool gattc_handle_event(ble_evt_hdr_t *event);

#endif /* GATTC_H_ */
