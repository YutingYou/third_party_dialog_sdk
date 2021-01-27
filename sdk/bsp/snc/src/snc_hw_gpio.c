/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SNC_GPIO
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file snc_hw_gpio.c
 *
 * @brief SNC-Implementation of GPIO Low Level Driver
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#if dg_configUSE_HW_SENSOR_NODE

#if dg_configUSE_SNC_HW_GPIO

#include "snc_defs.h"

#include "snc_hw_gpio.h"

/*
 * FUNCTION DEFINITIONS
 *****************************************************************************************
 */

//==================== Configuration functions =================================

void snc_hw_gpio_configure_pin(b_ctx_t* b_ctx,
        HW_GPIO_PORT port, HW_GPIO_PIN pin, HW_GPIO_MODE mode, HW_GPIO_FUNC function,
        const bool high)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING((port == HW_GPIO_PORT_0 && pin < HW_GPIO_PORT_0_NUM_PINS) ||
                (port == HW_GPIO_PORT_1 && pin < HW_GPIO_PORT_1_NUM_PINS));

        SENIS_wadva(da(&GPIO->P0_RESET_DATA_REG + port - 2*((uint8_t)high)), ((1 << pin)));
        SENIS_wadva(da(&GPIO->P0_00_MODE_REG +
                (&GPIO->P1_00_MODE_REG - &GPIO->P0_00_MODE_REG)*port + pin),
                (function | mode));
}

void snc_hw_gpio_set_pin_function(b_ctx_t* b_ctx,
        HW_GPIO_PORT port, HW_GPIO_PIN pin, HW_GPIO_MODE mode, HW_GPIO_FUNC function)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING((port == HW_GPIO_PORT_0 && pin < HW_GPIO_PORT_0_NUM_PINS) ||
                (port == HW_GPIO_PORT_1 && pin < HW_GPIO_PORT_1_NUM_PINS));

        SENIS_wadva(da(&GPIO->P0_00_MODE_REG +
                (&GPIO->P1_00_MODE_REG - &GPIO->P0_00_MODE_REG)*port + pin),
                (function | mode));
}

void snc_hw_gpio_set_active(b_ctx_t* b_ctx, HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING((port == HW_GPIO_PORT_0 && pin < HW_GPIO_PORT_0_NUM_PINS) ||
                (port == HW_GPIO_PORT_1 && pin < HW_GPIO_PORT_1_NUM_PINS));

        SENIS_wadva(da(&GPIO->P0_SET_DATA_REG + port), (1 << pin));
}

void snc_hw_gpio_set_inactive(b_ctx_t* b_ctx, HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING((port == HW_GPIO_PORT_0 && pin < HW_GPIO_PORT_0_NUM_PINS) ||
                (port == HW_GPIO_PORT_1 && pin < HW_GPIO_PORT_1_NUM_PINS));

        SENIS_wadva(da(&GPIO->P0_RESET_DATA_REG + port), (1 << pin));
}

void snc_hw_gpio_toggle(b_ctx_t* b_ctx, HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING((port == HW_GPIO_PORT_0 && pin < HW_GPIO_PORT_0_NUM_PINS) ||
                (port == HW_GPIO_PORT_1 && pin < HW_GPIO_PORT_1_NUM_PINS));

        SENIS_labels(label1, label2);

        SENIS_rdcbi(da(&GPIO->P0_DATA_REG + port), pin);
        SENIS_cobr_eq(l(label1));
        SENIS_wadva(da(&GPIO->P0_SET_DATA_REG + port), (1 << pin));
        SENIS_goto(l(label2));

        SENIS_label(label1);

        SENIS_wadva(da(&GPIO->P0_RESET_DATA_REG + port), (1 << pin));

        SENIS_label(label2);
}

