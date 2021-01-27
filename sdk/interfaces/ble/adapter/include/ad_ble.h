/**
 * \addtogroup MID_INT_BLE_ADAPTER
 * \{
 * \addtogroup BLE_ADAPTER_API API
 *
 * \brief BLE Adapter API
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_ble.h
 *
 * @brief BLE Adapter API
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef AD_BLE_H
#define AD_BLE_H

#include "osal.h"

#include "ad_ble_config.h"

#include "ad_nvparam.h"
#include "ble_stack_config.h"
#include "co_bt.h"


/* Event group bits */
#define mainBIT_BLE_GEN_IRQ             (1 << 0)
#define mainBIT_COMMAND_QUEUE           (1 << 1)
#define mainBIT_EVENT_QUEUE_AVAIL       (1 << 2)
#define mainBIT_EVENT_LPCLOCK_AVAIL     (1 << 3)
#define mainBIT_STAY_ACTIVE_UPDATED     (1 << 4)
#define mainBIT_BLE_CMAC_IRQ            (1 << 5)
#define mainBIT_BLE_TIMER_EXPIRED       (1 << 6)
#define mainBIT_BLE_WRITE_PEND          (1 << 7)
#define mainBIT_TEMP_MONITOR_ENABLE     (1 << 8)
#define mainBIT_TEMP_MONITOR_DISABLE    (1 << 9)
#define mainBIT_BLE_RF_CALIBRATION      (1 << 10)
#if (dg_configBLE_ADV_STOP_DELAY_ENABLE == 1)
#define mainBIT_EVENT_ADV_END           (1 << 31)
#endif

///Kernel message header length for transport through interface between App and SW stack.
#define HCI_CMD_HEADER_LENGTH      3
#define HCI_ACL_HEADER_LENGTH      4
#define HCI_SCO_HEADER_LENGTH      3
#define HCI_EVT_HEADER_LENGTH      2
#define GTL_MSG_HEADER_LENGTH      8

#define HCI_CMD_PARAM_LEN_OFFSET   3
#define HCI_ACL_PARAM_LEN_OFFSET   3
#define HCI_SCO_PARAM_LEN_OFFSET   3
#define HCI_EVT_PARAM_LEN_OFFSET   2
#define GTL_MSG_PARAM_LEN_OFFSET   7

#define HCI_RESET_CMD_OP_CODE      (0x0C03)

// Maximum wait time for BLE stack configuration operations
#define MAX_WAIT_TIME  portMAX_DELAY

// Operations for BLE adapter messages
typedef enum ad_ble_op_codes {
        AD_BLE_OP_CODE_STACK_MSG      = 0x00,
        AD_BLE_OP_CODE_ADAPTER_MSG    = 0x01,
        /*
         * last command ID
         * make sure above are sorted in ascending order, otherwise this one will be incorrect!
         */
        AD_BLE_OP_CODE_LAST,
} ad_ble_op_code_t;

// Operations for BLE adapter messages
typedef enum ad_ble_operations {
        AD_BLE_OP_CMP_EVT     = 0x00,
        AD_BLE_OP_INIT_CMD    = 0x01,
        AD_BLE_OP_RESET_CMD   = 0x02,
        /*
         * last command ID
         * make sure above are sorted in ascending order, otherwise this one will be incorrect!
         */
        AD_BLE_OP_LAST,
} ad_ble_operation_t;

// Statuses for BLE adapter operations
typedef enum ad_ble_statuses {
        AD_BLE_STATUS_NO_ERROR    = 0x00,
        AD_BLE_STATUS_TIMEOUT     = 0x01,
        /*
         * last error code for BLE adapter operations
         * make sure above are sorted in ascending order, otherwise this one will be incorrect!
         */
        BLE_ADAPTER_OP_LAST
} ad_ble_status_t;

// Statuses for BLE stack I/O callback operations
enum ad_ble_stack_statuses {
        BLE_STACK_IO_OK    = 0x00,
        BLE_STACK_IO_ERROR = 0x01,
        /*
         * last error code for BLE stack I/O operations
         * make sure above are sorted in ascending order, otherwise this one will be incorrect!
         */
        BLE_STACK_IO_LAST
};

typedef enum {
        BLE_HCI_CMD_MSG  = 0x01,
        BLE_HCI_ACL_MSG  = 0x02,
        BLE_HCI_SCO_MSG  = 0x03,
        BLE_HCI_EVT_MSG  = 0x04,
        BLE_GTL_MSG      = 0x05,
#ifdef CONFIG_USE_FTDF
        FTDF_DTS_MSG     = 0xAA,
#endif
} ble_msg_type_t;

typedef uint16_t hci_cmd_op_code_t;
typedef uint16_t hci_cmd_op_code_t;

/** HCI command message header format */
typedef struct ble_hci_cmd_hdr {
        uint8_t op_code_l;
        uint8_t op_code_h;
        uint8_t data_length;
} ble_hci_cmd_hdr_t;

/** HCI ACL data message header format */
typedef struct ble_hci_acl_msg_hdr {
        uint16_t handle_flags;
        uint16_t data_length;
} ble_hci_acl_hdr_t;

