/**
 ****************************************************************************************
 *
 * @file blp_sensor_task.c
 *
 * @brief Blood Pressure Sensor Profile task
 *
 * Copyright (C) 2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdio.h>
#include "osal.h"
#include "sys_watchdog.h"
#include "hw_wkup.h"
#include "ble_common.h"
#include "ble_gap.h"
#include "ble_service.h"
#include "bls.h"
#include "dis.h"
#include "sensor.h"
#include "blp_sensor_config.h"
#include "sdk_queue.h"

/* Notification from advertising mode control timer */
#define ADV_TIMER_NOTIF                         (1 << 1)
#define BLS_MEASUREMENT_NOTIF                   (1 << 2)
#define SENSOR_NOTIF                            (1 << 3)

/* Advertising modes of BLP Sensor application */
typedef enum {
        ADV_MODE_OFF,
        ADV_MODE_FAST_CONNECTION,
        ADV_MODE_REDUCED_POWER,
} adv_mode_t;

/*
 * BLP advertising and scan response data
 */
static const uint8_t adv_data[] = {
        0x03, GAP_DATA_TYPE_UUID16_LIST_INC,
        0x10, 0x18, // = 0x1810 (Blood Pressure Service UUID)
};

static const uint8_t scan_rsp[] = {
        0x12, GAP_DATA_TYPE_LOCAL_NAME,
        'D', 'i', 'a', 'l', 'o', 'g', ' ', 'B', 'L', 'P', ' ', 'S', 'e', 'n', 's', 'o', 'r',
};

static const dis_system_id_t dis_sys_id = {
        .oui = defaultBLE_DIS_SYSTEM_ID_OUI,
        .manufacturer = defaultBLE_DIS_SYSTEM_ID_MANUFACTURER,
};

static const dis_device_info_t dis_config = {
        .manufacturer = defaultBLE_DIS_MANUFACTURER,
        .model_number = defaultBLE_DIS_MODEL_NUMBER,
        .system_id = &dis_sys_id,
};

static const ble_service_config_t bls_service_config = {
        .service_type = GATT_SERVICE_PRIMARY,
        .sec_level = GAP_SEC_LEVEL_2,
        .num_includes = 0,
        .includes = NULL,
};

static const bls_config_t bls_config = {
        .feature_supp   = BLS_FEATURE_BODY_MOVEMENT_DETECTION |
                          BLS_FEATURE_CUFF_FIT_DETECTION |
                          BLS_FEATURE_IRREGULAR_PULSE_DETECTION |
                          BLS_FEATURE_PULSE_RATE_RANGE_DETECTION |
                          BLS_FEATURE_MEASUREMENT_POS_DETECTION,
        .supported_char = BLS_SUPPORTED_CHAR_INTERM_CUFF_PRESSURE,
};

/* Task used by application */
PRIVILEGED_DATA static OS_TASK app_task;
/* Blood Pressure Service instance */
PRIVILEGED_DATA static ble_service_t *bls;
/* Timer used for advertising mode control */
PRIVILEGED_DATA static OS_TIMER adv_timer;
/* Current advertising mode */
PRIVILEGED_DATA static adv_mode_t adv_mode;
/* Advertising interval minimum */
PRIVILEGED_DATA static uint16_t adv_intv_min;
/* Advertising interval maximum */
PRIVILEGED_DATA static uint16_t adv_intv_max;
/* Advertising timeout */
PRIVILEGED_DATA static unsigned adv_timeout;

typedef struct {
        void *next;
        bls_measurement_t measurement;
} bls_meas_queue_elem_t;

typedef struct {
        /* Client connection index */
        uint16_t conn_idx;
        /* Queue of BLP measurements */
        queue_t measurements;
        /* Measurement indication is in progress */
        bool measurement_sending;
        /* Cached intermediate measurement */
        bls_measurement_t *cached_intermediate_measurement;
} peer_info_t;

INITIALISED_PRIVILEGED_DATA static peer_info_t peer_info = {
        .conn_idx = BLE_CONN_IDX_INVALID,
        .measurement_sending = false,
        .cached_intermediate_measurement = NULL,
};

