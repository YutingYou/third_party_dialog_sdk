/**
\addtogroup BSP
\{
\addtogroup SYSTEM
\{
\addtogroup SYS_TIMER
\{
*/

/**
 ****************************************************************************************
 *
 * @file sys_timer.c
 *
 * @brief System timer
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if ((DEVICE_FAMILY == DA1469X) && dg_configUSE_HW_TIMER) || ((DEVICE_FAMILY == DA1468X) && dg_configUSE_HW_TIMER1)

#include <stdint.h>
#include "hw_timer.h"

#include "sys_timer_internal.h"

#ifdef OS_FREERTOS
#include "sys_timer.h"
#include "FreeRTOS.h"
#include "sys_power_mgr.h"
#include "sys_power_mgr_internal.h"
#endif

#define TICK_GUARD_PRESC_LIM                    2

/*
 * Global and / or retained variables
 */

__RETAINED uint32_t lp_last_trigger;               // counts in prescaled LP cycles

#if (SYS_TIM_DEBUG == 1)

__RETAINED uint32_t rt_elapsed_time;
__RETAINED uint32_t rt_elapsed_ticks;
__RETAINED uint32_t trigger_hit_at_ret;

struct sys_tim_trigger_mon_st {
        uint16_t type;  //0: lp_last_trigger, 1: trigger
        uint16_t value;
};

#define MAX_TRG_MON_SZ          64      // Must be a power of 2

__RETAINED struct sys_tim_trigger_mon_st rt_trigger_mon[MAX_TRG_MON_SZ];
__RETAINED uint32_t rt_trigger_mon_wr;

#endif


#ifdef OS_FREERTOS
__RETAINED static uint32_t current_time;
__RETAINED static uint64_t sys_rtc_time;
#endif


/*
 * Forward declarations
 */

void xPortTickAdvance(void);

/*
 * Function definitions
 */

#ifdef OS_FREERTOS

__RETAINED_CODE static uint32_t sys_timer_advance_time_compute(uint32_t *trigger)
{
        uint32_t elapsed_time;
        uint32_t elapsed_ticks;
        uint32_t tick_offset;
        uint32_t test_val;

        uint32_t timer_value = hw_timer_get_count(HW_TIMER2);

        hw_timer_unregister_int(HW_TIMER2);

        elapsed_time = (timer_value - lp_last_trigger) & LP_CNT_NATIVE_MASK;

        /* When in Idle mode, the elapsed time period cannot be equal to LP_CNT_NATIVE_MASK!
         * If this happens then it is a "fake" Timer2 interrupt caused by the 1 LP cycle propagation
         * delay of the internal interrupt of Timer2. This interrupt must be ignored!!! */
        if (pm_get_system_sleep_state() != sys_powered_down) {
                if (elapsed_time >= (LP_CNT_NATIVE_MASK - 1)) {
                        *trigger = (lp_last_trigger + TICK_PERIOD) & LP_CNT_NATIVE_MASK;
                        return 0;
                }
        }

        /* We know that we can be within 1 tick period "earlier" than the system time since we may
         * have to intentionally advance the system time by 1 tick during the trigger programming.
         */
        if (elapsed_time >= LP_CNT_NATIVE_MASK - TICK_PERIOD) {
                return 0;
        }

        elapsed_ticks = elapsed_time / TICK_PERIOD;
        tick_offset = elapsed_time - (elapsed_ticks * TICK_PERIOD);       // Offset in current tick (in prescaled LP cycles).

#if (SYS_TIM_DEBUG == 1)
        rt_elapsed_time = elapsed_time;
        rt_elapsed_ticks = elapsed_ticks;
#endif

        // Compute next trigger.
        *trigger = lp_last_trigger + (elapsed_ticks + 1) * TICK_PERIOD;
        if (TICK_GUARD_PRESC_LIM >=
                        (TICK_PERIOD - tick_offset)) {  // Too close in time
                *trigger += TICK_PERIOD;                // Set trigger at the next tick
                elapsed_ticks++;                        // Report 1 more tick spent sleeping
        }
        *trigger &= LP_CNT_NATIVE_MASK;              // Make sure it's within limits...

        lp_last_trigger = (*trigger - TICK_PERIOD) & LP_CNT_NATIVE_MASK;

#if (SYS_TIM_DEBUG == 1)
        rt_trigger_mon[rt_trigger_mon_wr].type = 0;
        rt_trigger_mon[rt_trigger_mon_wr].value = lp_last_trigger;
        rt_trigger_mon_wr++;
        rt_trigger_mon_wr %= MAX_TRG_MON_SZ;
#endif

        if ((dg_configUSE_LP_CLK == LP_CLK_32000) || (dg_configUSE_LP_CLK == LP_CLK_32768)) {
                test_val = lp_last_trigger + 1;
                /* test_val must be an exact multiple of TICK_PERIOD */
                ASSERT_WARNING((test_val % TICK_PERIOD) == 0);
        }
        return elapsed_ticks;
}

