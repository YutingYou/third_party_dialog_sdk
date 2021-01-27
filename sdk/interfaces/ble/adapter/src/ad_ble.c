/**
 ****************************************************************************************
 *
 * @file ad_ble.c
 *
 * @brief BLE FreeRTOS Adapter
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifdef CONFIG_USE_BLE
#include <string.h>

#include "co_version.h"
#include "ble_stack_config.h"
#include "ble_config.h"

#include "osal.h"
#include "queue.h"
#include "event_groups.h"
#if (BLE_WINDOW_STATISTICS == 1) || (BLE_SLEEP_PERIOD_DEBUG == 1)
#include "logging.h"
#endif

#include "sys_clock_mgr.h"
#include "sys_power_mgr.h"
#include "sys_watchdog.h"

#include "hw_gpio.h"
#include "hw_clk.h"

#include "ad_nvms.h"

#include "ad_nvparam.h"
#include "platform_nvparam.h"

#include "ad_ble.h"
#include "ad_ble_msg.h"
#include "ble_config.h"
#include "ble_common.h"
#include "ble_mgr.h"
#include "ble_mgr_common.h"
#include "ble_mgr_ad_msg.h"
#include "ble_mgr_gtl.h"
#if (dg_configBLE_ADV_STOP_DELAY_ENABLE == 1)
#include "sdk_list.h"
#include "rwble.h"
#endif

#include "rwip_config.h"
#include "gapm_task.h"
#include "sys_tcs.h"

#include "hw_sys.h"
#include "gapc.h"
#if (dg_configRF_ENABLE_RECALIBRATION == 1)
#include "sys_adc.h"
#endif /* (dg_configRF_ENABLE_RECALIBRATION == 1) */
#if (USE_BLE_SLEEP == 1)
#include "user_config_defs.h"
#endif /* USE_BLE_SLEEP */


#if (BLE_ADAPTER_DEBUG == 1)
#pragma message "BLE Adapter: GPIO debugging is on!"
#endif

#if (dg_configNVMS_ADAPTER == 0)
#pragma message "NVMS Adapter is disabled. BLE device is going to use default BD_ADDR and IRK."
#endif

#if (dg_configSYSTEMVIEW)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_BLE_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_BLE_ISR_EXIT()
#endif

/*------------------------------------- Local definitions ----------------------------------------*/

/* Task stack size */
#define mainBLE_TASK_STACK_SIZE 1024

/* Task priorities */
#define mainBLE_TASK_PRIORITY           ( OS_TASK_PRIORITY_HIGHEST - 3 )

/* BLE manager event group bits */
#define mainBIT_EVENT_QUEUE_TO_MGR      (1 << 1)

typedef enum {
        BLE_ACTIVE = 0,
        BLE_SLEEPING,
        BLE_WAKING_UP
} eSleepStatus;

#if (dg_configBLE_ADV_STOP_DELAY_ENABLE == 1)
/* Delayed list element */
struct delayed_msg {
        struct delayed_msg  *next;
        ble_mgr_common_stack_msg_t *msg;
};
#endif

/*------------------------------------- Local variables ------------------------------------------*/

static eSleepStatus sleep_status;        // BLE_ACTIVE is the initial value
__RETAINED static bool stay_active; // Disabled by default
__RETAINED static bool sleep_for_ever;
__RETAINED static ad_ble_op_code_t current_op;
__RETAINED static ad_ble_operation_t adapter_op;

__RETAINED static ad_ble_interface_t adapter_if;
__RETAINED static OS_TASK mgr_task;

#ifndef CONFIG_USE_FTDF
/* Notification flag to indicate an RX operation was performed, in order
 * to perform the RX dc offset calib check (and possible recovery)
 */
__RETAINED bool rf_dcoffset_failure;
#endif

/* BLE stack read buffer pointer */
__RETAINED static uint8_t* ad_ble_stack_rd_buf_p;
/* BLE stack read size */
__RETAINED static uint32_t ad_ble_stack_rd_size;
/* BLE stack read callback */
__RETAINED static void (*ad_ble_stack_rd_cb) (uint8_t);
/* BLE stack write buffer pointer */
__RETAINED static uint8_t* ad_ble_stack_wr_buf_p;
/* BLE stack write size */
__RETAINED static uint32_t ad_ble_stack_wr_size;
/* BLE stack write callback */
__RETAINED static void (*ad_ble_stack_wr_cb) (uint8_t);
#if (dg_configBLE_ADV_STOP_DELAY_ENABLE == 1)
/* Advertising flag */
__RETAINED static bool advertising;
/* Wait for event flag */
__RETAINED static bool waiting_for_evt;
/* Delayed message list */
__RETAINED static void *delayed_list;
#endif

__RETAINED static uint8_t public_address[BD_ADDR_LEN];

#if dg_configNVPARAM_ADAPTER
/* Global BLE NV-Parameter handle */
__RETAINED static nvparam_t ble_parameters;
#endif

__RETAINED bool ble_stack_initialized;

extern __RETAINED uint8_t cmac_system_tcs_length;
extern __RETAINED uint8_t cmac_synth_tcs_length;
extern __RETAINED uint8_t cmac_rfcu_tcs_length;
#if (dg_configRF_ENABLE_RECALIBRATION == 1)
__RETAINED_RW static sys_adc_monitor_par_t rf_calib_par = {
        .drift = dg_configRF_CALIB_TEMP_DIFF,
        .pollIntervalms = dg_configRF_CALIB_TEMP_POLL_INTV,
};
__RETAINED bool ad_ble_temp_meas_enabled;
#endif /* (dg_configRF_ENABLE_RECALIBRATION == 1) */
/*------------------------------------- Prototypes -----------------------------------------------*/

/* BLE stack internal scheduler */
void rwip_schedule(void);


/* BLE stack main function */
void ble_stack_init(void);

#if (USE_BLE_SLEEP == 1)
/* Force wake-up of the BLE core. */
bool ble_force_wakeup(void);
#endif

/* BLE check block function */
bool ble_block(void);

/* BLE platform initialization function */
void ble_platform_initialization(void);

void ble_controller_reset(void);
void ke_event_set(uint8_t event_type);

/*------------------------------------------ Prototypes ------------------------------------------*/

