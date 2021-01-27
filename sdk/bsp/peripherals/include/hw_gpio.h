/**
 * \addtogroup PLA_DRI_PER_COMM
 * \{
 * \addtogroup HW_GPIO GPIO Driver
 * \{
 * \brief General Purpose I/O Controller
 */

/**
 ****************************************************************************************
 *
 * @file hw_gpio.h
 *
 * @brief Definition of API for the GPIO Low Level Driver.
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_GPIO_H_
#define HW_GPIO_H_

#if dg_configUSE_HW_GPIO

#include <stdbool.h>
#include <stdint.h>
#include <sdk_defs.h>

/**
 * Definitions of boards UART retarget GPIO pins, mode and function
 */
#define HW_GPIO_SET_PIN_FUNCTION(X)     hw_gpio_set_pin_function(X##_PORT, X##_PIN, X##_MODE, X##_FUNC);
#define HW_GPIO_PAD_LATCH_ENABLE(X)     hw_gpio_pad_latch_enable(X##_PORT, X##_PIN);
#define HW_GPIO_PAD_LATCH_DISABLE(X)    hw_gpio_pad_latch_disable(X##_PORT, X##_PIN);
/* GPIO layout definitions */

/** Number of GPIO ports available */
#define HW_GPIO_NUM_PORTS       (2)
/** Number of GPIO pins available (cumulative) */
#define HW_GPIO_NUM_PINS        (HW_GPIO_PORT_0_NUM_PINS + HW_GPIO_PORT_1_NUM_PINS)
/** Number of bits required to store any pin number */
#define HW_GPIO_PIN_BITS        (5)
/** Number of GPIO pins available in port 0 */
#define HW_GPIO_PORT_0_NUM_PINS (32)
/** Number of GPIO pins available in port 1 */
#define HW_GPIO_PORT_1_NUM_PINS (23)
/** Definition for invalid GPIO port */
#define HW_GPIO_PORT_NONE       (HW_GPIO_PORT_MAX)
/** Definition for invalid GPIO pin */
#define HW_GPIO_PIN_NONE        (HW_GPIO_PIN_MAX)
/** Definition for invalid GPIO mode */
#define HW_GPIO_MODE_NONE       (HW_GPIO_MODE_INVALID)

extern const uint8_t hw_gpio_port_num_pins[HW_GPIO_NUM_PORTS];

/**
 * \brief GPIO input/output mode
 *
 */
typedef enum {
        HW_GPIO_MODE_INPUT = 0,                 /**< GPIO as an input */
        HW_GPIO_MODE_INPUT_PULLUP = 0x100,      /**< GPIO as an input with pull-up */
        HW_GPIO_MODE_INPUT_PULLDOWN = 0x200,    /**< GPIO as an input with pull-down */
        HW_GPIO_MODE_OUTPUT = 0x300,            /**< GPIO as an (implicitly push-pull) output */
        HW_GPIO_MODE_OUTPUT_PUSH_PULL = 0x300,  /**< GPIO as an (explicitly push-pull) output */
        HW_GPIO_MODE_OUTPUT_OPEN_DRAIN = 0x700, /**< GPIO as an open-drain output */
        HW_GPIO_MODE_INVALID = 0xFFF,           /**< GPIO configured as nothing */
} HW_GPIO_MODE;

/**
 * \brief GPIO power source
 *
 */
typedef enum {
        HW_GPIO_POWER_V33 = 0,          /**< V33 (3.3 V) power rail */
        HW_GPIO_POWER_VDD1V8P = 1,      /**< VDD1V8P (1.8 V) power rail */
        HW_GPIO_POWER_NONE = 2,         /**< Invalid power rail */
} HW_GPIO_POWER;

/**
 * \brief GPIO port number
 *
 */
typedef enum {
        HW_GPIO_PORT_0 = 0,     /**< GPIO Port 0 */
        HW_GPIO_PORT_1 = 1,     /**< GPIO Port 1 */
        HW_GPIO_PORT_MAX        /**< GPIO Port max */
} HW_GPIO_PORT;

/**
 * \brief GPIO pin number
 *
 */
