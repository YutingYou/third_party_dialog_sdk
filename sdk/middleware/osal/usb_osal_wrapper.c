/**
 * \addtogroup MIDDLEWARE
 * \{
 * \addtogroup OSAL
 *
 * \brief OS Abstraction Layer
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file usb_osal_wrapper.c
 *
 * @brief OS abstraction layer API
 *
 * Copyright (C) 2016-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if defined(OS_FREERTOS) && (dg_configUSE_HW_USB == 1)
#include "osal.h"

#if (dg_configUSE_SYS_CHARGER == 1)
void sys_usb_charger_enumeration_done(void);
#endif

void wrapper_os_queue_create(OS_QUEUE *queue, UBaseType_t item_size, UBaseType_t max_items)
{
        OS_QUEUE_CREATE(*queue, item_size, max_items);
}

void wrapper_vQueueDelete(OS_QUEUE *queue)
{
        OS_QUEUE_DELETE(*queue);
}

void wrapper_os_queue_overwrite_from_isr(OS_QUEUE *queue, unsigned *TransactCnt,
        BaseType_t *xHigherPriorityTaskWoken)
{
        xQueueOverwriteFromISR(*queue, TransactCnt, xHigherPriorityTaskWoken);
        //
        // If xHigherPriorityTaskWoken is now set to pdTRUE then a context
        // switch should be requested.  The macro used is port specific and will
        // be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR() - refer to
        // the documentation page for the port being used.
        //
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void wrapper_os_queue_overwrite(OS_QUEUE *queue, unsigned *TransactCnt)
{
        xQueueOverwrite(*queue, TransactCnt);
}

TickType_t wrapper_os_ms_2_ticks(unsigned ms)
{
        return OS_MS_2_TICKS(ms);
}

BaseType_t wrapper_os_queue_receive(OS_QUEUE *queue, unsigned *Cnt, TickType_t Ticks)
{
        return xQueueReceive(*queue, Cnt, Ticks);
}

void wrapper_os_leave_critical_section(void)
{
        OS_LEAVE_CRITICAL_SECTION();
}

void wrapper_os_enter_critical_section(void)
{
        OS_ENTER_CRITICAL_SECTION();
}

void wrapper_os_delay_ms(int ms)
{
        OS_DELAY_MS(ms);
}

TickType_t wrapper_os_get_tick_count(void)
{
        return OS_GET_TICK_COUNT();
}

uint32_t wrapper_os_ticks_2_ms(TickType_t ticks)
{
        return OS_TICKS_2_MS(ticks);
}

void wrapper_usb_charger_connected(void)
{
#if (dg_configUSE_SYS_CHARGER == 1)
        sys_usb_charger_enumeration_done();
#endif
}
#endif /* defined(OS_FREERTOS) && (dg_configUSE_HW_USB == 1) */

/**
 * \}
 * \}
 */