/**
 * \brief Send a message to the BLE stack
 *
 * \param[in] ptr_msg  Pointer to the BLE stack message buffer.
 */
static void ad_ble_send_to_stack(const ble_mgr_common_stack_msg_t *ptr_msg);

/**
 * \brief Handle a BLE stack message.
 *
 * \param [in] msg Pointer to the message to be handled.
 */
static void ad_ble_handle_stack_msg(ble_mgr_common_stack_msg_t *msg);

/**
 * \brief Handle a BLE adapter configuration message.
 *
 * \param [in] msg Pointer to the message to be handled.
 */
static void ad_ble_handle_adapter_msg(ad_ble_msg_t *msg);

bool ke_mem_is_empty(uint8_t type);
/*--------------------------------------- Global variables ---------------------------------------*/


/*--------------------------------------- Local functions  ---------------------------------------*/


void ad_ble_lpclock_available(void)
{
        if (adapter_if.task) {
               OS_TASK_NOTIFY(adapter_if.task, mainBIT_EVENT_LPCLOCK_AVAIL, OS_NOTIFY_SET_BITS);
        }
}

OS_BASE_TYPE ad_ble_command_queue_send( const void *item, OS_TICK_TIME wait_ticks)
{
        if (OS_QUEUE_PUT(adapter_if.cmd_q, item, wait_ticks) != OS_OK) {
                return OS_FAIL;
        }
        OS_TASK_NOTIFY(adapter_if.task, mainBIT_COMMAND_QUEUE, OS_NOTIFY_SET_BITS);

        return OS_OK;
}

OS_BASE_TYPE ad_ble_event_queue_send( const void *item, OS_TICK_TIME wait_ticks)
{
        if (OS_QUEUE_PUT(adapter_if.evt_q, item, wait_ticks) != OS_OK) {
                return OS_FAIL;
        }
        OS_TASK_NOTIFY(mgr_task, mainBIT_EVENT_QUEUE_TO_MGR, OS_NOTIFY_SET_BITS);

        return OS_OK;
}

void ad_ble_notify_event_queue_avail(void)
{
        OS_TASK_NOTIFY(adapter_if.task, mainBIT_EVENT_QUEUE_AVAIL, OS_NOTIFY_SET_BITS);
}

void ad_ble_task_notify(uint32_t value)
{
        if (in_interrupt()) {
                OS_TASK_NOTIFY_FROM_ISR(adapter_if.task, value, OS_NOTIFY_SET_BITS);
        } else {
                OS_TASK_NOTIFY(adapter_if.task, value, OS_NOTIFY_SET_BITS);
        }
}

#if (dg_configRF_ENABLE_RECALIBRATION == 1)
void ad_ble_notify_rf_calibration(void)
{
        ad_ble_task_notify(mainBIT_BLE_RF_CALIBRATION);
}
#endif

bool ad_ble_non_retention_heap_in_use(void)
{
        if (!ble_stack_initialized) {
                return false;
        } else {
                return !ke_mem_is_empty(KE_MEM_NON_RETENTION);
        }
}

/**
 * \brief Wake-up the BLE block.
 */
static void ad_ble_wake_up(void)
{
#if (USE_BLE_SLEEP == 1)
        /* Set sleep_status to BLE_ACTIVE */
        sleep_status = BLE_ACTIVE;
#endif /* (USE_BLE_SLEEP == 1) */
}

#if (BLE_SLEEP_PERIOD_DEBUG == 1)
extern uint32_t logged_sleep_duration;
extern uint32_t retained_slp_duration;
#endif

/**
 * \brief Reset the BLE stack
 *
 * Create and send a GAPM_RESET_CMD to the BLE stack.
 */
static __UNUSED void ble_stack_reset(void)
{
        ble_mgr_common_stack_msg_t *msg;
        struct gapm_reset_cmd *cmd;

        msg = ble_gtl_alloc(GAPM_RESET_CMD, TASK_ID_GAPM, sizeof(struct gapm_reset_cmd));
        cmd = (struct gapm_reset_cmd *) msg->msg.gtl.param;

        /* Reset the software stack only */
        cmd->operation = GAPM_RESET;

        /* Send command to stack */
        ad_ble_send_to_stack(msg);

        OS_FREE(msg);
}

/**
 * \brief Check if the BLE core can enter sleep and, if so, enter sleep.
 *
 * \return The status of the requested operation.
 *         <ul>
 *             <li> 0, if the BLE core cannot sleep
 *             <li> 1, if the BLE core was put to sleep
 *             <li> other, if the BLE core has to stay active but the caller may block
 *         </ul>
 */
static int sleep_when_possible(void)
{
        int ret = 0;
#if (USE_BLE_SLEEP == 1)
        do {
                if (sleep_status != BLE_ACTIVE) {
                        break;
                }

                /* Check if the BLE host has any pending actions */
                if (ble_block()) {
                        sleep_status = BLE_SLEEPING;
                }
        } while (0);
#endif /* (USE_BLE_SLEEP == 1) */

        return ret;
}

bool ad_ble_read_nvms_param(uint8_t* param, uint8_t len, uint8_t nvparam_tag, uint32_t nvms_addr)
{
#if (dg_configNVMS_ADAPTER == 1)
#if (dg_configNVPARAM_ADAPTER == 1)
        uint16_t read_len = 0;
        uint16_t param_len;
        uint8_t valid;

        /* Parameter length shall be long enough to store address and validity flag */
        param_len = ad_nvparam_get_length(ble_parameters, nvparam_tag, NULL);
        if (param_len == len + sizeof(valid)) {
                ad_nvparam_read_offset(ble_parameters, nvparam_tag,
                                                len, sizeof(valid), &valid);

                /* Read param from nvparam only if validity flag is set to 0x00 and read_len is correct */
                if (valid == 0x00) {
                        read_len = ad_nvparam_read(ble_parameters, nvparam_tag,
                                len, param);
                        if (read_len == len) {
                                return true; /* Success */
                        }
                }
        }
#else
        nvms_t nvms;
        int i;

        nvms = ad_nvms_open(NVMS_PARAM_PART);

        ad_nvms_read(nvms, nvms_addr, (uint8_t *) param, len);

        for (i = 0; i < len; i++) {
                if (param[i] != 0xFF) {
                        return true; /* Success */
                }
        }
#endif /* (dg_configNVPARAM_ADAPTER == 1) */
#endif /* (dg_configNVMS_ADAPTER == 1) */

        return false; /* Failure */
}

