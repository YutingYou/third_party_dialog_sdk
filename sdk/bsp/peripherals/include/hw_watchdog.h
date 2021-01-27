/**
 * \addtogroup PLA_DRI_PER_TIMERS
 * \{
 * \addtogroup HW_WATCHDOG_TIMER Watchdog Timer Driver
 * \{
 * \brief Watchdog Timer
 */

/**
 ****************************************************************************************
 *
 * @file hw_watchdog.h
 *
 * @brief Definition of API for the Watchdog timer Low Level Driver.
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_WATCHDOG_H_
#define HW_WATCHDOG_H_


#include <stdbool.h>
#include <stdint.h>
#include <sdk_defs.h>

#define NMI_MAGIC_NUMBER                0xDEADBEEF

/**
 * \brief Holds the stack contents when an NMI occurs.
 *
 * \details The stack contents are copied at this variable when an NMI occurs. The first position is
 *        marked with a special "flag" (0xDEADBEEF) to indicate that the data that follow are valid.
 */
extern volatile uint32_t nmi_event_data[9];

/**
 * \brief Types of generated states if reload value is 0
 *
 * Generate NMI (non-maskable interrupt) or RST (reset of the system)
 *
 */
typedef enum {
        HW_WDG_RESET_NMI = 0,     /**< Generate NMI if the watchdog reaches 0 and WDOG reset if the counter become less or equal to -16 */
        HW_WDG_RESET_RST = 1      /**< Generate WDOG reset it the counter becomes less or equal than 0 */
} HW_WDG_RESET;

/**
 * \brief Watchdog timer interrupt callback
 *
 * \param [in] hardfault_args pointer to call stack
 *
 */
typedef void (*hw_watchdog_interrupt_cb)(unsigned long *exception_args);

/**
 * \brief Freeze the watchdog
 *
 * \return true if operation is allowed, else false
 *
 */
__RETAINED_CODE bool hw_watchdog_freeze(void);

/**
 * \brief Unfreeze the watchdog
 *
 * \return true if operation is allowed, else false
 *
 */
__RETAINED_CODE bool hw_watchdog_unfreeze(void);


/**
 * \brief Enable/disable writing the Watchdog timer reload value.
 * This filter prevents unintentionally setting the watchdog with a SW run-away.
 *
 * \param [in] enable   true = write enable for Watchdog reload value
 *                      false = write disable for Watchdog reload value
 *
 * \sa hw_watchdog_set_pos_val
 * \sa hw_watchdog_set_neg_val
 *
 */
__STATIC_INLINE void hw_watchdog_write_value_ctrl(bool enable)
{
        while (REG_GETF(SYS_WDOG, WATCHDOG_CTRL_REG, WRITE_BUSY));
        if (enable) {
                REG_SETF(SYS_WDOG, WATCHDOG_REG, WDOG_WEN, 0x0);
        }
        else {
                REG_SETF(SYS_WDOG, WATCHDOG_REG, WDOG_WEN, 0xff);
        }
}

/**
 * \brief Set positive reload value of the watchdog timer
 *
 * \param [in] value reload value for 13 bits down counter in the PD_AON power domain
 *             which is running on either a 10,24 ms clock or a 20,5 ms clock period
 *             and can operate for 84 sec or 3 minutes (depending on the clock).
 *
 * \sa hw_watchdog_write_value_ctrl
 *
 */
__STATIC_FORCEINLINE void hw_watchdog_set_pos_val(uint16_t value)
{
        uint32_t tmp;

        ASSERT_WARNING(SYS_WDOG_WATCHDOG_REG_WDOG_VAL_Msk >= value); // check if reload value is greater than max allowed value
        ASSERT_WARNING(!REG_GETF(SYS_WDOG, WATCHDOG_REG, WDOG_WEN)); // can not write register if WDOG_WEN is not zero
        tmp = SYS_WDOG->WATCHDOG_REG;
        REG_SET_FIELD(SYS_WDOG, WATCHDOG_REG, WDOG_VAL_NEG, tmp, 0);
        REG_SET_FIELD(SYS_WDOG, WATCHDOG_REG, WDOG_VAL, tmp, value);
        while (REG_GETF(SYS_WDOG, WATCHDOG_CTRL_REG, WRITE_BUSY));
        SYS_WDOG->WATCHDOG_REG = tmp;
}


