/**
 ****************************************************************************************
 *
 * @file gpio.c
 *
 * @brief GPIO CMD API implementation
 *
 * Copyright (C) 2016-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <hw_gpio.h>
#include <hw_pdc.h>
#include <hw_wkup.h>
#include "cli_utils.h"
#include "debug_utils.h"
#include "gpio.h"
#include "osal.h"

typedef struct {
        HW_GPIO_PORT port;
        HW_GPIO_PIN pin;
        HW_WKUP_PIN_STATE state;
        uint32_t counter;
        uint32_t threshold;
} gpio_wkup_entry_t;

__RETAINED_RW static volatile gpio_wkup_entry_t wkup_gpio = {
        HW_GPIO_PORT_MAX,
        HW_GPIO_PIN_MAX,
};

__RETAINED static uint32_t wkup_notif_mask;
__RETAINED static OS_TASK wkup_task;
__RETAINED_RW static uint32_t wkup_pdc_idx = HW_PDC_INVALID_LUT_INDEX;

static bool gpio_pin_init(const char *argv[], HW_GPIO_PORT *gpio_port, HW_GPIO_PIN *gpio_pin)
{
        if (*argv[2] != 'p' || strlen(argv[2]) < 3) {
                printf("pin = pXY or pXYY where X is number of port, Y/YY is number of pin\r\n");
                return false;
        }

        /* Convert port */
        *gpio_port = argv[2][1] - '0';

        /* Convert pin from ascii to int */
        *gpio_pin = atoi(argv[2] + 2);

        /* Range of number of ports */
        if (*gpio_port > (HW_GPIO_NUM_PORTS - 1)) {
                printf("Port numbers must be in range from 0 to %d\r\n", HW_GPIO_NUM_PORTS - 1);
                return false;
        }

        /* Range of pins in port */
        if (*gpio_pin  > (hw_gpio_port_num_pins[*gpio_port] - 1)) {
                printf("Pin numbers must be in range from 0 to %d\r\n",
                                                         hw_gpio_port_num_pins[*gpio_port] - 1);
                return false;
        }

        return true;
}

static bool gpio_configure(int argc, const char **argv)
{
        const char *func = "gpio";
        HW_GPIO_PORT gpio_port;
        HW_GPIO_PIN gpio_pin;
        HW_GPIO_MODE gpio_mode;
        HW_GPIO_FUNC gpio_function;

        if (argc < 4) {
                return false;
        }

        if (!gpio_pin_init(argv, &gpio_port, &gpio_pin)) {
                return false;
        }

        if (!strcmp(argv[3], "input")) {
                gpio_mode = HW_GPIO_MODE_INPUT;
        } else if (!strcmp(argv[3], "input_pu")) {
                gpio_mode = HW_GPIO_MODE_INPUT_PULLUP;
        } else if (!strcmp(argv[3], "input_pd")) {
                gpio_mode = HW_GPIO_MODE_INPUT_PULLDOWN;
        } else if (!strcmp(argv[3], "output")) {
                gpio_mode = HW_GPIO_MODE_OUTPUT;
        } else if (!strcmp(argv[3], "output_pp")) {
                gpio_mode = HW_GPIO_MODE_OUTPUT_PUSH_PULL;
        } else if (!strcmp(argv[3], "output_od")) {
                gpio_mode = HW_GPIO_MODE_OUTPUT_OPEN_DRAIN;
        } else {
                printf("mode = input | input_pu | input_pd | output | output_pp | output_od\r\n");
                return false;
        }

        /* if user does not type a function parameter then gpio is default */
        if (argc > 4) {
                if (!strcmp(argv[4], "gpio")) {
                        gpio_function = HW_GPIO_FUNC_GPIO;
                } else if (!strcmp(argv[4], "tim_pwm")) {
                        gpio_function = HW_GPIO_FUNC_TIM_PWM;
                } else if (!strcmp(argv[4], "tim2_pwm")) {
                        gpio_function = HW_GPIO_FUNC_TIM2_PWM;
                } else if (!strcmp(argv[4], "tim3_pwm")) {
                        gpio_function = HW_GPIO_FUNC_TIM3_PWM;
                } else if (!strcmp(argv[4], "tim4_pwm")) {
                        gpio_function = HW_GPIO_FUNC_TIM4_PWM;
                } else {
                        printf("function = gpio | tim_pwm | tim2_pwm | tim3_pwm | tim4_pwm\r\n");
                        return false;
                }
                func = argv[4];
        } else {
                gpio_function = HW_GPIO_FUNC_GPIO;
        }

        hw_gpio_configure_pin(gpio_port, gpio_pin, gpio_mode, gpio_function, false);

        printf("Port %d Pin %d Mode %s Function %s\r\n", gpio_port, gpio_pin, argv[3], func);

        return true;
}

