/**
 * \addtogroup PLA_DRI_PER_COMM
 * \{
 * \addtogroup HW_USB_CHARGER
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file hw_usb_charger_da1469x.h
 *
 * @brief Implementation of the USB Charger Low Level Driver.
 *
 * Copyright (C) 2018-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_USB_CHARGER_DA1469x_H_
#define HW_USB_CHARGER_DA1469x_H_


#if dg_configUSE_HW_USB_CHARGER

#include <sdk_defs.h>

/**
 * \enum HW_USB_CHARGER_PRIMARY_CONN_TYPE
 * \brief Primary charger detection result.
 *
 */
typedef enum {
        HW_USB_CHARGER_PRIMARY_CONN_TYPE_NONE   = 0,    /**< Nothing connected. */
        HW_USB_CHARGER_PRIMARY_CONN_TYPE_SDP    = 0,    /**< SDP port */
        HW_USB_CHARGER_PRIMARY_CONN_TYPE_CDP    = 1,    /**< CDP port */
        HW_USB_CHARGER_PRIMARY_CONN_TYPE_DCP    = 1     /**< DCP port */
} HW_USB_CHARGER_PRIMARY_CONN_TYPE;

/**
 * \enum HW_USB_CHARGER_SECONDARY_CONN_TYPE
 * \brief Secondary charger detection result.
 *
 */
typedef enum {
        HW_USB_CHARGER_SECONDARY_CONN_TYPE_CDP  = 0,    /**< CDP port */
        HW_USB_CHARGER_SECONDARY_CONN_TYPE_DCP  = 1     /**< CDP port */
} HW_USB_CHARGER_SECONDARY_CONN_TYPE;

/*********************************  Charger detection related services **********************************************/

/**
 * \brief Enable USB Charger detection circuit and start contact detection.
 *
 */
__STATIC_INLINE void hw_usb_charger_start_contact_detection(void)
{
        USB->USB_CHARGER_CTRL_REG = REG_MSK(USB, USB_CHARGER_CTRL_REG, USB_CHARGE_ON)   |
                                    REG_MSK(USB, USB_CHARGER_CTRL_REG, IDP_SRC_ON);
}

/**
 * \brief Enable USB Charger detection circuit and start primary detection.
 *
 */
__STATIC_INLINE void hw_usb_charger_start_primary_detection(void)
{
        USB->USB_CHARGER_CTRL_REG = REG_MSK(USB, USB_CHARGER_CTRL_REG, USB_CHARGE_ON)   |
                                    REG_MSK(USB, USB_CHARGER_CTRL_REG, VDP_SRC_ON)      |
                                    REG_MSK(USB, USB_CHARGER_CTRL_REG, IDM_SINK_ON);
}

/**
 * \brief Enable USB Charger detection circuit and start secondary detection.
 *
 */
__STATIC_INLINE void hw_usb_charger_start_secondary_detection(void)
{
        USB->USB_CHARGER_CTRL_REG = REG_MSK(USB, USB_CHARGER_CTRL_REG, USB_CHARGE_ON)   |
                                    REG_MSK(USB, USB_CHARGER_CTRL_REG, VDM_SRC_ON)      |
                                    REG_MSK(USB, USB_CHARGER_CTRL_REG, IDP_SINK_ON);
}

/**
 * \brief Enable USB Charger detection circuit and pull D+ high.
 *
 */
__STATIC_INLINE void hw_usb_charger_set_dp_high(void)
{
        USB->USB_CHARGER_CTRL_REG = REG_MSK(USB, USB_CHARGER_CTRL_REG, USB_CHARGE_ON)   |
                                    REG_MSK(USB, USB_CHARGER_CTRL_REG, VDP_SRC_ON);
}

/**
 * \brief Get USB Charger primary detection result.
 *
 *  Detect > 500mA-capable ports (CDP and DCP) from < 500mA ports (SDP).
 *
 * \return Primary charger detection result.
 *
 */
__STATIC_INLINE HW_USB_CHARGER_PRIMARY_CONN_TYPE hw_usb_charger_get_primary_detection_result(void)
{
        return REG_GETF(USB, USB_CHARGER_STAT_REG, USB_CHG_DET);
}

/**
 * \brief Get USB Charger secondary detection result.
 *
 *  Detect CDP from  DCP ports.
 *
 * \return Secondary charger detection result.
 *
 */
__STATIC_INLINE HW_USB_CHARGER_SECONDARY_CONN_TYPE hw_usb_charger_get_secondary_detection_result(void)
{
        return REG_GETF(USB, USB_CHARGER_STAT_REG, USB_DCP_DET);
}

/**
 * \brief USB Charger detection circuit is enabled. Any other kind of detection
 *        (contact, primary or secondary) is disabled.
 *
 */
__STATIC_INLINE void hw_usb_charger_stop_any_detection(void)
{
        USB->USB_CHARGER_CTRL_REG = REG_MSK(USB, USB_CHARGER_CTRL_REG, USB_CHARGE_ON);
}

/**
 * \brief Disable USB Charger detection circuit.
 *
 */
__STATIC_INLINE void hw_usb_charger_disable_detection(void)
{
        USB->USB_CHARGER_CTRL_REG = 0;
}

/*********************************  USB IRQ related services **********************************************/

/**
 * \brief Get USB Charger status and clear the USB_IRQn interrupt.
 *
 * \note A ~20ms delay is needed for safely reading the charger status.
 *
 */
__STATIC_INLINE uint32_t hw_usb_charger_get_charger_status(void)
{
        return USB->USB_CHARGER_STAT_REG;
}

/**
 * \brief Check USB contact.
 *
 * \param[in] usb_charger_status: charger status
 *
 * \retval true:  Data pins make a contact.
 * \retval false: Data pins do not make a contact.
 *
 */
__STATIC_INLINE bool hw_usb_charger_has_data_pin_contact_detected(uint32_t usb_charger_status)
{
        return !(usb_charger_status & REG_MSK(USB, USB_CHARGER_STAT_REG, USB_DP_VAL));
}

#endif /* dg_configUSE_HW_USB_CHARGER */


#endif /* HW_USB_CHARGER_DA1469x_H_ */

/**
\}
\}
*/
