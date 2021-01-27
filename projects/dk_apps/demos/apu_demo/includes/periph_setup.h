/**
 ****************************************************************************************
 *
 * @file periph_setup.h
 *
 * @brief Configuration of devices connected to board
 *
 * Copyright (C) 2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef INCLUDES_PERIPH_SETUP_H_
#define INCLUDES_PERIPH_SETUP_H_

#include "audio_task.h"
#include "hw_gpio.h"

/* PCM Interface */
#define PCM_CLK_PIN      HW_GPIO_PORT_0, HW_GPIO_PIN_20
#define PCM_FSC_PIN      HW_GPIO_PORT_0, HW_GPIO_PIN_21
#define PCM_DO_PIN       HW_GPIO_PORT_0, HW_GPIO_PIN_24
#define PCM_DI_PIN       HW_GPIO_PORT_0, HW_GPIO_PIN_26

/* PDM Interface */
#define PDM_CLK_PIN      HW_GPIO_PORT_0, HW_GPIO_PIN_7
#define PDM_DATA_PIN     HW_GPIO_PORT_1, HW_GPIO_PIN_0
#define PDM_MIC_PE       HW_GPIO_PORT_1, HW_GPIO_PIN_19

/* CODEC7218 Interface */
#define _GPIO_LVL HW_GPIO_POWER_VDD1V8P
/* I2C */
#define I2C_SCL_PORT    HW_GPIO_PORT_0
#define I2C_SCL_PIN     HW_GPIO_PIN_30
#define I2C_SDA_PORT    HW_GPIO_PORT_0
#define I2C_SDA_PIN     HW_GPIO_PIN_31

#if DEMO_PDM_RECORD_PLAYBACK || DEMO_PCM_RECORD_PLAYBACK
#define BTN_PIN   KEY1_PORT, KEY1_PIN
#endif

#endif /* INCLUDES_PERIPH_SETUP_H_ */