static void apply_adv_parameters(adv_mode_t mode)
{
        printf("Set advertising mode: ");

        switch (mode) {
        case ADV_MODE_OFF:
                // leave all-zero
                adv_intv_min = 0;
                adv_intv_max = 0;
                adv_timeout = 0;
                printf("Off\r\n");
                break;
        case ADV_MODE_FAST_CONNECTION:
                adv_intv_min = 20;
                adv_intv_max = 30;
                adv_timeout = 30000;
                printf("Fast mode\r\n");
                break;
        case ADV_MODE_REDUCED_POWER:
                adv_intv_min = 1000;
                adv_intv_max = 2500;
                adv_timeout = 0;
                printf("Reduced power mode\r\n");
                break;
        default:
                return;
        }

        /* If both min and max intervals are non-zero, set them and start advertising */
        if (adv_intv_min && adv_intv_max) {
                ble_gap_adv_intv_set(BLE_ADV_INTERVAL_FROM_MS(adv_intv_min),
                                                        BLE_ADV_INTERVAL_FROM_MS(adv_intv_max));
                ble_gap_adv_start(GAP_CONN_MODE_UNDIRECTED);

                if (adv_timeout) {
                        OS_TIMER_CHANGE_PERIOD(adv_timer, OS_MS_2_TICKS(adv_timeout),
                                                                                OS_TIMER_FOREVER);
                        OS_TIMER_START(adv_timer, OS_TIMER_FOREVER);
                }
        }
}

static void set_adv_mode(adv_mode_t mode)
{
        ble_error_t ret;

        /* If request mode is the same, just restart timer */
        if (mode == adv_mode) {
                OS_TIMER_START(adv_timer, OS_TIMER_FOREVER);
                return;
        }

        adv_mode = mode;

        /* Always try to stop advertising - we need to change parameters and then start again */
        OS_TIMER_STOP(adv_timer, OS_TIMER_FOREVER);
        ret = ble_gap_adv_stop();

        /* Advertising isn't started, apply parameters and start advertising */
        if (ret == BLE_ERROR_NOT_ALLOWED) {
                apply_adv_parameters(adv_mode);
        }
}

static void adv_timer_cb(OS_TIMER timer)
{
        OS_TASK task = (OS_TASK) OS_TIMER_GET_TIMER_ID(timer);

        OS_TASK_NOTIFY_FROM_ISR(task, ADV_TIMER_NOTIF, OS_NOTIFY_SET_BITS);
}

static void handle_adv_timer_notif(void)
{
        switch (adv_mode) {
        case ADV_MODE_OFF:
                /* ignore */
                break;
        case ADV_MODE_FAST_CONNECTION:
                set_adv_mode(ADV_MODE_REDUCED_POWER);
                break;
        case ADV_MODE_REDUCED_POWER:
                set_adv_mode(ADV_MODE_OFF);
                break;
        default:
                OS_ASSERT(0);
                return;
        }
}

void button_interrupt_cb(void)
{
        OS_TASK_NOTIFY_FROM_ISR(app_task, BLS_MEASUREMENT_NOTIF, OS_NOTIFY_SET_BITS);
}

static char *ieee11703_to_string(const svc_ieee11073_float_t *value)
{
        static char buf[10];

        int mantissa = value->mantissa;
        int exp = value->exp;

        int decimal = mantissa;
        int rest = 0;
        int rest_index = 1;

        while (exp > 0) {
            exp--;
            decimal *= 10;
        }

        while (exp < 0) {
            exp++;
            rest += rest_index * (decimal % 10);
            rest_index *= 10;
            decimal /= 10;
        }

        snprintf(buf, sizeof(buf), "%d.%d", decimal, rest);

        return buf;
}

static void can_send_intermediate_measurement(void)
{
        gap_sec_level_t sec_level = GAP_SEC_LEVEL_1;

        if (!peer_info.cached_intermediate_measurement ||
                                                peer_info.conn_idx == BLE_CONN_IDX_INVALID) {
                return;
        }

        ble_gap_get_sec_level(peer_info.conn_idx, &sec_level);
        if (sec_level > GAP_SEC_LEVEL_1) {
                if (bls_notify_intermediate_cuff_pressure(bls, peer_info.conn_idx,
                                                peer_info.cached_intermediate_measurement)) {
                        OS_FREE(peer_info.cached_intermediate_measurement);
                        peer_info.cached_intermediate_measurement = NULL;
                }
        }
}