/** HCI synchronous data message header format */
typedef struct ble_hci_sco_hdr {
        uint16_t conn_handle_flags;
        uint8_t data_length;
} ble_hci_sco_hdr_t;

/** HCI event message header format */
typedef struct ble_hci_evt_hdr {
        uint8_t event_code_;
        uint8_t data_length;
} ble_hci_evt_hdr_t;

/** HCI command message format */
typedef struct hci_cmd_msg {
        uint16_t op_code;
        uint8_t  param_length;
        uint8_t  param[0];
} hci_cmd_msg_t;

/** HCI ACL data message format */
typedef struct hci_acl_msg {
        uint16_t handle_flags;
        uint16_t param_length;
        uint8_t  param[0];
} hci_acl_msg_t;

/** HCI synchronous data message format */
typedef struct hci_sco_msg {
        uint16_t handle_flags;
        uint8_t  param_length;
        uint8_t  param[0];
} hci_sco_msg_t;

/** HCI event message format */
typedef struct hci_evt_msg {
        uint8_t event_code;
        uint8_t param_length;
        uint8_t param[0];
} hci_evt_msg_t;

/** HCI message format */
typedef struct ble_hci_msg {
        union {
                hci_cmd_msg_t cmd;
                hci_acl_msg_t acl;
                hci_sco_msg_t sco;
                hci_evt_msg_t evt;
        };
} ble_hci_msg_t;

/** GTL message format */
typedef struct ble_gtl_msg {
        uint16_t msg_id;
        uint16_t dest_id;
        uint16_t src_id;
        uint16_t param_length;
        uint32_t param[0];
} ble_gtl_msg_t;

/** BLE stack message structure */
typedef struct ble_stack_msg {
        union {
                ble_gtl_msg_t gtl;
                ble_hci_msg_t hci;
        };
} ble_stack_msg_t;

/** BLE adapter message structure */
typedef struct ad_ble_msg {
        uint16_t            op_code;
        uint16_t            msg_size;
        ad_ble_operation_t  operation;
        uint8_t             param[0];
} ad_ble_msg_t;

/** BLE adapter message header structure */
typedef struct ad_ble_hdr {
        uint16_t    op_code;
        uint16_t    msg_size;
        uint8_t     param[0];
} ad_ble_hdr_t;

/** BLE Adapter interface */
typedef struct {
        OS_TASK  task;     /**< BLE Adapter task handle */
        OS_QUEUE cmd_q;    /**< BLE Adapter command queue */
        OS_QUEUE evt_q;    /**< BLE Adapter event queue */
} ad_ble_interface_t;

/**
 * \brief Send a message to the BLE adapter command queue
 *
 * Sends a message to the BLE adapter command queue and notifies the BLE adapter task.
 *
 * \param[in] item       Pointer to the item to be sent to the queue.
 * \param[in] wait_ticks Max time in ticks to wait for space in queue.
 *
 * \return OS_OK if the message was successfully sent to the queue, OS_FAIL otherwise.
 */
OS_BASE_TYPE ad_ble_command_queue_send( const void *item, OS_TICK_TIME wait_ticks);

/**
 * \brief Notify the BLE adapter that LP clock is available
 *
 * This will send a notification to the BLE adapter informing it for the availability of the LP clock.
 * From that moment onwards the BLE stack is allowed to enter the sleep state.
 *
 */
void ad_ble_lpclock_available(void);

/**
 * \brief Send a message to the BLE adapter event queue.
 *
 * Sends a message to the BLE adapter event queue and notifies the registered task.
 *
 * \param[in] item       Pointer to the item to be sent to the queue.
 * \param[in] wait_ticks Max time in ticks to wait for space in queue
 *
 * \return OS_OK if the message was successfully sent to the queue, OS_FAIL otherwise.
 */
OS_BASE_TYPE ad_ble_event_queue_send( const void *item, OS_TICK_TIME wait_ticks);

/**
 * \brief Notify BLE adapter
 *
 * Send a task notification on \p value notification bit(s) to the BLE adapter task.
 *
 * \param[in] value  Notification mask to be set.
 */
void ad_ble_task_notify(uint32_t value);

/**
 * \brief Initialize BLE adapter - create command and event queues.
 *
 */
void ad_ble_init(void);

/**
 * \brief Get BLE Adapter interface
 *
 * \return adapter interface pointer
 *
 */
const ad_ble_interface_t *ad_ble_get_interface(void);

/**
 * \brief Register task for BLE Adapter's event Queue notifications.
 *
 * \param[in] task_handle The handle of the FreeRTOS task that registers for notifications.
 *
 * \return pdPASS if task is successfully registered, pdFAIL otherwise.
 */
OS_BASE_TYPE ad_ble_event_queue_register(const OS_TASK task_handle);

/**
 * \brief Get public static address
 *
 * This will be either the address read from NVMS or the default address.
 * Since this address does not change once it has been loaded, it's safe to call it at any time
 * from any task
 *
 *\param[out] address Public static address
 */