typedef enum {
        HW_GPIO_PIN_0 = 0,      /**< GPIO Pin 0 */
        HW_GPIO_PIN_1 = 1,      /**< GPIO Pin 1 */
        HW_GPIO_PIN_2 = 2,      /**< GPIO Pin 2 */
        HW_GPIO_PIN_3 = 3,      /**< GPIO Pin 3 */
        HW_GPIO_PIN_4 = 4,      /**< GPIO Pin 4 */
        HW_GPIO_PIN_5 = 5,      /**< GPIO Pin 5 */
        HW_GPIO_PIN_6 = 6,      /**< GPIO Pin 6 */
        HW_GPIO_PIN_7 = 7,      /**< GPIO Pin 7 */
        HW_GPIO_PIN_8 = 8,      /**< GPIO Pin 8 */
        HW_GPIO_PIN_9 = 9,      /**< GPIO Pin 9 */
        HW_GPIO_PIN_10 = 10,    /**< GPIO Pin 10 */
        HW_GPIO_PIN_11 = 11,    /**< GPIO Pin 11 */
        HW_GPIO_PIN_12 = 12,    /**< GPIO Pin 12 */
        HW_GPIO_PIN_13 = 13,    /**< GPIO Pin 13 */
        HW_GPIO_PIN_14 = 14,    /**< GPIO Pin 14 */
        HW_GPIO_PIN_15 = 15,    /**< GPIO Pin 15 */
        HW_GPIO_PIN_16 = 16,    /**< GPIO Pin 16 */
        HW_GPIO_PIN_17 = 17,    /**< GPIO Pin 17 */
        HW_GPIO_PIN_18 = 18,    /**< GPIO Pin 18 */
        HW_GPIO_PIN_19 = 19,    /**< GPIO Pin 19 */
        HW_GPIO_PIN_20 = 20,    /**< GPIO Pin 20 */
        HW_GPIO_PIN_21 = 21,    /**< GPIO Pin 21 */
        HW_GPIO_PIN_22 = 22,    /**< GPIO Pin 22 */
        HW_GPIO_PIN_23 = 23,    /**< GPIO Pin 23 */
        HW_GPIO_PIN_24 = 24,    /**< GPIO Pin 24 */
        HW_GPIO_PIN_25 = 25,    /**< GPIO Pin 25 */
        HW_GPIO_PIN_26 = 26,    /**< GPIO Pin 26 */
        HW_GPIO_PIN_27 = 27,    /**< GPIO Pin 27 */
        HW_GPIO_PIN_28 = 28,    /**< GPIO Pin 28 */
        HW_GPIO_PIN_29 = 29,    /**< GPIO Pin 29 */
        HW_GPIO_PIN_30 = 30,    /**< GPIO Pin 30 */
        HW_GPIO_PIN_31 = 31,    /**< GPIO Pin 31 */
        HW_GPIO_PIN_MAX         /**< GPIO Pin max*/
} HW_GPIO_PIN;

/**
 * \brief GPIO function
 *
 */