static void can_send_measurement(void)
{
        bls_meas_queue_elem_t *elem = queue_peek_front(&peer_info.measurements);
        gap_sec_level_t sec_level = GAP_SEC_LEVEL_1;

        if (peer_info.conn_idx == BLE_CONN_IDX_INVALID || peer_info.measurement_sending ||
                                                                                        !elem) {
                return;
        }

        ble_gap_get_sec_level(peer_info.conn_idx, &sec_level);
        if (sec_level > GAP_SEC_LEVEL_1) {
                if (bls_indicate_pressure_measurement(bls, peer_info.conn_idx, &elem->measurement)) {
                        peer_info.measurement_sending = true;
                }
        }
}

static void bls_interm_cuff_pressure_notif_changed_cb(ble_service_t *service, uint16_t conn_idx,
                                                                                bool enabled)
{
        can_send_intermediate_measurement();
}

static void bls_meas_indication_sent_cb(ble_service_t *service, uint16_t conn_idx, bool status)
{
        if (status) {
                bls_meas_queue_elem_t *elem;

                elem = queue_pop_front(&peer_info.measurements);
                OS_FREE(elem);
        }

        peer_info.measurement_sending = false;

        can_send_measurement();
}

static void bls_meas_indication_changed_cb(ble_service_t *service, uint16_t conn_idx, bool enabled)
{
        can_send_measurement();
}

static const bls_callbacks_t bls_cb = {
        .meas_indication_changed = bls_meas_indication_changed_cb,
        .meas_indication_sent = bls_meas_indication_sent_cb,
        .interm_cuff_pressure_notif_changed = bls_interm_cuff_pressure_notif_changed_cb,
        .interm_cuff_pressure_notif_sent = NULL,
};

static void handle_sensor_event_intermediate(bls_measurement_t *measurement)
{
        gap_sec_level_t sec_level = GAP_SEC_LEVEL_1;

        printf("Intermediate Cuff Pressure\r\n");
        printf("\tUnit: %s\r\n", measurement->unit ? "kPa" : "mm Hg");
        printf("\tSystolic: %s\r\n", ieee11703_to_string(&measurement->pressure_systolic));

        if (peer_info.conn_idx == BLE_CONN_IDX_INVALID) {
                goto cache_measurement;
        }

        ble_gap_get_sec_level(peer_info.conn_idx, &sec_level);
        if (sec_level > GAP_SEC_LEVEL_1) {
                if (bls_notify_intermediate_cuff_pressure(bls, peer_info.conn_idx, measurement)) {
                        return;
                }
        }

cache_measurement:
        if (!peer_info.cached_intermediate_measurement) {
                peer_info.cached_intermediate_measurement = OS_MALLOC(sizeof(bls_measurement_t));
        }

        *peer_info.cached_intermediate_measurement = *measurement;
}

static void handle_sensor_event_measurement(bls_measurement_t *measurement)
{
        bls_meas_queue_elem_t *new_measurement;

        printf("Blood Pressure Measurement\r\n");
        printf("\tUnit: %s\r\n", measurement->unit ? "kPa" : "mm Hg");
        printf("\tSystolic: %s\r\n", ieee11703_to_string(&measurement->pressure_systolic));
        printf("\tDiastolic: %s\r\n", ieee11703_to_string(&measurement->pressure_diastolic));
        printf("\tMAP: %s\r\n", ieee11703_to_string(&measurement->pressure_map));

        /* Remove intermediate measurement */
        if (peer_info.cached_intermediate_measurement) {
                OS_FREE(peer_info.cached_intermediate_measurement);
                peer_info.cached_intermediate_measurement = NULL;
        }

        if (measurement->pulse_rate_present) {
                printf("\tPulse: %s\r\n", ieee11703_to_string(&measurement->pulse_rate));
        }

        new_measurement = OS_MALLOC(sizeof(bls_meas_queue_elem_t));
        new_measurement->measurement = *measurement;

        if (queue_length(&peer_info.measurements) == CFG_MAX_NUMBER_OF_MEASUREMENTS) {
                bls_meas_queue_elem_t *oldest_measurement;

                oldest_measurement = queue_pop_front(&peer_info.measurements);
                OS_FREE(oldest_measurement);
        }

        queue_push_back(&peer_info.measurements, new_measurement);

        can_send_measurement();
}

