/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup GPIO
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file hw_gpio.c
 *
 * @brief Implementation of the GPIO Low Level Driver.
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#if dg_configUSE_HW_GPIO


#include <stdint.h>
#include <hw_gpio.h>

/* Register adresses */
#define PX_DATA_REG_ADDR(_port)         ((volatile uint32_t *)(GPIO_BASE + offsetof(GPIO_Type, P0_DATA_REG)) + _port)
#define PX_DATA_REG(_port)              *PX_DATA_REG_ADDR(_port)
#define PX_SET_DATA_REG_ADDR(_port)     ((volatile uint32_t *)(GPIO_BASE + offsetof(GPIO_Type, P0_SET_DATA_REG)) + _port)
#define PX_SET_DATA_REG(_port)          *PX_SET_DATA_REG_ADDR(_port)
#define PX_RESET_DATA_REG_ADDR(_port)   ((volatile uint32_t *)(GPIO_BASE + offsetof(GPIO_Type, P0_RESET_DATA_REG)) + _port)
#define PX_RESET_DATA_REG(_port)        *PX_RESET_DATA_REG_ADDR(_port)
#define PXX_MODE_REG_ADDR(_port, _pin)  ((volatile uint32_t *)(GPIO_BASE + offsetof(GPIO_Type, P0_00_MODE_REG)) + (_port * 32)  + _pin)
#define PXX_MODE_REG(_port, _pin)       *PXX_MODE_REG_ADDR(_port, _pin)
#define PX_PADPWR_CTRL_REG_ADDR(_port)  ((volatile uint32_t *)(GPIO_BASE + offsetof(GPIO_Type, P0_PADPWR_CTRL_REG)) + _port)
#define PX_PADPWR_CTRL_REG(_port)       *PX_PADPWR_CTRL_REG_ADDR(_port)



#if (dg_configIMAGE_SETUP == PRODUCTION_MODE) && (DEBUG_GPIO_ALLOC_MONITOR_ENABLED == 1)
        #error "GPIO assignment monitoring is active in PRODUCTION build!"
#endif

static volatile uint32_t GPIO_status[HW_GPIO_NUM_PORTS] = { 0 };

const uint8_t hw_gpio_port_num_pins[HW_GPIO_NUM_PORTS] = {
                                                HW_GPIO_PORT_0_NUM_PINS, HW_GPIO_PORT_1_NUM_PINS};

#if dg_configUSE_STATIC_IO_CONFIG
/* Static GPIO power configuration per port */
__RETAINED_RW uint32_t io_static_power_configuration[HW_GPIO_NUM_PORTS] = { 0 };
#endif /* dg_configUSE_STATIC_IO_CONFIG */


/*
 * Global Functions
 ****************************************************************************************
 */

void hw_gpio_configure(const gpio_config cfg[])
{
#if dg_configIMAGE_SETUP == DEVELOPMENT_MODE
        int num_pins = 0;
        uint32_t set_mask[HW_GPIO_NUM_PORTS] = { };
#endif

        if (!cfg) {
                return;
        }

        while (cfg->pin != 0xFF) {
                uint8_t port = cfg->pin >> HW_GPIO_PIN_BITS;
                uint8_t pin = cfg->pin & ((1 << HW_GPIO_PIN_BITS) - 1);

#if dg_configIMAGE_SETUP == DEVELOPMENT_MODE
                if (port >= HW_GPIO_NUM_PORTS || pin >= hw_gpio_port_num_pins[port]) {
                        /*
                         * invalid port or pin number specified, it was either incorrectly specified
                         * of cfg was not terminated properly using HW_GPIO_PINCONFIG_END so we're
                         * reading garbage
                         */
                        __BKPT(0);
                }

                if (++num_pins > HW_GPIO_NUM_PINS) {
                        /*
                         * trying to set more pins than available, perhaps cfg was not terminated
                         * properly using HW_GPIO_PINCONFIG_END?
                         */
                        __BKPT(0);
                }

                if (set_mask[port] & (1 << pin)) {
                        /*
                         * trying to set pin which has been already set by this call which means
                         * there is duplicated pin in configuration - does not make sense
                         */
                        __BKPT(0);
                }

                set_mask[port] |= (1 << pin);
#endif

                if (cfg->reserve) {
                        hw_gpio_reserve_and_configure_pin(port, pin, cfg->mode, cfg->func, cfg->high);
                } else {
                        hw_gpio_configure_pin(port, pin, cfg->mode, cfg->func, cfg->high);
                }
                cfg++;
        }
}

bool hw_gpio_reserve_pin(HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
        if ((GPIO_status[port] & (1 << pin))) {
                return false;
        }

        GPIO_status[port] |= (1 << pin);

        return true;
}

