/**
 ****************************************************************************************
 *
 * @file plt_fw.c
 *
 * @brief PLT Firmware core code.
 *
 * Copyright (C) 2015-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#include <string.h>
#include <stdio.h>

#include "osal.h"
#include "hw_gpio.h"

#include "ble_mgr.h"
#include "ble_common.h"
#include "co_version.h"

#include "packers.h"
#include "plt_fw.h"
#include "Xtal_TRIM.h"

#include "hw_timer.h"
#include "hw_rtc.h"

#include "hw_led.h"
#include "hw_gpio.h"
#include "hw_spi.h"
#include "hw_i2c.h"
#include "hw_otpc.h"
#include "hw_uart.h"
#include "sys_power_mgr.h"

#include "dgtl.h"
#include "dgtl_msg.h"
#include "dgtl_pkt.h"
#include "platform_devices.h"

#if (dg_configUSE_HW_GPADC == 1)
#include "hw_gpadc.h"
#include "resmgmt.h"
#endif

#define MAX_TRIM 2047
#define MIN_TRIM 0

/* Minimum and maximum number of words in read/write OTP command */
#define MIN_RW_OTP_WORDS      (1)
#define MAX_RW_OTP_WORDS      (60)

#define mainBIT_HCI_RX (1 << 0)

/* PLT FW statuses uses in custom response fields */
#define PLT_FW_STATUS_OK        (0x00)
#define PLT_FW_STATUS_ERROR     (0xFF)

/* LED shouldn't be used */
#define PWM_INVALID_LED_ID      (0x00)

/* PLT FW XTAL auto-calibrations errors */
#define PLT_FW_AUTO_TRIM_INVALID_GPIO   (0x10)
#define PLT_FW_AUTO_TRIM_INVALID_XTAL   (0x11)

/* PLT FW XTAL16M/XTAL32M output pin */
#define PLT_FW_XTAL_OUT_PORT            (HW_GPIO_PORT_1)
#define PLT_FW_XTAL_OUT_PIN             (HW_GPIO_PIN_2)

/*
 * Valid port and pin values will be set in GPIO watchdog function. Port max and pin max aren't
 * a valid value - they won't be used as GPIO output without later initialization.
 */
__RETAINED_RW static HW_GPIO_PORT gpio_wd_port = HW_GPIO_PORT_MAX;
__RETAINED_RW static HW_GPIO_PIN gpio_wd_pin = HW_GPIO_PIN_MAX;
__RETAINED_RW static uint32_t gpio_wd_timer_cnt = 0;

#define PLT_CMD_READ_FROM_REG32(a)      ( *( (uint32_t *) a) )
#define PLT_CMD_READ_FROM_REG16(a)      ( *( (uint16_t *) a) )
#define PLT_CMD_WRITE_TO_REG32(a,b)     ( *( (uint32_t *) a) = (uint32_t) b )
#define PLT_CMD_WRITE_TO_REG16(a,b)     ( *( (uint16_t *) a) = (uint16_t) b )

/* Convert GPIO pad (1 byte) to GPIO port/pin */
#define GPIO_PAD_TO_PORT(pad)   (((pad) & 0xE0) >> 5)
#define GPIO_PAD_TO_PIN(pad)    ((pad) & 0x1F)

typedef void (*plt_cmd_handler)(dgtl_msg_t *msg);

static void plt_sleep_cb(void *param);

void hci_cmd_sleep(dgtl_msg_t *msg);
void xtal_trim(dgtl_msg_t *msg);
void hci_cmd_otp_read(dgtl_msg_t *msg);
void hci_cmd_otp_write(dgtl_msg_t *msg);
void hci_cmd_rw_reg(dgtl_msg_t *msg);
void fw_version_get(dgtl_msg_t *msg);
void hci_custom_action(dgtl_msg_t *msg);
#if  (dg_configUSE_HW_GPADC == 1)
void hci_read_adc(dgtl_msg_t *msg);
#endif /*  (dg_configUSE_HW_GPADC == 1) */
void hci_sensor_test(dgtl_msg_t *msg);
void hci_gpio_set(dgtl_msg_t *msg);
void hci_gpio_read(dgtl_msg_t *msg);
void hci_uart_loop(dgtl_msg_t *msg);
void hci_uart_baud(dgtl_msg_t *msg);
void hci_gpio_wd(dgtl_msg_t *msg);
void hci_ext32khz_test(dgtl_msg_t *msg);

plt_cmd_handler plt_cmd_handlers[] = {
                NULL,
                hci_cmd_sleep,          /* 0xFE01 */
                xtal_trim,
                NULL,
                hci_cmd_otp_read,       /* 0xFE04 */
                hci_cmd_otp_write,      /* 0xFE05 */
                hci_cmd_rw_reg,         /* 0xFE06 */
                NULL,
                fw_version_get,
                NULL,
                hci_custom_action,      /* 0xFE0A */
#if  (dg_configUSE_HW_GPADC == 1)
                hci_read_adc,           /* 0xFE0B */
#else
                NULL,
#endif /*  (dg_configUSE_HW_GPADC == 1) */
                hci_sensor_test,        /* 0xFE0C */
                hci_gpio_set,           /* 0xFE0D */
                hci_gpio_read,          /* 0xFE0E */
                hci_uart_loop,          /* 0xFE0F */
                hci_uart_baud,          /* 0xFE10 */
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                hci_ext32khz_test,      /* 0xFE16 */
                hci_gpio_wd,            /* 0xFE17 */
                NULL,
};

__RETAINED static OS_TASK current_task;
__RETAINED static OS_QUEUE cmd_queue;
__RETAINED static OS_TIMER sleep_timer;
__RETAINED static sleep_mode_t prev_sleep_mode;

static bool plt_check_gpio(HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
        if (port < HW_GPIO_PORT_0 || port >= HW_GPIO_PORT_MAX) {
                return false;
        }

        if (pin < HW_GPIO_PIN_0 || pin >= hw_gpio_port_num_pins[port]) {
                return false;
        }

        return true;
}

void dgtl_app_specific_hci_cb(const dgtl_msg_t *msg)
{
        OS_QUEUE_PUT(cmd_queue, &msg, OS_QUEUE_FOREVER);
        OS_TASK_NOTIFY(current_task, mainBIT_HCI_RX, OS_NOTIFY_SET_BITS);
}

static void timer_gpio_wd_cb(void)
{
        if (gpio_wd_timer_cnt == 0) {
                hw_gpio_set_active(gpio_wd_port, gpio_wd_pin);
        } else {
                hw_gpio_set_inactive(gpio_wd_port, gpio_wd_pin);
        }

        /* 15ms high, 2s low. Callback is called every 15ms by timer. 2000 / 15 = 133.33 */
        gpio_wd_timer_cnt = (gpio_wd_timer_cnt + 1) % 134;
}

void plt_task(void *pvParameters)
{
        uint32_t notif;
        dgtl_msg_t *msg;

        current_task = OS_GET_CURRENT_TASK();

        OS_QUEUE_CREATE(cmd_queue, sizeof(dgtl_msg_t *), 10);
        OS_ASSERT(cmd_queue);

        sleep_timer = OS_TIMER_CREATE("dgtl_sleep", /* not relevant, needed only to create the timer */ 1,
                                                                OS_TIMER_FAIL, NULL, plt_sleep_cb);

        while (1) {
                /*
                * Wait on any of the event group bits, then clear them all
                */
                OS_TASK_NOTIFY_WAIT(0x0, OS_TASK_NOTIFY_ALL_BITS, &notif, OS_TASK_NOTIFY_FOREVER);

                if (notif & mainBIT_HCI_RX) {
                        if (OS_QUEUE_GET(cmd_queue, &msg, 0) == OS_QUEUE_OK) {
                                plt_parse_dgtl_msg(msg);
                        }
                        if (OS_QUEUE_MESSAGES_WAITING(cmd_queue)) {
                                OS_TASK_NOTIFY(current_task, mainBIT_HCI_RX, OS_NOTIFY_SET_BITS);
                        }
                }
        }
}