void read_public_address()
{
        uint8_t default_addr[BD_ADDR_LEN] = defaultBLE_STATIC_ADDRESS;
        uint32_t *values;
        uint8_t size = 0;
        sys_tcs_get_custom_values(SYS_TCS_GROUP_BD_ADDR, &values, &size);

        if (size) {
                memcpy(public_address, values, BD_ADDR_LEN);
                return;
        }
        bool valid;

        valid = ad_ble_read_nvms_param(public_address, BD_ADDR_LEN, NVPARAM_BLE_PLATFORM_BD_ADDRESS,
                NVPARAM_OFFSET_BLE_PLATFORM_BD_ADDRESS);
        if (!valid) {
                memcpy(public_address, &default_addr, BD_ADDR_LEN);
        }
}

static void check_and_enable_ble_sleep(void)
{
#if (USE_BLE_SLEEP == 1)
        if (ble_stack_initialized) {
                ad_ble_update_wakeup_time();
                cmac_dynamic_config_table_ptr->sleep_enable = !stay_active;
        }
#endif /* USE_BLE_SLEEP */
}

/**
 * \brief Main BLE Interrupt and event queue handling task
 */
static void ad_ble_task(void *pvParameters)
{
        ad_ble_hdr_t *received_msg;
        uint32_t ulNotifiedValue;
        OS_BASE_TYPE xResult __UNUSED;
        int ret;
        int8_t wdog_id;

        #if (dg_configRF_ENABLE_RECALIBRATION == 1)
        /* Initialize SYS ADC for RF calibration */
        sys_adc_config(&TEMP_SENSOR_RADIO_INTERNAL, ad_ble_notify_rf_calibration, &rf_calib_par);
        #endif /* (dg_configRF_ENABLE_RECALIBRATION == 1) */

        /* Register task to be monitored by watch dog. */
        wdog_id = sys_watchdog_register(false);

        DBG_SET_HIGH(BLE_ADAPTER_DEBUG, BLEBDG_ADAPTER); // Debug LED active (i.e. not sleeping)

        /* Run BLE stack internal scheduler once before entering task's main function. */
        rwip_schedule();

        sleep_for_ever = true;

        for (;;) {
                /* Notify watch dog on each loop since there's no other trigger for this. */
                sys_watchdog_notify(wdog_id);

                /* Suspend monitoring while task is blocked on OS_TASK_NOTIFY_WAIT(). */
                sys_watchdog_suspend(wdog_id);

                /*
                 * Wait on any of the event group bits, then clear them all.
                 */
                xResult = OS_TASK_NOTIFY_WAIT(0x0, OS_TASK_NOTIFY_ALL_BITS, &ulNotifiedValue,
                                                                            OS_TASK_NOTIFY_FOREVER);
                /* Guaranteed to succeed since we're waiting forever for the notification */
                OS_ASSERT(xResult == OS_OK);

                /* Resume watch dog monitoring. */
                sys_watchdog_notify_and_resume(wdog_id);

                /* Check if CMAC is active */
                if (ulNotifiedValue & mainBIT_BLE_CMAC_IRQ) {
                        /* Update sleep_status */
                        sleep_status = BLE_ACTIVE;
                }

                #if (dg_configRF_ENABLE_RECALIBRATION == 1)
                if (ulNotifiedValue & mainBIT_TEMP_MONITOR_ENABLE) {
                        /* Enable temperature monitoring */
                        ad_ble_temp_meas_enabled = true;
                        sys_adc_enable();
                }

                if (ulNotifiedValue & mainBIT_TEMP_MONITOR_DISABLE) {
                        /* Disable temperature monitoring */
                        sys_adc_disable();
                        ad_ble_temp_meas_enabled = false;
                }

                if (ulNotifiedValue & mainBIT_BLE_RF_CALIBRATION) {
                        /* Re-set temperature monitoring bounds */
                        uint16_t adc_val = sys_adc_get_value();
                        sys_adc_set_bounds(adc_val);

                        /* Send command to controller to perform RF calibration */
                        sys_rf_calibration_start();
                }
                #endif /* (dg_configRF_ENABLE_RECALIBRATION == 1) */

                /* Check if there is a BLE stack write pending */
                if (ulNotifiedValue & mainBIT_BLE_WRITE_PEND) {
                        /* Perform deferred write */
                        ad_ble_stack_write(ad_ble_stack_wr_buf_p, ad_ble_stack_wr_size,
                                           ad_ble_stack_wr_cb);
                }

                /* Check if we should call the previously skipped TX done callback */
                if (ulNotifiedValue & mainBIT_EVENT_QUEUE_AVAIL) {
                        sleep_status = BLE_ACTIVE;
                        if (ad_ble_stack_wr_cb && uxQueueSpacesAvailable(adapter_if.evt_q)) {
                                /* Call pending BLE stack write callback */
                                ad_ble_stack_wr_cb(BLE_STACK_IO_OK);

                                ble_mgr_notify_adapter_blocked(false);
                        }
                }

#if (dg_configBLE_ADV_STOP_DELAY_ENABLE == 1)
                if (ulNotifiedValue & mainBIT_EVENT_ADV_END) {
                        struct delayed_msg *d_msg;

                        /* Don't delay next commands. */
                        waiting_for_evt = false;

                        /* Disable end of advertising event notifications. */
                        rwble_evt_end_adv_ntf_set(false);

                        /* Run stack scheduler. */
                        rwip_schedule();

                        /* Send delayed messages to stack. */
                        do {
                                d_msg = list_pop_back(&delayed_list);

                                if (d_msg) {
                                        /* Send delayed message to stack */
                                        ad_ble_send_to_stack(d_msg->msg);

                                        /* Free previously allocated message buffer. */
                                        OS_FREE(d_msg->msg);

                                        /* Free allocated list element. */
                                        OS_FREE(d_msg);
                                }
                        } while (d_msg != NULL);
                }
#endif

                if (ulNotifiedValue & mainBIT_COMMAND_QUEUE) {
                        /* The message may have already been read in the while () loop below! */
                        if ( OS_QUEUE_GET(adapter_if.cmd_q, &received_msg, 0)) {
                                /* Make sure a valid OP CODE is received */
                                OS_ASSERT(received_msg->op_code < AD_BLE_OP_CODE_LAST);
                                current_op = received_msg->op_code;

                                if (current_op == AD_BLE_OP_CODE_STACK_MSG) {
                                        /* Send message to BLE stack */
                                        ble_mgr_common_stack_msg_t *stack_msg = (ble_mgr_common_stack_msg_t *) received_msg;
                                                ad_ble_handle_stack_msg(stack_msg);
                                }
                                else if (current_op == AD_BLE_OP_CODE_ADAPTER_MSG) {
                                        ad_ble_handle_adapter_msg((ad_ble_msg_t *) received_msg);

                                        /* Free previously allocated message buffer. */
                                        OS_FREE(received_msg);
                                }

                                if (sleep_status == BLE_SLEEPING) {
                                        ad_ble_wake_up();
                                }
                        }
                }

                if (ulNotifiedValue & mainBIT_EVENT_LPCLOCK_AVAIL) {
                     /* LP Clock is available, check if BLE sleep is possible */
                        check_and_enable_ble_sleep();
                }
                if (ulNotifiedValue & mainBIT_STAY_ACTIVE_UPDATED) {
                        /* BLE's stay_active status updated */
                        if (ble_stack_initialized) {
                                cmac_dynamic_config_table_ptr->sleep_enable = !stay_active;
                        }
                        if (stay_active && (sleep_status == BLE_SLEEPING)) {
                                ad_ble_wake_up();
                        }
                }
                if (ulNotifiedValue & mainBIT_BLE_TIMER_EXPIRED) {
                        /* The Host should process the timeout, update sleep_status */
                        sleep_status = BLE_ACTIVE;

                        ke_event_set(KE_EVENT_KE_TIMER);
                }
                ret = 0;

                /* Run this loop as long as BLE is active and there are pending BLE actions */
                while ((sleep_status == BLE_ACTIVE) && (ret != -1)) {
                        /* Run the BLE stack internal scheduler. */
                        rwip_schedule();


                        /* Check command queue for incoming messages */
                        if (uxQueueMessagesWaiting(adapter_if.cmd_q)) {
                                /* Get message from the command queue. */
                                OS_QUEUE_GET(adapter_if.cmd_q, &received_msg, 0);
                                /* Make sure a valid op code is received */
                                OS_ASSERT(received_msg->op_code < AD_BLE_OP_CODE_LAST);

                                /* Save message's OP code. */
                                current_op = received_msg->op_code;

                                if (current_op == AD_BLE_OP_CODE_STACK_MSG) {
                                        /* Send message to BLE stack */
                                        ble_mgr_common_stack_msg_t *stack_msg = (ble_mgr_common_stack_msg_t *) received_msg;
                                                ad_ble_handle_stack_msg(stack_msg);
                                }
                                else if (current_op == AD_BLE_OP_CODE_ADAPTER_MSG) {
                                        ad_ble_handle_adapter_msg((ad_ble_msg_t *) received_msg);

                                        /* Free previously allocated message buffer. */
                                        OS_FREE(received_msg);
                                }
                        }
                        else if ( (USE_BLE_SLEEP == 1) && (cm_lp_clk_is_avail()) && (!stay_active) ) {
                                /* Sleep is possible only when the LP clock is ready! */
                                ret = sleep_when_possible();
                        }
                        else if (ble_block()) {
                                /* There are no pending BLE actions, so exit the while () loop. */
                                ret = -1;
                        }

#if ((BLE_WINDOW_STATISTICS == 1) && (stat_runs == WINSTAT_LOG_THRESHOLD))
                        log_printf(LOG_NOTICE, 2,
                                "sca:{M=%d, S=%d, dft=%d}, sync=%4d, type=%4d, len=%4d, crc=%4d, evt=%5d, zero=%3d, pos=%5d (%5d), neg=%5d (%5d)\r\n",
                                mst_sca, slv_sca, sca_drift, sync_errors, type_errors,
                                len_errors, crc_errors, diff_events, diff_zero, diff_pos,
                                max_pos_diff, diff_neg, max_neg_diff);

                        stat_runs = 0;
#endif

                        /* Now is a good time to notify the watch dog. */
                        sys_watchdog_notify(wdog_id);
                }
        }
}

