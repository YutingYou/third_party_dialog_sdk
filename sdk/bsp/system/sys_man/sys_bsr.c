/**
****************************************************************************************
*
* @file sys_bsr.c
*
* @brief Busy Status Register (BSR) driver
*
* Copyright (C) 2018-2019 Dialog Semiconductor.
* This computer program includes Confidential, Proprietary Information
* of Dialog Semiconductor. All Rights Reserved.
*
****************************************************************************************
*/

#include "sdk_defs.h"
#include "hw_sys.h"
#include "sys_bsr.h"
#ifndef OS_BAREMETAL
#include "osal.h"
#endif /* OS_BAREMETAL */

#ifndef OS_BAREMETAL
__RETAINED static OS_MUTEX sys_sw_bsr_mutex;
#endif

void sys_sw_bsr_init(void)
{
#ifndef OS_BAREMETAL
        OS_MUTEX_CREATE(sys_sw_bsr_mutex);
#endif /* OS_BAREMETAL */
}

void sys_sw_bsr_acquire(SW_BSR_MASTER_ID sw_bsr_master_id, uint32_t periph_id)
{
#ifndef OS_BAREMETAL
        OS_ASSERT(sys_sw_bsr_mutex);
        OS_MUTEX_GET(sys_sw_bsr_mutex, OS_MUTEX_FOREVER);
#endif /* OS_BAREMETAL */
        while (!hw_sys_sw_bsr_try_acquire(sw_bsr_master_id, periph_id)) {
        }
#ifndef OS_BAREMETAL
        OS_MUTEX_PUT(sys_sw_bsr_mutex);
#endif /* OS_BAREMETAL */
}

bool sys_sw_bsr_acquired(SW_BSR_MASTER_ID sw_bsr_master_id, uint32_t periph_id)
{
        bool acquired;
#ifndef OS_BAREMETAL
        OS_ASSERT(sys_sw_bsr_mutex);
        OS_MUTEX_GET(sys_sw_bsr_mutex, OS_MUTEX_FOREVER);
#endif /* OS_BAREMETAL */
        acquired = hw_sys_sw_bsr_acquired(sw_bsr_master_id, periph_id);
#ifndef OS_BAREMETAL
        OS_MUTEX_PUT(sys_sw_bsr_mutex);
#endif /* OS_BAREMETAL */
        return acquired;
}

void sys_sw_bsr_release(SW_BSR_MASTER_ID sw_bsr_master_id, uint32_t periph_id)
{
#ifndef OS_BAREMETAL
        OS_ASSERT(sys_sw_bsr_mutex);
        OS_MUTEX_GET(sys_sw_bsr_mutex, OS_MUTEX_FOREVER);
#endif /* OS_BAREMETAL */
        hw_sys_sw_bsr_release(sw_bsr_master_id, periph_id);
#ifndef OS_BAREMETAL
        OS_MUTEX_PUT(sys_sw_bsr_mutex);
#endif /* OS_BAREMETAL */
}

