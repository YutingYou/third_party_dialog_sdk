/**
 ****************************************************************************************
 *
 * @file sys_usb_da1469x_internal.h
 *
 * @brief System USB internal header file.
 *
 * Copyright (C) 2018-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#ifndef SYS_USB_INTERNAL_H_
#define SYS_USB_INTERNAL_H_

/******************** Weak functions to trigger sys_charger fsm, if sys_charger is used ***********/

/**
 * \brief sys_charger hook to be called when VBUS is attached.
 *
 */
__WEAK void sys_usb_int_charger_hook_attach(void);

/**
 * \brief sys_charger hook to be called when VBUS is detached.
 *
 */
__WEAK void sys_usb_int_charger_hook_detach(void);

/**
 * \brief sys_charger hook to be called when charging event is received.
 *
 */
__WEAK void sys_usb_int_charger_hook_ch_event(void);

/**
 * \brief sys_charger hook to be called when suspend event is received.
 *
 */
__WEAK void sys_usb_int_charger_hook_suspend_event(void);

/**
 * \brief sys_charger hook to be called when resume event is received.
 *
 */
__WEAK void sys_usb_int_charger_hook_resume_event(void);

#endif /* SYS_USB_INTERNAL_H_ */