/**
 * \brief Set negative reload value of the watchdog timer
 *
 * \param [in] value reload value from 0x1FFF to 0x00
 *
 * \sa hw_watchdog_write_value_ctrl
 *
 */
__STATIC_INLINE void hw_watchdog_set_neg_val(uint16_t value)
{
        uint32_t tmp;

        ASSERT_WARNING(SYS_WDOG_WATCHDOG_REG_WDOG_VAL_Msk >= value); // check if reload value is greater than max allowed value
        ASSERT_WARNING(!REG_GETF(SYS_WDOG, WATCHDOG_REG, WDOG_WEN)); // can not write register if WDOG_WEN is not zero
        tmp = SYS_WDOG->WATCHDOG_REG;
        REG_SET_FIELD(SYS_WDOG, WATCHDOG_REG, WDOG_VAL_NEG, tmp, 1);
        REG_SET_FIELD(SYS_WDOG, WATCHDOG_REG, WDOG_VAL, tmp, value);
        while (REG_GETF(SYS_WDOG, WATCHDOG_CTRL_REG, WRITE_BUSY));
        SYS_WDOG->WATCHDOG_REG = tmp;
}

/**
 * \brief Get reload value of the watchdog timer
 *
 */
__STATIC_INLINE uint16_t hw_watchdog_get_val(void)
{
        // The watchdog value cannot be read while watchdog is busy writing a new value
        ASSERT_WARNING(REG_GETF(SYS_WDOG, WATCHDOG_CTRL_REG, WRITE_BUSY) == 0);

        return REG_GETF(SYS_WDOG, WATCHDOG_REG, WDOG_VAL);
}

/**
 * \brief Generate a reset signal of the system when reload value reaches 0
 *
 */
__STATIC_FORCEINLINE void hw_watchdog_gen_RST(void)
{
        REG_SET_BIT(SYS_WDOG, WATCHDOG_CTRL_REG, NMI_RST);
}

/**
 * \brief Generate an NMI when reload value reaches 0
 *
 */
__STATIC_FORCEINLINE void hw_watchdog_gen_NMI(void)
{
        REG_CLR_BIT(SYS_WDOG, WATCHDOG_CTRL_REG, NMI_RST);
}

/**
 * \brief Enable/disable Watchdog freeze functionality
 *
 * \param [in] enable   true = Watchdog timer can not be frozen when NMI_RST=0.
 *                      false = Watchdog timer can be frozen/resumed when NMI_RST=0
 *
 * \sa hw_watchdog_freeze
 * \sa hw_watchdog_unfreeze
 * \sa hw_watchdog_gen_RST
 *
 */
__STATIC_INLINE void hw_watchdog_freeze_ctrl(bool enable)
{
        if (enable) {
                REG_SET_BIT(SYS_WDOG, WATCHDOG_CTRL_REG, WDOG_FREEZE_EN);
        }
        else {
                REG_CLR_BIT(SYS_WDOG, WATCHDOG_CTRL_REG, WDOG_FREEZE_EN);
        }
}

/**
 * \brief Register an interrupt handler
 *
 * \param [in] handler function pointer to handler to call when an interrupt occurs
 *
 */
void hw_watchdog_register_int(hw_watchdog_interrupt_cb handler);

/**
 * \brief Unregister an interrupt handler
 *
 */
__RETAINED_CODE void hw_watchdog_unregister_int(void);

/**
 * \brief Handle NMI interrupt.
 *
 * \param [in] hardfault_args pointer to call stack
 *
 */
__RETAINED_CODE void hw_watchdog_handle_int(unsigned long *hardfault_args);

/**
 * \brief Check whether the timer has expired
 *
 * \return true, if the timer has expired, false otherwise
 *
 */
bool hw_watchdog_is_timer_expired(void);

/**
 * \brief Check what is generated when watchdog reaches 0 value
 *
 * If it is NMI (interrupt) or RST (system/wdog reset).
 *
 * \return HW_WDG_RESET_NMI if NMI interrupt is generated, otherwise HW_WDG_RESET_RST
 *
 */
HW_WDG_RESET hw_watchdog_is_irq_or_rst_gen(void);


#endif /* HW_WATCHDOG_H_ */


/**
 * \}
 * \}
 */