dgtl_msg_t *init_response_evt(dgtl_pkt_hci_cmd_t *cmd, size_t length)
{
        dgtl_msg_t *msg_evt;
        plt_evt_hdr_t *evt;
        size_t param_len;

        param_len = length - sizeof(dgtl_pkt_hci_evt_t);

        msg_evt = dgtl_msg_prepare_hci_evt(NULL, 0x0E /* Command Complete Event */, param_len, NULL);
        evt = (plt_evt_hdr_t *) msg_evt;

        /* Clear parameters of event packet */
        memset(dgtl_msg_get_param_ptr(msg_evt, NULL), 0, param_len);

        evt->num_hci_cmd_packets = 1;
        evt->opcode = cmd->opcode;

        return msg_evt;
}

dgtl_msg_t *init_cmd_status_evt(dgtl_pkt_hci_cmd_t *cmd)
{
        dgtl_msg_t *msg_evt;
        plt_cmd_status_evt_t *evt;
        size_t param_len;

        param_len = sizeof(plt_cmd_status_evt_t) - sizeof(dgtl_pkt_hci_evt_t);

        msg_evt = dgtl_msg_prepare_hci_evt(NULL, 0x0F /* Command Status Event */, param_len, NULL);
        evt = (plt_cmd_status_evt_t *) msg_evt;

        /* Clear parameters of event packet */
        memset(dgtl_msg_get_param_ptr(msg_evt, NULL), 0, param_len);

        evt->num_hci_cmd_packets = 1;
        evt->opcode = cmd->opcode;

        return msg_evt;
}

void plt_parse_dgtl_msg(dgtl_msg_t *msg)
{
        const dgtl_pkt_hci_cmd_t *pkt = (const dgtl_pkt_hci_cmd_t *) msg;

        /* Get only lower 8-bits of OCF (others are unused) */
        uint8_t cmd = pkt->opcode;

        if ((cmd < (sizeof(plt_cmd_handlers) / sizeof(plt_cmd_handler))) && plt_cmd_handlers[cmd]) {
                plt_cmd_handlers[cmd](msg);
        }

        dgtl_msg_free(msg);
}

__STATIC_INLINE void enable_output_xtal(void)
{
        /* Select XTAL32M as clock output */
        REG_SETF(GPIO, GPIO_CLK_SEL_REG, FUNC_CLOCK_SEL, 0x03);
        REG_SET_BIT(GPIO, GPIO_CLK_SEL_REG, FUNC_CLOCK_EN);

        /* Configure GPIO as XTAL32M clock output */
        hw_gpio_set_pin_function(PLT_FW_XTAL_OUT_PORT, PLT_FW_XTAL_OUT_PIN, HW_GPIO_MODE_OUTPUT,
                                                                                HW_GPIO_FUNC_CLOCK);
        hw_gpio_pad_latch_enable(PLT_FW_XTAL_OUT_PORT, PLT_FW_XTAL_OUT_PIN);
}

__STATIC_INLINE void disable_output_xtal(void)
{
        /* Disable clock output */
        REG_CLR_BIT(GPIO, GPIO_CLK_SEL_REG, FUNC_CLOCK_EN);

        hw_gpio_set_pin_function(PLT_FW_XTAL_OUT_PORT, PLT_FW_XTAL_OUT_PIN, HW_GPIO_MODE_INPUT,
                                                                                HW_GPIO_FUNC_GPIO);
}

__STATIC_INLINE uint16_t auto_xtal_trim(uint16_t config)
{
        int r;
        HW_GPIO_PORT port;
        HW_GPIO_PIN pin;
        HW_GPIO_MODE mode;
        HW_GPIO_FUNC function;
        uint8_t gpio = config & 0xFF;

        port = GPIO_PAD_TO_PORT(gpio);
        pin = GPIO_PAD_TO_PIN(gpio);

        if (gpio == 0xFE) { /* PULSE_IN_RX */
                bool rx_pin_found = false;

                port = HW_GPIO_PORT_0;
                pin = HW_GPIO_PIN_0;

                /* Find the UART2 RX pin */
                while (!rx_pin_found) {
                        hw_gpio_get_pin_function(port, pin, &mode, &function);
                        rx_pin_found = (function == HW_GPIO_FUNC_UART2_RX);

                        if (rx_pin_found) {
                                /* GPIO found. */
                                break;
                        }

                        /* Change port... */
                        if (pin == HW_GPIO_PIN_31) {
                                port = HW_GPIO_PORT_1;
                                pin = HW_GPIO_PIN_0;
                                continue;
                        }
                        pin++;
                }
        }

        if (!plt_check_gpio(port, pin)) {
                /* Invalid GPIO port and/or pin */
                return PLT_FW_AUTO_TRIM_INVALID_GPIO;
        }

        /* Store pulse input gpio previous mode and function */
        hw_gpio_get_pin_function(port, pin, &mode, &function);

        r = auto_trim(port, pin);

        // Wait for the line to return to idle...
        while (!hw_gpio_get_pin_status(port, pin));

        /* Restore pulse input gpio previous mode and functions.
         * This is needed because they use the UART RX pin for
         * pulse input. It must be restored to resume UART operation
         */
        hw_gpio_set_pin_function(port, pin, mode, function);


        if (r < 0)
                return -r;
        else
                return 0;
}

static void plt_sleep_cb(void *param)
{
        /* Set to the previous sleep mode */
        pm_sleep_mode_set(prev_sleep_mode);
        /* This must be called from ISR */
        dgtl_wkup_handler();
}

static void hci_command_sent_cb(void *user_data)
{
        OS_EVENT evt = (OS_EVENT) user_data;

        OS_EVENT_SIGNAL(evt);
}

