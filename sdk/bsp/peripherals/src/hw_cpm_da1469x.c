/**
\addtogroup BSP
\{
\addtogroup DEVICES
\{
\addtogroup CPM
\{
*/

/**
****************************************************************************************
*
* @file hw_cpm_da1469x.c
*
* @brief Clock and Power Manager Driver
*
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
*
****************************************************************************************
*/

#if dg_configUSE_HW_CPM

#include "hw_cpm.h"
#include "hw_watchdog.h"

__RETAINED_CODE void hw_cpm_reset_system(void)
{
        __disable_irq();

        hw_watchdog_unregister_int();
        hw_watchdog_set_pos_val(1);
        hw_watchdog_unfreeze();

        while (1);
}

__RETAINED_CODE void hw_cpm_reboot_system(void)
{
        __disable_irq();

        hw_watchdog_gen_RST();
        hw_watchdog_set_pos_val(1);
        hw_watchdog_unfreeze();

        while (1);
}

#endif /* dg_configUSE_HW_CPM */

/**
\}
\}
\}
*/