typedef enum {
        HW_GPIO_FUNC_GPIO = 0,                  /**< GPIO */
        HW_GPIO_FUNC_UART_RX = 1,               /**< GPIO as UART RX */
        HW_GPIO_FUNC_UART_TX = 2,               /**< GPIO as UART TX */
        HW_GPIO_FUNC_UART2_RX = 3,              /**< GPIO as UART2 RX */
        HW_GPIO_FUNC_UART2_TX = 4,              /**< GPIO as UART2 TX */
        HW_GPIO_FUNC_UART2_CTSN = 5,            /**< GPIO as UART2 CTSN */
        HW_GPIO_FUNC_UART2_RTSN = 6,            /**< GPIO as UART2 RTSN */
        HW_GPIO_FUNC_UART3_RX = 7,              /**< GPIO as UART3 RX */
        HW_GPIO_FUNC_UART3_TX = 8,              /**< GPIO as UART3 TX */
        HW_GPIO_FUNC_UART3_CTSN = 9,            /**< GPIO as UART3 CTSN */
        HW_GPIO_FUNC_UART3_RTSN = 10,           /**< GPIO as UART3 RTSN */
        HW_GPIO_FUNC_ISO_CLK = 11,              /**< GPIO as ISO CLK */
        HW_GPIO_FUNC_ISO_DATA = 12,             /**< GPIO as ISO DATA */
        HW_GPIO_FUNC_SPI_DI = 13,               /**< GPIO as SPI DI */
        HW_GPIO_FUNC_SPI_DO = 14,               /**< GPIO as SPI DO */
        HW_GPIO_FUNC_SPI_CLK = 15,              /**< GPIO as SPI CLK */
        HW_GPIO_FUNC_SPI_EN = 16,               /**< GPIO as SPI EN */
        HW_GPIO_FUNC_SPI2_DI = 17,              /**< GPIO as SPI2 DI */
        HW_GPIO_FUNC_SPI2_DO = 18,              /**< GPIO as SPI2 DO */
        HW_GPIO_FUNC_SPI2_CLK = 19,             /**< GPIO as SPI2 CLK */
        HW_GPIO_FUNC_SPI2_EN = 20,              /**< GPIO as SPI2 EN */
        HW_GPIO_FUNC_I2C_SCL = 21,              /**< GPIO as I2C SCL */
        HW_GPIO_FUNC_I2C_SDA = 22,              /**< GPIO as I2C SDA */
        HW_GPIO_FUNC_I2C2_SCL = 23,             /**< GPIO as I2C2 SCL */
        HW_GPIO_FUNC_I2C2_SDA = 24,             /**< GPIO as I2C2 SDA */
        HW_GPIO_FUNC_USB_SOF = 25,              /**< GPIO as USB SOF */
        HW_GPIO_FUNC_ADC = 26,                  /**< GPIO as ADC (dedicated pin) */
        HW_GPIO_FUNC_USB = 27,                  /**< GPIO as USB */
        HW_GPIO_FUNC_PCM_DI = 28,               /**< GPIO as PCM DI */
        HW_GPIO_FUNC_PCM_DO = 29,               /**< GPIO as PCM DO */
        HW_GPIO_FUNC_PCM_FSC = 30,              /**< GPIO as PCM FSC */
        HW_GPIO_FUNC_PCM_CLK = 31,              /**< GPIO as PCM CLK */
        HW_GPIO_FUNC_PDM_DATA = 32,             /**< GPIO as PDM DATA */
        HW_GPIO_FUNC_PDM_CLK = 33,              /**< GPIO as PDM CLK */
        HW_GPIO_FUNC_COEX_EXT_ACT = 34,         /**< GPIO as COEX EXT ACT0 */
        HW_GPIO_FUNC_COEX_SMART_ACT = 35,       /**< GPIO as COEX SMART ACT */
        HW_GPIO_FUNC_COEX_SMART_PRI = 36,       /**< GPIO as COEX SMART PRI */
        HW_GPIO_FUNC_PORT0_DCF = 37,            /**< GPIO as PORT0 DCF */
        HW_GPIO_FUNC_PORT1_DCF = 38,            /**< GPIO as PORT1 DCF */
        HW_GPIO_FUNC_PORT2_DCF = 39,            /**< GPIO as PORT2 DCF */
        HW_GPIO_FUNC_PORT3_DCF = 40,            /**< GPIO as PORT3 DCF */
        HW_GPIO_FUNC_PORT4_DCF = 41,            /**< GPIO as PORT4 DCF */
        HW_GPIO_FUNC_CLOCK = 42,                /**< GPIO as CLOCK */
        HW_GPIO_FUNC_PG = 43,                   /**< GPIO as PG */
        HW_GPIO_FUNC_LCD = 44,                  /**< GPIO as LCD */
        HW_GPIO_FUNC_LCD_SPI_DC = 45,           /**< GPIO as LCD SPI DC */
        HW_GPIO_FUNC_LCD_SPI_DO = 46,           /**< GPIO as LCD SPI DO */
        HW_GPIO_FUNC_LCD_SPI_CLK = 47,          /**< GPIO as LCD SPI CLK */
        HW_GPIO_FUNC_LCD_SPI_EN = 48,           /**< GPIO as LCD SPI EN */
        HW_GPIO_FUNC_TIM_PWM = 49,              /**< GPIO as TIM PWM */
        HW_GPIO_FUNC_TIM2_PWM = 50,             /**< GPIO as TIM2 PWM */
        HW_GPIO_FUNC_TIM_1SHOT = 51,            /**< GPIO as TIM 1SHOT */
        HW_GPIO_FUNC_TIM2_1SHOT = 52,           /**< GPIO as TIM2 1SHOT */
        HW_GPIO_FUNC_TIM3_PWM = 53,             /**< GPIO as TIM3 PWM */
        HW_GPIO_FUNC_TIM4_PWM = 54,             /**< GPIO as TIM4 PWM */
        HW_GPIO_FUNC_AGC_EXT = 55,              /**< GPIO as AGC EXT */
        HW_GPIO_FUNC_CMAC_DIAG0 = 56,           /**< GPIO as CMAC DIAG0 */
        HW_GPIO_FUNC_CMAC_DIAG1 = 57,           /**< GPIO as CMAC DIAG1 */
        HW_GPIO_FUNC_CMAC_DIAG2 = 58,           /**< GPIO as CMAC DIAG2 */
        HW_GPIO_FUNC_CMAC_DIAGX = 59,           /**< GPIO as CMAC DIAGX */
        HW_GPIO_FUNC_LAST,
} HW_GPIO_FUNC;

