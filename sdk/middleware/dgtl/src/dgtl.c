/**
 ****************************************************************************************
 *
 * @file dgtl.c
 *
 * @brief DGTL
 *
 * Copyright (C) 2016-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configUSE_DGTL

#include <string.h>
#include <osal.h>
#ifdef DGTL_CUSTOM_UART_CONFIG_HEADER
#       ifndef DGTL_CUSTOM_UART_CONFIG
#               error Please define DGTL_CUSTOM_UART_CONFIG
#       else
#               include DGTL_CUSTOM_UART_CONFIG_HEADER
#               define DGTL_UART_CONFIG DGTL_CUSTOM_UART_CONFIG
#       endif
#else
#       include "../adapters/src/sys_platform_devices_internal.h"
#       define DGTL_UART_CONFIG sys_platform_dgtl_controller_conf
#endif
#include "dgtl_config.h"
#include "dgtl.h"
#include "dgtl_msg.h"
#include "dgtl_pkt.h"

#define NOTIF_QUEUE_TX_DONE     0x00000001
#define NOTIF_UART_RX_DONE      0x00000002
#define NOTIF_CLOSE_UART        0x00000004

/* HCI commands vendor specific opcodes which shall be forwarded to APPHCI instead of HCI queue */
#define APP_SPECIFIC_HCI_MASK   0xFE00

/* Number of queues in high-priority queues list */
#define TX_QUEUES_HI_COUNT (sizeof(tx_queues_hi) / sizeof(tx_queues_hi[0]))

/* DGTL close interval */
#if CONFIG_DGTL_CLOSE_INTERVAL_MS > 0
#       define CLOSE_INTERVAL_MS        (CONFIG_DGTL_CLOSE_INTERVAL_MS)
#else
#       define CLOSE_INTERVAL_MS        (2)
#endif

typedef enum {
#if DGTL_QUEUE_ENABLE_HCI
        QUEUE_IDX_HCI_TX,
        QUEUE_IDX_HCI_RX,
#endif
#if DGTL_QUEUE_ENABLE_APP
        QUEUE_IDX_APP_TX,
        QUEUE_IDX_APP_RX,
#endif
#if DGTL_QUEUE_ENABLE_LOG
        QUEUE_IDX_LOG_TX,
#endif
        QUEUE_IDX_LAST,
} queue_idx_t;

typedef struct {
        OS_TASK owner;
        uint32_t notif;
} queue_info_t;

typedef struct {
        dgtl_msg_t *msg;
        dgtl_sent_cb_t cb;
        void *user_data;
} dgtl_send_data_t;

typedef struct {
        OS_TASK task;
        /* Mutex used for closing DGTL UART */
        OS_MUTEX mutex;

        /* Available queues */
        OS_QUEUE queue[QUEUE_IDX_LAST];
        queue_info_t queue_info[QUEUE_IDX_LAST];
        /* Last position in high-priority queues list, for round-robin scheduling */
        size_t tx_queues_hi_pos;
        /* Buffer pending to be freed */
        dgtl_send_data_t *deferred_free;

#if DGTL_DROPPED_LOG_QUEUE_COUNTER
        size_t log_queue_dropped;
#endif

} dgtl_state_t;

typedef enum {
        UART_STATE_W4_TYPE,
        UART_STATE_W4_HEADER,
        UART_STATE_W4_PARAMETERS,
        UART_STATE_RESYNC,
} uart_rx_state_t;

typedef uart_rx_state_t (* uart_state_func_t) (void);

/* GTL resync pattern is put at the beginning of buffer */
#define RESYNC_PATTERN_GTL_POS  (0)
#define RESYNC_PATTERN_GTL_LEN  (3)
#define RESYNC_PATTERN_GTL_END  ((RESYNC_PATTERN_GTL_POS) + (RESYNC_PATTERN_GTL_LEN))
/* HCI resync pattern follows GTL resync pattern */
#define RESYNC_PATTERN_HCI_POS  (RESYNC_PATTERN_GTL_END)
#define RESYNC_PATTERN_HCI_LEN  (4)
#define RESYNC_PATTERN_HCI_END  ((RESYNC_PATTERN_HCI_POS) + (RESYNC_PATTERN_HCI_LEN))
/* Length of all patterns */
#define RESYNC_PATTERN_LEN      ((RESYNC_PATTERN_GTL_LEN) + (RESYNC_PATTERN_HCI_LEN))