/**
 * \brief Initialization function of BLE adapter
 */
void ad_ble_init(void)
{
        // BLE ROM variables initialization
        ble_platform_initialization();

        OS_QUEUE_CREATE(adapter_if.cmd_q, sizeof(ble_mgr_common_stack_msg_t *), AD_BLE_COMMAND_QUEUE_LENGTH);
        OS_QUEUE_CREATE(adapter_if.evt_q, sizeof(ble_mgr_common_stack_msg_t *), AD_BLE_EVENT_QUEUE_LENGTH);

        OS_ASSERT(adapter_if.cmd_q);
        OS_ASSERT(adapter_if.evt_q);

#if dg_configNVPARAM_ADAPTER
        /* Open BLE NV-Parameters - area name is defined in platform_nvparam.h */
        ble_parameters = ad_nvparam_open("ble_platform");
#endif

        // create FreeRTOS task
        OS_TASK_CREATE("bleA",                     // Text name assigned to the task
                       ad_ble_task,                // Function implementing the task
                       NULL,                       // No parameter passed
                       mainBLE_TASK_STACK_SIZE,    // Size of the stack to allocate to task
                       mainBLE_TASK_PRIORITY,      // Priority of the task
                       adapter_if.task);           // No task handle

        OS_ASSERT(adapter_if.task);

        DBG_CONFIGURE_LOW(BLE_ADAPTER_DEBUG, BLEBDG_ADAPTER); /* led (on: active, off: sleeping) */

        read_public_address();

#ifdef BLE_STACK_PASSTHROUGH_MODE
        /* Initialize BLE stack */
        ble_stack_init();
#endif /* BLE_STACK_PASSTHROUGH_MODE */
}