static void handle_evt_gap_connected(ble_evt_gap_connected_t *evt)
{
        ble_gap_pair(evt->conn_idx, true);

        printf("Device connected\r\n");
        printf("\tConnection index: %d\r\n", evt->conn_idx);
        printf("\tAddress: %s\r\n", ble_address_to_string(&evt->peer_address));

        peer_info.conn_idx = evt->conn_idx;
}

static void handle_evt_gap_adv_completed(ble_evt_gap_adv_completed_t *evt)
{
        /* Update mode if advertising is called due to something else than our stop request */
        if (evt->status != BLE_ERROR_CANCELED) {
                set_adv_mode(ADV_MODE_OFF);
        }

        apply_adv_parameters(adv_mode);
}
static void handle_evt_gap_disconnected(ble_evt_gap_disconnected_t *evt)
{
        printf("Device disconnected\r\n");
        printf("\tConnection index: %d\r\n", evt->conn_idx);
        printf("\tBD address of disconnected device: %s, %s\r\n",
                                evt->address.addr_type == PUBLIC_ADDRESS ? "public" : "private",
                                ble_address_to_string(&evt->address));
        printf("\tReason of disconnection: 0x%02x\r\n", evt->reason);

        peer_info.conn_idx = BLE_CONN_IDX_INVALID;
        peer_info.measurement_sending = false;

        set_adv_mode(ADV_MODE_FAST_CONNECTION);
}

static void handle_evt_gap_pair_req(ble_evt_gap_pair_req_t *evt)
{
        ble_error_t err;

        printf("Pair request\r\n");
        printf("\tConnection index: %d\r\n", evt->conn_idx);
        printf("\tBond: %d\r\n", evt->bond);

        err = ble_gap_pair_reply(evt->conn_idx, true, evt->bond);
        if (err != BLE_STATUS_OK) {
                /*
                 * Reply may be rejected by stack due to e.g. insufficient resources in case we are
                 * already bonded but different remote tries to bond again. In this case we reject
                 * pairing and user needs to remove bonding in order to pair with other host.
                 */
                ble_gap_pair_reply(evt->conn_idx, false, evt->bond);
        }
}

static void handle_evt_gap_sec_level_changed(ble_evt_gap_sec_level_changed_t *evt)
{
        printf("Security level changed\r\n");
        printf("\tConnection index: %d\r\n", evt->conn_idx);
        printf("\tSecurity level: %u\r\n", evt->level + 1);

        can_send_intermediate_measurement();
        can_send_measurement();
}


static void create_timers(void)
{
        /*
         * Create timer for controlling advertising mode. We need to set any non-zero period (i.e. 1)
         * but this will be changed later, when timer is started.
         */
        adv_timer = OS_TIMER_CREATE("adv", /* don't care */ 1, OS_TIMER_FAIL,
                                                        OS_GET_CURRENT_TASK(), adv_timer_cb);
        OS_ASSERT(adv_timer);
}