/**
 * \brief GPIO pin configuration
 *
 * It's recommended to use \p HW_GPIO_PINCONFIG and \p HW_GPIO_PINCONFIG_RESERVE to set pin entries.
 * Each configuration must be terminated using \p HW_GPIO_PINCONFIG_END macro.
 *
 */
typedef __PACKED_STRUCT {
        uint8_t         pin;     /**< pin name, high-nibble is port number and low-nibble is pin */
        HW_GPIO_MODE    mode;    /**< pin mode */
        HW_GPIO_FUNC    func;    /**< pin function */
        bool            high;    /**< initial pin state, true for high and false for low */
        bool            reserve; /*<< true if pin should be also reserved */
} gpio_config;

/**
 * \brief GPIO pin configuration for \p gpio_config
 *
 * \p xport and \p xpin are specified as symbols from \p HW_GPIO_PORT and \p HW_GPIO_PIN enums
 * respectively or more conveniently as plain numeric values.
 * \p xmode and \p xfunc have the same values as defined in \p HW_GPIO_MODE and \p HW_GPIO_FUNC enums
 * respectively, except they have prefix stripped.
 *
 * \param [in] xport port number
 * \param [in] xpin pin number
 * \param [in] xmode pin mode
 * \param [in] xfunc pin function
 * \param [in] xhigh true for high state, false otherwise
 *
 */
#define HW_GPIO_PINCONFIG(xport, xpin, xmode, xfunc, xhigh) \
        {                                               \
                .pin = (xport << HW_GPIO_PIN_BITS) | (xpin & ((1 << HW_GPIO_PIN_BITS) - 1)),    \
                .mode = HW_GPIO_MODE_ ## xmode,         \
                .func = HW_GPIO_FUNC_ ## xfunc,         \
                .high = xhigh,                          \
                .reserve = false,                       \
        }

/**
 * \brief GPIO pin configuration and reservation for \p gpio_config
 *
 * This macro is virtually identical to \p HW_GPIO_PINCONFIG, except it also reserves pin.
 *
 * \param [in] xport port number
 * \param [in] xpin pin number
 * \param [in] xmode pin mode
 * \param [in] xfunc pin function
 * \param [in] xhigh true for high state, false otherwise
 *
 * \sa HW_GPIO_PINCONFIG
 *
 */
#define HW_GPIO_PINCONFIG_RESERVE(xport, xpin, xmode, xfunc, xhigh) \
        {                                               \
                .pin = (xport << HW_GPIO_PIN_BITS) | (xpin & ((1 << HW_GPIO_PIN_BITS) - 1)),    \
                .mode = HW_GPIO_MODE_ ## xmode,         \
                .func = HW_GPIO_FUNC_ ## xfunc,         \
                .high = xhigh,                          \
                .reserve = true,                        \
        }