static bool gpio_set(int argc, const char **argv)
{
        HW_GPIO_PORT gpio_port;
        HW_GPIO_PIN gpio_pin;

        if (argc < 4) {
                return false;
        }

        if (!gpio_pin_init(argv, &gpio_port, &gpio_pin)) {
                return false;
        }

        if (!strcmp(argv[3], "1")) {
                hw_gpio_set_active(gpio_port, gpio_pin);
                hw_gpio_pad_latch_enable(gpio_port, gpio_pin);
                hw_gpio_pad_latch_disable(gpio_port, gpio_pin);
        } else if (!strcmp(argv[3], "0")) {
                hw_gpio_set_inactive(gpio_port, gpio_pin);
                hw_gpio_pad_latch_enable(gpio_port, gpio_pin);
                hw_gpio_pad_latch_disable(gpio_port, gpio_pin);
        } else {
                printf("state = 0 | 1\r\n");
                return false;
        }

        printf("Set Port %d Pin %d State %s \r\n", gpio_port, gpio_pin, argv[3]);

        return true;
}

static bool gpio_get(int argc, const char **argv)
{
        HW_GPIO_PORT gpio_port;
        HW_GPIO_PIN gpio_pin;

        if (argc < 3) {
                return false;
        }

        if (!gpio_pin_init(argv, &gpio_port, &gpio_pin)) {
                return false;
        }

        printf("Port %d Pin %d State %d \r\n", gpio_port, gpio_pin,
                                                      hw_gpio_get_pin_status(gpio_port, gpio_pin));

        return true;
}

static bool gpio_toggle(int argc, const char **argv)
{
        HW_GPIO_PORT gpio_port;
        HW_GPIO_PIN gpio_pin;

        if (argc < 3) {
                return false;
        }

        if (!gpio_pin_init(argv, &gpio_port, &gpio_pin)) {
                return false;
        }

        hw_gpio_toggle(gpio_port, gpio_pin);
        hw_gpio_pad_latch_enable(gpio_port, gpio_pin);
        hw_gpio_pad_latch_disable(gpio_port, gpio_pin);

        printf("Port %d Pin %d Toggle\r\n", gpio_port, gpio_pin);

        return true;
}

static bool gpio_power(int argc, const char **argv)
{
        HW_GPIO_PORT gpio_port;
        HW_GPIO_PIN gpio_pin;
        HW_GPIO_POWER gpio_power;
        const char *power_str;

        if (argc < 4) {
                return false;
        }

        if (!gpio_pin_init(argv, &gpio_port, &gpio_pin)) {
                return false;
        }

        if (!strcmp(argv[3], "v33")) {
                gpio_power = HW_GPIO_POWER_V33;
                power_str = "3.3V";
        } else if (!strcmp(argv[3], "vdd1v8p")) {
                gpio_power = HW_GPIO_POWER_VDD1V8P;
                power_str = "1.8V";
        } else {
                printf("power = v33 | vdd1v8p\r\n");
                return false;
        }

        hw_gpio_configure_pin_power(gpio_port, gpio_pin, gpio_power);

        printf("Port %d Pin %d Power %s\r\n", gpio_port, gpio_pin, power_str);

        return true;
}

