/**
 ****************************************************************************************
 *
 * @file pwm.c
 *
 * @brief Hardware PWM CMD implementation
 *
 * Copyright (C) 2019-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "hw_timer.h"
#include "cli_utils.h"
#include "debug_utils.h"
#include "pwm.h"

static HW_TIMER_ID pwm_name_to_timer_id(const char *pwm_name)
{
        if (!strcmp(pwm_name, "tim_pwm")) {
                return HW_TIMER;
        } else if (!strcmp(pwm_name, "tim2_pwm")) {
                return HW_TIMER2;
        } else if (!strcmp(pwm_name, "tim3_pwm")) {
                return HW_TIMER3;
        } else if (!strcmp(pwm_name, "tim4_pwm")) {
                return HW_TIMER4;
        }

        return NULL;
}

static bool pwm_configure_da1469x(int argc, const char **argv)
{
        HW_TIMER_ID id;
        HW_TIMER_CLK_SRC clk;
        uint8_t prescaler = 0;
        uint32_t frequency = 0;
        uint32_t duty_cycle = 0;

        if (argc < 7) {
                return false;
        }

        id = pwm_name_to_timer_id(argv[2]);
        if (id == NULL) {
                printf("pwm = tim_pwm | tim2_pwm | tim3_pwm | tim4_pwm\r\n");
                return false;
        }

        if (!strcmp(argv[3], "slow")) {
                clk = HW_TIMER_CLK_SRC_INT;
        } else if (!strcmp(argv[3], "fast")) {
                clk = HW_TIMER_CLK_SRC_EXT;
        } else {
                printf("clk = slow | fast\r\n");
                return false;
        }

        if (!parse_u8(argv[4], &prescaler) || prescaler > TIMER_MAX_PRESCALER_VAL) {
                printf("div = u8 <= 0x1F\r\n");
                return false;
        }

        if (!parse_u32(argv[5], &frequency) || frequency > TIMER_MAX_PWM_FREQ_VAL) {
                printf("cfg_val_1 = u32 <= 0xFFFF\r\n");
                return false;
        }

        if (!parse_u32(argv[6], &duty_cycle) || duty_cycle > TIMER_MAX_PWM_DC_VAL) {
                printf("cfg_val_2 = u32 <= 0xFFFF\r\n");
                return false;
        }

        hw_timer_init(id, NULL);
        /* Internal (LP clock), external (DIVN) */
        hw_timer_set_clk(id, clk);
        /* timer_freq = freq_clock / (value + 1) */
        hw_timer_set_prescaler(id, prescaler);
        /* Enable timer's clock */
        hw_timer_enable_clk(id);
        /* Timer mode */
        hw_timer_set_mode(id, HW_TIMER_MODE_TIMER);
        hw_timer_set_direction(id, HW_TIMER_DIR_UP);
        /* pwm_freq = timer_freq / (value + 1) */
        hw_timer_set_pwm_freq(id, frequency);
        /* pwm_dc = value / (pwm_freq + 1) */
        hw_timer_set_pwm_duty_cycle(id, duty_cycle);

        printf("PWM %s configured: Clock %s Prescaler %u Frequency %lu Duty Cycle %lu\r\n",
                                                argv[2], argv[3], prescaler, frequency, duty_cycle);

        return true;
}

static bool pwm_configure(int argc, const char **argv)
{
        if (argc < 7) {
                return false;
        }

        /*
         * argv[2] (pwm):       PWM ID: tim_pwm, tim2_pwm, tim3_pwm, tim4_pwm
         * argv[3] (clk):       Clock:
         *                              - slow = HW_TIMER_CLK_SRC_INT
         *                              - fast = HW_TIMER_CLK_SRC_EXT
         * argv[4] (div):       Prescaler: <= 0x1F, the timer frequency (timer_freq) is
         *                                          clock_freq / (div + 1)
         * argv[5] (cfg_val_1): Frequency: <= 0xFFFF, the PWM frequency (pwm_freq) is
         *                                            timer_freq / (cfg_val_1 + 1)
         * argv[6] (cfg_val_2): Duty Cycle: <= 0xFFFF, the duty cycle is cfg_val_2 / (pwm_freq + 1)
         */
        return pwm_configure_da1469x(argc, argv);
}

static bool pwm_enable_disable(const char *pwm_name, bool enable)
{
        HW_TIMER_ID timer_id;

        /* Each timer handles separate PWM */
        timer_id = pwm_name_to_timer_id(pwm_name);

        if (timer_id == NULL) {
                printf("pwm = tim_pwm | tim2_pwm | tim3_pwm | tim4_pwm\r\n");
                return false;
        }

        if (enable) {
                /* hw_timer_disable also disables the clock, so it must be enabled here for sure */
                hw_timer_enable_clk(timer_id);
                hw_timer_enable(timer_id);
        } else {
                hw_timer_disable(timer_id);
        }

        printf("PWM %s timer %s\r\n", pwm_name, (enable ? "enabled" : "disabled"));

        return true;
}

static bool pwm_enable(int argc, const char **argv)
{
        if (argc < 3) {
                return false;
        }

        return pwm_enable_disable(argv[2], true);
}

static bool pwm_disable(int argc, const char **argv)
{
        if (argc < 3) {
                return false;
        }

        return pwm_enable_disable(argv[2], false);
}

static const debug_handler_t pwm_handlers[] = {
        { "configure", "<pwm> <clk> <div> <cfg_val_1> <cfg_val_2>", pwm_configure },
        { "enable", "<pwm>", pwm_enable},
        { "disable", "<pwm>", pwm_disable},
        { NULL },
};

void pwm_command(int argc, const char *argv[], void *user_data)
{
        debug_handle_message(argc, argv, pwm_handlers);
}