__RETAINED_CODE static void sys_timer_advance_time_apply(uint32_t trigger)
{
        uint32_t dummy __UNUSED;
        uint32_t timer_current_value;
        uint32_t diff;

        timer_current_value = (hw_timer_get_count(HW_TIMER2) + 1) & LP_CNT_NATIVE_MASK;
        diff = (timer_current_value - trigger) & LP_CNT_NATIVE_MASK;

        while (diff <= LP_CNT_NATIVE_MASK/2) {
                trigger += TICK_PERIOD;              // Set trigger at the next tick
                trigger &= LP_CNT_NATIVE_MASK;       // Make sure it's within limits...

                timer_current_value = (hw_timer_get_count(HW_TIMER2) + 1) & LP_CNT_NATIVE_MASK;
                diff = (timer_current_value - trigger) & LP_CNT_NATIVE_MASK;
        }

        sys_timer_set_trigger(trigger);

#if (SYS_TIM_DEBUG == 1)
        rt_trigger_mon[rt_trigger_mon_wr].type = 1;
        rt_trigger_mon[rt_trigger_mon_wr].value = trigger;
        rt_trigger_mon_wr++;
        rt_trigger_mon_wr %= MAX_TRG_MON_SZ;
#endif
}

/**
 * \brief Advances time from the previous tick that hit.
 *
 * \details Calculate how many ticks have passed since the last tick.
 *
 * \return uint32_t The number of ticks passed.
 *
 */
__RETAINED_CODE static uint32_t sys_timer_advance_time(void)
{
        uint32_t elapsed_ticks;
        uint32_t trigger;

        elapsed_ticks = sys_timer_advance_time_compute(&trigger);
        sys_timer_advance_time_apply(trigger);

        return elapsed_ticks;
}

__RETAINED_CODE uint32_t sys_timer_update_slept_time(void)
{
        uint32_t trigger;
        uint32_t elapsed_ticks;
        /*
         * Update Real Time Clock value and calculate the time spent sleeping.
         * lp_prescaled_time - lp_last_trigger : sleep time in lp cycles
         * lp_previous_time : lp timer offset at sleep entry
         */
#if (SYS_TIM_DEBUG == 1)
        trigger_hit_at_ret = hw_timer_get_count(HW_TIMER2);
#endif

        // Calculate time spent sleeping in ticks and the offset in this tick period.
        elapsed_ticks = sys_timer_advance_time_compute(&trigger);

        // Advance time
        if (elapsed_ticks > 0) {
                xTaskIncrementTick();
                vTaskStepTick( elapsed_ticks - 1 );
        }

        sys_timer_advance_time_apply(trigger);

        return elapsed_ticks;
}

#endif /* OS_FREERTOS */

/**
 * \brief Interrupt handler of Timer2
 *
 */
__RETAINED_CODE void OS_TICK_Handler(void)
{
#ifdef OS_FREERTOS
        uint32_t ulPreviousMask;

        ulPreviousMask = portSET_INTERRUPT_MASK_FROM_ISR();

        if (pm_get_system_sleep_state() == sys_active)
        {
                uint32_t elapsed_ticks;

                DBG_SET_HIGH(PWR_MGR_FUNCTIONAL_DEBUG, PWRDBG_TICK);

                elapsed_ticks = sys_timer_advance_time();
                while (elapsed_ticks) {
                        xPortTickAdvance();
                        elapsed_ticks--;
                }
                DBG_SET_LOW(PWR_MGR_FUNCTIONAL_DEBUG, PWRDBG_TICK);
        }

        portCLEAR_INTERRUPT_MASK_FROM_ISR(ulPreviousMask);
#endif
}

void sys_timer_start(uint32_t period)
{
        timer_config timer_cfg = {
                .clk_src = HW_TIMER_CLK_SRC_INT,
                .prescaler = 0,
                .timer = { .direction = HW_TIMER_DIR_UP,
                           .reload_val = period-1,
                           .free_run = true },
                .pwm = { .frequency = 0, .duty_cycle = 0},
        };

        lp_last_trigger = LP_CNT_NATIVE_MASK;

        hw_timer_init(HW_TIMER2, &timer_cfg);
        hw_timer_register_int(HW_TIMER2, OS_TICK_Handler);
        hw_timer_enable(HW_TIMER2);
}

void sys_timer_stop(void)
{
        hw_timer_disable(HW_TIMER2);
}