bool hw_gpio_reserve_and_configure_pin(HW_GPIO_PORT port, HW_GPIO_PIN pin, HW_GPIO_MODE mode,
                                                                HW_GPIO_FUNC function, bool high)
{
        if (!hw_gpio_reserve_pin(port, pin)) {
                return false;
        }

        hw_gpio_configure_pin(port, pin, mode, function, high);

        return true;
}

void hw_gpio_unreserve_pin(HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
        GPIO_status[port] &= ~(1 << pin);
}

static void hw_gpio_verify_reserved(HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
#if (DEBUG_GPIO_ALLOC_MONITOR_ENABLED == 1)
        if (!(GPIO_status[port] & (1 << pin))) {
                // If debugger stops at this line, there is configuration problem
                // pin is used without being reserved first
                __BKPT(0); /* this pin has not been previously reserved! */
        }
#endif // (DEBUG_GPIO_ALLOC_MONITOR_ENABLED == 1)
}


void hw_gpio_set_pin_function(HW_GPIO_PORT port, HW_GPIO_PIN pin, HW_GPIO_MODE mode,
                                                                        HW_GPIO_FUNC function)
{
        hw_gpio_verify_reserved(port, pin);

        PXX_MODE_REG(port, pin) = mode | function;
}

void hw_gpio_get_pin_function(HW_GPIO_PORT port, HW_GPIO_PIN pin, HW_GPIO_MODE* mode,
                                                                        HW_GPIO_FUNC* function)
{
        uint16_t val;

        hw_gpio_verify_reserved(port, pin);

        val = PXX_MODE_REG(port, pin);
        *mode = val & 0x0700;
        *function = val & 0x00ff;
}

void hw_gpio_configure_pin(HW_GPIO_PORT port, HW_GPIO_PIN pin, HW_GPIO_MODE mode,
                                                        HW_GPIO_FUNC function, const bool high)
{
        hw_gpio_verify_reserved(port, pin);

        if (high)
                hw_gpio_set_active(port, pin);
        else
                hw_gpio_set_inactive(port, pin);

        hw_gpio_set_pin_function(port, pin, mode, function);
}

void hw_gpio_configure_pin_power(HW_GPIO_PORT port, HW_GPIO_PIN pin, HW_GPIO_POWER power)
{
        uint32_t padpwr;

        GLOBAL_INT_DISABLE();
#if dg_configUSE_STATIC_IO_CONFIG
        padpwr = io_static_power_configuration[port];
#else
        padpwr = PX_PADPWR_CTRL_REG(port);
#endif /* dg_configUSE_STATIC_IO_CONFIG */
        padpwr &= ~(1 << pin);
        if (power == HW_GPIO_POWER_VDD1V8P) {
                padpwr |= (1 << pin);
        }
#if dg_configUSE_STATIC_IO_CONFIG
        io_static_power_configuration[port] = padpwr;
#endif /* dg_configUSE_STATIC_IO_CONFIG */
        PX_PADPWR_CTRL_REG(port) = padpwr;
        GLOBAL_INT_RESTORE();
}

void hw_gpio_set_active(HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
        hw_gpio_verify_reserved(port, pin);

        PX_SET_DATA_REG(port) = 1 << pin;
}

void hw_gpio_set_inactive(HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
        hw_gpio_verify_reserved(port, pin);

        PX_RESET_DATA_REG(port) = 1 << pin;
}

bool hw_gpio_get_pin_status(HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
        hw_gpio_verify_reserved(port, pin);

        return ( (PX_DATA_REG(port) & (1 << pin)) != 0 );
}

void hw_gpio_toggle(HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
        hw_gpio_verify_reserved(port, pin);

        if (hw_gpio_get_pin_status(port, pin))
                hw_gpio_set_inactive(port, pin);
        else
                hw_gpio_set_active(port, pin);
}

int hw_gpio_get_pins_with_function(HW_GPIO_FUNC func, uint8_t *buf, int buf_size)
{
        int count = 0;
        int port;
        int pin;
        HW_GPIO_MODE mode;
        HW_GPIO_FUNC pin_func;

        for (port = 0; port < HW_GPIO_NUM_PORTS; ++port) {
                for (pin = 0; pin < hw_gpio_port_num_pins[port]; ++pin) {
                        hw_gpio_get_pin_function(port, pin, &mode, &pin_func);
                        if (pin_func != func) {
                                continue;
                        }
                        if (count < buf_size && buf != NULL) {
                                buf[count] = (uint8_t) ((port << HW_GPIO_PIN_BITS) | pin);
                        }
                        count++;
                }
        }
        return count;
}

#endif /* dg_configUSE_HW_GPIO */
/**
 * \}
 * \}
 * \}
 */
