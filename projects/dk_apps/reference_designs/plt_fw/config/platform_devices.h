/**
 ****************************************************************************************
 *
 * @file platform_devices.h
 *
 * @brief Configuration of customized devices connected to board.
 *
 * Copyright (C) 2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef PLATFORM_DEVICES_H_
#define PLATFORM_DEVICES_H_


#ifdef __cplusplus
extern "C" {
#endif

#if ((dg_configUART_ADAPTER == 1) && dg_configUSE_DGTL == 1)
#include "ad_uart.h"
#include "dgtl_config.h"

extern ad_uart_io_conf_t platform_dgtl_io_conf;
extern ad_uart_driver_conf_t platform_dgtl_uart_driver_conf;
extern ad_uart_controller_conf_t platform_dgtl_controller_conf;

#endif /* (dg_configUART_ADAPTER == 1) && dg_configUSE_DGTL == 1 */

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_DEVICES_H_ */
