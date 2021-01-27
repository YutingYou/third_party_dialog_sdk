/**
 ****************************************************************************************
 *
 * @file sys_platform_devices_internal.h
 *
 * @brief Configuration of devices connected to board-Should be hidden from documentation
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SYS_PLATFORM_DEVICES_INTERNAL_H_
#define SYS_PLATFORM_DEVICES_INTERNAL_H_


#ifdef __cplusplus
extern "C" {
#endif

#if (dg_configGPADC_ADAPTER == 1)
#include "ad_gpadc.h"
/*
 * Define sources connected to GPADC
 */

extern const ad_gpadc_controller_conf_t TEMP_SENSOR_RADIO_INTERNAL;
#if (dg_configUSE_LP_CLK == LP_CLK_RCX)
extern const ad_gpadc_controller_conf_t BATTERY_LEVEL_INTERNAL;
extern const ad_gpadc_controller_conf_t TEMP_SENSOR_BANDGAP_INTERNAL;
#endif

#endif /* dg_configGPADC_ADAPTER */

#if ((dg_configUART_ADAPTER == 1) && (dg_configUSE_CONSOLE == 1))
#include "ad_uart.h"

extern const ad_uart_io_conf_t sys_platform_console_io_conf;
extern const ad_uart_driver_conf_t sys_platform_console_uart_driver_conf;
extern const ad_uart_controller_conf_t sys_platform_console_controller_conf;

#endif /* ((dg_configUART_ADAPTER == 1) && (dg_configUSE_CONSOLE == 1) */

#if ((dg_configUART_ADAPTER == 1) && dg_configUSE_DGTL == 1)
#include "ad_uart.h"

extern const ad_uart_io_conf_t sys_platform_dgtl_io_conf;
extern const ad_uart_driver_conf_t sys_platform_dgtl_uart_driver_conf;
extern const ad_uart_controller_conf_t sys_platform_dgtl_controller_conf;

#endif /* (dg_configUART_ADAPTER == 1) && dg_configUSE_DGTL == 1 */

#ifdef __cplusplus
}
#endif

#endif /* SYS_PLATFORM_DEVICES_INTERNAL_H_ */