static const uint8_t resync_pattern[RESYNC_PATTERN_LEN] = {
        'R',  'W',  '!',        // GTL resync pattern
        0x01, 0x03, 0x0C, 0x00, // HCI resync pattern
};

typedef struct {
        ad_uart_handle_t dev;
        uart_rx_state_t rx_state;
        bool tx_state;

        dgtl_msg_t *msg;
        dgtl_pkt_t frame_header;

        uint8_t resync_buf;
        uint8_t resync_idx;
        OS_EVENT data_ready;
        OS_EVENT uart_closed;
} uart_state_t;

/* High-priority queues, handled in round-robin fashion */
static const queue_idx_t tx_queues_hi[] = {
#if DGTL_QUEUE_ENABLE_HCI
        QUEUE_IDX_HCI_TX,
#endif
#if DGTL_QUEUE_ENABLE_APP
        QUEUE_IDX_APP_TX,
#endif
};

__RETAINED static dgtl_state_t dgtl;
__RETAINED static uart_state_t uart;

#if DGTL_APP_SPECIFIC_HCI_ENABLE
void dgtl_app_specific_hci_cb(const dgtl_msg_t *msg) __WEAK;
#endif

static void push_frame_to_queue(void)
{
        queue_idx_t qidx;
        queue_info_t *qinfo;

        OS_ASSERT(uart.frame_header.pkt_type == uart.msg->pkt_type);

        switch (uart.msg->pkt_type) {
#if DGTL_QUEUE_ENABLE_HCI
        case DGTL_PKT_TYPE_HCI_CMD:
#if DGTL_APP_SPECIFIC_HCI_ENABLE
                /* Any command within defined address space will be handled immediately vi callback */
                if ((uart.frame_header.hci_cmd.opcode & APP_SPECIFIC_HCI_MASK)
                                                                        == APP_SPECIFIC_HCI_MASK) {
                        dgtl_app_specific_hci_cb(uart.msg);
                        uart.msg = NULL;
                        return;
                }
                /* no break */
#endif
        case DGTL_PKT_TYPE_HCI_ACL:
        case DGTL_PKT_TYPE_HCI_SCO:
        case DGTL_PKT_TYPE_GTL:
                qidx = QUEUE_IDX_HCI_RX;
                break;
#endif
#if DGTL_QUEUE_ENABLE_APP
        case DGTL_PKT_TYPE_APP_CMD:
                qidx = QUEUE_IDX_APP_RX;
                break;
#endif
        default:
                /*
                 * Drop any unrecognized message. This also includes following known packet types,
                 * since they should only be used on TX, not on RX:
                 * - DGTL_PKT_TYPE_HCI_EVT
                 * - DGTL_PKT_TYPE_APP_RSP
                 * - DGTL_PKT_TYPE_LOG
                 */
                dgtl_msg_free(uart.msg);
                uart.msg = NULL;
                return;
        }

        qinfo = &dgtl.queue_info[qidx];

        OS_QUEUE_PUT(dgtl.queue[qidx], &uart.msg, OS_QUEUE_FOREVER);
        OS_TASK_NOTIFY(qinfo->owner, qinfo->notif, OS_NOTIFY_SET_BITS);

        uart.msg = NULL;
}

static void uart_read_cb(void *user_data, uint16_t transferred)
{
        OS_TASK_NOTIFY_FROM_ISR(dgtl.task, NOTIF_UART_RX_DONE, OS_NOTIFY_SET_BITS);
}

static void uart_resync(bool cont)
{
        uart.rx_state = UART_STATE_RESYNC;
        if (!cont) {
                uart.resync_idx = 0;
        }

        ad_uart_read_async(uart.dev, (char *) &uart.resync_buf, 1, uart_read_cb, NULL);
}

static void uart_start_packet(void)
{
        OS_ASSERT(uart.msg == NULL);

        uart.frame_header.pkt_type = 0;

        uart.rx_state = UART_STATE_W4_TYPE;
        ad_uart_read_async(uart.dev, (char *) &uart.frame_header.pkt_type, 1, uart_read_cb, NULL);
}

static void uart_handle_rx_type(void)
{
        size_t header_len;

        header_len = dgtl_pkt_get_header_length(&uart.frame_header);

        /* Check for unknown packet type */
        if (header_len == 0) {
                uart_resync(false);
                return;
        }

        /* Packet type received, receive rest of the header of appropriate size */
        uart.rx_state = UART_STATE_W4_HEADER;
        ad_uart_read_async(uart.dev,
                                (char *) &uart.frame_header.pkt_type + sizeof(uart.frame_header.pkt_type),
                                header_len - 1, uart_read_cb, NULL);
}

