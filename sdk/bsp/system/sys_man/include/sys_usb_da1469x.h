/**
 * \addtogroup MID_SYS_SERVICES
 * \{
 * \addtogroup USB_SERVICE USB System Service
 *
 * \brief System USB
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file sys_usb_da1469x.h
 *
 * @brief System USB header file.
 *
 * Copyright (C) 2018-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#ifndef SYS_USB_DA1469x_H_
#define SYS_USB_DA1469x_H_

#include <sdk_defs.h>
#include <hw_usb.h>


/**
 * \brief Initialize sys_usb service.
 *
 */
void sys_usb_init(void);

/**
 * \brief Finalize the attach procedure.
 *        Called from sys_usb or sys_charger, depending on configuration
 *
 */
void sys_usb_finalize_attach(void);

/******************** Weak functions to be implemented, if needed, by the application code ********/

/**
 * \brief Notification hook to be called when VBUS is attached.
 *
 */
__WEAK void sys_usb_ext_hook_attach(void);

/**
 * \brief Notification hook to be called when VBUS is detached.
 *
 */
__WEAK void sys_usb_ext_hook_detach(void);

/**
 * \brief Notification hook to be called when charger detection,
 * if any, is completed, and the device can begin enumeration.
 *
 */
__WEAK void sys_usb_ext_hook_begin_enumeration(void);

#endif /* SYS_USB_DA1469x_H_ */


/**
\}
\}
*/