static void ad_ble_handle_stack_msg(ble_mgr_common_stack_msg_t *msg)
{
#if (dg_configBLE_ADV_STOP_DELAY_ENABLE == 1)
        if (waiting_for_evt) {
                struct delayed_msg *d_msg = OS_MALLOC(sizeof(*d_msg));

                d_msg->msg = msg;

                /* Add command to delayed command queue. */
                list_add(&delayed_list, d_msg);
        }
        else if (advertising && (msg->msg_type == GTL_MSG)
                && (msg->msg.gtl.msg_id == GAPM_CANCEL_CMD)) {
                struct delayed_msg *d_msg = OS_MALLOC(sizeof(*d_msg));

                d_msg->msg = msg;

                /* Set wait for event flag. */
                waiting_for_evt = true;

                /* Enable EVENT END notification.*/
                rwble_evt_end_adv_ntf_set(true);

                /* Add command to delayed command queue. */
                list_add(&delayed_list, d_msg);
        }
        else
#endif
        {
                /* Send message to stack. */
                ad_ble_send_to_stack(msg);

                /* Free previously allocated message buffer. */
                OS_FREE(msg);
        }
}

static void ad_ble_handle_adapter_msg(ad_ble_msg_t *msg)
{
        adapter_op = msg->operation;

        switch (msg->operation) {
        // Only handle initialization command for now
        case AD_BLE_OP_INIT_CMD:
        {
                /* Initialize BLE stack */
                ble_stack_init();
                break;
        }
        case AD_BLE_OP_RESET_CMD:
        {
                ble_stack_initialized = false;
                /* Reset BLE controller, GAPM_RESET_CMD will be sent upon reception of
                 * GAPM_DEVICE_READY_IND */
                ble_controller_reset();
                break;
        }
        default:
                break;
        }
}

void ad_ble_send_to_stack(const ble_mgr_common_stack_msg_t *ptr_msg)
{
        ble_stack_msg_type_t msg_type = ptr_msg->msg_type;
        uint16_t msgSize = ptr_msg->hdr.msg_len + sizeof(uint8_t);
        uint8_t *msgPtr = (uint8_t *) &ptr_msg->msg;
#if (dg_configBLE_ADV_STOP_DELAY_ENABLE == 1)
        ble_gtl_msg_t *gtl_msg = (ble_gtl_msg_t *) &(ptr_msg->msg.gtl);

        if ((msg_type == GTL_MSG) && (gtl_msg->msg_id == GAPM_START_ADVERTISE_CMD)) {
                struct gapm_start_advertise_cmd *cmd =
                                                 (struct gapm_start_advertise_cmd *) gtl_msg->param;
                if ((cmd->op.code >= GAPM_ADV_NON_CONN) && (cmd->op.code <= GAPM_ADV_DIRECT_LDC)) {
                        /* Set advertising flag. */
                        advertising = true;
                }
        }
#endif

        if (ad_ble_stack_rd_buf_p != NULL) {
                // Indicate message type to BLE stack
                *ad_ble_stack_rd_buf_p++ = msg_type;
                // Decrement message size
                msgSize--;

                // Call the BLE stack to decide on the message type
                if (ad_ble_stack_rd_cb != NULL) {
                        ad_ble_stack_rd_cb(BLE_STACK_IO_OK);
                }
                else
                {
                        ASSERT_ERROR(0);
                }

                // Continue sending the message
                while (msgSize) {
                        memcpy(ad_ble_stack_rd_buf_p, msgPtr, ad_ble_stack_rd_size);
                        msgSize -= ad_ble_stack_rd_size;
                        msgPtr += ad_ble_stack_rd_size;
                        ad_ble_stack_rd_cb(BLE_STACK_IO_OK);
                }
        }
}

