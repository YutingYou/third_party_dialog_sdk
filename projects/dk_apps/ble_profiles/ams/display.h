/**
 ****************************************************************************************
 *
 * @file display.h
 *
 * @brief Media Remote display interface
 *
 * Copyright (C) 2018 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <stdint.h>
#include "ams_client.h"

void display_device_disconnected(void);

void display_update(ams_client_entity_id_t entity_id, uint8_t attribute_id, uint16_t length,
                                                                        const uint8_t *value);

#endif /* DISPLAY_H_ */