void hci_cmd_sleep(dgtl_msg_t *msg)
{
        plt_cmd_hci_sleep_t *cmd = (plt_cmd_hci_sleep_t *) msg;
        dgtl_msg_t *msg_evt;
        plt_cmd_status_evt_t *evt;
        sleep_mode_t sleep_mode = pm_mode_idle;
        uint32_t sleep_time_sec = 60 * cmd->min + cmd->sec;
        OS_EVENT event;
        lp_clk_is_t lp_clock = hw_clk_get_lpclk();

        msg_evt = init_cmd_status_evt(&cmd->hci_cmd);
        evt = (plt_cmd_status_evt_t *) msg_evt;
        evt->status = BLE_HCI_ERROR_NO_ERROR;

        if (lp_clock == LP_CLK_IS_INVALID) {
                evt->status = BLE_HCI_ERROR_HARDWARE_FAILURE;
                dgtl_send(msg_evt);
                return;
        }

        switch (cmd->mode) {
        case PLT_SLEEP_MODE_ACTIVE:
                sleep_mode = pm_mode_active;
                break;
        case PLT_SLEEP_MODE_EXTENDED_SLEEP:
                sleep_mode = pm_mode_extended_sleep;
                break;
        case PLT_SLEEP_MODE_DEEP_SLEEP:
                sleep_mode = pm_mode_deep_sleep;
                break;
        default:
                evt->status = BLE_HCI_ERROR_INVALID_HCI_PARAM;
                dgtl_send(msg_evt);
                return;
        }

        if (sleep_mode == pm_mode_active) {
                pm_sleep_mode_set(sleep_mode);
                dgtl_send(msg_evt);
                return;
        }

        /* Response must be sent before closing the UART adapter */
        OS_EVENT_CREATE(event);

        if (dgtl_send_ex(msg_evt, hci_command_sent_cb, (void *) event)) {
                OS_EVENT_WAIT(event, OS_EVENT_FOREVER);
        }

        OS_EVENT_DELETE(event);

        /*
         * Close UART in DGTL task to release idle mode in sleep mode to have a chance to change
         * sleep mode to the new one.
         */
        dgtl_close();

        /* Set sleep mode */
        prev_sleep_mode = pm_sleep_mode_set(sleep_mode);

        if (sleep_mode == pm_mode_extended_sleep && sleep_time_sec > 0) {
                /* Start timer to count time in which platform sleeps */
                OS_TIMER_CHANGE_PERIOD(sleep_timer, OS_MS_2_TICKS(sleep_time_sec * 1000),
                                                                                OS_TIMER_FOREVER);
                OS_TIMER_START(sleep_timer, OS_TIMER_FOREVER);
        } else if (sleep_mode == pm_mode_deep_sleep) {
                /* Configure RTC alarm to wake-up system from deep sleep */
                extern uint16_t rcx_clock_hz;
                hw_rtc_time_t time;
                uint8_t minutes;
                uint16_t div_int = lp_clock == LP_CLK_IS_RCX ? (rcx_clock_hz / 100) : 0x147;
                uint16_t div_frac = lp_clock == LP_CLK_IS_RCX ? (rcx_clock_hz % 100) : 0x2a8;

                hw_rtc_get_time_clndr(&time, NULL);

                sleep_time_sec += time.sec;
                time.sec = (sleep_time_sec) % 60;

                minutes = sleep_time_sec / 60 + time.minute;
                time.minute = minutes % 60;

                time.hour = (time.hour + minutes / 60) % (time.hour_mode == RTC_24H_CLK ? 24 : 12);

                hw_rtc_get_event_flags(); // clear pending interrupts
                hw_rtc_clk_config(RTC_DIV_DENOM_1000, div_int, div_frac);
                hw_rtc_set_alarm(&time, NULL, HW_RTC_ALARM_SEC | HW_RTC_ALARM_MIN | HW_RTC_ALARM_HOUR);
        }
}

void xtal_trim(dgtl_msg_t *msg)
{
        plt_cmd_hci_xtrim_t *cmd = (plt_cmd_hci_xtrim_t *) msg;
        dgtl_msg_t *msg_evt;
        plt_evt_hci_xtrim_t *evt;
        uint16_t trim_value;

        msg_evt = init_response_evt(&cmd->hci_cmd, sizeof(*evt));
        evt = (plt_evt_hci_xtrim_t *) msg_evt;

        switch (cmd->operation) {
        case 0: /* Read trim value */
                evt->trim_value = REG_GETF(CRG_XTAL, CLK_FREQ_TRIM_REG, XTAL32M_TRIM);
                break;
        case 1: /* Write trim value */
                REG_SETF(CRG_XTAL, CLK_FREQ_TRIM_REG, XTAL32M_TRIM, cmd->value);
                break;
        case 2: /* Enable output xtal on P1_2 */
                enable_output_xtal();
                break;
        case 3: /* Increase trim value by delta */
                trim_value = REG_GETF(CRG_XTAL, CLK_FREQ_TRIM_REG, XTAL32M_TRIM);

                REG_SETF(CRG_XTAL, CLK_FREQ_TRIM_REG, XTAL32M_TRIM, cmd->value + trim_value);
                break;
        case 4: /* Decrease trim value by delta */
                trim_value = REG_GETF(CRG_XTAL, CLK_FREQ_TRIM_REG, XTAL32M_TRIM);

                REG_SETF(CRG_XTAL, CLK_FREQ_TRIM_REG, XTAL32M_TRIM, trim_value - cmd->value);
                break;
        case 5: /* Disable output xtal on P1_2 */
                disable_output_xtal();
                break;
        case 6: /* Auto calibration test */
                evt->trim_value = auto_xtal_trim(cmd->value);
                break;
        }

        dgtl_send(msg_evt);
}

static bool verify_otp_range(uint32_t otp_addr, uint8_t word_count)
{
        /* OTP cell size is 4 byte on 690 */
        if (otp_addr + word_count <= HW_OTP_CELL_NUM) {
                return true;
        }
        return false;
}

static uint8_t otp_read(uint32_t otp_addr, uint8_t word_count, uint32_t *data_ptr)
{
        uint8_t status;

        hw_otpc_init();
        hw_otpc_set_speed(HW_OTPC_CLK_FREQ_32MHz);
        hw_otpc_read(data_ptr, otp_addr, word_count);
        hw_otpc_close();

        status = BLE_HCI_ERROR_NO_ERROR;
done:
        return status;
}

void hci_cmd_otp_read(dgtl_msg_t *msg)
{
        plt_cmd_hci_read_otp_t *cmd = (plt_cmd_hci_read_otp_t *) msg;
        plt_evt_hci_read_otp_t *evt;
        dgtl_msg_t *msg_evt;
        uint8_t status;
        uint8_t word_count = 0;
        uint32_t *data = NULL;

        if (cmd->word_count < MIN_RW_OTP_WORDS || cmd->word_count > MAX_RW_OTP_WORDS) {
                status = BLE_HCI_ERROR_INVALID_HCI_PARAM;
                goto done;
        }

        if (!verify_otp_range(cmd->addr, cmd->word_count)) {
                status = BLE_HCI_ERROR_UNSUPPORTED;
                goto done;
        }

        word_count = cmd->word_count;
        data = OS_MALLOC(word_count * sizeof(uint32_t));
        if (!data) {
                status = BLE_HCI_ERROR_MEMORY_CAPA_EXCEED;
                goto done;
        }

        status = otp_read(cmd->addr, word_count, data);

done:
        if (status != BLE_HCI_ERROR_NO_ERROR) {
                word_count = 0;
        }

        msg_evt = init_response_evt(&cmd->hci_cmd, sizeof(*evt) + (sizeof(uint32_t) * word_count));
        evt = (plt_evt_hci_read_otp_t *) msg_evt;
        evt->status = status;
        evt->word_count = word_count;

        if (word_count) {
                memcpy(evt->data, data, sizeof(uint32_t) * word_count);
        }

        if (data) {
                OS_FREE(data);
        }

        dgtl_send(msg_evt);
}

static uint8_t otp_write(uint32_t otp_addr, uint8_t word_count, uint32_t *data_ptr)
{
        uint8_t status = BLE_HCI_ERROR_NO_ERROR;

        hw_otpc_init();
        hw_otpc_set_speed(HW_OTPC_CLK_FREQ_32MHz);
        if (!hw_otpc_prog_and_verify(data_ptr, otp_addr, word_count)) {
                status = BLE_HCI_ERROR_HARDWARE_FAILURE;
        }

        hw_otpc_close();

        return status;
}