void ad_ble_stack_write(uint8_t* bufPtr, uint32_t size, void (*callback) (uint8_t))
{
        /* Sanity checks */
        OS_ASSERT(bufPtr != NULL);
        OS_ASSERT(size != 0);
        OS_ASSERT(callback != NULL);

        /* Get msg id - bufPtr points to a packed message */
        uint16_t stack_msg_id = *( bufPtr +1 ) + ( *( bufPtr + 2) << 8 );

#ifndef BLE_STACK_PASSTHROUGH_MODE
        if (current_op == AD_BLE_OP_CODE_ADAPTER_MSG)
        {

                switch (stack_msg_id) {
                case GAPM_DEVICE_READY_IND:
                {
                        // Notify the stack that the message has been consumed
                        callback(BLE_STACK_IO_OK);

                        /* Send GAPM_RESET_CMD to properly initialize the stack */
                        ble_stack_reset();

                        break;
                }
                case GAPM_CMP_EVT:
                {
                        ad_ble_msg_t *ad_msg;
                        ad_ble_cmp_evt_t *ad_evt;

                        /* BLE stack has been initialized */
                        ble_stack_initialized = true;

                        /* Apply TCS settings */
                        ad_ble_sys_tcs_config();
                        ad_ble_synth_tcs_config();
                        ad_ble_rfcu_tcs_config();
#if (USE_BLE_SLEEP == 1)
                        ad_ble_update_wakeup_time();
                        #if (dg_configUSE_LP_CLK == LP_CLK_RCX)
                        ad_ble_update_rcx();
                        cmac_dynamic_config_table_ptr->sleep_enable = !stay_active;
                        #endif /* (dg_configUSE_LP_CLK == LP_CLK_RCX) */
#endif /* (USE_BLE_SLEEP == 1) */

                        /* Make sure the reset was completed successfully */
                        OS_ASSERT( * ( bufPtr + 9 ) == GAPM_RESET );
                        OS_ASSERT( * ( bufPtr + 10 ) == GAP_ERR_NO_ERROR );

                        /* Create and send an AD_BLE_CMP_EVT */
                        ad_msg = ble_ad_msg_alloc(AD_BLE_OP_CMP_EVT, sizeof(ad_ble_cmp_evt_t));
                        ad_evt = (ad_ble_cmp_evt_t *) ad_msg->param;
                        ad_evt->op_req = adapter_op;
                        ad_evt->status = AD_BLE_STATUS_NO_ERROR;

                        ad_ble_event_queue_send(&ad_msg, OS_QUEUE_FOREVER);

                        // Notify the stack that the message has been consumed
                        callback(BLE_STACK_IO_OK);

                        break;
                }
                default:
                        break;
                }
        }
        else if (current_op == AD_BLE_OP_CODE_STACK_MSG)
#else
        if (stack_msg_id == GAPM_DEVICE_READY_IND) {
                /* The stack has been initialized */
                ble_stack_initialized = true;

                /* Apply TCS settings */
                ad_ble_sys_tcs_config();
                ad_ble_synth_tcs_config();
                ad_ble_rfcu_tcs_config();
#if (USE_BLE_SLEEP == 1)
                ad_ble_update_wakeup_time();
                #if (dg_configUSE_LP_CLK == LP_CLK_RCX)
                ad_ble_update_rcx();
                cmac_dynamic_config_table_ptr->sleep_enable = !stay_active;
                #endif /* (dg_configUSE_LP_CLK == LP_CLK_RCX) */
#endif /* (USE_BLE_SLEEP == 1) */
        }
#endif /* BLE_STACK_PASSTHROUGH_MODE */
        {
                /* Check if the write operation needs to be deferred */
                if (in_interrupt()) {
                        ad_ble_stack_wr_buf_p = bufPtr;
                        ad_ble_stack_wr_size = size;
                        ad_ble_stack_wr_cb = callback;

                        OS_TASK_NOTIFY_FROM_ISR(adapter_if.task, mainBIT_BLE_WRITE_PEND,
                                                                                OS_NOTIFY_SET_BITS);

                        return;
                }

                // Get msg id - bufPtr points to a packed message
                uint16_t stack_msg_id = *(bufPtr+1) + (*(bufPtr+2)<<8);

                if (!ble_stack_initialized && (stack_msg_id == GAPM_CMP_EVT)) {
                        /* Reset of the GAP layer has been completed */

                        /* The stack has been initialized */
                        ble_stack_initialized = true;

                        /* Make sure the reset was completed successfully */
                        OS_ASSERT(*(bufPtr+9)==GAPM_RESET);
                        OS_ASSERT(*(bufPtr+10)==GAP_ERR_NO_ERROR);

                        // Notify the stack that the message has been consumed
                        callback(BLE_STACK_IO_OK);
                } else {
                        ble_mgr_common_stack_msg_t* msgBuf = NULL;
                        uint8_t *pxMsgPacked = bufPtr;
                        uint16_t param_length;
                        uint8_t header_length;

                        // Sanity check
                        ASSERT_ERROR(bufPtr != NULL);
                        ASSERT_ERROR(size != 0);

                        // Extract message parameter length in bytes
                        switch (*pxMsgPacked) {
                        case BLE_HCI_CMD_MSG:
                                param_length = *(pxMsgPacked + HCI_CMD_PARAM_LEN_OFFSET);
                                header_length = HCI_CMD_HEADER_LENGTH;
                                break;

                        case BLE_HCI_ACL_MSG:
                                param_length = *(pxMsgPacked + HCI_ACL_PARAM_LEN_OFFSET) +
                                ( *(pxMsgPacked + HCI_ACL_PARAM_LEN_OFFSET + 1) << 8 );
                                header_length = HCI_ACL_HEADER_LENGTH;
                                break;

                        case BLE_HCI_SCO_MSG:
                                param_length = *(pxMsgPacked + HCI_SCO_PARAM_LEN_OFFSET);
                                header_length = HCI_SCO_HEADER_LENGTH;
                                break;

                        case BLE_HCI_EVT_MSG:
                                param_length = *(pxMsgPacked + HCI_EVT_PARAM_LEN_OFFSET);
                                header_length = HCI_EVT_HEADER_LENGTH;
                                break;

                        case BLE_GTL_MSG:
                                param_length = *(pxMsgPacked + GTL_MSG_PARAM_LEN_OFFSET) +
                                ( *(pxMsgPacked + GTL_MSG_PARAM_LEN_OFFSET+1) << 8 );
                                header_length = GTL_MSG_HEADER_LENGTH;
                                break;

                        default:
                                /* Call the ble-stack TX done callback with error status */
                                callback(BLE_STACK_IO_ERROR);

                                /* The message should be either HCI or GTL */
                                ASSERT_ERROR(0);

                                return;
                        }

                        // Allocate the space needed for the message
                        msgBuf = OS_MALLOC(sizeof(ble_mgr_common_stack_msg_t) + param_length);

                        msgBuf->hdr.op_code = BLE_MGR_COMMON_STACK_MSG;     // fill message OP code
                        msgBuf->msg_type = *pxMsgPacked++;                  // fill stack message type
                        msgBuf->hdr.msg_len = header_length + param_length; // fill stack message length

                        // copy the rest of the message
                        memcpy(&msgBuf->msg, pxMsgPacked, msgBuf->hdr.msg_len);

#if (dg_configBLE_ADV_STOP_DELAY_ENABLE == 1)
                        if (msgBuf->msg.gtl.msg_id == GAPM_CMP_EVT) {
                                struct gapm_cmp_evt *evt = (struct gapm_cmp_evt *) msgBuf->msg.gtl.param;

                                if ((evt->operation >= GAPM_ADV_NON_CONN)
                                        && (evt->operation <= GAPM_ADV_DIRECT_LDC)) {
                                        /* Set advertising flag. */
                                        advertising = false;

                                        /* Notify adapter because no ADV event is expected. */
                                        OS_TASK_NOTIFY(adapter_if.task, mainBIT_EVENT_ADV_END,
                                                OS_NOTIFY_SET_BITS);
                                }
                        }
#endif /* (dg_configBLE_ADV_STOP_DELAY_ENABLE == 1) */

                        /* Post item to queue. */
                        if (ad_ble_event_queue_send(&msgBuf, 0) == OS_OK) {
                                /* Check free space on BLE adapter's event queue. */
                                if (uxQueueSpacesAvailable(adapter_if.evt_q)) {
                                        /* Call BLE stack I/O TX done callback right away. */
                                        callback(BLE_STACK_IO_OK);
                                } else {
                                        /*
                                         * Save pointer of BLE stack write callback to be called
                                         * when there is free space on the BLE adapter's event queue.
                                         */
                                        ad_ble_stack_wr_cb = callback;

                                        /*
                                         * Notify BLE manager that the adapter has blocked on a full
                                         * event queue. BLE manager will notify the adapter when
                                         * there is free space in the event queue.
                                         */
                                        ble_mgr_notify_adapter_blocked(true);
                                }
                        }
                        else {
                                /* The following line should never be reached! */
                                ASSERT_ERROR(0);
                        }
                }
        }
}

