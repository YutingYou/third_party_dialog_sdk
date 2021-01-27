/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup SENSOR_NODE
 * \{
 */

/**
 *****************************************************************************************
 *
 * @file hw_snc.c
 *
 * @brief Implementation of Sensor Node Controller Low Level Driver
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#if (dg_configUSE_HW_SENSOR_NODE) && (!dg_configUSE_HW_SENSOR_NODE_EMU)

#include <sdk_defs.h>

#include <hw_snc.h>

#if (dg_configSYSTEMVIEW == 1)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif /* (dg_configSYSTEMVIEW == 1) */

/*
 * DATA STRUCTURE DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief Sensor Node local interrupt callback
 *
 */
static hw_snc_interrupt_cb_t snc_intr_cb;

/*
 * FUNCTION DEFINITIONS
 *****************************************************************************************
 */

//==================== Initialization function =================================

void hw_snc_init(const hw_snc_config_t *cfg)
{
        // initialize SNC_CTRL_REG register and manually disable SNC
        SNC->SNC_CTRL_REG = 0;
        hw_snc_manual_disable();

        // set the SNC base address
        if (cfg->base_address) {
                hw_snc_set_base_address(cfg->base_address);
        }

        // set the SNC clock division factor
        hw_snc_set_clock_div(cfg->clk_div);

        // if SW-Control is enabled
        if (cfg->sw_ctrl_en) {
                // set the bus error detection
                if (cfg->bus_error_detect_en) {
                        hw_snc_bus_error_detection_enable();
                } else {
                        hw_snc_bus_error_detection_disable();
                }

                // set PC value to base address
                hw_snc_set_pc(hw_snc_get_base_address());

                // enable/disable PDC event related to SNC
                if (cfg->snc_irq_to_pdc_en) {
                        hw_snc_pdc_event_enable();
                } else {
                        hw_snc_pdc_event_disable();
                }

                // enable/disable SNC interrupt to CM33
                if (cfg->snc_irq_to_cm33_en) {
                        hw_snc_interrupt_enable();
                } else {
                        hw_snc_interrupt_disable();
                }
        }
        // if PDC control is enabled
        else {
                hw_snc_pdc_ctrl_enable();
        }
}

//==================== Configuration functions =================================

void hw_snc_register_int(hw_snc_interrupt_cb_t handler)
{
        GLOBAL_INT_DISABLE();
        snc_intr_cb = handler;
        NVIC_ClearPendingIRQ(SNC_IRQn);
        REG_SETF(SNC, SNC_CTRL_REG, SNC_IRQ_ACK, 1);
        GLOBAL_INT_RESTORE();
        NVIC_EnableIRQ(SNC_IRQn);
}

void hw_snc_unregister_int(void)
{
        NVIC_DisableIRQ(SNC_IRQn);
        NVIC_ClearPendingIRQ(SNC_IRQn);
        snc_intr_cb = NULL;
}

//==================== IRQ Handler =============================================

void Sensor_Node_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        REG_SETF(SNC, SNC_CTRL_REG, SNC_IRQ_ACK, 1);

        if (snc_intr_cb) {
                snc_intr_cb();
        }

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}


#endif /* dg_configUSE_HW_SENSOR_NODE */


/**
 \}
 \}
 \}
 */
