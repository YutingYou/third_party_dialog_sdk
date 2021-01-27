/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup Wakeup_Timer
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file hw_wkup.c
 *
 * @brief Implementation of the Wakeup timer Low Level Driver.
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#if dg_configUSE_HW_WKUP


#include <stdio.h>
#include <string.h>
#include <hw_wkup_da1469x.h>

#if (dg_configSYSTEMVIEW == 1)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif /* (dg_configSYSTEMVIEW == 1) */


__RETAINED static hw_wkup_interrupt_cb intr_cb_key;
__RETAINED static hw_wkup_interrupt_cb intr_cb_p0;
__RETAINED static hw_wkup_interrupt_cb intr_cb_p1;


void hw_wkup_init(const wkup_config *cfg)
{
        unsigned int i;

        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, CLK_TMR_REG, WAKEUPCT_ENABLE);
        GLOBAL_INT_RESTORE();

        /* reset configuration */
        WAKEUP->WKUP_CTRL_REG = 0;

        /* reset all pin settings to default */
        for (i = 0; i < HW_GPIO_NUM_PORTS; i++) {
                *((volatile uint32_t *)(&WAKEUP->WKUP_POL_P0_REG) + i) = 0;
                *((volatile uint32_t *)(&WAKEUP->WKUP_SELECT_P0_REG) + i) = 0;
                *((volatile uint32_t *)(&WAKEUP-> WKUP_SEL_GPIO_P0_REG) + i) = 0;
                *((volatile uint32_t *)(&WAKEUP-> WKUP_CLEAR_P0_REG) + i) = 0xFFFFFFFF;
        }

        /* Disable interrupts */
        NVIC_DisableIRQ(KEY_WKUP_GPIO_IRQn);
        NVIC_DisableIRQ(GPIO_P0_IRQn);
        NVIC_DisableIRQ(GPIO_P1_IRQn);

        hw_wkup_configure(cfg);
}

void hw_wkup_configure(const wkup_config *cfg)
{
        int i;

        if (!cfg) {
                return;
        }
        hw_wkup_set_debounce_time(cfg->debounce);
        for (i = 0; i < HW_GPIO_NUM_PORTS; i++) {
                hw_wkup_configure_port(i,  cfg->pin_wkup_state[i],  cfg->pin_gpio_state[i],  cfg->pin_trigger[i]);
        }
}

void hw_wkup_register_key_interrupt(hw_wkup_interrupt_cb cb, uint32_t prio)
{
        intr_cb_key = cb;

        NVIC_ClearPendingIRQ(KEY_WKUP_GPIO_IRQn);
        NVIC_SetPriority(KEY_WKUP_GPIO_IRQn, prio);
        NVIC_EnableIRQ(KEY_WKUP_GPIO_IRQn);
}

void hw_wkup_register_gpio_p0_interrupt(hw_wkup_interrupt_cb cb, uint32_t prio)
{
        intr_cb_p0 = cb;

        NVIC_ClearPendingIRQ(GPIO_P0_IRQn);
        NVIC_SetPriority(GPIO_P0_IRQn, prio);
        NVIC_EnableIRQ(GPIO_P0_IRQn);
}


void hw_wkup_register_gpio_p1_interrupt(hw_wkup_interrupt_cb cb, uint32_t prio)
{
        intr_cb_p1 = cb;

        NVIC_ClearPendingIRQ(GPIO_P1_IRQn);
        NVIC_SetPriority(GPIO_P1_IRQn, prio);
        NVIC_EnableIRQ(GPIO_P1_IRQn);
}



void hw_wkup_unregister_interrupts(void)
{
        intr_cb_key = NULL;
        intr_cb_p0  = NULL;
        intr_cb_p1  = NULL;

        NVIC_DisableIRQ(KEY_WKUP_GPIO_IRQn);
        NVIC_DisableIRQ(GPIO_P0_IRQn);
        NVIC_DisableIRQ(GPIO_P1_IRQn);
}

void hw_wkup_key_handler(void)
{
        if (intr_cb_key) {
                intr_cb_key();
        }
}

void hw_wkup_p0_handler(void)
{
        if (intr_cb_p0) {
                intr_cb_p0();
        }
        else {
                hw_wkup_clear_status(HW_GPIO_PORT_0, 0xFFFFFFFF);
        }
}

void hw_wkup_p1_handler(void)
{
        if (intr_cb_p1) {
                intr_cb_p1();
        } else {
                hw_wkup_clear_status(HW_GPIO_PORT_1, 0x7FFFFF);
        }

}

void Key_Wkup_GPIO_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        hw_wkup_reset_interrupt();
        NVIC_ClearPendingIRQ(KEY_WKUP_GPIO_IRQn);
        hw_wkup_key_handler();

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

void GPIO_P0_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        NVIC_ClearPendingIRQ(GPIO_P0_IRQn);
        hw_wkup_p0_handler();

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

void GPIO_P1_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        NVIC_ClearPendingIRQ(GPIO_P1_IRQn);
        hw_wkup_p1_handler();

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}


#endif /* dg_configUSE_HW_WKUP */
/**
 * \}
 * \}
 * \}
 */