void ad_ble_stack_read(uint8_t* bufPtr, uint32_t size, void (*callback) (uint8_t))
{
        GLOBAL_INT_DISABLE();
        ad_ble_stack_rd_buf_p = bufPtr;
        ad_ble_stack_rd_size = size;
        ad_ble_stack_rd_cb = callback;
        GLOBAL_INT_RESTORE();
}

const ad_ble_interface_t *ad_ble_get_interface()
{
        return &adapter_if;
}

OS_BASE_TYPE ad_ble_event_queue_register(const OS_TASK task_handle)
{
        // Set event queue task handle
        mgr_task = task_handle;

        return OS_OK;
}

void ad_ble_stack_flow_on(void)
{
}

bool ad_ble_stack_flow_off(void)
{
        return true;
}

void ad_ble_get_public_address(uint8_t address[BD_ADDR_LEN])
{
        memcpy(address, public_address, BD_ADDR_LEN);
}

void ad_ble_get_irk(uint8_t irk[KEY_LEN])
{
        uint8_t default_irk[KEY_LEN] = defaultBLE_IRK;

        bool valid;

        valid = ad_ble_read_nvms_param(irk, KEY_LEN, NVPARAM_BLE_PLATFORM_IRK,
                                                        NVPARAM_OFFSET_BLE_PLATFORM_IRK);
        if (!valid) {
                memcpy(irk, &default_irk, KEY_LEN);
        }
}

#if (dg_configNVPARAM_ADAPTER == 1)

nvparam_t ad_ble_get_nvparam_handle(void)
{
        return ble_parameters;
}
#if (dg_configPMU_ADAPTER == 1)
ADAPTER_INIT_DEP2(ad_ble_adapter, ad_ble_init, ad_pmu_adapter, ad_nvparam_adapter);
#else
ADAPTER_INIT_DEP1(ad_ble_adapter, ad_ble_init, ad_nvparam_adapter);
#endif /* dg_configPMU_ADAPTER */

#elif (dg_configNVMS_ADAPTER == 1)

#if (dg_configPMU_ADAPTER == 1)
ADAPTER_INIT_DEP2(ad_ble_adapter, ad_ble_init, ad_pmu_adapter, ad_nvms_adapter);
#else
ADAPTER_INIT_DEP1(ad_ble_adapter, ad_ble_init, ad_nvms_adapter);
#endif /* dg_configPMU_ADAPTER */

#else

#if (dg_configPMU_ADAPTER == 1)
ADAPTER_INIT_DEP1(ad_ble_adapter, ad_ble_init, ad_pmu_adapter);
#else
ADAPTER_INIT(ad_ble_adapter, ad_ble_init);
#endif /* dg_configPMU_ADAPTER */

#endif /* dg_configNVPARAM_ADAPTER */

void ad_ble_stay_active(bool status)
{
        stay_active = status;
        OS_TASK_NOTIFY(adapter_if.task, mainBIT_STAY_ACTIVE_UPDATED, OS_NOTIFY_SET_BITS);
}
#endif /* CONFIG_USE_BLE */

void cmac2sys_notify()
{
        OS_TASK_NOTIFY_FROM_ISR(adapter_if.task, mainBIT_BLE_CMAC_IRQ, OS_NOTIFY_SET_BITS);
}

#if ( dg_configSYSTEMVIEW == 1 )
void cmac2sys_isr_enter()
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();
}

void cmac2sys_isr_exit()
{
        SEGGER_SYSTEMVIEW_ISR_EXIT();
}
#endif /* ( dg_configSYSTEMVIEW == 1 ) */

/*
 * @brief BLE timer callback
 */
void ble_timer_callback(void* varg)
{
        OS_TASK_NOTIFY(adapter_if.task, mainBIT_BLE_TIMER_EXPIRED, OS_NOTIFY_SET_BITS);
}

#if (USE_BLE_SLEEP == 1)

void ad_ble_update_wakeup_time(void)
{
        if (ble_stack_initialized) {
                uint16_t value, prev_value;
                value = pm_get_sys_wakeup_cycles();

                GLOBAL_INT_DISABLE();
                while (hw_sys_hw_bsr_try_lock(HW_BSR_MASTER_SYSCPU, HW_BSR_WAKEUP_CONFIG_POS) == false);
                prev_value = cmac_dynamic_config_table_ptr->wakeup_time;
#if (dg_configUSE_LP_CLK == LP_CLK_RCX)
                value += XTALRDY_CYCLES_TO_LP_CLK_CYCLES(hw_clk_get_xtalm_settling_time(), rcx_clock_hz);
#else
                value += XTALRDY_CYCLES_TO_LP_CLK_CYCLES(hw_clk_get_xtalm_settling_time(), dg_configXTAL32K_FREQ);
#endif
                if (value != prev_value) {
                        cmac_dynamic_config_table_ptr->wakeup_time = value;
                }
                hw_sys_hw_bsr_unlock(HW_BSR_MASTER_SYSCPU, HW_BSR_WAKEUP_CONFIG_POS);
                GLOBAL_INT_RESTORE();
                if (value  > prev_value) {
                        /*
                         * The wake-up time has been increased. Wake-up CMAC to re-calculate the sleep time.
                         */
                        ble_force_wakeup();
                }
        }
}
#endif /* USE_BLE_SLEEP */

void ad_ble_sys_tcs_config(void)
{
        if (ble_stack_initialized) {
                uint8_t i;
                hw_sys_reg_config_t *source;
                hw_sys_reg_config_t *dest = (hw_sys_reg_config_t *)cmac_sys_tcs_table_ptr;
                uint32_t num_of_entries = *hw_sys_reg_get_num_of_config_entries();

                ASSERT_ERROR(num_of_entries <= cmac_system_tcs_length);

                cmac_config_table_ptr->system_tcs_length = 0;

                for (i = 0; i < num_of_entries && i < cmac_system_tcs_length; i++) {

                        source = hw_sys_reg_get_config(i);
                        dest[i].value = source->value;

                        /* Address must be written after value to prevent race condition */
                        dest[i].addr = source->addr;
                }
                cmac_config_table_ptr->system_tcs_length = num_of_entries;
        }
}