void ad_ble_get_public_address(uint8_t address[BD_ADDR_LEN]);


/**
 * \brief Read configuration parameters form NVMS parameter area
 *
 * \param[out] param storage for the read parameter
 * \param[in] len length of data to read
 * \param[in] nvparam_tag parameter tag
 * \param[in] nvms_addr offset address
 * \return true if parameter exists, false otherwise
 */
bool ad_ble_read_nvms_param(uint8_t* param, uint8_t len, uint8_t nvparam_tag, uint32_t nvms_addr);

/**
 * \brief Get device's IRK
 *
 * This will be either the IRK read from NVMS or the default IRK.
 *
 ** \param[out] irk Device's IRK
 */
void ad_ble_get_irk(uint8_t irk[KEY_LEN]);

/**
 * \brief Notifies BLE adapter that there is free space on the event queue
 */
void ad_ble_notify_event_queue_avail(void);

/**
 * \brief Get non-volatile parameter handle
 *
 * Function gets parameter handle to be used
 *
 * \return valid parameter handle on success, NULL otherwise
 */
#if dg_configNVPARAM_ADAPTER
nvparam_t ad_ble_get_nvparam_handle(void);
#endif /* dg_configNVPARAM_ADAPTER */

/**
 * \brief Force BLE to stay active
 *
 * Forcing BLE to stay active could be helpful in periods with notable BLE traffic.
 * This will result to reduced interrupt latencies, as BLE is not going to
 * wake up too often due to the expected traffic load.
 *
 * \param [in] status defines if BLE sleep in allowed or not
 */
void ad_ble_stay_active(bool status);

/**
 * \brief Unblock BLE adapter to process new messages generated from IRQ context.
 */
void ad_ble_notify_gen_irq(void);

#if (dg_configRF_ENABLE_RECALIBRATION == 1)
/**
 * \brief Notify BLE adapter to perform RF calibration
 *
 * Should be called by sys_gpadc when there has been a temperature drift for which an RF calibration
 * should be performed.
 */
void ad_ble_notify_rf_calibration(void);
#endif /* (dg_configRF_ENABLE_RECALIBRATION == 1) */

/**
 * \brief Check if the non retention BLE heap is in use.
 */
bool ad_ble_non_retention_heap_in_use(void);

/**
 * \brief Read hook for BLE stack
 *
 * The BLE stack uses this hook to get a message from BLE adapter's command queue, parse it and
 * allocate a buffer for it internally. This functions updates the variables accordingly and enables
 * ad_ble_send_to_stack() to pass the message to the stack.
 *
 * \param [in] bufPtr    Pointer to the message buffer that the read data should be stored.
 * \param [in] size      Size of the data to be read from the current message.
 * \param [in] callback  Pointer to the function to be called when the requested size has been read.
 */
void ad_ble_stack_read(uint8_t* bufPtr, uint32_t size, void (*callback) (uint8_t));

/**
 * \brief Write hook for BLE stack
 *
 * BLE stack uses this hook to deliver a received message to the BLE adapter layer.
 *
 * \param [in]  bufPtr     Pointer to the message buffer.
 * \param [in]  size       Size of the message (including the message type byte).
 * \param [in]  callback   BLE stack callback that must be called when the message is consumed.
 */
void ad_ble_stack_write(uint8_t* bufPtr, uint32_t size, void (*callback) (uint8_t));

#if (USE_BLE_SLEEP == 1)
/**
 * \brief Update CMAC wake-up time
 */
void ad_ble_update_wakeup_time(void);
#endif /* USE_BLE_SLEEP */

/**
 * \brief Configure CMAC SYS TCS table
 */
void ad_ble_sys_tcs_config(void);

/**
 * \brief Configure CMAC SYNTH TCS table
 */
void ad_ble_synth_tcs_config(void);

/**
 * \brief Configure CMAC RFCU TCS table
 */
void ad_ble_rfcu_tcs_config(void);

/**
 * \brief Structure for low level driver statistics
 */
struct ad_ble_lld_stats {
        // Total number of connection events
        uint32_t conn_evt_counter[BLE_CONNECTION_MAX_USER];
        // Number of connection events that completed without apfm status
        uint32_t conn_evt_counter_non_apfm[BLE_CONNECTION_MAX_USER];
        // Total number of advertising events
        uint32_t adv_evt_counter;
        // Number of advertising events that completed without apfm status
        uint32_t adv_evt_counter_non_apfm;
};

/**
 * \brief Gets low level driver statistics
 */
void ad_ble_get_lld_stats(struct ad_ble_lld_stats *stats);

#if (USE_BLE_SLEEP == 1)
#if (dg_configUSE_LP_CLK == LP_CLK_RCX)
/**
 * \brief Notify CMAC about new RCX estimation
 */
void ad_ble_update_rcx(void);
#endif /* (USE_BLE_SLEEP == 1) */
#endif /* (dg_configUSE_LP_CLK == LP_CLK_RCX) */


#endif /* AD_BLE_H */
/**
 \}
 \}
 */