static bool gpio_latch(int argc, const char **argv)
{
        HW_GPIO_PORT gpio_port;
        HW_GPIO_PIN gpio_pin;

        if (argc < 4) {
                return false;
        }

        if (!gpio_pin_init(argv, &gpio_port, &gpio_pin)) {
                return false;
        }

        if (!strcmp(argv[3], "enable")) {
                hw_gpio_pad_latch_enable(gpio_port, gpio_pin);
        } else if (!strcmp(argv[3], "disable")) {
                hw_gpio_pad_latch_disable(gpio_port, gpio_pin);
        } else {
                printf("state = enable | disable\r\n");
                return false;
        }

        printf("Port %d Pin %d Latch %s \r\n", gpio_port, gpio_pin, argv[3]);

        return true;
}

void gpio_wkup_cmd_handler()
{
        if (wkup_gpio.port == HW_GPIO_PORT_MAX || wkup_gpio.pin == HW_GPIO_PIN_MAX) {
                /*
                 * This handler is called at the last handler - all other notifications should be
                 * handled already (e.g. CTS).
                 */
                hw_wkup_clear_status(HW_GPIO_PORT_0, (1 << hw_gpio_port_num_pins[HW_GPIO_PORT_0]) - 1);
                hw_wkup_clear_status(HW_GPIO_PORT_1, (1 << hw_gpio_port_num_pins[HW_GPIO_PORT_1]) - 1);
                return;
        }

        if (hw_wkup_get_status(wkup_gpio.port) & (1 << wkup_gpio.pin)) {
                hw_wkup_clear_status(wkup_gpio.port, (1 << wkup_gpio.pin));

                if (wkup_task) {
                        OS_TASK_NOTIFY_FROM_ISR(wkup_task, wkup_notif_mask, OS_NOTIFY_SET_BITS);
                }
        }
}

static bool wkup_disable()
{
        if (wkup_gpio.port == HW_GPIO_PORT_MAX && wkup_gpio.pin == HW_GPIO_PIN_MAX) {
                return false;
        }

        hw_wkup_configure_pin(wkup_gpio.port, wkup_gpio.pin, false, HW_WKUP_PIN_STATE_HIGH);
        hw_wkup_clear_status(wkup_gpio.port, (1 << wkup_gpio.pin));
        wkup_gpio.counter = 0;
        wkup_gpio.threshold = 0;
        wkup_gpio.port = HW_GPIO_PORT_MAX;
        wkup_gpio.pin = HW_GPIO_PIN_MAX;

        if (wkup_pdc_idx != HW_PDC_INVALID_LUT_INDEX) {
                hw_pdc_remove_entry(wkup_pdc_idx);
                wkup_pdc_idx = HW_PDC_INVALID_LUT_INDEX;
        }

        return true;
}