void hci_cmd_otp_write(dgtl_msg_t *msg)
{
        plt_cmd_hci_write_otp_t *cmd = (plt_cmd_hci_write_otp_t *) msg;
        dgtl_msg_t *msg_evt;
        plt_evt_hci_write_otp_t *evt;
        uint32_t *data;
        uint8_t status;

        if (cmd->word_count < MIN_RW_OTP_WORDS || cmd->word_count > MAX_RW_OTP_WORDS) {
                status = BLE_HCI_ERROR_INVALID_HCI_PARAM;
                goto done;
        }

        if (!verify_otp_range(cmd->addr, cmd->word_count)) {
                status = BLE_HCI_ERROR_UNSUPPORTED;
                goto done;
        }


        /* This is required as cmd->data is not be aligned */
        data = OS_MALLOC(sizeof(uint32_t) * cmd->word_count);
        if (!data) {
                status = BLE_HCI_ERROR_MEMORY_CAPA_EXCEED;
                goto done;
        }

        memcpy(data, cmd->data, sizeof(uint32_t) * cmd->word_count);
        status = otp_write(cmd->addr, cmd->word_count, data);
        OS_FREE(data);

done:
        msg_evt = init_response_evt(&cmd->hci_cmd, sizeof(*evt));
        evt = (plt_evt_hci_write_otp_t *) msg_evt;
        evt->status = status;
        evt->word_count = (evt->status == BLE_HCI_ERROR_NO_ERROR) ? cmd->word_count : 0;

        dgtl_send(msg_evt);
}

void hci_cmd_rw_reg(dgtl_msg_t *msg)
{
        plt_cmd_hci_rw_reg_t *cmd = (plt_cmd_hci_rw_reg_t *) msg;
        dgtl_msg_t *msg_evt;
        plt_evt_hci_rw_reg_t *evt;
        void *reg_address = NULL;

        reg_address = (void *) cmd->address;

        msg_evt = init_response_evt(&cmd->hci_cmd, sizeof(*evt));
        evt = (plt_evt_hci_rw_reg_t *) msg_evt;

        evt->operation_mode = cmd->operation_mode;
        evt->status = BLE_HCI_ERROR_NO_ERROR;

        /*
         * First, check if address is valid. Next, check alignment of address argument.
         */
        if (IS_REMAPPED_ADDRESS(reg_address) || IS_ROM_ADDRESS(reg_address) ||
                        IS_CACHERAM_ADDRESS(reg_address) || IS_SYSRAM_ADDRESS(reg_address) ||
                        IS_QSPIF_S_ADDRESS(reg_address) ||
                        IS_QSPIF_ADDRESS(reg_address) || IS_OTP_ADDRESS(reg_address)) {
                evt->status = BLE_HCI_ERROR_INVALID_HCI_PARAM;
        } else if (((cmd->operation_mode == PLT_OPERATION_READ_REG32) ||
                                                (cmd->operation_mode == PLT_OPERATION_WRITE_REG32))
                                                && ((uint32_t) reg_address & 0x03)) {
                evt->status = BLE_HCI_ERROR_INVALID_HCI_PARAM;
        } else if (((cmd->operation_mode == PLT_OPERATION_READ_REG16) ||
                                                (cmd->operation_mode == PLT_OPERATION_WRITE_REG16))
                                                && ((uint32_t) reg_address & 0x01)) {
                evt->status = BLE_HCI_ERROR_INVALID_HCI_PARAM;
        } else {
                switch (cmd->operation_mode) {
                case PLT_OPERATION_READ_REG32:
                        evt->data = PLT_CMD_READ_FROM_REG32(reg_address);
                        break;

                case PLT_OPERATION_WRITE_REG32:
                        PLT_CMD_WRITE_TO_REG32(reg_address, cmd->data);
                        break;

                case PLT_OPERATION_READ_REG16:
                        evt->data = PLT_CMD_READ_FROM_REG16(reg_address);
                        break;

                case PLT_OPERATION_WRITE_REG16:
                        PLT_CMD_WRITE_TO_REG16(reg_address, cmd->data);
                        break;

                default:
                        evt->status = BLE_HCI_ERROR_INVALID_HCI_PARAM;
                }
        }

        dgtl_send(msg_evt);
}

void fw_version_get(dgtl_msg_t *msg)
{
        plt_cmd_hci_firmware_version_get_t *cmd = (plt_cmd_hci_firmware_version_get_t *) msg;
        dgtl_msg_t *msg_evt;
        plt_evt_hci_firmware_version_get_t *evt;

        msg_evt = init_response_evt(&cmd->hci_cmd, sizeof(*evt));
        evt = (plt_evt_hci_firmware_version_get_t *) msg_evt;

        evt->ble_version_length = snprintf(evt->ble_fw_version, sizeof(evt->ble_fw_version),
                                                "%d.%d.%d.%d", RWBLE_SW_VERSION_MAJOR,
                                                RWBLE_SW_VERSION_MINOR, RWBLE_SW_VERSION_BUILD,
                                                RWBLE_SW_VERSION_SUB_BUILD ) + 1;

        evt->app_version_length = strlen(PLT_VERSION_STR) + 1;
        memcpy(evt->app_fw_version, PLT_VERSION_STR, evt->app_version_length);

        dgtl_send(msg_evt);
}

void hci_custom_action(dgtl_msg_t *msg)
{
        plt_cmd_hci_custom_action_t *cmd = (plt_cmd_hci_custom_action_t *) msg;
        dgtl_msg_t *msg_evt;
        plt_evt_hci_custom_action_t *evt;

        msg_evt = init_response_evt(&cmd->hci_cmd, sizeof(*evt));
        evt = (plt_evt_hci_custom_action_t *) msg_evt;

        evt->custom_action = cmd->custom_action;

        dgtl_send(msg_evt);
}

#if  (dg_configUSE_HW_GPADC == 1)
void hci_read_adc(dgtl_msg_t *msg)
{
        plt_cmd_hci_read_adc_t *cmd = (plt_cmd_hci_read_adc_t *) msg;
        dgtl_msg_t *msg_evt;
        plt_evt_hci_read_adc_t *evt;
        uint16_t adc_value;
        uint16_t adc_offs_p = 0x200;
        uint16_t adc_offs_n = 0x200;
        uint32_t sum = 0;
        int i;

        msg_evt = init_response_evt(&cmd->hci_cmd, sizeof(*evt));
        evt = (plt_evt_hci_read_adc_t *) msg_evt;

        /* Hack: We can't really use ADC adapter here, since it only
         * returns the 10-bit result, not the 16-bit oversampled one
         */
        resource_acquire(RES_MASK(RES_ID_GPADC), RES_WAIT_FOREVER);

        adc_offs_p = hw_gpadc_get_offset_positive();
        adc_offs_n = hw_gpadc_get_offset_negative();

        hw_gpadc_reset();

        hw_gpadc_set_input(HW_GPADC_INPUT_SE_VBAT);
        hw_gpadc_set_input_mode(HW_GPADC_INPUT_MODE_SINGLE_ENDED);
        hw_gpadc_set_ldo_constant_current(true);
        hw_gpadc_set_ldo_dynamic_current(true);
        hw_gpadc_adc_measure(); // dummy (fast) measurement
        hw_gpadc_set_sample_time(15);

        hw_gpadc_set_offset_positive(0x200);
        hw_gpadc_set_offset_negative(0x200);

        hw_gpadc_set_chopping(true);
        hw_gpadc_set_oversampling(7); // 128 samples
        for (volatile int i = 10; i > 0; i--)
                ; // Make sure 1usec has passed since the mode setting (VBAT).

        OS_ENTER_CRITICAL_SECTION();
        for (i = 0; i < cmd->samples_nr; i++) {
                hw_gpadc_adc_measure();
                adc_value = hw_gpadc_get_raw_value();
                sum += adc_value;
                if (cmd->samples_period > 0)
                        OS_DELAY_MS(cmd->samples_period);
        }
        OS_LEAVE_CRITICAL_SECTION();

        hw_gpadc_set_offset_positive(adc_offs_p);
        hw_gpadc_set_offset_negative(adc_offs_n);

        resource_release(RES_MASK(RES_ID_GPADC));

        evt->result = ((cmd->samples_nr != 0) ? sum / cmd->samples_nr : 0);

        dgtl_send(msg_evt);
}
#endif /*  (dg_configUSE_HW_GPADC == 1) */

