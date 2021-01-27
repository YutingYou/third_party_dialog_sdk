/**
 * \addtogroup PLA_BSP_CONFIG
 * \{
 * \addtogroup PLA_BSP_CFG_BOARDS
 * \{
 */

 /**
 ****************************************************************************************
 *
 * @file brd_prodk_da1469x.h
 *
 * @brief Board Support Package. DA1469x Board I/O configuration.
 *
 * Copyright (C) 2018 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BRD_PRODK_DA1469X_H
#define BRD_PRODK_DA1469X_H

/* Serial port configuration section */
#define SER1_UART       (HW_UART2)

#define SER1_TX_PORT    (HW_GPIO_PORT_0)
#define SER1_TX_PIN     (HW_GPIO_PIN_9)
#define SER1_TX_MODE    (HW_GPIO_MODE_OUTPUT)
#define SER1_TX_FUNC    (HW_GPIO_FUNC_UART2_TX)

#define SER1_RX_PORT    (HW_GPIO_PORT_0)
#define SER1_RX_PIN     (HW_GPIO_PIN_8)
#define SER1_RX_MODE    (HW_GPIO_MODE_INPUT)
#define SER1_RX_FUNC    (HW_GPIO_FUNC_UART2_RX)

#define SER1_RTS_PORT   (HW_GPIO_PORT_1)
#define SER1_RTS_PIN    (HW_GPIO_PIN_0)
#define SER1_RTS_MODE   (HW_GPIO_MODE_OUTPUT)
#define SER1_RTS_FUNC   (HW_GPIO_FUNC_UART2_RTSN)

#define SER1_CTS_PORT   (HW_GPIO_PORT_0)
#define SER1_CTS_PIN    (HW_GPIO_PIN_7)
#define SER1_CTS_MODE   (HW_GPIO_MODE_INPUT)
#define SER1_CTS_FUNC   (HW_GPIO_FUNC_UART2_CTSN)

/* LED configuration section */
#define LED1_PORT       (HW_GPIO_PORT_1)
#define LED1_PIN        (HW_GPIO_PIN_1)
#define LED1_MODE       (HW_GPIO_MODE_OUTPUT)
#define LED1_FUNC       (HW_GPIO_FUNC_GPIO)

/* KEY configuration section */
#define KEY1_PORT       (HW_GPIO_PORT_0)
#define KEY1_PIN        (HW_GPIO_PIN_6)
#define KEY1_MODE       (HW_GPIO_MODE_INPUT_PULLUP)
#define KEY1_FUNC       (HW_GPIO_FUNC_GPIO)

#endif /* BRD_PRODK_DA1469X_H */
/**
\}
\}
*/