static bool gpio_wkup_enable(int argc, const char **argv)
{
        HW_PDC_TRIG_SELECT pdc_trig_sel;
        HW_GPIO_MODE mode;
        HW_GPIO_FUNC function;
        gpio_wkup_entry_t entry = { };

        if (argc < 5) {
                return false;
        }

        if (!gpio_pin_init(argv, &entry.port, &entry.pin)) {
                return false;
        }

        if (!strcmp(argv[3], "low")) {
                entry.state = HW_WKUP_PIN_STATE_LOW;
        } else if (!strcmp(argv[3], "high")) {
                entry.state = HW_WKUP_PIN_STATE_HIGH;
        } else {
                printf("state = low | high\r\n");
                return false;
        }

        if (!parse_u32(argv[4], &entry.threshold)) {
                printf("threshold = u32\r\n");
                return false;
        }

        hw_gpio_get_pin_function(entry.port, entry.pin, &mode, &function);

        if (function != HW_GPIO_FUNC_GPIO) {
                printf("Invalid GPIO function Port %d Pin %d\r\n", entry.port, entry.pin);
                return true;
        }

        if (mode != HW_GPIO_MODE_INPUT && mode != HW_GPIO_MODE_INPUT_PULLDOWN &&
                                                                mode != HW_GPIO_MODE_INPUT_PULLUP) {
                printf("Invalid GPIO mode Port %d Pin %d\r\n", entry.port, entry.pin);
                return true;
        }

        /* Disable previous Wkup GPIO source */
        wkup_disable();

        /* Configure new Wkup GPIO source */
        wkup_gpio = entry;
        /* Enable wake up from non debounced GPIO */
        hw_wkup_gpio_configure_pin(entry.port, entry.pin, true, entry.state);

        /* Port 0 callback has been already registered for CTS */
        if (entry.port == HW_GPIO_PORT_1) {
                hw_wkup_register_gpio_p1_interrupt(gpio_wkup_cmd_handler, 1);
        }

        /* Setup PDC entry to wake up from specified pin */
        pdc_trig_sel = (entry.port == HW_GPIO_PORT_1) ? HW_PDC_TRIG_SELECT_P1_GPIO :
                                                                        HW_PDC_TRIG_SELECT_P0_GPIO;
        wkup_pdc_idx = hw_pdc_add_entry(HW_PDC_LUT_ENTRY_VAL(pdc_trig_sel, entry.pin,
                                                HW_PDC_MASTER_CM33, HW_PDC_LUT_ENTRY_EN_XTAL));
        hw_pdc_set_pending(wkup_pdc_idx);
        hw_pdc_acknowledge(wkup_pdc_idx);

        printf("Port %d Pin %d Wkup enable State %s Threshold %lu \r\n", entry.port, entry.pin,
                                                                        argv[3], entry.threshold);
        return true;
}

static bool gpio_wkup_disable(int argc, const char **argv)
{
        HW_GPIO_PORT gpio_port = wkup_gpio.port;
        HW_GPIO_PIN gpio_pin = wkup_gpio.pin;

        if (wkup_disable()) {
                printf("Port %d Pin %d Wkup disable\r\n", gpio_port, gpio_pin);
        } else {
                printf("Wkup not disable\r\n");
        }

        return true;
}

static const debug_handler_t gpio_handlers[] = {
        { "configure", "<pin> <mode> [<function>]", gpio_configure },
        { "set", "<pin> <state>", gpio_set },
        { "get", "<pin>", gpio_get },
        { "toggle", "<pin>", gpio_toggle },
        { "power", "<pin> <power>", gpio_power },
        { "latch", "<pin> <state>", gpio_latch },
        { "wkup_enable", "<pin> <state> <threshold>", gpio_wkup_enable },
        { "wkup_disable", "", gpio_wkup_disable },
        { NULL },
};

void gpio_command(int argc, const char *argv[], void *user_data)
{
        debug_handle_message(argc, argv, gpio_handlers);
}

void gpio_wkup_init(uint32_t notif_mask)
{
        if (wkup_task) {
                /* Already done */
                return;
        }

        wkup_task = OS_GET_CURRENT_TASK();
        wkup_notif_mask = notif_mask;
}

void gpio_wkup_handle_notified()
{
        if (wkup_task != OS_GET_CURRENT_TASK()) {
                /* Wasn't initialized or called from different task */
                return;
        }

        ++wkup_gpio.counter;

        if ((wkup_gpio.threshold) > 0 && (wkup_gpio.counter >= wkup_gpio.threshold)) {
                printf("Port %d Pin %d Wkup event Event counter %lu\r\n", wkup_gpio.port, wkup_gpio.pin, wkup_gpio.counter);

                wkup_gpio.counter = 0;
        }
}
