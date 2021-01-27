/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup CACHE
 * \{
 * \brief Cache Controller
 */

/**
 *****************************************************************************************
 *
 * @file hw_cache.c
 *
 * @brief Implementation of the Cache Low Level Driver.
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *****************************************************************************************
 */


#if dg_configUSE_HW_CACHE

#include "hw_cache.h"

__RETAINED static hw_cache_cb_t hw_cache_cb;

void hw_cache_mrm_enable_interrupt(hw_cache_cb_t cb)
{
        ASSERT_WARNING(cb);
        hw_cache_cb = cb;
        REG_SET_BIT(CACHE, CACHE_MRM_CTRL_REG, MRM_IRQ_MASK);
        NVIC_ClearPendingIRQ(MRM_IRQn);
        NVIC_EnableIRQ(MRM_IRQn);
}

void hw_cache_mrm_disable_interrupt(void)
{
        REG_CLR_BIT(CACHE, CACHE_MRM_CTRL_REG, MRM_IRQ_MASK);
        NVIC_DisableIRQ(MRM_IRQn);
        NVIC_ClearPendingIRQ(MRM_IRQn);
        hw_cache_cb = NULL;
}

__RETAINED_CODE void MRM_Handler(void)
{
        if (hw_cache_cb) {
                hw_cache_cb();
        }
}

#endif /* dg_configUSE_HW_CACHE */


/**
 * \}
 * \}
 * \}
 */