void blp_sensor_task(void *params)
{
        int8_t wdog_id;

        /* Register blp_sensor_task to be monitored by watchdog */
        wdog_id = sys_watchdog_register(false);

        /* Store application task handle for task notifications */
        app_task = OS_GET_CURRENT_TASK();

        /* Start BLE device as peripheral */
        ble_peripheral_start();
        ble_register_app();

        /* Set device name */
        ble_gap_device_name_set("Dialog BLP Sensor", ATT_PERM_READ);

        /* Register DIS */
        dis_init(NULL, &dis_config);

        /* Register BLS */
        bls = bls_init(&bls_service_config, &bls_config, &bls_cb);

        /* Create various timers */
        create_timers();

        /*
         * If device is started with K1 button pressed, remove bonding information to allow
         * new host to be bonded.
         */
        if (!hw_gpio_get_pin_status(KEY1_PORT, KEY1_PIN)) {
                gap_device_t dev;
                size_t len = 1;
                ble_error_t err;

                err = ble_gap_get_devices(GAP_DEVICE_FILTER_BONDED, NULL, &len, &dev);
                if (err == BLE_STATUS_OK && len > 0) {
                        printf("Unpairing %s\r\n", ble_address_to_string(&dev.address));
                        ble_gap_unpair(&dev.address);
                }
        }

        /* Set advertising data and scan response */
        ble_gap_adv_data_set(sizeof(adv_data), adv_data, sizeof(scan_rsp), scan_rsp);

        /* Initialize sensor */
        sensor_init(SENSOR_NOTIF);

        printf("Blood Pressure Sensor application started\r\n");

        /* Start advertising after reboot */
        set_adv_mode(ADV_MODE_FAST_CONNECTION);

        for (;;) {
                OS_BASE_TYPE ret;
                uint32_t notif;

                /* Notify watchdog on each loop */
                sys_watchdog_notify(wdog_id);

                /* Suspend watchdog while blocking on OS_TASK_NOTIFY_WAIT() */
                sys_watchdog_suspend(wdog_id);

                ret = OS_TASK_NOTIFY_WAIT(0, OS_TASK_NOTIFY_ALL_BITS, &notif,
                                                                        OS_TASK_NOTIFY_FOREVER);
                OS_ASSERT(ret == OS_OK);

                /* Resume watchdog */
                sys_watchdog_notify_and_resume(wdog_id);

                /* Notified from BLE manager, can get event */
                if (notif & BLE_APP_NOTIFY_MASK) {
                        ble_evt_hdr_t *hdr;

                        /*
                         * No need to wait for event, should be already there since we were notified
                         * from manager
                         */
                        hdr = ble_get_event(false);

                        if (!hdr) {
                                goto no_event;
                        }

                        if (!ble_service_handle_event(hdr)) {
                                switch (hdr->evt_code) {
                                case BLE_EVT_GAP_CONNECTED:
                                        handle_evt_gap_connected((ble_evt_gap_connected_t *) hdr);
                                        break;
                                case BLE_EVT_GAP_ADV_COMPLETED:
                                        handle_evt_gap_adv_completed((ble_evt_gap_adv_completed_t *) hdr);
                                        break;
                                case BLE_EVT_GAP_DISCONNECTED:
                                        handle_evt_gap_disconnected((ble_evt_gap_disconnected_t *) hdr);
                                        break;
                                case BLE_EVT_GAP_PAIR_REQ:
                                        handle_evt_gap_pair_req((ble_evt_gap_pair_req_t *) hdr);
                                        break;
                                case BLE_EVT_GAP_SEC_LEVEL_CHANGED:
                                        handle_evt_gap_sec_level_changed((ble_evt_gap_sec_level_changed_t *) hdr);
                                        break;
                                default:
                                        ble_handle_event_default(hdr);
                                        break;
                                }
                        }

                        /* Free event buffer (it's not needed anymore) */
                        OS_FREE(hdr);

no_event:
                        /*
                         * If there are more events waiting in queue, application should process
                         * them now.
                         */
                        if (ble_has_event()) {
                                OS_TASK_NOTIFY(OS_GET_CURRENT_TASK(), BLE_APP_NOTIFY_MASK,
                                                                               OS_NOTIFY_SET_BITS);
                        }
                }

                if (notif & ADV_TIMER_NOTIF) {
                        handle_adv_timer_notif();
                }

                if (notif & BLS_MEASUREMENT_NOTIF) {
                        /* Get measurements from blood pressure sensor */
                        sensor_do_measurement();
                }

                if (notif & SENSOR_NOTIF) {
                        sensor_event_t event;

                        if (sensor_get_event(&event)) {
                                switch (event.type) {
                                case SENSOR_EVENT_INTERMEDIATE:
                                        handle_sensor_event_intermediate(&event.value);
                                        break;
                                case SENSOR_EVENT_MEASUREMENT:
                                        handle_sensor_event_measurement(&event.value);
                                        break;
                                default:
                                        printf("Measurement failed\r\n");
                                        break;
                                }
                        }

                        if (sensor_has_event()) {
                                OS_TASK_NOTIFY(OS_GET_CURRENT_TASK(), SENSOR_NOTIF,
                                                                        OS_NOTIFY_SET_BITS);
                        }
                }
        }
}