void snc_hw_gpio_pad_latch_enable(b_ctx_t* b_ctx, HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING((port == HW_GPIO_PORT_0 && pin < HW_GPIO_PORT_0_NUM_PINS) ||
                (port == HW_GPIO_PORT_1 && pin < HW_GPIO_PORT_1_NUM_PINS));

        if (port == HW_GPIO_PORT_0) {
                SENIS_assign(da(&CRG_TOP->P0_SET_PAD_LATCH_REG), (1 << pin));
        } else if (port == HW_GPIO_PORT_1) {
                SENIS_assign(da(&CRG_TOP->P1_SET_PAD_LATCH_REG), (1 << pin));
        }
}

void snc_hw_gpio_pad_latch_disable(b_ctx_t* b_ctx, HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING((port == HW_GPIO_PORT_0 && pin < HW_GPIO_PORT_0_NUM_PINS) ||
                (port == HW_GPIO_PORT_1 && pin < HW_GPIO_PORT_1_NUM_PINS));

        if (port == HW_GPIO_PORT_0) {
                SENIS_assign(da(&CRG_TOP->P0_RESET_PAD_LATCH_REG), (1 << pin));
        } else if (port == HW_GPIO_PORT_1) {
                SENIS_assign(da(&CRG_TOP->P1_RESET_PAD_LATCH_REG), (1 << pin));
        }
}

void snc_hw_gpio_pads_latch_enable(b_ctx_t* b_ctx, HW_GPIO_PORT port, uint32_t pins)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING((port == HW_GPIO_PORT_0) ||
                (port == HW_GPIO_PORT_1 && pins <= ((1 << HW_GPIO_PORT_1_NUM_PINS) - 1)));

        if (port == HW_GPIO_PORT_0) {
                SENIS_assign(da(&CRG_TOP->P0_SET_PAD_LATCH_REG), pins);
        } else if (port == HW_GPIO_PORT_1) {
                SENIS_assign(da(&CRG_TOP->P1_SET_PAD_LATCH_REG), pins);
        }
}

void snc_hw_gpio_pads_latch_disable(b_ctx_t* b_ctx, HW_GPIO_PORT port, uint32_t pins)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING((port == HW_GPIO_PORT_0) ||
                (port == HW_GPIO_PORT_1 && pins <= ((1 << HW_GPIO_PORT_1_NUM_PINS) - 1)));

        if (port == HW_GPIO_PORT_0) {
                SENIS_assign(da(&CRG_TOP->P0_RESET_PAD_LATCH_REG), pins);
        } else if (port == HW_GPIO_PORT_1) {
                SENIS_assign(da(&CRG_TOP->P1_RESET_PAD_LATCH_REG), pins);
        }
}

void snc_hw_gpio_check_pin_set(b_ctx_t* b_ctx, HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING((port == HW_GPIO_PORT_0 && pin < HW_GPIO_PORT_0_NUM_PINS) ||
                (port == HW_GPIO_PORT_1 && pin < HW_GPIO_PORT_1_NUM_PINS));

        SENIS_rdcbi(da(&GPIO->P0_DATA_REG + port), pin);
}

void snc_hw_gpio_get_pin_status(b_ctx_t* b_ctx, HW_GPIO_PORT port, HW_GPIO_PIN pin,
        SENIS_OPER_TYPE stat_type, uint32_t *stat)
{
        SENIS_labels(pin_is_set, pin_not_set);
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING((port == HW_GPIO_PORT_0 && pin < HW_GPIO_PORT_0_NUM_PINS) ||
                (port == HW_GPIO_PORT_1 && pin < HW_GPIO_PORT_1_NUM_PINS));

        SENIS_rdcbi(da(&GPIO->P0_DATA_REG + port), pin);
        SENIS_cobr_eq(l(pin_is_set));
        senis_assign(b_ctx, stat_type, stat, _SNC_OP(0));
        SENIS_goto(l(pin_not_set));

        SENIS_label(pin_is_set);
        senis_assign(b_ctx, stat_type, stat, _SNC_OP(1));

        SENIS_label(pin_not_set);
}

#endif /* dg_configUSE_SNC_HW_GPIO */

#endif /* dg_configUSE_HW_SENSOR_NODE */


/**
 * \}
 * \}
 */