static void uart_handle_rx_header(void)
{
        size_t header_len;
        size_t param_len;

        OS_ASSERT(uart.msg == NULL);

        header_len = dgtl_pkt_get_header_length(&uart.frame_header);
        param_len = dgtl_pkt_get_param_length(&uart.frame_header);

        uart.msg = dgtl_msg_alloc(uart.frame_header.pkt_type, header_len + param_len);
        memcpy(uart.msg, &uart.frame_header, header_len);

        /* No parameters to receive for this packet, push to queue immediately. */
        if (param_len == 0) {
                push_frame_to_queue();
                uart_start_packet();
                return;
        }

        /* Packet header received, receive parameters of appropriate size */
        uart.rx_state = UART_STATE_W4_PARAMETERS;
        ad_uart_read_async(uart.dev, (char *) &uart.msg->data[header_len], param_len, uart_read_cb, NULL);
}

static void uart_handle_rx_parameters(void)
{
        push_frame_to_queue();
        uart_start_packet();
}

static void uart_handle_resync(void)
{
        /* Check if current byte match pattern */
        if (uart.resync_buf == resync_pattern[uart.resync_idx]) {
                uart.resync_idx++;
        }
        /* Check also if it matches HCI pattern and set pattern index accordingly (only on 1st byte) */
        else if ((uart.resync_idx == 0) &&
                                (uart.resync_buf == resync_pattern[RESYNC_PATTERN_HCI_POS])) {
                uart.resync_idx = RESYNC_PATTERN_HCI_POS + 1;
        }
        /* Restart resynchronization if pattern does not match */
        else {
                uart_resync(false);
                return;
        }

        if ((uart.resync_idx == RESYNC_PATTERN_GTL_END) ||
                                                (uart.resync_idx == RESYNC_PATTERN_HCI_END)) {
                /* We are resynchronized, start waiting for new packet */
                uart_start_packet();
                return;
        }

        /* Continue resynchronization */
        uart_resync(true);
}

static void uart_rx_done(void)
{
        switch (uart.rx_state) {
        case UART_STATE_W4_TYPE:
                uart_handle_rx_type();
                break;
        case UART_STATE_W4_HEADER:
                uart_handle_rx_header();
                break;
        case UART_STATE_W4_PARAMETERS:
                uart_handle_rx_parameters();
                break;
        case UART_STATE_RESYNC:
                uart_handle_resync();
                break;
        default:
                OS_ASSERT(0);
                break;
        }
}

dgtl_send_data_t *send_data_create(dgtl_msg_t *msg, dgtl_sent_cb_t cb, void *user_data)
{
        dgtl_send_data_t *send_data;

        send_data = OS_MALLOC(sizeof(*send_data));
        send_data->cb = cb;
        send_data->msg = msg;
        send_data->user_data = user_data;

        return send_data;
}

void send_data_destroy(dgtl_send_data_t *send_data)
{
        dgtl_msg_free(send_data->msg);

        OS_FREE(send_data);
}

static void uart_tx_done(void *user_data, uint16_t transferred)
{
        /* There should not be another deferred free operation pending */
        OS_ASSERT(dgtl.deferred_free == NULL);

        /* Store buffer pointer */
        dgtl.deferred_free = user_data;

        /* Notify DGTL task to free the buffer */
        OS_TASK_NOTIFY_FROM_ISR(dgtl.task, NOTIF_QUEUE_TX_DONE, OS_NOTIFY_SET_BITS);
}

static dgtl_send_data_t *pick_message_from_hi_queue(void)
{
        dgtl_send_data_t *msg = NULL;
        size_t i;

        for (i = 0; !msg && (i < TX_QUEUES_HI_COUNT); i++) {
                queue_idx_t qidx = tx_queues_hi[dgtl.tx_queues_hi_pos];

                OS_QUEUE_GET(dgtl.queue[qidx], &msg, OS_QUEUE_NO_WAIT);

                if (++dgtl.tx_queues_hi_pos >= TX_QUEUES_HI_COUNT) {
                        dgtl.tx_queues_hi_pos = 0;
                }
        }

        return msg;
}

