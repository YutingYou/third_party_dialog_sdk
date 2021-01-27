/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SNC_GPIO
 *
 * \brief GPIO LLD macros for SNC context
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file snc_hw_gpio_macros.h
 *
 * @brief SNC definitions of GPIO Low Level Driver Macros
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SNC_HW_GPIO_MACROS_H_
#define SNC_HW_GPIO_MACROS_H_


#if dg_configUSE_HW_SENSOR_NODE

#if dg_configUSE_SNC_HW_GPIO

//==================== Configuration functions =================================

void snc_hw_gpio_configure_pin(b_ctx_t* b_ctx,
        HW_GPIO_PORT port, HW_GPIO_PIN pin, HW_GPIO_MODE mode, HW_GPIO_FUNC function,
        const bool high);
#define _SNC_hw_gpio_configure_pin(port, pin, mode, function, high)                             \
        SNC_STEP_BY_STEP();                                                                     \
        snc_hw_gpio_configure_pin(b_ctx, _SNC_OP_VALUE(HW_GPIO_PORT, port),                     \
                                         _SNC_OP_VALUE(HW_GPIO_PIN, pin),                       \
                                         _SNC_OP_VALUE(HW_GPIO_MODE, mode),                     \
                                         _SNC_OP_VALUE(HW_GPIO_FUNC, function),                 \
                                         _SNC_OP_VALUE(const bool, high))

void snc_hw_gpio_set_pin_function(b_ctx_t* b_ctx,
        HW_GPIO_PORT port, HW_GPIO_PIN pin, HW_GPIO_MODE mode, HW_GPIO_FUNC function);
#define _SNC_hw_gpio_set_pin_function(port, pin, mode, function)                                \
        SNC_STEP_BY_STEP();                                                                     \
        snc_hw_gpio_set_pin_function(b_ctx, _SNC_OP_VALUE(HW_GPIO_PORT, port),                  \
                                            _SNC_OP_VALUE(HW_GPIO_PIN, pin),                    \
                                            _SNC_OP_VALUE(HW_GPIO_MODE, mode),                  \
                                            _SNC_OP_VALUE(HW_GPIO_FUNC, function))

//==================== Control functions =======================================

void snc_hw_gpio_set_active(b_ctx_t* b_ctx, HW_GPIO_PORT port, HW_GPIO_PIN pin);
#define _SNC_hw_gpio_set_active(port, pin)                                                      \
        SNC_STEP_BY_STEP();                                                                     \
        snc_hw_gpio_set_active(b_ctx, _SNC_OP_VALUE(HW_GPIO_PORT, port),                        \
                                      _SNC_OP_VALUE(HW_GPIO_PIN, pin))

void snc_hw_gpio_set_inactive(b_ctx_t* b_ctx, HW_GPIO_PORT port, HW_GPIO_PIN pin);
#define _SNC_hw_gpio_set_inactive(port, pin)                                                    \
        SNC_STEP_BY_STEP();                                                                     \
        snc_hw_gpio_set_inactive(b_ctx, _SNC_OP_VALUE(HW_GPIO_PORT, port),                      \
                                        _SNC_OP_VALUE(HW_GPIO_PIN, pin))

void snc_hw_gpio_toggle(b_ctx_t* b_ctx, HW_GPIO_PORT port, HW_GPIO_PIN pin);
#define _SNC_hw_gpio_toggle(port, pin)                                                          \
        SNC_STEP_BY_STEP();                                                                     \
        snc_hw_gpio_toggle(b_ctx, _SNC_OP_VALUE(HW_GPIO_PORT, port),                            \
                                  _SNC_OP_VALUE(HW_GPIO_PIN, pin))

void snc_hw_gpio_pad_latch_enable(b_ctx_t* b_ctx, HW_GPIO_PORT port, HW_GPIO_PIN pin);
#define _SNC_hw_gpio_pad_latch_enable(port, pin)                                                \
        SNC_STEP_BY_STEP();                                                                     \
        snc_hw_gpio_pad_latch_enable(b_ctx, _SNC_OP_VALUE(HW_GPIO_PORT, port),                  \
                                            _SNC_OP_VALUE(HW_GPIO_PIN, pin))

void snc_hw_gpio_pad_latch_disable(b_ctx_t* b_ctx, HW_GPIO_PORT port, HW_GPIO_PIN pin);
#define _SNC_hw_gpio_pad_latch_disable(port, pin)                                               \
        SNC_STEP_BY_STEP();                                                                     \
        snc_hw_gpio_pad_latch_disable(b_ctx, _SNC_OP_VALUE(HW_GPIO_PORT, port),                 \
                                             _SNC_OP_VALUE(HW_GPIO_PIN, pin))

void snc_hw_gpio_pads_latch_enable(b_ctx_t* b_ctx, HW_GPIO_PORT port, uint32_t pins);
#define _SNC_hw_gpio_pads_latch_enable(port, pins)                                              \
        SNC_STEP_BY_STEP();                                                                     \
        snc_hw_gpio_pads_latch_enable(b_ctx, _SNC_OP_VALUE(HW_GPIO_PORT, port),                 \
                                             _SNC_OP_VALUE(uint32_t, pins))

void snc_hw_gpio_pads_latch_disable(b_ctx_t* b_ctx, HW_GPIO_PORT port, uint32_t pins);
#define _SNC_hw_gpio_pads_latch_disable(port, pins)                                             \
        SNC_STEP_BY_STEP();                                                                     \
        snc_hw_gpio_pads_latch_disable(b_ctx, _SNC_OP_VALUE(HW_GPIO_PORT, port),                \
                                              _SNC_OP_VALUE(uint32_t, pins))

void snc_hw_gpio_check_pin_set(b_ctx_t* b_ctx, HW_GPIO_PORT port, HW_GPIO_PIN pin);
#define _SNC_hw_gpio_check_pin_set(port, pin)                                                   \
        SNC_STEP_BY_STEP();                                                                     \
        snc_hw_gpio_check_pin_set(b_ctx, _SNC_OP_VALUE(HW_GPIO_PORT, port),                     \
                                         _SNC_OP_VALUE(HW_GPIO_PIN, pin))

void snc_hw_gpio_get_pin_status(b_ctx_t* b_ctx, HW_GPIO_PORT port, HW_GPIO_PIN pin,
        SENIS_OPER_TYPE stat_type, uint32_t *stat);
#define _SNC_hw_gpio_get_pin_status(port, pin, stat)                                            \
        SNC_STEP_BY_STEP();                                                                     \
        snc_hw_gpio_get_pin_status(b_ctx, port, pin, _SNC_OP_ADDRESS(stat));

#endif /* dg_configUSE_SNC_HW_GPIO */

#endif /* dg_configUSE_HW_SENSOR_NODE */


#endif /* SNC_HW_GPIO_MACROS_H_ */

/**
 * \}
 * \}
 */