/**
 * \brief Macro to properly terminate array of \p gpio_config definition
 *
 */
#define HW_GPIO_PINCONFIG_END \
        {                       \
                .pin = 0xFF,    \
        }

/**
 * \brief GPIO configuration
 *
 * This is a shortcut to configure multiple GPIOs in one call.
 * \p cfg is an array of GPIO pins configuration, it should be terminated by dummy element with
 * \p pin member set to 0xFF (macro \p HW_GPIO_PINCONFIG can be used for this purpose).
 *
 * \param [in] cfg GPIO pins configuration
 *
 * \sa hw_gpio_configure_pin
 * \sa hw_gpio_set_pin_function
 *
 */
void hw_gpio_configure(const gpio_config cfg[]);

/**
 * \brief Reserve GPIO pin
 *
 * Reserve pin for exclusive usage.
 * This macro can be used in application peripheral_setup function to detect
 * usage of same GPIO pin by different applications.
 *
 * \param [in] port GPIO port number
 * \param [in] pin GPIO pin number
 *
 * \return true if pin was successfully reserved and setup, false if pin was already reserved
 *
 */
bool hw_gpio_reserve_pin(HW_GPIO_PORT port, HW_GPIO_PIN pin);

/**
 * \brief Reserve GPIO pin and set pin function
 *
 * Reserve pin and set up it function. If pin was already reserved do nothing.
 *
 * \param [in] port GPIO port number
 * \param [in] pin GPIO pin number
 * \param [in] mode GPIO access mode
 * \param [in] function GPIO function
 * \param [in] high in case of PID_GPIO and OUTPUT value to set on pin
 *
 * \return true if pin was successfully reserved and setup, false if pin was already reserved
 *
 */
bool hw_gpio_reserve_and_configure_pin(HW_GPIO_PORT port, HW_GPIO_PIN pin, HW_GPIO_MODE mode,
                                                                HW_GPIO_FUNC function, bool high);

/**
 * \brief Unreserve GPIO pin
 *
 * Free reserved pin. If pin was not reserved do nothing.
 * Configuration of pin does not change just reservation.
 *
 * \note If pin was reserved using RESERVE_GPIO it will also be unreserved.
 * If RESERVE_GPIO was not enabled by compile time flags call to this function
 * may cause unexpected result.
 *
 * \param [in] port GPIO port number
 * \param [in] pin GPIO pin number
 *
 * \sa hw_gpio_reserve_and_configure_pin
 * \sa hw_gpio_reserve_pin
 * \sa RESERVE_GPIO
 *
 */
void hw_gpio_unreserve_pin(HW_GPIO_PORT port, HW_GPIO_PIN pin);

#if (DEBUG_GPIO_ALLOC_MONITOR_ENABLED == 1)
/**
 * \brief Reserve GPIO pin
 *
 * Reserve pin for exclusive usage. If pin is already allocated trigger breakpoint.
 * This macro should be used in application peripheral_setup function to detect
 * usage of same GPIO pin by different applications.
 *
 * If runtime GPIO reservation is needed, use hw_gpio_reserve_pin,
 * hw_gpio_reserve_and_configure_pin and hw_gpio_unreserve_pin instead.
 *
 * \param [in] name parameter ignored, used for debug only
 * \param [in] port GPIO port number
 * \param [in] pin GPIO pin number
 * \param [in] func parameter ignored (for compatibility)
 *
 * \sa hw_gpio_reserve_pin
 * \sa hw_gpio_reserve_and_configure_pin
 * \sa hw_gpio_unreserve_pin instead
 *
 */
#define RESERVE_GPIO(name, port, pin, func)                                                     \
        do {                                                                                    \
                if (!hw_gpio_reserve_pin((port), (pin))) {                                      \
                        /* If debugger stops at this line, there is configuration problem */    \
                        /* pin is used without being reserved first */                          \
                        __BKPT(0); /* this pin has not been previously reserved! */             \
                }                                                                               \
        } while (0)

#else

#define RESERVE_GPIO( name, port, pin, func )   \
        do {                                    \
                                                \
        } while (0)

