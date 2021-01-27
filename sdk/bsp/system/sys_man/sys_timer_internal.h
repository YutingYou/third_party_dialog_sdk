/**
 * \addtogroup BSP
 * \{
 * \addtogroup SYSTEM
 * \{
 * \addtogroup SYS_TIMER
 * \{
 * \addtogroup INTERNAL
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file sys_timer_internal.h
 *
 * @brief System timer internal header file.
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SYS_TIMER_INTERNAL_H_
#define SYS_TIMER_INTERNAL_H_

#include "sdk_defs.h"

#if ((DEVICE_FAMILY == DA1469X) && dg_configUSE_HW_TIMER) || ( (DEVICE_FAMILY == DA1468X) && dg_configUSE_HW_TIMER1)

#include "hw_timer.h"


#define LP_CNT_NATIVE_MASK      ( TIMER2_TIMER2_TIMER_VAL_REG_TIM_TIMER_VALUE_Msk >> TIMER2_TIMER2_TIMER_VAL_REG_TIM_TIMER_VALUE_Pos )

/**
 * \brief Update Real Time Clock value and get current time (in LP cycles).
 *
 * \param[out] timer_value Pointer to the value of the timer
 * \return The current RTC time.
 *
 * \warning This function is used only by the Clock and Power Manager.
 *
 */
__RETAINED_CODE uint64_t sys_timer_get_timestamp_fromCPM(uint32_t *timer_value);

/**
 * \brief Set an "invalid" trigger value, which refers far away in the future
 *
 */
__STATIC_FORCEINLINE void sys_timer_invalidate_trigger(void)
{
        uint32_t lp_current_time;
        uint32_t trigger;

        lp_current_time = hw_timer_get_reload(HW_TIMER2);         // Get current time
        trigger = (lp_current_time - 1) & LP_CNT_NATIVE_MASK;  // Get an "invalid" trigger
        bool timer2_irq_en = HW_TIMER_REG_GETF(HW_TIMER2, TIMER_CTRL_REG, TIM_IRQ_EN);
        if (timer2_irq_en) {
                HW_TIMER_REG_SETF(HW_TIMER2, TIMER_CTRL_REG, TIM_IRQ_EN, 0);
        }
        hw_timer_set_reload(HW_TIMER2, trigger);
        if (timer2_irq_en) {
                HW_TIMER_REG_SETF(HW_TIMER2, TIMER_CTRL_REG, TIM_IRQ_EN, 1);
        }
}

/**
 * \brief Calculate how many ticks have passed since the time the system entered sleep or idle
 *        mode and update OS
 *
 * \return The number of ticks spent while sleeping
 */
__RETAINED_CODE uint32_t sys_timer_update_slept_time(void);

/**
 * \brief Get the address of the current Real Time Clock time.
 *
 * \return The address of the current RTC time.
 *
 */
uint64_t* sys_timer_get_rtc_time(void);

/**
 * \brief Get the address of the current time.
 *
 * \return The address of the current time.
 *
 */
uint32_t* sys_timer_get_current_time(void);


#endif /* dg_configUSE_HW_TIMER */

#endif /* SYS_TIMER_INTERNAL_H_ */

/**
\}
\}
\}
\}
*/