void hci_sensor_test(dgtl_msg_t *msg)
{
        plt_cmd_hci_sensor_test_t *cmd = (plt_cmd_hci_sensor_test_t *) msg;
        dgtl_msg_t *msg_evt;
        plt_evt_hci_sensor_test_t *evt;
        size_t i2c_status = 0;
        HW_I2C_ABORT_SOURCE abrt_src = HW_I2C_ABORT_NONE;

        /* SPI initialization structure. */
        const spi_config spi_init = {
                .cs_pad.port = cmd->spi_cs_port,
                .cs_pad.pin = cmd->spi_cs_pin,
                .word_mode = HW_SPI_WORD_8BIT,
                .smn_role = HW_SPI_MODE_MASTER,
                .polarity_mode = HW_SPI_POL_HIGH,
                .phase_mode = HW_SPI_PHA_MODE_1,
                .mint_mode = HW_SPI_MINT_DISABLE,
                .xtal_freq = HW_SPI_FREQ_DIV_2,
                .fifo_mode = HW_SPI_FIFO_RX_TX,
                .disabled = 0,
                .use_dma = 0,
                .rx_dma_channel = 0,
                .tx_dma_channel = 0
        };

        /* I2C initialization structure. */
        const i2c_config i2c_init = {
                .speed = HW_I2C_SPEED_STANDARD,
                .mode = HW_I2C_MODE_MASTER,
                .addr_mode = HW_I2C_ADDRESSING_7B,
                .address = cmd->i2c_slave_addr,
        };

        /* Prepare the UART response message. */
        msg_evt = init_response_evt(&cmd->hci_cmd, sizeof(*evt));
        evt = (plt_evt_hci_sensor_test_t *) msg_evt;
        evt->error = PLT_FW_STATUS_OK; // Initialize with no error.

        /* If DRDY/INT GPIO test is enabled, then initialize it. */
        if (cmd->int_gpio_check == 0x01) {
                hw_gpio_configure_pin(cmd->int_port, cmd->int_pin, HW_GPIO_MODE_INPUT_PULLUP, HW_GPIO_FUNC_GPIO, false);
                hw_gpio_configure_pin_power(cmd->int_port, cmd->int_pin, cmd->gpio_lvl ? HW_GPIO_POWER_VDD1V8P : HW_GPIO_POWER_V33);
        }

        /* Sensor interface is SPI. */
        if (cmd->iface == 0)
        {
                /* Initialize the SPI interface. */
                hw_spi_init(HW_SPI1, &spi_init);

                /* Configure the SPI GPIOs. */
                hw_gpio_configure_pin(cmd->spi_cs_port, cmd->spi_cs_pin, HW_GPIO_MODE_OUTPUT_PUSH_PULL, HW_GPIO_FUNC_SPI_EN, true);
                hw_gpio_configure_pin(cmd->spi_clk_i2c_scl_port, cmd->spi_clk_i2c_scl_pin, HW_GPIO_MODE_OUTPUT_PUSH_PULL, HW_GPIO_FUNC_SPI_CLK, true);
                hw_gpio_configure_pin(cmd->spi_do_port, cmd->spi_do_pin, HW_GPIO_MODE_OUTPUT_PUSH_PULL, HW_GPIO_FUNC_SPI_DO, true);
                hw_gpio_configure_pin(cmd->spi_di_i2c_sda_port, cmd->spi_di_i2c_sda_pin, HW_GPIO_MODE_INPUT_PULLUP, HW_GPIO_FUNC_SPI_DI, true);
                hw_gpio_pad_latch_enable(cmd->spi_cs_port, cmd->spi_cs_pin);
                hw_gpio_pad_latch_enable(cmd->spi_clk_i2c_scl_port, cmd->spi_clk_i2c_scl_pin);
                hw_gpio_pad_latch_enable(cmd->spi_do_port, cmd->spi_do_pin);
                hw_gpio_pad_latch_enable(cmd->spi_di_i2c_sda_port, cmd->spi_di_i2c_sda_pin);
                hw_gpio_configure_pin_power(cmd->spi_cs_port, cmd->spi_cs_pin, cmd->gpio_lvl ? HW_GPIO_POWER_VDD1V8P : HW_GPIO_POWER_V33);
                hw_gpio_configure_pin_power(cmd->spi_clk_i2c_scl_port, cmd->spi_clk_i2c_scl_pin, cmd->gpio_lvl ? HW_GPIO_POWER_VDD1V8P : HW_GPIO_POWER_V33);
                hw_gpio_configure_pin_power(cmd->spi_do_port, cmd->spi_do_pin, cmd->gpio_lvl ? HW_GPIO_POWER_VDD1V8P : HW_GPIO_POWER_V33);
                hw_gpio_configure_pin_power(cmd->spi_di_i2c_sda_port, cmd->spi_di_i2c_sda_pin, cmd->gpio_lvl ? HW_GPIO_POWER_VDD1V8P : HW_GPIO_POWER_V33);

                /* Toggle CS. Needed for some devices to enter SPI mode. */
                hw_spi_set_cs_low(HW_SPI1);
                hw_clk_delay_usec(1000);
                hw_spi_set_cs_high(HW_SPI1);
                hw_clk_delay_usec(1000);

                hw_spi_set_cs_low(HW_SPI1);

                /* Check if we want to write to the sensor. */
                if (cmd->rd_wr == 0x01) {
                        cmd->reg_addr = cmd->reg_addr & 0x7F;
                        hw_spi_write_buf(HW_SPI1, &cmd->reg_addr, 1, NULL, NULL);
                        hw_spi_write_buf(HW_SPI1, &cmd->reg_data, 1, NULL, NULL);
                }

                /* Read from the sensor. */
                cmd->reg_addr = cmd->reg_addr | 0x80;
                hw_spi_write_buf(HW_SPI1, &cmd->reg_addr, 1, NULL, NULL);
                hw_spi_read_buf(HW_SPI1, &evt->data, 1, NULL, NULL);

                /* Set the SPI CS high. */
                hw_spi_set_cs_high(HW_SPI1);

                /* Check the status of the DRDY/INT GPIO. */
                if (cmd->int_gpio_check == 0x01) {
                    hw_clk_delay_usec(1000);
                    evt->data = hw_gpio_get_pin_status((HW_GPIO_PORT) cmd->int_port, (HW_GPIO_PIN) cmd->int_pin);
                }

                /* Reset the SPI GPIOs. */
                hw_gpio_configure_pin(cmd->spi_cs_port, cmd->spi_cs_pin, HW_GPIO_MODE_INPUT_PULLUP, HW_GPIO_FUNC_GPIO, false);
                hw_gpio_configure_pin(cmd->spi_clk_i2c_scl_port, cmd->spi_clk_i2c_scl_pin, HW_GPIO_MODE_INPUT_PULLDOWN, HW_GPIO_FUNC_GPIO, false);
                hw_gpio_configure_pin(cmd->spi_do_port, cmd->spi_do_pin, HW_GPIO_MODE_INPUT_PULLDOWN, HW_GPIO_FUNC_GPIO, false);
                hw_gpio_configure_pin(cmd->spi_di_i2c_sda_port, cmd->spi_di_i2c_sda_pin, HW_GPIO_MODE_INPUT_PULLDOWN, HW_GPIO_FUNC_GPIO, false);
                hw_gpio_pad_latch_disable(cmd->spi_cs_port, cmd->spi_cs_pin);
                hw_gpio_pad_latch_disable(cmd->spi_clk_i2c_scl_port, cmd->spi_clk_i2c_scl_pin);
                hw_gpio_pad_latch_disable(cmd->spi_do_port, cmd->spi_do_pin);
                hw_gpio_pad_latch_disable(cmd->spi_di_i2c_sda_port, cmd->spi_di_i2c_sda_pin);
                /* Reset and disable the SPI1 interface. */
                hw_spi_reset(HW_SPI1);
                hw_spi_enable(HW_SPI1, 0);

        /* Sensor interface is I2C. */
        } else {
                /* Initialize the I2C interface. */
                hw_i2c_init(HW_I2C1, &i2c_init);
                hw_i2c_enable(HW_I2C1);
                /* Configure the I2C GPIOs. */
                hw_gpio_configure_pin(cmd->spi_clk_i2c_scl_port, cmd->spi_clk_i2c_scl_pin, HW_GPIO_MODE_INPUT, HW_GPIO_FUNC_I2C_SCL, true);
                hw_gpio_configure_pin(cmd->spi_di_i2c_sda_port, cmd->spi_di_i2c_sda_pin, HW_GPIO_MODE_INPUT, HW_GPIO_FUNC_I2C_SDA, true);
                hw_gpio_pad_latch_enable(cmd->spi_clk_i2c_scl_port, cmd->spi_clk_i2c_scl_pin);
                hw_gpio_pad_latch_enable(cmd->spi_di_i2c_sda_port, cmd->spi_di_i2c_sda_pin);
                hw_gpio_configure_pin_power(cmd->spi_clk_i2c_scl_port, cmd->spi_clk_i2c_scl_pin, cmd->gpio_lvl ? HW_GPIO_POWER_VDD1V8P : HW_GPIO_POWER_V33);
                hw_gpio_configure_pin_power(cmd->spi_di_i2c_sda_port, cmd->spi_di_i2c_sda_pin, cmd->gpio_lvl ? HW_GPIO_POWER_VDD1V8P : HW_GPIO_POWER_V33);

                /* Check if we want to write to the sensor. */
                if (cmd->rd_wr == 0x01) {
                        hw_i2c_write_byte(HW_I2C1, cmd->reg_addr);
                        i2c_status = hw_i2c_write_buffer_sync(HW_I2C1, &cmd->reg_data, 1, &abrt_src, HW_I2C_F_ADD_STOP);
                        if ((i2c_status < 1) || (abrt_src != HW_I2C_ABORT_NONE)) {
                                evt->data = 0xFF;
                                evt->error = 0x01;
                                dgtl_send(msg_evt);
                                return;
                        }
                }

                /* Read from the sensor. */
                hw_i2c_write_byte(HW_I2C1, cmd->reg_addr);
                i2c_status = hw_i2c_read_buffer_sync(HW_I2C1, &evt->data, 1, &abrt_src, HW_I2C_F_ADD_STOP);

                if ((i2c_status < 1) || (abrt_src != HW_I2C_ABORT_NONE)) {
                        evt->data = 0xFF;
                        evt->error = 0x01;
                        dgtl_send(msg_evt);
                        return;
                }

                /* Check the status of the DRDY/INT GPIO. */
                if (cmd->int_gpio_check == 0x01) {
                    hw_clk_delay_usec(1000);
                    evt->data = hw_gpio_get_pin_status((HW_GPIO_PORT) cmd->int_port, (HW_GPIO_PIN) cmd->int_pin);
                }

                while (hw_i2c_controler_is_busy(HW_I2C1));
                /* Reset and disable the I2C interface. */
                hw_i2c_reset_abort_source(HW_I2C1);
                hw_i2c_reset_int_all(HW_I2C1);
                hw_i2c_deinit(HW_I2C1);

                /* Reset the I2C GPIOs. */
                hw_gpio_configure_pin(cmd->spi_clk_i2c_scl_port, cmd->spi_clk_i2c_scl_pin, HW_GPIO_MODE_INPUT_PULLDOWN, HW_GPIO_FUNC_GPIO, false);
                hw_gpio_configure_pin(cmd->spi_di_i2c_sda_port, cmd->spi_di_i2c_sda_pin, HW_GPIO_MODE_INPUT_PULLDOWN, HW_GPIO_FUNC_GPIO, false);
                hw_gpio_pad_latch_disable(cmd->spi_clk_i2c_scl_port, cmd->spi_clk_i2c_scl_pin);
                hw_gpio_pad_latch_disable(cmd->spi_di_i2c_sda_port, cmd->spi_di_i2c_sda_pin);

        }
        /* Send the UART response. */
        dgtl_send(msg_evt);
}