#endif // (DEBUG_GPIO_ALLOC_MONITOR_ENABLED == 1)

/**
 * \brief Set the pin type and mode
 *
 * \param [in] port GPIO port
 * \param [in] pin GPIO pin
 * \param [in] mode GPIO pin mode
 * \param [in] function GPIO pin usage
 *
 */
void hw_gpio_set_pin_function(HW_GPIO_PORT port, HW_GPIO_PIN pin, HW_GPIO_MODE mode,
                                                                        HW_GPIO_FUNC function);

/**
 * \brief Get the pin type and mode
 *
 * \param [in] port GPIO port
 * \param [in] pin GPIO pin
 * \param [out] mode GPIO pin mode
 * \param [out] function GPIO pin usage
 *
 */
void hw_gpio_get_pin_function(HW_GPIO_PORT port, HW_GPIO_PIN pin, HW_GPIO_MODE* mode,
                                                                        HW_GPIO_FUNC* function);

/**
 * \brief Combined function to set the state and the type and mode of the GPIO pin
 *
 * \param [in] port GPIO port
 * \param [in] pin GPIO pin
 * \param [in] mode GPIO pin mode
 * \param [in] function GPIO pin usage
 * \param [in] high set to TRUE to set the pin into high else low
 *
 */
void hw_gpio_configure_pin(HW_GPIO_PORT port, HW_GPIO_PIN pin, HW_GPIO_MODE mode,
                                                        HW_GPIO_FUNC function, const bool high);

/**
 * \brief Configure power source for pin output
 *
 * \param [in] port GPIO port
 * \param [in] pin GPIO pin
 * \param [in] power GPIO power source
 *
 */
void hw_gpio_configure_pin_power(HW_GPIO_PORT port, HW_GPIO_PIN pin, HW_GPIO_POWER power);

/**
 * \brief Set a GPIO in high state
 *
 * The GPIO should have been previously configured as an output!
 *
 * \param [in] port GPIO port
 * \param [in] pin GPIO pin
 *
 */
void hw_gpio_set_active(HW_GPIO_PORT port, HW_GPIO_PIN pin);

/**
 * \brief Set a GPIO in low state
 *
 * The GPIO should have been previously configured as an output!
 *
 * \param [in] port GPIO port
 * \param [in] pin GPIO pin
 *
 */
void hw_gpio_set_inactive(HW_GPIO_PORT port, HW_GPIO_PIN pin);

/**
 * \brief Get the GPIO status
 *
 * The GPIO should have been previously configured as input!
 *
 * \param [in] port GPIO port
 * \param [in] pin GPIO pin
 *
 * \return true if the pin is high, false if low
 *
 */
bool hw_gpio_get_pin_status(HW_GPIO_PORT port, HW_GPIO_PIN pin);

/**
 * \brief Toggle GPIO pin state
 *
 * \param [in] port GPIO port
 * \param [in] pin GPIO pin
 *
 */
void hw_gpio_toggle(HW_GPIO_PORT port, HW_GPIO_PIN pin);

/**
 * \brief Find pins with specific function
 *
 * Function searches for pins configured for specific function.
 * If buf is not NULL and buf_size is greater than 0 pins are stored in buf
 * high-nibble is port number and low-nibble is pin.
 * If number of pins found is greater then buf_size only buf_size entries are filled, though
 * the returned number of found pins is correct.
 *
 * \param [in] func function to lookup
 * \param [out] buf buffer for port-pin pairs that are configured for specific function
 * \param [in] buf_size size of buf
 *
 * \return number of pins with specific function put in buf
 *         0 - no pin is configured for this function
 *
 */
int hw_gpio_get_pins_with_function(HW_GPIO_FUNC func, uint8_t *buf, int buf_size);

/**
 * \brief Disables the latches of all gpios.
 *
 */
