/**
 ****************************************************************************************
 *
 * @file custom_config_qspi.h
 *
 * @brief Board Support Package. User Configuration file for cached QSPI mode.
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifndef CUSTOM_CONFIG_QSPI_H_
#define CUSTOM_CONFIG_QSPI_H_

#include "bsp_definitions.h"

#define CONFIG_USE_BLE

/*************************************************************************************************\
 * System configuration
 */
#define dg_configUSE_LP_CLK                     ( LP_CLK_32768 )
#define dg_configEXEC_MODE                      ( MODE_IS_CACHED )
#define dg_configCODE_LOCATION                  ( NON_VOLATILE_IS_FLASH )

#define dg_configUSE_WDOG                       ( 0 )

#define dg_configFLASH_CONNECTED_TO             ( FLASH_CONNECTED_TO_1V8 )
#define dg_configFLASH_POWER_DOWN               ( 0 )
#define dg_configPOWER_1V8_ACTIVE               ( 1 )
#define dg_configPOWER_1V8_SLEEP                ( 1 )
#define dg_configPOWER_1V8P                     ( 1 )


#define dg_configUSE_SW_CURSOR                  ( 1 )

#define dg_configCACHEABLE_QSPI_AREA_LEN        ( NVMS_PARAM_PART_start - MEMORY_QSPIF_BASE )


/*************************************************************************************************\
 * FreeRTOS configuration
 */
#define OS_FREERTOS                              /* Define this to use FreeRTOS */
#define configTOTAL_HEAP_SIZE                    ( 24888 )  /* FreeRTOS Total Heap Size */

/*************************************************************************************************\
 * Peripherals configuration
 */
#define dg_configFLASH_ADAPTER                  ( 1 )
#define dg_configNVMS_ADAPTER                   ( 1 )
#define dg_configNVMS_VES                       ( 1 )
#define dg_configNVPARAM_ADAPTER                ( 1 )
#define dg_configGPADC_ADAPTER                  ( 1 )
#define dg_configUSE_HW_SENSOR_NODE             ( 1 )
#define dg_configUSE_SNC_HW_GPADC               ( 1 )
#define dg_configSNC_ADAPTER                    ( 1 )

#define dg_configUART_ADAPTER                   ( 1 )
#define dg_configUSE_DGTL                       ( 1 )

/*************************************************************************************************\
 * BLE configuration
 */
#define BLE_EXTERNAL_HOST

#define BLE_MGR_COMMAND_QUEUE_LENGTH            ( 8 )
#define BLE_MGR_EVENT_QUEUE_LENGTH              ( 8 )

#define DGTL_QUEUE_ENABLE_APP                   ( 0 )
#define DGTL_QUEUE_ENABLE_LOG                   ( 0 )

/* Include bsp default values */
#include "bsp_defaults.h"
/* Include middleware default values */
#include "middleware_defaults.h"

#endif /* CUSTOM_CONFIG_QSPI_H_ */