__STATIC_INLINE HW_GPIO_MODE plt_gpio_get_mode(uint8_t mode)
{
        switch (mode) {
        case PLT_GPIO_MODE_INPUT:
                return HW_GPIO_MODE_INPUT;
        case PLT_GPIO_MODE_INPUT_PULLUP:
                return HW_GPIO_MODE_INPUT_PULLUP;
        case PLT_GPIO_MODE_INPUT_PULLDOWN:
                return HW_GPIO_MODE_INPUT_PULLDOWN;
        case PLT_GPIO_MODE_OUTPUT:
                return HW_GPIO_MODE_OUTPUT;
        case PLT_GPIO_MODE_OUTPUT_PUSH_PULL:
                return HW_GPIO_MODE_OUTPUT_PUSH_PULL;
        case PLT_GPIO_MODE_OUTPUT_OPEN_DRAIN:
                return HW_GPIO_MODE_OUTPUT_OPEN_DRAIN;
        }

        return HW_GPIO_MODE_INVALID;
}

/* USB port pins need special handling to power them and use them as GPIOs. */
__STATIC_INLINE void configure_usb_pins(HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
        /* Check P0_14 and P0_15 pins */
        if ((port == HW_GPIO_PORT_0 && pin == HW_GPIO_PIN_14) ||
                                                (port == HW_GPIO_PORT_0 && pin == HW_GPIO_PIN_15)) {
                REG_CLR_BIT(USB, USB_MCTRL_REG, USBEN);
                REG_SET_BIT(GPREG, USBPAD_REG, USBPAD_EN);
        }
}

/* JTAG pins need disabling debugger before using them as GPIOs. */
__STATIC_INLINE void configure_debugger_pins(HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
        /* Check P0_10 and P0_11 pins */
        if ((port == HW_GPIO_PORT_0 && pin == HW_GPIO_PIN_10) ||
                                                (port == HW_GPIO_PORT_0 && pin == HW_GPIO_PIN_11)) {
                DISABLE_DEBUGGER;
        }

        /* Check P0_12 and P0_13 pins */
        if ((port == HW_GPIO_PORT_0 && pin == HW_GPIO_PIN_12) ||
                                                (port == HW_GPIO_PORT_0 && pin == HW_GPIO_PIN_13)) {
                DISABLE_CMAC_DEBUGGER;
        }
}

