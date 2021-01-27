/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SNC_HW_GPIO
 *
 * \brief GPIO LLD for SNC context
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file snc_hw_gpio.h
 *
 * @brief SNC-Definition of GPIO Low Level Driver API
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SNC_HW_GPIO_H_
#define SNC_HW_GPIO_H_


#if dg_configUSE_HW_SENSOR_NODE

#if dg_configUSE_SNC_HW_GPIO

#include "snc_defs.h"
#include "hw_gpio.h"

/*
 * MACRO DEPENDENCIES
 *****************************************************************************************
 */

#include "snc_hw_gpio_macros.h"

/*
 * FUNCTION DECLARATIONS
 *****************************************************************************************
 */

//==================== Configuration functions =================================

/**
 * \brief Function used in SNC context to configure a GPIO
 *
 * \param [in] port     (HW_GPIO_PORT: build-time-only value)
 *                      GPIO port number
 * \param [in] pin      (HW_GPIO_PIN: build-time-only value)
 *                      GPIO pin number
 * \param [in] mode     (HW_GPIO_MODE: build-time-only value)
 *                      GPIO pin access mode
 * \param [in] function (HW_GPIO_FUNC: build-time-only value)
 *                      GPIO pin function
 * \param [in] high     (bool: build-time-only value)
 *                      GPIO pin state
 *                      (if configured as simple GPIO and OUTPUT, then true: high, false: low)

 *
 */
#define SNC_hw_gpio_configure_pin(port, pin, mode, function, high)                              \
        _SNC_hw_gpio_configure_pin(port, pin, mode, function, high)

/**
 * \brief Function used in SNC context to configure GPIO pin type and mode
 *
 * \param [in] port     (HW_GPIO_PORT: build-time-only value)
 *                      GPIO port number
 * \param [in] pin      (HW_GPIO_PIN: build-time-only value)
 *                      GPIO pin number
 * \param [in] mode     (HW_GPIO_MODE: build-time-only value)
 *                      GPIO pin mode
 * \param [in] function (HW_GPIO_FUNC: build-time-only value)
 *                      GPIO pin function
 *
 */
#define SNC_hw_gpio_set_pin_function(port, pin, mode, function)                                 \
        _SNC_hw_gpio_set_pin_function(port, pin, mode, function)

//==================== Control functions =======================================

/**
 * \brief Function used in SNC context to set a GPIO in high state
 *
 * The GPIO should have been previously configured as an output.
 *
 * \param [in] port     (HW_GPIO_PORT: build-time-only value)
 *                      GPIO port number
 * \param [in] pin      (HW_GPIO_PIN: build-time-only value)
 *                      GPIO pin number
 *
 */
#define SNC_hw_gpio_set_active(port, pin)                                                       \
        _SNC_hw_gpio_set_active(port, pin)

/**
 * \brief Function used in SNC context to set a GPIO in low state
 *
 * The GPIO should have been previously configured as an output.
 *
 * \param [in] port     (HW_GPIO_PORT: build-time-only value)
 *                      GPIO port number
 * \param [in] pin      (HW_GPIO_PIN: build-time-only value)
 *                      GPIO pin number
 *
 */
#define SNC_hw_gpio_set_inactive(port, pin)                                                     \
        _SNC_hw_gpio_set_inactive(port, pin)

/**
 * \brief Function used in SNC context to toggle a GPIO's state
 *
 * The GPIO should have been previously configured as an output.
 *
 * \param [in] port     (HW_GPIO_PORT: build-time-only value)
 *                      GPIO port number
 * \param [in] pin      (HW_GPIO_PIN: build-time-only value)
 *                      GPIO pin number
 *
 */
#define SNC_hw_gpio_toggle(port, pin)                                                           \
        _SNC_hw_gpio_toggle(port, pin)

/**
 * \brief Enables the latch for the specific GPIO.
 *
 * \param [in] port     (HW_GPIO_PORT: build-time-only value)
 *                      GPIO port number
 * \param [in] pin      (HW_GPIO_PIN: build-time-only value)
 *                      GPIO pin number
 *
 */
