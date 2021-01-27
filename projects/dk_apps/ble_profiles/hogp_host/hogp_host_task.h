/**
 ****************************************************************************************
 *
 * @file hogp_host_task.h
 *
 * @brief HOGP Host task header
 *
 * Copyright (C) 2015-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/**
 * Definition of available space for scan device list
 */
#define MAX_FOUND_DEVICES       25

/**
 * Client types
 */
typedef enum {
        CLIENT_TYPE_NONE,
        CLIENT_TYPE_HIDS,
        CLIENT_TYPE_BAS,
        CLIENT_TYPE_SCPS,
        CLIENT_TYPE_GATT,
        CLIENT_TYPE_DIS,
} client_type_t;

/**
 * Client struct
 */
typedef struct {
        void *next;
        uint8_t id;
        client_type_t type;
        ble_client_t *client;
} hogp_client_t;

/**
 * Function returns client struct with given type and id
 */
hogp_client_t *get_client(uint8_t id, client_type_t type);

void hogp_scan(bool start, bool scan_any);

void hogp_connect(bd_address_t *address, size_t dev_index);

void hogp_connect_cancel(void);

void hogp_disconnect(void);

void hogp_unbond_all(void);

void hogp_unbond_by_address(bd_address_t *address);

void hogp_show_devices(gap_device_filter_t filter);