static void queue_tx_done(void)
{
        dgtl_send_data_t *send_data = NULL;

        if (uart.tx_state) {
                if (dgtl.deferred_free) {
                        if (dgtl.deferred_free->cb) {
                                dgtl.deferred_free->cb(dgtl.deferred_free->user_data);
                        }

                        /* UART TX has just been completed, buffer free operation pending */
                        send_data_destroy(dgtl.deferred_free);
                        dgtl.deferred_free = NULL;
                        uart.tx_state = false;
                } else {
                        /* We are already transmitting something, will go back here when finished */
                        return;
                }
        }

        /* Leave this for compiler to optimize-out unused paths depending on TX_QUEUES_HI_COUNT */
        if (TX_QUEUES_HI_COUNT == 1) {
                /* Always fetch from 1st queue if only single queue is enabled */
                queue_idx_t qidx = tx_queues_hi[0];
                OS_QUEUE_GET(dgtl.queue[qidx], &send_data, OS_QUEUE_NO_WAIT);
        } else if (TX_QUEUES_HI_COUNT > 1) {
                send_data = pick_message_from_hi_queue();
        }

        /*
         * If no message in any high-priority queue, try to get something from logs queue (or just
         * return if log queue is not available)
         */
        if (!send_data) {
#if DGTL_QUEUE_ENABLE_LOG
                OS_QUEUE_GET(dgtl.queue[QUEUE_IDX_LOG_TX], &send_data, OS_QUEUE_NO_WAIT);

                /* Still nothing, just wait for another event */
                if (!send_data) {
                        return;
                }
#else
                return;
#endif
        }

        uart.tx_state = true;
        ad_uart_write_async(uart.dev, (char *) send_data->msg->data,
                                dgtl_pkt_get_length((dgtl_pkt_t *) send_data->msg), uart_tx_done, send_data);
}

void dgtl_wkup_handler(void)
{
        if (dgtl.task) {
                if (in_interrupt()) {
                        OS_EVENT_SIGNAL_FROM_ISR(uart.data_ready);
                } else {
                        OS_EVENT_SIGNAL(uart.data_ready);
                }
        }
}

static void dgtl_task_func(void *param)
{
        for (;;) {
                OS_EVENT_WAIT(uart.data_ready, OS_EVENT_FOREVER);

                uart.dev = ad_uart_open(&DGTL_UART_CONFIG);
                /* Wait for first packet type indicator */
                uart_start_packet();

                while (uart.dev) {
                        uint32_t notif;

                        OS_TASK_NOTIFY_WAIT(0, (unsigned) -1, &notif, OS_TASK_NOTIFY_FOREVER);

                        if (notif & NOTIF_UART_RX_DONE) {
                                uart_rx_done();
                        }

                        if (notif & NOTIF_QUEUE_TX_DONE) {
                                queue_tx_done();
                        }

                        if (notif & NOTIF_CLOSE_UART) {
                                break;
                        }
                }
                ad_uart_complete_async_read(uart.dev);

                /* Wait until any pending operation is completed */
                while (ad_uart_close(uart.dev, false) != AD_UART_ERROR_NONE) {
                        OS_DELAY_MS(CLOSE_INTERVAL_MS);
                }

                uart.dev = NULL;
                OS_EVENT_SIGNAL(uart.uart_closed);
                OS_TASK_NOTIFY_TAKE(1, OS_TASK_NOTIFY_NO_WAIT);
        }

        OS_ASSERT(0);
}

void dgtl_init(void)
{
        int i;

        /* Silently ignore double-init */
        if (dgtl.task) {
                return;
        }

        for (i = 0; i < QUEUE_IDX_LAST; i++) {
                OS_QUEUE_CREATE(dgtl.queue[i], sizeof(void *), 10);
        }

        OS_MUTEX_CREATE(dgtl.mutex);
        OS_EVENT_CREATE(uart.data_ready);
        OS_EVENT_CREATE(uart.uart_closed);
        OS_TASK_CREATE("dgtl", dgtl_task_func, NULL, 768, OS_TASK_PRIORITY_NORMAL, dgtl.task);
        OS_EVENT_SIGNAL(uart.data_ready);
}

void dgtl_close(void)
{
        OS_MUTEX_GET(dgtl.mutex, OS_MUTEX_FOREVER);

        /* Check if UART is already opened */
        if (uart.dev) {
                OS_TASK_NOTIFY(dgtl.task, NOTIF_CLOSE_UART, OS_NOTIFY_SET_BITS);
                OS_EVENT_WAIT(uart.uart_closed, OS_EVENT_FOREVER);
        }

        OS_MUTEX_PUT(dgtl.mutex);
}