#define SNC_hw_gpio_pad_latch_enable(port, pin)                                                 \
        _SNC_hw_gpio_pad_latch_enable(port, pin)

/**
 * \brief Disables the latch for the specific GPIO.
 *
 * \param [in] port     (HW_GPIO_PORT: build-time-only value)
 *                      GPIO port number
 * \param [in] pin      (HW_GPIO_PIN: build-time-only value)
 *                      GPIO pin number
 *
 */
#define SNC_hw_gpio_pad_latch_disable(port, pin)                                                \
        _SNC_hw_gpio_pad_latch_disable(port, pin)

/**
 * \brief Enables the latch for the GPIOs of the specific port.
 *
 * \param [in] port     (HW_GPIO_PORT: build-time-only value)
 *                      GPIO port number
 * \param [in] pins     (uint32_t: build-time-only value)
 *                      GPIO pin status bitmask
 *
 */
#define SNC_hw_gpio_pads_latch_enable(port, pins)                                               \
        _SNC_hw_gpio_pads_latch_enable(port, pins)

/**
 * \brief Disables the latch for the GPIOs of the specific port.
 *
 * \param [in] port     (HW_GPIO_PORT: build-time-only value)
 *                      GPIO port number
 * \param [in] pins     (uint32_t: build-time-only value)
 *                      GPIO pin status bitmask
 *
 */
#define SNC_hw_gpio_pads_latch_disable(port, pins)                                              \
        _SNC_hw_gpio_pads_latch_disable(port, pins)

/**
 * \brief Checks if a specific GPIO is set
 *
 * This macro will set (or not) the EQ_FLAG in SNC_STATUS_REG. In order to check
 * if the GPIO was set, the SENIS_cobr_eq(\<label_to_go\>) command can be used.
 * Where \<label_to_go\> is the label to jump if the pin is set.
 *
 * \param [in] port     (HW_GPIO_PORT: build-time-only value)
 *                      GPIO port number
 * \param [in] pin      (HW_GPIO_PIN: build-time-only value)
 *                      GPIO pin number
 *
 *
 * Example usage:
 * \code{.c}
 * uint32_t pin_is_not_set_cnt = 0x0;
 * uint32_t pin_is_set_cnt = 0x0;
 * {
 *         SENIS_labels(pin_is_set, pin_check_end);
 *         ...
 *         // If the pin is set increment the pin_is_set_cnt
 *         // else increment the pin_is_not_set_cnt
 *         SNC_hw_gpio_check_pin_set(HW_GPIO_PORT_0, HW_GPIO_PIN_0);
 *         SENIS_cobr_eq(l(pin_is_set));
 *         SENIS_inc1(da(&pin_is_not_set_cnt));
 *         SENIS_goto(l(pin_check_end));
 *
 *         SENIS_label(pin_is_set);
 *         SENIS_inc1(da(&pin_is_set_cnt));
 *
 *         SENIS_label(pin_check_end);
 *         ...
 * }
 * \endcode
 *
 */
#define SNC_hw_gpio_check_pin_set(port, pin)                                                   \
        _SNC_hw_gpio_check_pin_set(port, pin)

/**
 * \brief Returns the status of a GPIO pin
 *
 * \param [in] port     (HW_GPIO_PORT: build-time-only value)
 *                      GPIO port number
 * \param [in] pin      (HW_GPIO_PIN: build-time-only value)
 *                      GPIO pin number
 * \param [out] stat    (uint32_t*: use da() or ia())
 *                      Register/RAM address to store the pin status (1 if set, 0 if not set)
 */
#define SNC_hw_gpio_get_pin_status(port, pin, stat)                                            \
        _SNC_hw_gpio_get_pin_status(port, pin, stat)

#endif /* dg_configUSE_SNC_HW_GPIO */

#endif /* dg_configUSE_HW_SENSOR_NODE */


#endif /* SNC_HW_GPIO_H_ */

/**
 * \}
 * \}
 */
