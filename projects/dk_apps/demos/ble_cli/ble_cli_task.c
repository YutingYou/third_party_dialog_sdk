/**
 ****************************************************************************************
 *
 * @file ble_cli_task.c
 *
 * @brief BLE CLI task
 *
 * Copyright (C) 2016-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdio.h>
#include "osal.h"
#include "ble_common.h"
#include "cli.h"
#include "ble_cli_config.h"
#include "gap.h"
#include "gattc.h"
#include "gatts.h"
#include "gpio.h"
#include "pwm.h"

/**
 * CLI notification mask.
 */
#define CLI_NOTIF       (1 << 31)

/**
 * GPIO Wkup notification mask.
 */
#define GPIO_WKUP_NOTIF (1 << 30)

static const cli_command_t cli_commands[] = {
        { "gap", gap_command, NULL },
        { "gattc", gattc_command, NULL },
        { "gatts", gatts_command, NULL },
        { "gpio", gpio_command, NULL },
        { "pwm", pwm_command, NULL },
        { NULL },
};

static void default_handler(int argc, const char *argv[], void *user_data)
{
        const cli_command_t *command;

        printf("Unknown category: %s\r\n", argv[0]);
        printf("command categories: \r\n");
        for (command = cli_commands; command->name; command++) {
                printf("\t%s\r\n", command->name);
        }
        printf("To list available commands type category name.\r\n");
}

void ble_cli_task(void *params)
{
        ble_error_t status;
        cli_t cli;

        status = ble_enable();

        if (status == BLE_STATUS_OK) {
                ble_gap_role_set(GAP_PERIPHERAL_ROLE | GAP_CENTRAL_ROLE);
        } else {
                printf("%s: failed. Status=%d\r\n", __func__, status);
        }

        ble_register_app();
        ble_gap_device_name_set("Dialog BLE CLI", ATT_PERM_READ);

        cli = cli_register(CLI_NOTIF, cli_commands, default_handler);
        gpio_wkup_init(GPIO_WKUP_NOTIF);

        printf("ble_cli application started\r\n");

        for (;;) {
                OS_BASE_TYPE ret;
                uint32_t notif;

                ret = OS_TASK_NOTIFY_WAIT(0, (uint32_t) -1, &notif, portMAX_DELAY);
                OS_ASSERT(ret == OS_TASK_NOTIFY_SUCCESS);

                /* notified from BLE manager, can get event */
                if (notif & BLE_APP_NOTIFY_MASK) {
                        ble_evt_hdr_t *hdr;

                        hdr = ble_get_event(false);
                        if (!hdr) {
                                goto no_event;
                        }

                        if (gap_handle_event(hdr)) {
                                goto free_event;
                        }

                        if (gattc_handle_event(hdr)) {
                                goto free_event;
                        }

                        if (gatts_handle_event(hdr)) {
                                goto free_event;
                        }

free_event:
                        OS_FREE(hdr);

no_event:
                        // notify again if there are more events to process in queue
                        if (ble_has_event()) {
                                OS_TASK_NOTIFY(OS_GET_CURRENT_TASK(), BLE_APP_NOTIFY_MASK,
                                                                               OS_NOTIFY_SET_BITS);
                        }
                }

                if (notif & CLI_NOTIF) {
                        cli_handle_notified(cli);
                }

                if (notif & GPIO_WKUP_NOTIF) {
                        gpio_wkup_handle_notified();
                }
        }
}
