/**
 * \addtogroup MID_SYS_ADAPTERS
 * \{
 * \addtogroup ADAPTER Adapter
 *
 * \brief Common definitions for I/O adapters
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file  ad.h
 *
 * @brief Adapters shared definitions
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef AD_H_
#define AD_H_

#include "hw_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Data types definitions section
 */

/**
 * \brief Adapters IO configuration state
 */
typedef enum {
        /** Off configuration*/
        AD_IO_CONF_OFF = 0,
        /** On configuration */
        AD_IO_CONF_ON  = 1,
} AD_IO_CONF_STATE;

/**
 * \brief Adapters pin configuration
 */
typedef struct {
        HW_GPIO_MODE mode;
        HW_GPIO_FUNC function;
        bool         high; /**< IO level when pin is configured as gpio output */
} ad_pin_conf_t;

/**
 * \brief Adapters IO configuration
 */
typedef struct {
        HW_GPIO_PORT port;
        HW_GPIO_PIN  pin;
        ad_pin_conf_t on;
        ad_pin_conf_t off;
} ad_io_conf_t;

#define AD_GPIO_PIN_PORT_VALID(_gpio_)  ((_gpio_->pin != HW_GPIO_PIN_NONE) && (_gpio_->port != HW_GPIO_PORT_NONE))

#ifdef __cplusplus
}
#endif

#endif /* AD_H_ */

/**
 * \}
 * \}
 */