/* Returns true if passed GPIO is used by UART */
__STATIC_INLINE bool check_uart_pins(HW_GPIO_PORT port, HW_GPIO_PIN pin)
{
        if (port == platform_dgtl_io_conf.tx.port && pin == platform_dgtl_io_conf.tx.pin) {
                return true;
        }

        if (port == platform_dgtl_io_conf.rx.port && pin == platform_dgtl_io_conf.rx.pin) {
                return true;
        }

#if (CONFIG_USE_HW_FLOW_CONTROL == 1)
        if (port == platform_dgtl_io_conf.ctsn.port && pin == platform_dgtl_io_conf.ctsn.pin) {
                return true;
        }

        if (port == platform_dgtl_io_conf.rtsn.port && pin == platform_dgtl_io_conf.rtsn.pin) {
                return true;
        }
#endif

        return false;
}

__STATIC_INLINE bool plt_gpio_set_check_cond(uint8_t val, uint8_t gpio_lvl, HW_GPIO_MODE mode,
                                                HW_GPIO_PORT port, HW_GPIO_PIN pin, bool is_led,
                                                bool is_pwm, uint8_t duty_cycle)
{
        /* Check set/reset value: 0 = reset, 1 = set, other = invalid*/
        if (val > 1) {
                return false;
        }

        /* Check the voltage rail: 0 = 3.3V, 1 = 1.8V, other = invalid */
        if (gpio_lvl > 1) {
                return false;
        }

        /* Duty cycle must be in range 0 - 100 */
        if (is_pwm && duty_cycle > 100) {
                return false;
        }

        if (is_led) {
                /* There is no need to check GPIO configuration - LED is used */
                return true;
        }

        /* Check QPIO pin mode */
        if (mode == HW_GPIO_MODE_INVALID) {
                return false;
        }

        if (!plt_check_gpio(port, pin)) {
                return false;
        }

        /* UART pin cannot be used in this command - this can break communication */
        return !check_uart_pins(port, pin);
}

__STATIC_INLINE void plt_gpio_set_output_gpio(HW_GPIO_PORT port, HW_GPIO_PIN pin, HW_GPIO_POWER power,
                                                HW_GPIO_MODE mode, HW_GPIO_FUNC function, bool high)
{
        /* If the USB pin selected then it must be configured as GPIO */
        configure_usb_pins(port, pin);

        /* If the debugger pin selected then it must be configured as GPIO */
        configure_debugger_pins(port, pin);

        /* Configure the pin voltage rail. */
        hw_gpio_configure_pin_power(port, pin, power);

        /* Configure the pin. */
        hw_gpio_configure_pin(port, pin, mode, function, high);
        hw_gpio_pad_latch_enable(port, pin);
        hw_gpio_pad_latch_disable(port, pin);
}

__STATIC_INLINE bool plt_gpio_set_is_led(uint8_t gpio_pad)
{
        return (gpio_pad == PLT_LED_ID_1) || (gpio_pad == PLT_LED_ID_2);
}

__STATIC_INLINE void plt_gpio_set_output_led(uint8_t led_id, bool set, bool is_pwm)
{
        hw_led_pwm_duty_cycle_t duty_cycle_led = {
                .hw_led_pwm_start = 0,
                .hw_led_pwm_end = 20,
        };
        hw_led_pwm_set_frequency(255);

        switch (led_id) {
        case PLT_LED_ID_1:
                hw_led1_pwm_set_duty_cycle(&duty_cycle_led);
                hw_led_set_pwm_state(set);
                hw_led_enable_led1(set);
                break;
        case PLT_LED_ID_2:
                hw_led2_pwm_set_duty_cycle(&duty_cycle_led);
                hw_led_set_pwm_state(set);
                hw_led_enable_led2(set);
                break;
        default:
                return;
        }

}


__STATIC_INLINE void plt_gpio_set_output_pwm(HW_GPIO_PORT port, HW_GPIO_PIN pin, HW_GPIO_POWER power,
                                                     uint8_t pwm, uint8_t duty_cycle, bool set,
                                                                                     uint8_t led_id)
{
        HW_GPIO_FUNC gpio_func = HW_GPIO_FUNC_TIM2_PWM;
        uint16_t pwm_freq_reg = 2000.0 / pwm;
        timer_config cfg = {
                .clk_src = HW_TIMER_CLK_SRC_EXT,
                .prescaler = 15,
                .mode = HW_TIMER_MODE_TIMER,
                .timer = {
                        .direction = HW_TIMER_DIR_UP,
                        .reload_val = 1000000,
                        .free_run = false,
                },

                .pwm = {
                        .port = port,
                        .pin = pin,
                        .pwm_active_in_sleep = false,
                        .duty_cycle = (duty_cycle / 100.0) * pwm_freq_reg,
                        .frequency = pwm_freq_reg
                },
        };

        if (!set) {
                hw_timer_disable(HW_TIMER2);
                if (led_id == PWM_INVALID_LED_ID) {
                        /* Restore default pin GPIO configuration */
                        plt_gpio_set_output_gpio(port, pin, power, HW_GPIO_MODE_INPUT_PULLDOWN,
                                                                        HW_GPIO_FUNC_GPIO, false);
                }
        } else {
                if (led_id == PWM_INVALID_LED_ID) {
                        /* LED should be configured earlier */
                        plt_gpio_set_output_gpio(port, pin, power, HW_GPIO_MODE_OUTPUT, gpio_func,
                                                                                        false);
                }

                /* Initialize and start timer */
                hw_timer_init(HW_TIMER2, &cfg);
                hw_timer_enable(HW_TIMER2);
        }
}

void hci_gpio_set(dgtl_msg_t *msg)
{
        plt_cmd_hci_gpio_set_t *cmd = (plt_cmd_hci_gpio_set_t *) msg;
        dgtl_msg_t *msg_evt;
        plt_evt_hci_gpio_set_t *evt;
        HW_GPIO_PORT port;
        HW_GPIO_PIN pin;
        HW_GPIO_MODE mode;
        bool is_led;
        bool is_pwm;

        /* Prepare the UART response message. */
        msg_evt = init_response_evt(&cmd->hci_cmd, sizeof(*evt));
        evt = (plt_evt_hci_gpio_set_t *) msg_evt;
        evt->error = PLT_FW_STATUS_OK;

        /* Transform the input. */
        port = GPIO_PAD_TO_PORT(cmd->gpio_pad);
        pin = GPIO_PAD_TO_PIN(cmd->gpio_pad);

        /* Transform the GPIO mode. */
        mode = plt_gpio_get_mode(cmd->mode);

        /* Check that given GPIO pad is one of the LEDs */
        is_led = plt_gpio_set_is_led(cmd->gpio_pad);

        /* Check that pin or LED will be handled by PWM timer */
        is_pwm = (cmd->pwm > 0);

        /* Check given parameters */
        if (plt_gpio_set_check_cond(cmd->val, cmd->gpio_lvl, mode, port, pin, is_led, is_pwm,
                                                                                cmd->duty_cycle)) {
                bool set = (cmd->val == 1);
                HW_GPIO_POWER power = (cmd->gpio_lvl ? HW_GPIO_POWER_VDD1V8P : HW_GPIO_POWER_V33);

                if (is_led) {
                        /* LED must be configured in both cases - if the PWM is used or not */
                        plt_gpio_set_output_led(cmd->gpio_pad, set, is_pwm);
                }
               else
               {
                       if (is_pwm) {
                               /* PWM source - LED or GPIO */
                               plt_gpio_set_output_pwm(port, pin, power, cmd->pwm, cmd->duty_cycle, set,
                                                               is_led ? cmd->gpio_pad : PWM_INVALID_LED_ID);
                       } else if (!is_led) {
                               /* Just change GPIO state */
                               plt_gpio_set_output_gpio(port, pin, power, mode, HW_GPIO_FUNC_GPIO, set);
                       }
              }
        } else {
                evt->error = PLT_FW_STATUS_ERROR;
        }

        /* Send the UART response. */
        dgtl_send(msg_evt);
}

