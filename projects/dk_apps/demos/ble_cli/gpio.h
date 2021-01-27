/**
 ****************************************************************************************
 *
 * @file gpio.h
 *
 * @brief GPIO CMD API
 *
 * Copyright (C) 2016-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef GPIO_H_
#define GPIO_H_

#include "ble_common.h"

void gpio_command(int argc, const char *argv[], void *user_data);
void gpio_wkup_init(uint32_t notif_mask);
void gpio_wkup_handle_notified();

#endif /* GPIO_H_ */
