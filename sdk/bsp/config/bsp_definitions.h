/**
 * \addtogroup PLA_BSP_CONFIG
 * \{
 * \addtogroup BSP_CONFIG_DEFINITIONS Configuration Definitions
 *
 * \brief Doxygen documentation is not yet available for this module.
 *        Please check the source code file(s)
 *
 *\{
 */

/**
 ****************************************************************************************
 *
 * @file bsp_definitions.h
 *
 * @brief Board Support Package. System Configuration file definitions.
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BSP_DEFINITIONS_H_
#define BSP_DEFINITIONS_H_

/* ------------------------------------ DEVICE DEFINITIONS -------------------------------------- */

#include "bsp_device_definitions.h"

/* ------------------------------------ Generic definitions ------------------------------------- */

#define PRO_DK_DA1469x_BRD              1
#define PRO_DK_DA1468x_BRD              2
#define FPGA_DA1469x_BRD                3
#define FPGA_DA1468x_BRD                4

#define LP_CLK_32000                    0
#define LP_CLK_32768                    1
#define LP_CLK_RCX                      2
#define LP_CLK_ANY                      3

#define LP_CLK_IS_ANALOG                0
#define LP_CLK_IS_DIGITAL               1

#define MODE_IS_MIRRORED                0
#define MODE_IS_CACHED                  1
#define MODE_IS_RAM                     MODE_IS_MIRRORED

#define NON_VOLATILE_IS_OTP             0       // Code is in OTP
#define NON_VOLATILE_IS_FLASH           1       // Code is in QSPI Flash
#define NON_VOLATILE_IS_NONE            2       // Debug mode! Code is in RAM!

#define EXT_CRYSTAL_IS_16M              0
#define EXT_CRYSTAL_IS_32M              1

#define DEVELOPMENT_MODE                0       // Code is built for debugging
#define PRODUCTION_MODE                 1       // Code is built for production

#define FLASH_IS_NOT_CONNECTED          0
#define FLASH_CONNECTED_TO_1V8          1
#define FLASH_CONNECTED_TO_1V8P         2
#define FLASH_CONNECTED_TO_1V8F         3

#define BATTERY_TYPE_2xNIMH             0
#define BATTERY_TYPE_3xNIMH             1
#define BATTERY_TYPE_LICOO2             2       // 2.5V discharge voltage, 4.20V charge voltage
#define BATTERY_TYPE_LIMN2O4            3       // 2.5V discharge voltage, 4.20V charge voltage
#define BATTERY_TYPE_NMC                4       // 2.5V discharge voltage, 4.20V charge voltage
#define BATTERY_TYPE_LIFEPO4            5       // 2.5V discharge voltage, 3.65V charge voltage
#define BATTERY_TYPE_LINICOAIO2         6       // 3.0V discharge voltage, 4.20V charge voltage
#define BATTERY_TYPE_CUSTOM             7
#define BATTERY_TYPE_NO_RECHARGE        8
#define BATTERY_TYPE_NO_BATTERY         9

#define BATTERY_TYPE_2xNIMH_ADC_VOLTAGE         (2785)
#define BATTERY_TYPE_3xNIMH_ADC_VOLTAGE         (4013)
#define BATTERY_TYPE_LICOO2_ADC_VOLTAGE         (3440)
#define BATTERY_TYPE_LIMN2O4_ADC_VOLTAGE        (3440)
#define BATTERY_TYPE_NMC_ADC_VOLTAGE            (3440)
#define BATTERY_TYPE_LIFEPO4_ADC_VOLTAGE        (2989)
#define BATTERY_TYPE_LINICOAIO2_ADC_VOLTAGE     (3440)

/*
 * Legacy DK motherboards, which are not supported by the SDK.
 * The definitions exist just so that we don't break compilation of old projects.
 */
#define BLACK_ORCA_MB_REV_A             0
#define BLACK_ORCA_MB_REV_B             1
/*
 * The supported DK motherboards.
 */
#define BLACK_ORCA_MB_REV_D             2


/*
 * The supported RF Front-End Modules
 */
#define FEM_NOFEM                       0
#define FEM_SKY66112_11                 1

/*
 * The BLE event notification user hook types
 */
#define BLE_EVENT_NOTIF_USER_ISR        0       /// User-defined hooks directly from ISR context
#define BLE_EVENT_NOTIF_USER_TASK       1       /// Notification of the user task, using task notifications.

/*
 * Definitions for the different USB suspend modes.
 */
#define USB_SUSPEND_MODE_NONE          0       /// No action, just stop the PLL clock
#define USB_SUSPEND_MODE_PAUSE         1       /// Pause the system and wake only from VBUS and USB interrupts
#define USB_SUSPEND_MODE_IDLE          2       /// System state changes to idle

#endif /* BSP_DEFINITIONS_H_ */

/**
\}
\}
*/
