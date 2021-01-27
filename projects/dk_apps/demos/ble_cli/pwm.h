/**
 ****************************************************************************************
 *
 * @file pwm.h
 *
 * @brief Hardware PWM CMD API
 *
 * Copyright (C) 2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef PWM_H_
#define PWM_H_

#include "ble_common.h"

void pwm_command(int argc, const char *argv[], void *user_data);

#endif /* PWM_H_ */