void sys_timer_set_trigger(uint32_t trigger)
{
        TIMER2->TIMER2_CLEAR_IRQ_REG = 1;
        hw_timer_set_reload(HW_TIMER2, trigger);
        NVIC_ClearPendingIRQ(TIMER2_IRQn);                  // Clear any pending IRQ from this source
        hw_timer_register_int(HW_TIMER2, OS_TICK_Handler);  // Enable interrupt

#if (SYS_TIM_DEBUG == 1)
        rt_trigger_mon[rt_trigger_mon_wr].type = 1;
        rt_trigger_mon[rt_trigger_mon_wr].value = trigger;
        rt_trigger_mon_wr++;
        rt_trigger_mon_wr %= MAX_TRG_MON_SZ;
#endif
}

uint32_t sys_timer_get_tick_offset(void)
{
        uint32_t lp_tick_offset, lp_current_time;

        lp_current_time = hw_timer_get_count(HW_TIMER2);

        lp_tick_offset = lp_current_time - lp_last_trigger;
         // 1b. Set within valid limits.
         lp_tick_offset &= LP_CNT_NATIVE_MASK;
         if (lp_tick_offset > (LP_CNT_NATIVE_MASK / 2)) {
                 lp_tick_offset = 0;
         }
         return lp_tick_offset;
}


#ifdef OS_FREERTOS
#if (dg_configUSE_LP_CLK == LP_CLK_RCX)
__RETAINED static uint64_t sys_rtc_clock_time_fine;         // counts time in usec*1024*1024
__RETAINED static uint64_t sys_rtc_clock_time;         // counts time in usec
#endif

__RETAINED_CODE static void update_timestamp_values(void)
{
        uint32_t prev_time = current_time;

        current_time = hw_timer_get_count(HW_TIMER2);
        uint64_t rtc_tick = (current_time - prev_time) & LP_CNT_NATIVE_MASK;

#if (dg_configUSE_LP_CLK == LP_CLK_RCX)
        uint64_t rcx_time_advance = 0;
        rcx_time_advance = rtc_tick * (uint64_t)cm_get_rcx_clock_period();
        if ((sys_rtc_clock_time_fine + rcx_time_advance) < sys_rtc_clock_time_fine) {
                sys_rtc_clock_time +=  (sys_rtc_clock_time_fine >> 20);
                /* Keep fractional part */
                sys_rtc_clock_time_fine &= ((1 << 20) - 1);
        }
        sys_rtc_clock_time_fine += rcx_time_advance;
#endif
        sys_rtc_time += rtc_tick;
}

uint64_t sys_timer_get_uptime_ticks(void)
{
        vPortEnterCritical();
        update_timestamp_values();
        vPortExitCritical();

        return sys_rtc_time;
}

__RETAINED_CODE uint64_t sys_timer_get_uptime_ticks_fromISR(void)
{
        uint32_t ulPreviousMask = portSET_INTERRUPT_MASK_FROM_ISR();
        update_timestamp_values();
        portCLEAR_INTERRUPT_MASK_FROM_ISR( ulPreviousMask );

        return sys_rtc_time;
}

uint64_t sys_timer_get_uptime_usec(void)
{
        vPortEnterCritical();
        update_timestamp_values();
        vPortExitCritical();

#if (dg_configUSE_LP_CLK == LP_CLK_RCX)
        return sys_rtc_clock_time + (sys_rtc_clock_time_fine >> 20);
#else
        return (sys_rtc_time * 1000000) / configSYSTICK_CLOCK_HZ;
#endif
}

__RETAINED_CODE uint64_t sys_timer_get_uptime_usec_fromISR(void)
{
        uint32_t ulPreviousMask = portSET_INTERRUPT_MASK_FROM_ISR();
        update_timestamp_values();
        portCLEAR_INTERRUPT_MASK_FROM_ISR( ulPreviousMask );

#if (dg_configUSE_LP_CLK == LP_CLK_RCX)
        return sys_rtc_clock_time + (sys_rtc_clock_time_fine >> 20);
#else
        return (sys_rtc_time * 1000000) / configSYSTICK_CLOCK_HZ;
#endif
}


__RETAINED_CODE uint64_t sys_timer_get_timestamp_fromCPM(uint32_t* timer_value)
{
        update_timestamp_values();
        *timer_value = current_time;

        return sys_rtc_time;
}

uint64_t* sys_timer_get_rtc_time(void)
{
        return &sys_rtc_time;
}

uint32_t* sys_timer_get_current_time(void)
{
        return &current_time;
}


#endif /* OS_FREERTOS */

#endif /* dg_configUSE_HW_TIMER */

/**
\}
\}
\}
*/