void dgtl_register(dgtl_queue_t queue, uint32_t notif)
{
        queue_idx_t qidx;
        queue_info_t *qinfo;

        switch (queue) {
#if DGTL_QUEUE_ENABLE_HCI
        case DGTL_QUEUE_HCI:
                qidx = QUEUE_IDX_HCI_RX;
                break;
#endif
#if DGTL_QUEUE_ENABLE_APP
        case DGTL_QUEUE_APP:
                qidx = QUEUE_IDX_APP_RX;
                break;
#endif
        default:
                OS_ASSERT(0);
                return;
        }

        qinfo = &dgtl.queue_info[qidx];
        if (qinfo->owner) {
                OS_ASSERT(0);
                return;
        }

        qinfo->owner = OS_GET_CURRENT_TASK();
        qinfo->notif = notif;
}

bool dgtl_send_ex(dgtl_msg_t *msg, dgtl_sent_cb_t cb, void *user_data)
{
        dgtl_send_data_t *send_data;
        queue_idx_t qidx;
        OS_BASE_TYPE timeout = OS_QUEUE_FOREVER;
        OS_BASE_TYPE ret;

        switch (msg->pkt_type) {
#if DGTL_QUEUE_ENABLE_HCI
        case DGTL_PKT_TYPE_HCI_ACL:
        case DGTL_PKT_TYPE_HCI_SCO:
        case DGTL_PKT_TYPE_HCI_EVT:
        case DGTL_PKT_TYPE_GTL:
                qidx = QUEUE_IDX_HCI_TX;
                break;
#endif
#if DGTL_QUEUE_ENABLE_APP
        case DGTL_PKT_TYPE_APP_RSP:
                qidx = QUEUE_IDX_APP_TX;
                break;
#endif
#if DGTL_QUEUE_ENABLE_LOG
        case DGTL_PKT_TYPE_LOG:
                qidx = QUEUE_IDX_LOG_TX;
                timeout = OS_QUEUE_NO_WAIT;
                break;
#endif
        default:
                /*
                 * Discard any unrecognized message. This also includes following known packet types,
                 * since they should only be used on RX, not on TX:
                 * - DGTL_PKT_TYPE_HCI_CMD
                 * - DGTL_PKT_TYPE_APP_CMD
                 */
                OS_ASSERT(0);
                /* There is no queue for this packet type so we just discard it */
                dgtl_msg_free(msg);
                return false;
        }

        send_data = send_data_create(msg, cb, user_data);
        ret = OS_QUEUE_PUT(dgtl.queue[qidx], &send_data, timeout);
        if (ret == OS_QUEUE_OK) {
                OS_TASK_NOTIFY(dgtl.task, NOTIF_QUEUE_TX_DONE, OS_NOTIFY_SET_BITS);
                return true;
        }
#if DGTL_QUEUE_ENABLE_LOG
        else if (qidx == QUEUE_IDX_LOG_TX) {
                send_data_destroy(send_data);
#if DGTL_DROPPED_LOG_QUEUE_COUNTER
                OS_ENTER_CRITICAL_SECTION();
                dgtl.log_queue_dropped++;
                OS_LEAVE_CRITICAL_SECTION();
#endif
        }
#endif
        else {
                /* Free message */
                send_data_destroy(send_data);
        }

        return false;
}

void dgtl_send(dgtl_msg_t *msg)
{
        dgtl_send_ex(msg, NULL, NULL);
}

dgtl_msg_t *dgtl_receive(dgtl_queue_t queue)
{
        OS_BASE_TYPE ret;
        dgtl_msg_t *msg;
        queue_idx_t qidx;
        queue_info_t *qinfo;

        switch (queue) {
#if DGTL_QUEUE_ENABLE_HCI
        case DGTL_QUEUE_HCI:
                qidx = QUEUE_IDX_HCI_RX;
                break;
#endif
#if DGTL_QUEUE_ENABLE_APP
        case DGTL_QUEUE_APP:
                qidx = QUEUE_IDX_APP_RX;
                break;
#endif
        default:
                OS_ASSERT(0);
                return NULL;
        }

        /* Make sure only task which registered for queue can receive messages */
        qinfo = &dgtl.queue_info[qidx];
        if (qinfo->owner != OS_GET_CURRENT_TASK()) {
                OS_ASSERT(0);
                return NULL;
        }

        ret = OS_QUEUE_GET(dgtl.queue[qidx], &msg, OS_QUEUE_NO_WAIT);
        if (ret != OS_QUEUE_OK) {
                return NULL;
        }

        return msg;
}

#endif /* dg_configUSE_DGTL */
