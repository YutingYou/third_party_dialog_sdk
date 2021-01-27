/**
\addtogroup PLA_DRI_PER_ANALOG
\{
\addtogroup HW_CPM Clock and Power Manager
\{
\brief Clock and Power Manager
*/

/**
****************************************************************************************
*
* @file hw_cpm_da1469x.h
*
* @brief Clock and Power Manager header file.
*
* Copyright (C) 2015-2019 Dialog Semiconductor.
* This computer program includes Confidential, Proprietary Information
* of Dialog Semiconductor. All Rights Reserved.
*
****************************************************************************************
*/

#ifndef HW_CPM_DA1469x_H_
#define HW_CPM_DA1469x_H_


#if dg_configUSE_HW_CPM

#include "sdk_defs.h"

/**
 * \brief   Issue a HW reset (due to a fault condition).
 *
 * \details A HW reset (WDOG) will be generated. The NMI Handler will be called and "status" will be
 *          stored in the Retention RAM.
 *
 */
__RETAINED_CODE void hw_cpm_reset_system(void);

/**
 * \brief   Issue a HW reset (intentionally, i.e. after a SW upgrade).
 *
 * \details A HW reset (WDOG) will be generated. The NMI Handler is bypassed. Thus, no "status" is
 *          stored in the Retention RAM.
 *
 */
__RETAINED_CODE void hw_cpm_reboot_system(void);

#endif /* dg_configUSE_HW_CPM */


#endif /* HW_CPM_DA1469x_H_ */

/**
\}
\}
\}
*/