void hci_gpio_read(dgtl_msg_t *msg)
{
        plt_cmd_hci_gpio_read_t *cmd = (plt_cmd_hci_gpio_read_t *) msg;
        dgtl_msg_t *msg_evt;
        plt_evt_hci_gpio_read_t *evt;
        HW_GPIO_PORT port;
        HW_GPIO_PIN pin;

        /* Prepare the UART response message. */
        msg_evt = init_response_evt(&cmd->hci_cmd, sizeof(*evt));
        evt = (plt_evt_hci_gpio_read_t *) msg_evt;

        /* Transform the input. */
        port = GPIO_PAD_TO_PORT(cmd->gpio_pad);
        pin = GPIO_PAD_TO_PIN(cmd->gpio_pad);

        if (!plt_check_gpio(port, pin)) {
                /* Invalid GPIO port and/or pin - reply with error */
                evt->data = 0xFF;
        } else {
                evt->data = hw_gpio_get_pin_status(port, pin);
        }

        /* Send the UART response. */
        dgtl_send(msg_evt);
}

void hci_uart_loop(dgtl_msg_t *msg)
{
        plt_cmd_hci_uart_loop_t *cmd = (plt_cmd_hci_uart_loop_t *) msg;
        dgtl_msg_t *msg_evt;
        plt_evt_hci_uart_loop_t *evt;

        /* Get the length of the received data. */
        size_t pkt_len = (size_t)(cmd->hci_cmd.length);

        /* Prepare the UART response message. */
        msg_evt = init_response_evt(&cmd->hci_cmd, sizeof(*evt) + pkt_len);
        evt = (plt_evt_hci_uart_loop_t *) msg_evt;

        /* Copy the received data to the output UART buffer. */
        memcpy(evt->data, cmd->data, pkt_len);

        /* Send the UART response. */
        dgtl_send(msg_evt);
}

void hci_uart_baud(dgtl_msg_t *msg)
{
        plt_cmd_hci_uart_baud_t *cmd = (plt_cmd_hci_uart_baud_t *) msg;
        dgtl_msg_t *msg_evt;
        plt_evt_hci_uart_baud_t *evt;
        OS_EVENT event;

        /* Prepare the UART response message. */
        msg_evt = init_response_evt(&cmd->hci_cmd, sizeof(*evt));
        evt = (plt_evt_hci_uart_baud_t *) msg_evt;

        if ((cmd->data == 4) ||
            (cmd->data == 3) ||
            (cmd->data == 2) ||
            (cmd->data == 1) ||
            (cmd->data == 0))
                evt->error = PLT_FW_STATUS_OK;
        else evt->error = 1;

        /* Response must be sent before closing the UART adapter */
        OS_EVENT_CREATE(event);

        if (dgtl_send_ex(msg_evt, hci_command_sent_cb, (void *) event)) {
                OS_EVENT_WAIT(event, OS_EVENT_FOREVER);
        }

        OS_EVENT_DELETE(event);

        dgtl_close();

        switch (cmd->data) {
        case 4:
                platform_dgtl_uart_driver_conf.hw_conf.baud_rate = HW_UART_BAUDRATE_1000000;
                break;
        case 3:
                platform_dgtl_uart_driver_conf.hw_conf.baud_rate = HW_UART_BAUDRATE_115200;
                break;
        case 2:
                platform_dgtl_uart_driver_conf.hw_conf.baud_rate = HW_UART_BAUDRATE_57600;
                break;
        case 1:
                platform_dgtl_uart_driver_conf.hw_conf.baud_rate = HW_UART_BAUDRATE_19200;
                break;
        case 0:
                platform_dgtl_uart_driver_conf.hw_conf.baud_rate = HW_UART_BAUDRATE_9600;
                break;
        default:
                break;
        }

        dgtl_wkup_handler();
}

static void plt_configure_gpio_wd(HW_GPIO_PORT port, HW_GPIO_PIN pin, HW_GPIO_POWER power)
{
        timer_config timer_cfg = {
                .clk_src = HW_TIMER_CLK_SRC_EXT,
                .prescaler = 0x1f, // 32MHz / (31 + 1) = 1MHz
                .mode = HW_TIMER_MODE_TIMER,
                .timer = {
                        .direction = HW_TIMER_DIR_UP,
                        .reload_val = 15000, // interrupt every 15ms
                },
        };

        hw_timer_disable(HW_TIMER2);

        /* Update global variable */
        gpio_wd_port = port;
        gpio_wd_pin = pin;

        /* Configure the pin voltage rail and function. */
        hw_gpio_configure_pin_power(port, pin, power);
        hw_gpio_set_pin_function(port, pin, HW_GPIO_MODE_OUTPUT, HW_GPIO_FUNC_GPIO);
        hw_gpio_pad_latch_enable_all();

        /* Reset counter used in callback. Initialize and start timer. */
        gpio_wd_timer_cnt = 0;
        hw_timer_init(HW_TIMER2, &timer_cfg);
        hw_timer_register_int(HW_TIMER2, timer_gpio_wd_cb);
        hw_timer_enable(HW_TIMER2);
}

void hci_gpio_wd(dgtl_msg_t *msg)
{
        HW_GPIO_PORT port;
        HW_GPIO_PIN pin;
        plt_cmd_hci_gpio_wd_t *cmd = (plt_cmd_hci_gpio_wd_t *) msg;
        dgtl_msg_t *msg_evt;
        plt_evt_hci_gpio_wd_t *evt;

        /* Prepare the UART response message. */
        msg_evt = init_response_evt(&cmd->hci_cmd, sizeof(*evt));
        evt = (plt_evt_hci_gpio_wd_t *) msg_evt;

        /* Transform the input. */
        port = GPIO_PAD_TO_PORT(cmd->gpio_pad);
        pin = GPIO_PAD_TO_PIN(cmd->gpio_pad);

        if (plt_check_gpio(port, pin) && (cmd->gpio_lvl == 0 || cmd->gpio_lvl == 1)) {
                plt_configure_gpio_wd(port, pin, (cmd->gpio_lvl ? HW_GPIO_POWER_VDD1V8P :
                                                                                HW_GPIO_POWER_V33));
                evt->error = PLT_FW_STATUS_OK;
        } else {
                evt->error = PLT_FW_STATUS_ERROR;
        }

        dgtl_send(msg_evt);
}

void hci_ext32khz_test(dgtl_msg_t *msg)
{
        plt_cmd_hci_ext32khz_test_t *cmd = (plt_cmd_hci_ext32khz_test_t *) msg;
        dgtl_msg_t *msg_evt;
        plt_evt_hci_ext32khz_test_t *evt;
        uint8_t k = 0;

        /* Prepare the UART response message. */
        msg_evt = init_response_evt(&cmd->hci_cmd, sizeof(*evt));
        evt = (plt_evt_hci_ext32khz_test_t *) msg_evt;

        evt->error = PLT_FW_STATUS_OK;

        /* Run a dummy calibration to make sure the external 32Khz clock is connected. */
        hw_clk_start_calibration(CALIBRATE_XTAL32K, CALIBRATE_REF_XTAL32K, 25);

        /* Wait until it's finished or counters terminate. */
        for (k = 0; (k < 4) && !hw_clk_calibration_finished(); k++) {
                hw_clk_delay_usec(2250); // 2.25ms
        }

        if (!hw_clk_calibration_finished()) {
                evt->error = PLT_FW_STATUS_ERROR;
        }

        /* Send the UART response. */
        dgtl_send(msg_evt);
}
