/**
 * \addtogroup PLA_DRI_PER_TIMERS
 * \{
 * \addtogroup HW_Wakeup_Timer Wakeup Timer Driver
 * \{
 * \brief Wakeup Timer
 */

/**
 *****************************************************************************************
 *
 * @file hw_wkup_da1469x.h
 *
 * @brief Definition of API for the Wakeup timer Low Level Driver.
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *****************************************************************************************
 */

#ifndef HW_WKUP_DA1469x_H_
#define HW_WKUP_DA1469x_H_


#if dg_configUSE_HW_WKUP

#include <stdbool.h>
#include <stdint.h>
#include <sdk_defs.h>
#include "hw_gpio.h"

#undef  HW_GPIO_NUM_PORTS
#define HW_GPIO_NUM_PORTS       (2)


#define WKUP_SELECT_WKUP_P0_BASE_REG (volatile uint32_t *)(&WAKEUP->WKUP_SELECT_P0_REG)
#define WKUP_SELECT_GPIO_P0_BASE_REG (volatile uint32_t *)(&WAKEUP->WKUP_SEL_GPIO_P0_REG)

/**
 * \brief Get the mask of a field of an WKUP register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to access
 *
 */
#define HW_WKUP_REG_FIELD_MASK(reg, field) \
                (WAKEUP_WKUP_##reg##_REG_##field##_Msk)

/**
 * \brief Get the bit position of a field of an WKUP register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to access
 *
 */
#define HW_WKUP_REG_FIELD_POS(reg, field) \
                (WAKEUP_WKUP_##reg##_REG_##field##_Pos)

/**
 * \brief Get the value of a field of an WKUP register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to write
 *
 * \return the value of the register field
 *
 */
#define HW_WKUP_REG_GETF(reg, field) \
                ((WAKEUP->WKUP_##reg##_REG & (WAKEUP_WKUP_##reg##_REG_##field##_Msk)) >> (WAKEUP_WKUP_##reg##_REG_##field##_Pos))

/**
 * \brief Set the value of a field of an WKUP register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to write
 * \param [in] new_val is the value to write
 *
 */
#define HW_WKUP_REG_SETF(reg, field, new_val) \
               WAKEUP->WKUP_##reg##_REG = ((WAKEUP->WKUP_##reg##_REG & ~(WAKEUP_WKUP_##reg##_REG_##field##_Msk)) | \
                        ((WAKEUP_WKUP_##reg##_REG_##field##_Msk) & ((new_val) << (WAKEUP_WKUP_##reg##_REG_##field##_Pos))))


/**
 * \brief Pin state which triggers an event
 *
 */
typedef enum {
        HW_WKUP_PIN_STATE_HIGH = 0,
        HW_WKUP_PIN_STATE_LOW = 1
} HW_WKUP_PIN_STATE;

/**
 * \brief Wakeup timer configuration
 *
 */
typedef struct {
        uint8_t debounce;       /**< debounce time in ms */
        uint32_t pin_wkup_state[HW_GPIO_NUM_PORTS];   /**< pin states in each port, see hw_wkup_configure_port */
        uint32_t pin_gpio_state[HW_GPIO_NUM_PORTS];   /**< pin states in each port, see hw_wkup_configure_port */
        uint32_t pin_trigger[HW_GPIO_NUM_PORTS]; /**< pin triggers in each port, see hw_wkup_configure_port */
} wkup_config;

typedef void (*hw_wkup_interrupt_cb)(void);

/**
 * \brief Initialize peripheral
 *
 * Resets Wakeup Timer to initial state, i.e. interrupt is disabled and all pin triggers are
 * disabled.
 *
 * \p cfg can be NULL - no configuration is performed in such case.
 *
 * \param [in] cfg configuration
 *
 */
void hw_wkup_init(const wkup_config *cfg);

/**
 * \brief Configure peripheral
 *
 * Shortcut to call appropriate configuration function. If \p cfg is NULL, this function does
 * nothing.
 *
 * \param [in] cfg configuration
 *
 */
void hw_wkup_configure(const wkup_config *cfg);


/**
 * \brief Register KEY interrupt handler
 *
 * Callback function is called when interrupt is generated. Interrupt is automatically enabled
 * after calling this function. Application should reset
 * interrupt in callback function using hw_wkup_reset_interrupt(). If no callback is specified,
 * interrupt will be automatically cleared by the driver.
 *
 * \param [in] cb callback function
 * \param [in] prio the priority of the interrupt
 *
 */

void hw_wkup_register_key_interrupt(hw_wkup_interrupt_cb cb, uint32_t prio);

/**
 * \brief Register GPIO P0 interrupt handler
 *
 * Callback function is called when interrupt is generated. Interrupt is automatically enabled
 * after calling this function. Application should reset
 * interrupt in callback function using hw_wkup_reset_interrupt(). If no callback is specified,
 * interrupt will be automatically cleared by the driver.
 *
 * \param [in] cb callback function
 * \param [in] prio the priority of the interrupt
 *
 */

void hw_wkup_register_gpio_p0_interrupt(hw_wkup_interrupt_cb cb, uint32_t prio);

/**
 * \brief Register GPIO P1 interrupt handler
 *
 * Callback function is called when interrupt is generated. Interrupt is automatically enabled
 * after calling this function. Application should reset
 * interrupt in callback function using hw_wkup_reset_interrupt(). If no callback is specified,
 * interrupt will be automatically cleared by the driver.
 *
 * \param [in] cb callback function
 * \param [in] prio the priority of the interrupt
 *
 */

void hw_wkup_register_gpio_p1_interrupt(hw_wkup_interrupt_cb cb, uint32_t prio);


/**
 * \brief Unregister interrupt handler
 *
 * Interrupt is automatically disabled after calling this function.
 *
 */
void hw_wkup_unregister_interrupts(void);

/**
 * \brief Reset interrupt
 *
 * This function MUST be called by any user-specified interrupt callback, to clear the interrupt.
 *
 */
__STATIC_INLINE void hw_wkup_reset_interrupt(void)
{
        WAKEUP->WKUP_RESET_IRQ_REG = 1;
}

/**
 * \brief Interrupt handler
 *
 */
void hw_wkup_handler(void);


/**
 * \brief Set debounce time
 *
 * Setting debounce time to 0 will disable hardware debouncing. Maximum debounce time is 63ms.
 *
 * \param [in] time_ms debounce time in milliseconds
 *
 */
__STATIC_INLINE void hw_wkup_set_debounce_time(uint8_t time_ms)
{
        HW_WKUP_REG_SETF(CTRL, WKUP_DEB_VALUE, time_ms);
}

/**
 * \brief Get current debounce time
 *
 * \return debounce time in milliseconds
 *
 */
__STATIC_INLINE uint8_t hw_wkup_get_debounce_time(void)
{
        return HW_WKUP_REG_GETF(CTRL, WKUP_DEB_VALUE);
}

/**
 * \brief Set WKUP pin event state
 *
 * Once enabled, the pin generates an interrupt
 *
 * \param [in] port port number
 * \param [in] pin pin number
 * \param [in] enabled pin for wakeup
 *
 * \sa hw_wkup_set_pin_trigger
 * \sa hw_wkup_configure_pin
 * \sa hw_wkup_configure_port
 *
 */
__STATIC_INLINE void hw_wkup_set_pin_state(HW_GPIO_PORT port, HW_GPIO_PIN pin, bool enabled)
{
        volatile uint32_t *wkup_pin_enable_reg = WKUP_SELECT_WKUP_P0_BASE_REG + port;
        uint32_t wkup_pin_enable_val = *wkup_pin_enable_reg;
        wkup_pin_enable_val &= ~(1 << pin);
        wkup_pin_enable_val |= (!!enabled) << pin;
        *wkup_pin_enable_reg = wkup_pin_enable_val;
}

/**
 * \brief Set GPIO pin event state
 *
 * Once enabled, the pin generates an interrupt
 *
 * \param [in] port port number
 * \param [in] pin pin number
 * \param [in] enabled pin for wakeup
 *
 * \sa hw_wkup_set_pin_trigger
 * \sa hw_wkup_configure_pin
 * \sa hw_wkup_configure_port
 *
 */
__STATIC_INLINE void hw_wkup_gpio_set_pin_state(HW_GPIO_PORT port, HW_GPIO_PIN pin, bool enabled)
{
        volatile uint32_t *wkup_pin_enable_reg = WKUP_SELECT_GPIO_P0_BASE_REG + port;
        uint32_t wkup_pin_enable_val = *wkup_pin_enable_reg;
        wkup_pin_enable_val &= ~(1 << pin);
        wkup_pin_enable_val |= (!!enabled) << pin;
        *wkup_pin_enable_reg = wkup_pin_enable_val;
}


/** \brief Get WKUP pin state
 *
 * \param [in] port port number
 * \param [in] pin pin number
 *
 * \return pin event triggering state
 *
 * \sa hw_wkup_set_pin_state
 *
 */
__STATIC_INLINE bool hw_wkup_get_pin_state(HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
        uint32_t wkup_pin_enable_val = *(WKUP_SELECT_WKUP_P0_BASE_REG + port);
        return (wkup_pin_enable_val & (1 << pin)) >> pin;
}

/** \brief Get GPIO WKUP pin state
 *
 * \param [in] port port number
 * \param [in] pin pin number
 *
 * \return pin event triggering state
 *
 * \sa hw_wkup_set_pin_state
 *
 */
__STATIC_INLINE bool hw_wkup_gpio_get_pin_state(HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
        uint32_t wkup_pin_enable_val = *(WKUP_SELECT_GPIO_P0_BASE_REG + port);
        return (wkup_pin_enable_val & (1 << pin)) >> pin;
}


/**
 * \brief Set GPIO pin state which triggers event
 *
 * Pin event triggering should be enabled for this setting to have any effect.
 *
 * \param [in] port port number
 * \param [in] pin pin number
 * \param [in] state pin state
 *
 *
 */
__STATIC_INLINE void hw_wkup_set_pin_trigger(HW_GPIO_PORT port, HW_GPIO_PIN pin,
        HW_WKUP_PIN_STATE state)
{
        uint32_t pol_rx_reg = *((volatile uint32_t *)(&WAKEUP->WKUP_POL_P0_REG) + port);
        pol_rx_reg &= ~(1 << pin);
        pol_rx_reg |= (!!state) << pin;
        *((volatile uint32_t *)(&WAKEUP->WKUP_POL_P0_REG) + port) = pol_rx_reg;
}

/** \brief Get GPIO pin state which triggers event
 *
 * \param [in] port port number
 * \param [in] pin pin number
 *
 * \return pin state
 *
 * \sa hw_wkup_set_pin_trigger
 *
 */
__STATIC_INLINE HW_WKUP_PIN_STATE hw_wkup_get_pin_trigger(HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
        uint32_t pol_rx_reg = *((volatile uint32_t *)(&WAKEUP->WKUP_POL_P0_REG) + port);
        return (pol_rx_reg & (1 << pin)) >> pin;
}

/**
 * \brief Set WKUP pin event triggered state
 *
 * Effectively, this is shortcut for calling hw_wkup_set_pin_state() and hw_wkup_set_pin_trigger().
 *
 * \param [in] port port number
 * \param [in] pin pin number
 * \param [in] enabled pin event triggering state
 * \param [in] state pin state
 *
 */
__STATIC_INLINE void hw_wkup_configure_pin(HW_GPIO_PORT port, HW_GPIO_PIN pin, bool enabled,
        HW_WKUP_PIN_STATE state)
{
        // first set up the proper polarity...
        hw_wkup_set_pin_trigger(port, pin, state);
        // ...then enable trigger on the specific GPIO
        hw_wkup_set_pin_state(port, pin, enabled);
}

/**
 * \brief Set GPIO pin event triggering state
 *
 * Effectively, this is shortcut for calling hw_wkup_set_pin_state() and hw_wkup_set_pin_trigger().
 *
 * \param [in] port port number
 * \param [in] pin pin number
 * \param [in] enabled pin event triggring state
 * \param [in] state pin state
 *
 */
__STATIC_INLINE void hw_wkup_gpio_configure_pin(HW_GPIO_PORT port, HW_GPIO_PIN pin, bool enabled,
        HW_WKUP_PIN_STATE state)
{
        // first set up the proper polarity...
        hw_wkup_set_pin_trigger(port, pin, state);
        // ...then enable trigger on the specific GPIO
        hw_wkup_gpio_set_pin_state(port, pin, enabled);
}



/**
 * \brief Configure event triggering state for whole GPIO port
 *
 * In \p enabled and \p state bitmasks each bit describes state of corresponding pin in port.
 * For \p enabled 0 means disabled and 1 means enabled.
 * For \p state 0 means event is triggered on low state and 1 means trigger is on high state.
 *
 * \param [in] port port number
 * \param [in] enabled_wkup pins enabled
 * \param [in] enabled_gpio pins enabled
 * \param [in] polarity pin state bitmask
 *
 *
 */
__STATIC_INLINE void hw_wkup_configure_port(HW_GPIO_PORT port, uint32_t enabled_wkup, uint32_t enabled_gpio, uint32_t polarity)
{
        *((volatile uint32_t *)(&WAKEUP->WKUP_POL_P0_REG) + port) = ~polarity; // register has inverted logic than state bitmask
        *(WKUP_SELECT_GPIO_P0_BASE_REG + port) = enabled_gpio;
        *(WKUP_SELECT_WKUP_P0_BASE_REG + port) = enabled_wkup;
}


/**
 * \brief Get state (enabled/disabled) of all pins in GPIO port
 *
 * Meaning of bits in returned bitmask is the same as in hw_wkup_configure_port().
 *
 * \return port pin event state bitmask
 *
 * \sa hw_wkup_configure_port
 *
 */
__STATIC_INLINE uint32_t hw_wkup_get_port_state(HW_GPIO_PORT port)
{
        return *((volatile uint32_t *)(&WAKEUP->WKUP_SELECT_P0_REG) + port);
}

/**
 * \brief Get event triggering state for all pins in GPIO port
 *
 * Meaning of bits in returned bitmask is the same as in hw_wkup_configure_port().
 *
 * \return port pin event triggering state bitmask
 *
 * \sa hw_wkup_configure_port
 *
 */
__STATIC_INLINE uint32_t hw_wkup_get_port_trigger(HW_GPIO_PORT port)
{
        return ~(*((volatile uint32_t *)(&WAKEUP->WKUP_POL_P0_REG) + port)); // register has inverted logic that returned bitmask
}

/**
 * \brief Emulate key hit
 *
 * Key event simulation
 *
 */
__STATIC_INLINE void hw_wkup_emulate_key_hit(void)
{
        HW_WKUP_REG_SETF(CTRL, WKUP_SFT_KEYHIT, 1);
        HW_WKUP_REG_SETF(CTRL, WKUP_SFT_KEYHIT, 0);
}

/**
 * \brief Enable WKUP interrupts
 *
 */
__STATIC_INLINE void hw_wkup_enable_irq(void)
{
        HW_WKUP_REG_SETF(CTRL, WKUP_ENABLE_IRQ, 1);
}
/**
 * \brief Disable WKUP interrupts
 *
 */
__STATIC_INLINE void hw_wkup_disable_irq(void)
{
        HW_WKUP_REG_SETF(CTRL, WKUP_ENABLE_IRQ, 0);
}


/**
 * \brief Freeze wakeup timer
 *
 */
__STATIC_INLINE void hw_wkup_freeze(void)
{
        GPREG->SET_FREEZE_REG = GPREG_SET_FREEZE_REG_FRZ_WKUPTIM_Msk;
}

/**
 * \brief Unfreeze wakeup timer
 *
 */
__STATIC_INLINE void hw_wkup_unfreeze(void)
{
        GPREG->RESET_FREEZE_REG = GPREG_RESET_FREEZE_REG_FRZ_WKUPTIM_Msk;
}

/**
 * \brief Get port status on last wake up
 *
 * Meaning of bits in returned bitmask is the same as in hw_wkup_configure_port().
 *
 * \return port pin event state bitmask
 *
 * \sa hw_wkup_configure_port
 *
 */
__STATIC_INLINE uint32_t hw_wkup_get_status(HW_GPIO_PORT port)
{
        switch (port) {
        case HW_GPIO_PORT_0:
                return HW_WKUP_REG_GETF(STATUS_P0, WKUP_STAT_P0);
        case HW_GPIO_PORT_1:
                return HW_WKUP_REG_GETF(STATUS_P1, WKUP_STAT_P1);
        default:
                ASSERT_WARNING(0);// Invalid argument
                return 0;         // Should never reach here
        }
}


/**
 * \brief Clear latch status
 *
 * This function MUST be called by any user-specified interrupt callback,
 * to clear the interrupt latch status.
 *
 * \param [in] port port number
 * \param [in] status pin status bitmask
 *
 * \sa hw_wkup_get_status
 */

__STATIC_INLINE void hw_wkup_clear_status(HW_GPIO_PORT port, uint32_t status)
{
        switch (port) {
        case HW_GPIO_PORT_0:
                HW_WKUP_REG_SETF(CLEAR_P0, WKUP_CLEAR_P0, status);
                break;
        case HW_GPIO_PORT_1:
                HW_WKUP_REG_SETF(CLEAR_P1, WKUP_CLEAR_P1, status);
                break;
        default:
                ASSERT_WARNING(0);//Invalid argument
        }
}

/**
 * \brief Access WKUP_SEL_GPIO_Px_REG
 *
 *
 * \param [in] port port number
 * \param [in] status pin status bitmask
 *
 * \sa hw_wkup_get_status
 */

__STATIC_INLINE void hw_wkup_sel_gpio(HW_GPIO_PORT port, uint32_t status)
{
        switch (port) {
        case HW_GPIO_PORT_0:
                HW_WKUP_REG_SETF(SEL_GPIO_P0, WKUP_SEL_GPIO_P0, status);
                break;
        case HW_GPIO_PORT_1:
                HW_WKUP_REG_SETF(SEL_GPIO_P1, WKUP_SEL_GPIO_P1, status);
                break;
        default:
                ASSERT_WARNING(0);//Invalid argument
        }
}

#endif /* dg_configUSE_HW_WKUP */
#endif /* HW_WKUP_DA1469x_H_ */

/**
 * \}
 * \}
 */