__STATIC_FORCEINLINE void hw_gpio_pad_latch_disable_all(void)
{
        GLOBAL_INT_DISABLE();
        REG_SETF(CRG_TOP, P0_RESET_PAD_LATCH_REG, P0_RESET_LATCH_EN, CRG_TOP_P0_RESET_PAD_LATCH_REG_P0_RESET_LATCH_EN_Msk);
        REG_SETF(CRG_TOP, P1_RESET_PAD_LATCH_REG, P1_RESET_LATCH_EN, CRG_TOP_P1_RESET_PAD_LATCH_REG_P1_RESET_LATCH_EN_Msk);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Enables the latches of all gpios.
 *
 */
__STATIC_FORCEINLINE void hw_gpio_pad_latch_enable_all(void)
{
        GLOBAL_INT_DISABLE();
        REG_SETF(CRG_TOP, P0_SET_PAD_LATCH_REG, P0_SET_LATCH_EN, CRG_TOP_P0_SET_PAD_LATCH_REG_P0_SET_LATCH_EN_Msk);
        REG_SETF(CRG_TOP, P1_SET_PAD_LATCH_REG, P1_SET_LATCH_EN, CRG_TOP_P1_SET_PAD_LATCH_REG_P1_SET_LATCH_EN_Msk);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Enables the latch for the specific gpio.
 *
 * \param [in] port The GPIO port to latch
 * \param [in] pin The GPIO pin to latch
 */
__STATIC_FORCEINLINE void hw_gpio_pad_latch_enable(HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
        ASSERT_WARNING((port == HW_GPIO_PORT_0) || (port == HW_GPIO_PORT_1));

        if (port == HW_GPIO_PORT_0) {
                ASSERT_WARNING(pin < HW_GPIO_PORT_0_NUM_PINS);
                CRG_TOP->P0_SET_PAD_LATCH_REG = 1 << pin;
        } else if (port == HW_GPIO_PORT_1) {
                ASSERT_WARNING(pin < HW_GPIO_PORT_1_NUM_PINS);
                CRG_TOP->P1_SET_PAD_LATCH_REG = 1 << pin;
        }
}

/**
 * \brief Disables the latch for the specific gpio.
 *
 * \param [in] port The GPIO port to unlatch
 * \param [in] pin The GPIO pin to unlatch
 */
__STATIC_FORCEINLINE void hw_gpio_pad_latch_disable(HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
        ASSERT_WARNING((port == HW_GPIO_PORT_0) || (port == HW_GPIO_PORT_1));

        if (port == HW_GPIO_PORT_0) {
                ASSERT_WARNING(pin < HW_GPIO_PORT_0_NUM_PINS);
                CRG_TOP->P0_RESET_PAD_LATCH_REG = 1 << pin;
        } else if (port == HW_GPIO_PORT_1) {
                ASSERT_WARNING(pin < HW_GPIO_PORT_1_NUM_PINS);
                CRG_TOP->P1_RESET_PAD_LATCH_REG = 1 << pin;
        }
}

/**
 * \brief Checks if the specific GPIO is latched or not.
 *
 * \param [in] port The GPIO port to check latch status for
 * \param [in] pin The GPIO pin to check latch status for
 */
__STATIC_FORCEINLINE bool hw_gpio_pad_latch_is_enabled(HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
        ASSERT_WARNING((port == HW_GPIO_PORT_0) || (port == HW_GPIO_PORT_1));

        if (port == HW_GPIO_PORT_0) {
                ASSERT_WARNING(pin < HW_GPIO_PORT_0_NUM_PINS);
                return (CRG_TOP->P0_PAD_LATCH_REG & (1 << pin));
        } else if (port == HW_GPIO_PORT_1) {
                ASSERT_WARNING(pin < HW_GPIO_PORT_1_NUM_PINS);
                return (CRG_TOP->P1_PAD_LATCH_REG & (1 << pin));
        }

        return false;
}

/**
* \brief Find GPIO with specific function.
*
* \param [in]  func function to lookup
* \param [out] port The GPIO port of the gpio configured with the specific function
* \param [out] pin The GPIO pin of the gpio configured with the specific function
*
* \return true if the port / pin pair for the specific function was found, else false.
*/
bool hw_gpio_get_pin_with_function(HW_GPIO_FUNC func, HW_GPIO_PORT* port, HW_GPIO_PIN* pin);


#endif /* dg_configUSE_HW_GPIO */
#endif /* HW_GPIO_H_ */

/**
 * \}
 * \}
 */