void ad_ble_synth_tcs_config(void)
{
        if (ble_stack_initialized) {
                uint8_t i;
                hw_sys_reg_config_t *source;
                hw_sys_reg_config_t *dest = (hw_sys_reg_config_t *)cmac_synth_tcs_table_ptr;
                uint8_t num_of_entries;

                sys_tcs_get_reg_pairs(SYS_TCS_GROUP_PD_SYNTH, (uint32_t **)&source, &num_of_entries);

                /* num_of_entries should be divided by 2
                 * entries in TCS array are 32 bit but in hw_sys_reg_config_t are 64 bits*/
                num_of_entries = num_of_entries / 2 ;

                ASSERT_ERROR(num_of_entries <= cmac_synth_tcs_length);

                cmac_config_table_ptr->synth_tcs_length = 0;
                for (i = 0; i < num_of_entries && i < cmac_synth_tcs_length; i++) {
                        ASSERT_ERROR(source != NULL);
                        dest[i].value = source[i].value;
                        /* Address must be written after value to prevent race condition */
                        dest[i].addr = source[i].addr;
                }
                cmac_config_table_ptr->synth_tcs_length = i;
        }
}

void ad_ble_rfcu_tcs_config(void)
{
        if (ble_stack_initialized) {
                uint8_t i;
                hw_sys_reg_config_t *source;
                hw_sys_reg_config_t *dest = (hw_sys_reg_config_t *)cmac_rfcu_tcs_table_ptr;
                uint8_t num_of_entries;
                uint32_t second_harmonic_0dbm = 0;
                uint32_t second_harmonic_6dbm = 0;
                uint32_t default_harmonic = 0;

                sys_tcs_get_reg_pairs(SYS_TCS_GROUP_PD_RAD, (uint32_t **)&source, &num_of_entries);

                /* num_of_entries should be divided by 2
                 * entries in TCS array are 32 bit but in hw_sys_reg_config_t are 64 bits*/
                num_of_entries = num_of_entries / 2 ;

                ASSERT_ERROR(num_of_entries <= cmac_rfcu_tcs_length);

                cmac_config_table_ptr->rfcu_tcs_length = 0;
                for (i = 0; i < num_of_entries && i < cmac_rfcu_tcs_length; i++) {
                        ASSERT_ERROR(source != NULL);
                        if (source[i].addr == ((volatile uint32_t *)0x4002004C)) {
                                default_harmonic = source[i].value;
                        }
                        dest[i].value = source[i].value;
                        /* Address must be written after value to prevent race condition */
                        dest[i].addr = source[i].addr;
                }
                cmac_config_table_ptr->rfcu_tcs_length = i;

                /* Second harmonic from RAD_MODE1*/
                sys_tcs_get_reg_pairs(SYS_TCS_GROUP_PD_RAD_MODE1, (uint32_t **)&source, &num_of_entries);


                for (i = 0; i < num_of_entries/2; i++) {
                        ASSERT_ERROR(source != NULL);
                        if (source[i].addr == ((volatile uint32_t *)0x4002004C)) {
                                second_harmonic_0dbm = source[i].value;
                        }
                }
                /* Second harmonic from RAD_MODE2*/
                sys_tcs_get_reg_pairs(SYS_TCS_GROUP_PD_RAD_MODE2, (uint32_t **)&source, &num_of_entries);

                for (i = 0; i < num_of_entries/2; i++) {
                        ASSERT_ERROR(source != NULL);
                        if (source[i].addr == ((volatile uint32_t *)0x4002004C)) {
                                second_harmonic_6dbm = source[i].value;

                        }
                }

                if (!second_harmonic_0dbm || !second_harmonic_6dbm) {
                        if (!default_harmonic) {
                                default_harmonic = 0x300;
                        }
                        cmac_dynamic_config_table_ptr->tx_0dbm_2ndharm_trim = default_harmonic;
                        cmac_dynamic_config_table_ptr->tx_6dbm_2ndharm_trim = default_harmonic;
                }
                else {
                        cmac_dynamic_config_table_ptr->tx_0dbm_2ndharm_trim = second_harmonic_0dbm;
                        cmac_dynamic_config_table_ptr->tx_6dbm_2ndharm_trim = second_harmonic_6dbm;
                }


        }
}

void ad_ble_get_lld_stats(struct ad_ble_lld_stats *stats)
{
        if (stats) {
                memset(stats, 0, sizeof(struct ad_ble_lld_stats));

                if (cmac_info_table_ptr) {
                        for (int conhdl = 0; conhdl < BLE_CONNECTION_MAX_USER; conhdl++) {
                                 uint16_t conidx = gapc_get_conidx(conhdl);
                                 if ((conidx != GAP_INVALID_CONIDX) && (conidx < BLE_CONNECTION_MAX_USER)) {
                                         stats->conn_evt_counter_non_apfm[conidx] = cmac_info_table_ptr->ble_conn_evt_counter_non_apfm[conhdl];
                                         stats->conn_evt_counter[conidx] = cmac_info_table_ptr->ble_conn_evt_counter[conhdl];
                                 }
                        }

                        stats->adv_evt_counter_non_apfm = cmac_info_table_ptr->ble_adv_evt_counter_non_apfm;
                        stats->adv_evt_counter = cmac_info_table_ptr->ble_adv_evt_counter;
                }
        }
}

#if (USE_BLE_SLEEP == 1)
#if (dg_configUSE_LP_CLK == LP_CLK_RCX)
void ad_ble_update_rcx(void)
{
        if (ble_stack_initialized) {
                /* Put the new RCX values into the dynamic configuration table */
                cmac_dynamic_config_table_ptr->rcx_period = cm_get_rcx_clock_period();
                cmac_dynamic_config_table_ptr->rcx_clock_hz_acc = cm_get_rcx_clock_hz_acc();

                /*
                 * Wake up CMAC to pick up the new values in case:
                 *   - it is sleeping or
                 *   - it is on its way to enter sleep mode
                 */
                ble_force_wakeup();
        }
}
#endif /* (USE_BLE_SLEEP == 1) */
#endif /* (dg_configUSE_LP_CLK == LP_CLK_RCX) */

