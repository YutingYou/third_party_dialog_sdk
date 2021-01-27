/**
 ****************************************************************************************
 *
 * @file ams_task.c
 *
 * @brief Apple Media Service task
 *
 * Copyright (C) 2018-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include "osal.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "ble_client.h"
#include "ble_service.h"
#include "ble_storage.h"
#include "ble_gattc.h"
#include "ble_uuid.h"
#include "gatt_client.h"
#include "ams_client.h"
#include "cli.h"
#include "cli_utils.h"
#include "sdk_queue.h"
#include "sys_watchdog.h"
#include "hw_gpio.h"
#include "ams_config.h"
#include "display.h"

#define UUID_AMS                        "89D3502B-0F36-433A-8EF4-C502AD55F8DC"

#define GATT_CLIENT_STORAGE_ID          BLE_STORAGE_KEY_APP(0, 0)
#define AMS_CLIENT_STORAGE_ID           BLE_STORAGE_KEY_APP(0, 1)

#define CLI_NOTIF                       (1 << 1)  // CLI notifications
#define BUTTON_NOTIF                    (1 << 2)  // Button press notifications

typedef enum {
        PENDING_ACTION_GATT_ENABLE_SERVICE_CHANGED_IND = (1 << 0),
        PENDING_ACTION_AMS_ENABLE_REMOTE_COMMAND_NOTIF = (1 << 1),
        PENDING_ACTION_AMS_ENABLE_ENTITY_UPDATE_NOTIF = (1 << 2),
        PENDING_ACTION_AMS_ENABLE_ENTITY_UPDATE_PLAYER = (1 << 3),
        PENDING_ACTION_AMS_ENABLE_ENTITY_UPDATE_QUEUE = (1 << 4),
        PENDING_ACTION_AMS_ENABLE_ENTITY_UPDATE_TRACK = (1 << 5),
} pending_action_t;

typedef enum {
        ATTRIBUTE_REQUEST_STATE_IDLE,
        ATTRIBUTE_REQUEST_STATE_WRITE,
        ATTRIBUTE_REQUEST_STATE_READ,
} attribute_request_state_t;

typedef struct {
        void *next;
        attribute_request_state_t state;
        ams_client_entity_id_t entity_id;
        uint8_t attribute_id;
} attribute_request_t;

typedef struct {
        uint16_t conn_idx;

        bool browsing;

        bool busy_init;
        pending_action_t pending_init;

        queue_t attribute_requests;
        queue_t svc_changed_queue;

        ble_client_t *ams_client;
        ble_client_t *gatt_client;
} peer_info_t;

typedef struct {
        void *next;
        uint16_t start_h;
        uint16_t end_h;
} browse_req_t;

__RETAINED_RW static peer_info_t peer_info = {
        .conn_idx = BLE_CONN_IDX_INVALID,
};

__RETAINED static OS_TASK current_task;

static const uint8_t adv_data[] = {
        0x11, GAP_DATA_TYPE_UUID128_SOLIC,
        0xDC, 0xF8, 0x55, 0xAD, 0x02, 0xC5, 0xF4, 0x8E, 0x3A, 0x43, 0x36, 0x0F, 0x2B, 0x50, 0xD3,
                                                                                        0x89,
        // 89D3502B-0F36-433A-8EF4-C502AD55F8DC (AMS UUID)
};

static const uint8_t scan_rsp[] = {
        0x10, GAP_DATA_TYPE_LOCAL_NAME,
        'D', 'i', 'a', 'l', 'o', 'g', ' ', 'A', 'M', 'S', ' ', 'D', 'e', 'm', 'o'
};

#define pending_init_execute_and_check(FLAG, FUNCTION, ...) \
        ({                                                                      \
                if (peer_info.pending_init & FLAG) {                            \
                        peer_info.busy_init = FUNCTION(__VA_ARGS__);            \
                        if (!peer_info.busy_init) {                             \
                                /* Failed to execute action, clear bit */       \
                                peer_info.pending_init &= ~FLAG;                \
                        }                                                       \
                }                                                               \
                peer_info.busy_init;                                            \
        })

static const uint8_t player_attributes[] = {
        AMS_CLIENT_PLAYER_ATTRIBUTE_ID_NAME,
        AMS_CLIENT_PLAYER_ATTRIBUTE_ID_PLAYBACK_INFO,
        AMS_CLIENT_PLAYER_ATTRIBUTE_ID_VOLUME,
};

static const uint8_t queue_attributes[] = {
        AMS_CLIENT_QUEUE_ATTRIBUTE_ID_INDEX,
        AMS_CLIENT_QUEUE_ATTRIBUTE_ID_COUNT,
        AMS_CLIENT_QUEUE_ATTRIBUTE_ID_SHUFFLE_MODE,
        AMS_CLIENT_QUEUE_ATTRIBUTE_ID_REPEAT_MODE,
};

static const uint8_t track_attributes[] = {
        AMS_CLIENT_TRACK_ATTRIBUTE_ID_ARTIST,
        AMS_CLIENT_TRACK_ATTRIBUTE_ID_ALBUM,
        AMS_CLIENT_TRACK_ATTRIBUTE_ID_TITLE,
        AMS_CLIENT_TRACK_ATTRIBUTE_ID_DURATION,
};

static void process_pending_actions(void)
{
        if (peer_info.busy_init) {
                return;
        }

        if (pending_init_execute_and_check(PENDING_ACTION_GATT_ENABLE_SERVICE_CHANGED_IND,
                                                gatt_client_set_event_state, peer_info.gatt_client,
                                                GATT_CLIENT_EVENT_SERVICE_CHANGED_INDICATE, true)) {
                return;
        }

        if (pending_init_execute_and_check(PENDING_ACTION_AMS_ENABLE_REMOTE_COMMAND_NOTIF,
                                                ams_client_set_event_state, peer_info.ams_client,
                                                AMS_CLIENT_EVENT_REMOTE_COMMAND_NOTIFY, true)) {
                return;
        }

        if (pending_init_execute_and_check(PENDING_ACTION_AMS_ENABLE_ENTITY_UPDATE_NOTIF,
                                                ams_client_set_event_state, peer_info.ams_client,
                                                AMS_CLIENT_EVENT_ENTITY_UPDATE_NOTIFY, true)) {
                return;
        }

        if (pending_init_execute_and_check(PENDING_ACTION_AMS_ENABLE_ENTITY_UPDATE_PLAYER,
                                                ams_client_entity_update_command,
                                                peer_info.ams_client, AMS_CLIENT_ENTITY_ID_PLAYER,
                                                sizeof(player_attributes), player_attributes)) {
                return;
        }

        if (pending_init_execute_and_check(PENDING_ACTION_AMS_ENABLE_ENTITY_UPDATE_QUEUE,
                                                ams_client_entity_update_command,
                                                peer_info.ams_client, AMS_CLIENT_ENTITY_ID_QUEUE,
                                                sizeof(queue_attributes), queue_attributes)) {
                return;
        }

        if (pending_init_execute_and_check(PENDING_ACTION_AMS_ENABLE_ENTITY_UPDATE_TRACK,
                                                ams_client_entity_update_command,
                                                peer_info.ams_client, AMS_CLIENT_ENTITY_ID_TRACK,
                                                sizeof(track_attributes), track_attributes)) {
                return;
        }
}

static void add_pending_action(pending_action_t action)
{
        peer_info.pending_init |= action;

        process_pending_actions();
}

static void clear_pending_action(pending_action_t action, att_error_t error)
{
        /* Do nothing if we try to clear action which is not pending */
        if ((peer_info.pending_init & action) == 0) {
                return;
        }

        /* Try to authenticate if action failed due to insufficient authentication/encryption */
        if ((error == ATT_ERROR_INSUFFICIENT_AUTHENTICATION) ||
                                                (error == ATT_ERROR_INSUFFICIENT_ENCRYPTION)) {
                ble_error_t status = ble_gap_set_sec_level(peer_info.conn_idx, GAP_SEC_LEVEL_2);

                peer_info.busy_init = false;

                if (status == BLE_ERROR_ALREADY_DONE) {
                        /* Security has been elevated in the meantime, retry action */
                        process_pending_actions();
                }

                return;
        }

        peer_info.busy_init = false;
        peer_info.pending_init &= ~action;

        process_pending_actions();

        if (!peer_info.pending_init) {
                printf("Ready.\r\n");
        }
}

static attribute_request_t *attribute_request_new(ams_client_entity_id_t entity_id,
                                                                        uint8_t attribute_id)
{
        attribute_request_t *attribute = OS_MALLOC(sizeof(attribute_request_t));

        attribute->entity_id = entity_id;
        attribute->attribute_id = attribute_id;
        attribute->state = ATTRIBUTE_REQUEST_STATE_IDLE;

        return attribute;
}

static void process_attribute_requests(void)
{
        attribute_request_t *attribute;
        bool status = false;

        do {
                attribute = queue_peek_front(&peer_info.attribute_requests);
                if (!attribute) {
                        // noting to process
                        return;
                }

                if (attribute->state != ATTRIBUTE_REQUEST_STATE_IDLE) {
                        // current attribute is processed
                        return;
                }

                status = ams_client_entity_attribute_write(peer_info.ams_client, attribute->entity_id,
                                                                                attribute->attribute_id);
                if (status) {
                        attribute->state = ATTRIBUTE_REQUEST_STATE_WRITE;
                } else {
                        queue_pop_front(&peer_info.attribute_requests);
                        OS_FREE(attribute);
                }
        } while (!status);
}

static void purge_gatt(void)
{
        ble_storage_remove(peer_info.conn_idx, GATT_CLIENT_STORAGE_ID);

        ble_client_remove(peer_info.gatt_client);
        ble_client_cleanup(peer_info.gatt_client);
        peer_info.gatt_client = NULL;
}

static void purge_ams(void)
{
        ble_storage_remove(peer_info.conn_idx, AMS_CLIENT_STORAGE_ID);

        ble_client_remove(peer_info.ams_client);
        ble_client_cleanup(peer_info.ams_client);
        peer_info.ams_client = NULL;
}

static void purge_clients(void)
{
        purge_gatt();
        purge_ams();
}

static bool browse_req_match_range(const void *data, const void *match_data)
{
        const browse_req_t *req = data;
        const browse_req_t *range = match_data;

        return ((req->start_h <= range->start_h) && (req->end_h >= range->end_h));
}

static browse_req_t *find_browse_req(uint16_t start_h, uint16_t end_h)
{
        browse_req_t range;

        range.start_h = start_h;
        range.end_h = end_h;

        return queue_find(&peer_info.svc_changed_queue, browse_req_match_range, &range);
}

static void add_browse_req(uint16_t start_h, uint16_t end_h)
{
        browse_req_t *req;

        if (find_browse_req(start_h, end_h)) {
                return;
        }

        req = OS_MALLOC(sizeof(browse_req_t));
        req->start_h = start_h;
        req->end_h = end_h;

        queue_push_back(&peer_info.svc_changed_queue, req);
}

static void gatt_service_changed_cb(ble_client_t *gatt_client, uint16_t start_handle,
                                                                        uint16_t end_handle)
{
        uint16_t conn_idx = gatt_client->conn_idx;

        printf("Service changed, start_h: 0x%04x, end_h: 0x%04x\r\n", start_handle, end_handle);

        if (peer_info.gatt_client && ble_client_in_range(peer_info.gatt_client, start_handle,
                                                                                end_handle)) {
                purge_gatt();
        }

        if (peer_info.ams_client && ble_client_in_range(peer_info.ams_client, start_handle,
                                                                                end_handle)) {
                purge_ams();
        }

        if (peer_info.browsing) {
                // Start browse once again when the current will be completed
                add_browse_req(start_handle, end_handle);
                return;
        }

        printf("Browsing...\r\n");

        peer_info.browsing = true;
        peer_info.pending_init = 0;
        peer_info.busy_init = false;

        ble_gattc_browse_range(conn_idx, start_handle, end_handle, NULL);
}

static void gatt_set_event_state_cb(ble_client_t *gatt_client, gatt_client_event_t event,
                                                                                att_error_t status)
{
        if (event != GATT_CLIENT_EVENT_SERVICE_CHANGED_INDICATE) {
                return;
        }

        clear_pending_action(PENDING_ACTION_GATT_ENABLE_SERVICE_CHANGED_IND, status);
}

static const gatt_client_callbacks_t gatt_callbacks = {
        .service_changed = gatt_service_changed_cb,
        .set_event_state_completed = gatt_set_event_state_cb,
};

static void ams_set_event_state_cb(ble_client_t *ams_client, att_error_t status,
                                                                        ams_client_event_t event)
{
        if (event == AMS_CLIENT_EVENT_REMOTE_COMMAND_NOTIFY) {
                clear_pending_action(PENDING_ACTION_AMS_ENABLE_REMOTE_COMMAND_NOTIF, status);
        } else if (event == AMS_CLIENT_EVENT_ENTITY_UPDATE_NOTIFY) {
                if (status == ATT_ERROR_OK) {
                        add_pending_action(PENDING_ACTION_AMS_ENABLE_ENTITY_UPDATE_PLAYER);
                }

                clear_pending_action(PENDING_ACTION_AMS_ENABLE_ENTITY_UPDATE_NOTIF, status);
        }
}
#if CFG_VERBOSE_LOG
static const char *command2str(ams_client_remote_command_t command)
{
        switch (command) {
        case AMS_CLIENT_REMOTE_COMMAND_ID_PLAY:
                return "RemoteCommandIDPlay";
        case AMS_CLIENT_REMOTE_COMMAND_ID_PAUSE:
                return "RemoteCommandIDPause";
        case AMS_CLIENT_REMOTE_COMMAND_ID_TOGGLE_PLAY_PAUSE:
                return "RemoteCommandIDTogglePlayPause";
        case AMS_CLIENT_REMOTE_COMMAND_ID_NEXT_TRACK:
                return "RemoteCommandIDNextTrack";
        case AMS_CLIENT_REMOTE_COMMAND_ID_PREVIOUS_TRACK:
                return "RemoteCommandIDPreviousTrack";
        case AMS_CLIENT_REMOTE_COMMAND_ID_VOLUME_UP:
                return "RemoteCommandIDVolumeUp";
        case AMS_CLIENT_REMOTE_COMMAND_ID_VOLUME_DOWN:
                return "RemoteCommandIDVolumeDown";
        case AMS_CLIENT_REMOTE_COMMAND_ID_REPEAT_MODE:
                return "RemoteCommandIDAdvanceRepeatMode";
        case AMS_CLIENT_REMOTE_COMMAND_ID_SHUFFLE_MODE:
                return "RemoteCommandIDAdvanceShuffleMode";
        case AMS_CLIENT_REMOTE_COMMAND_ID_SKIP_FORWARD:
                return "RemoteCommandIDSkipForward";
        case AMS_CLIENT_REMOTE_COMMAND_ID_SKIP_BACKWARD:
                return "RemoteCommandIDSkipBackward";
        case AMS_CLIENT_REMOTE_COMMAND_ID_LIKE_TRACK:
                return "RemoteCommandIDLikeTrack";
        case AMS_CLIENT_REMOTE_COMMAND_ID_DISLIKE_TRACK:
                return "RemoteCommandIDDislikeTrack";
        case AMS_CLIENT_REMOTE_COMMAND_ID_BOOKMARK_TRACK:
                return "RemoteCommandIDBookmarkTrack";
        }

        return "<unknown>";
}

static const char *entity2str(ams_client_entity_id_t entity_id)
{
        switch (entity_id) {
        case AMS_CLIENT_ENTITY_ID_PLAYER:
                return "EntityIDPlayer";
        case AMS_CLIENT_ENTITY_ID_QUEUE:
                return "EntityIDQueue";
        case AMS_CLIENT_ENTITY_ID_TRACK:
                return "EntityIDTrack";
        }

        return "<unknown>";
}

const char *player_attribute2str(ams_client_player_attribute_id_t attribute_id)
{
        switch (attribute_id) {
        case AMS_CLIENT_PLAYER_ATTRIBUTE_ID_NAME:
                return "PlayerAttributeIDName";
        case AMS_CLIENT_PLAYER_ATTRIBUTE_ID_PLAYBACK_INFO:
                return "PlayerAttributeIDPlaybackInfo";
        case AMS_CLIENT_PLAYER_ATTRIBUTE_ID_VOLUME:
                return "PlayerAttributeIDVolume";
        }

        return "<unknown>";
}

const char *queue_attribute2str(ams_client_queue_attribute_id_t attribute_id)
{
        switch (attribute_id) {
        case AMS_CLIENT_QUEUE_ATTRIBUTE_ID_INDEX:
                return "QueueAttributeIDIndex";
        case AMS_CLIENT_QUEUE_ATTRIBUTE_ID_COUNT:
                return "QueueAttributeIDCount";
        case AMS_CLIENT_QUEUE_ATTRIBUTE_ID_SHUFFLE_MODE:
                return "QueueAttributeIDShuffleMode";
        case AMS_CLIENT_QUEUE_ATTRIBUTE_ID_REPEAT_MODE:
                return "QueueAttributeIDRepeatMode";
        }

        return "<unknown>";
}

const char *track_attribute2str(ams_client_track_attribute_id_t attribute_id)
{
        switch (attribute_id) {
        case AMS_CLIENT_TRACK_ATTRIBUTE_ID_ARTIST:
                return "TrackAttributeIDArtist";
        case AMS_CLIENT_TRACK_ATTRIBUTE_ID_ALBUM:
                return "TrackAttributeIDAlbum";
        case AMS_CLIENT_TRACK_ATTRIBUTE_ID_TITLE:
                return "TrackAttributeIDTitle";
        case AMS_CLIENT_TRACK_ATTRIBUTE_ID_DURATION:
                return "TrackAttributeIDDuration";
        }

        return "<unknown>";
}

static const char *attribute2str(ams_client_entity_id_t entity_id, uint8_t attribute_id)
{
        switch (entity_id) {
        case AMS_CLIENT_ENTITY_ID_PLAYER:
                return player_attribute2str(attribute_id);
        case AMS_CLIENT_ENTITY_ID_QUEUE:
                return queue_attribute2str(attribute_id);
        case AMS_CLIENT_ENTITY_ID_TRACK:
                return track_attribute2str(attribute_id);
        }

        return "<unknown>";
}
#endif
static void ams_remote_commands_update_cb(ble_client_t *client, uint16_t length,
                                                                        const uint8_t *commands)
{
#if CFG_VERBOSE_LOG
        uint16_t i;

        printf("Remote commands update\r\n");
        printf("\tNumber of supported commands: 0x%02X\r\n", length);
        printf("\tCommands:\r\n");

        for (i = 0; i < length; i++) {
                printf("\t\t%s (0x%02X)\r\n", command2str(commands[i]), commands[i]);
        }
#endif
}

static void ams_entity_update_command_completed_cb(ble_client_t *client, att_error_t status)
{
        if (peer_info.pending_init & PENDING_ACTION_AMS_ENABLE_ENTITY_UPDATE_PLAYER) {
                if (status == ATT_ERROR_OK) {
                        add_pending_action(PENDING_ACTION_AMS_ENABLE_ENTITY_UPDATE_QUEUE);
                }

                clear_pending_action(PENDING_ACTION_AMS_ENABLE_ENTITY_UPDATE_PLAYER, status);
        } else if (peer_info.pending_init & PENDING_ACTION_AMS_ENABLE_ENTITY_UPDATE_QUEUE) {
                if (status == ATT_ERROR_OK) {
                        add_pending_action(PENDING_ACTION_AMS_ENABLE_ENTITY_UPDATE_TRACK);
                }

                clear_pending_action(PENDING_ACTION_AMS_ENABLE_ENTITY_UPDATE_QUEUE, status);
        } else {
                clear_pending_action(PENDING_ACTION_AMS_ENABLE_ENTITY_UPDATE_TRACK, status);
        }
}

static void ams_entity_update_cb(ble_client_t *client, ams_client_entity_id_t entity_id,
                                uint8_t attribute_id, ams_client_entity_update_flag_t flags,
                                uint16_t length, const uint8_t *value)
{
#if CFG_VERBOSE_LOG
        printf("Entity Update\r\n");
        printf("\tEntity ID: %s (0x%02X)\r\n", entity2str(entity_id), entity_id);
        printf("\tAttribute ID: %s (0x%02X)\r\n", attribute2str(entity_id, attribute_id),
                                                                                attribute_id);
        printf("\tFlags: %s(0x%02X)\r\n", flags & AMS_CLIENT_ENTITY_UPDATE_FLAG_TRUNCATED ?
                                                                        "Truncated, " : "", flags);
        printf("\tValue: %.*s\r\n", length, value);
#endif
        if (flags & AMS_CLIENT_ENTITY_UPDATE_FLAG_TRUNCATED) {
                attribute_request_t *attribute = attribute_request_new(entity_id, attribute_id);

                queue_push_back(&peer_info.attribute_requests, attribute);
                process_attribute_requests();
        }

        display_update(entity_id, attribute_id, length, value);
}

static void ams_remote_command_completed_cb(ble_client_t *client, att_error_t status)
{
#if CFG_VERBOSE_LOG
        printf("Remote command completed, status: 0x%02X\r\n", status);
#endif
}

static void ams_entity_attribute_write_completed(ble_client_t *client, att_error_t status)
{
        attribute_request_t *attribute = queue_peek_front(&peer_info.attribute_requests);
        if (!attribute || attribute->state != ATTRIBUTE_REQUEST_STATE_WRITE) {
                // noting to process
                return;
        }

        if (status != ATT_ERROR_OK) {
#if CFG_VERBOSE_LOG
                printf("Failed to write Entity Attribute characteristic, status: 0x%02X\r\n",
                                                                                        status);
#endif
                queue_pop_front(&peer_info.attribute_requests);
                OS_FREE(attribute);
        } else {
                bool status = ams_client_entity_attribute_read(client);
                if (status) {
                        attribute->state = ATTRIBUTE_REQUEST_STATE_READ;

                        return;
                } else {
#if CFG_VERBOSE_LOG
                        printf("Failed to trigger read of Entity Attribute characteristic\r\n");
#endif
                        queue_pop_front(&peer_info.attribute_requests);
                        OS_FREE(attribute);
                }
        }

        process_attribute_requests();
}

static void ams_entity_attribute_read_completed_cb(ble_client_t *client, att_error_t status,
                                                        uint16_t length, const uint8_t *value)
{
        attribute_request_t *attribute = queue_peek_front(&peer_info.attribute_requests);
        if (!attribute || attribute->state != ATTRIBUTE_REQUEST_STATE_READ) {
                // noting to process
                return;
        }

        if (status == ATT_ERROR_OK) {
                display_update(attribute->entity_id, attribute->attribute_id, length, value);
#if CFG_VERBOSE_LOG
                printf("Entity Attribute Read Completed\r\n");
                printf("\tEntity ID: %s (0x%02X)\r\n", entity2str(attribute->entity_id),
                                                                        attribute->entity_id);
                printf("\tAttribute ID: %s (0x%02X)\r\n", attribute2str(attribute->entity_id,
                                                attribute->attribute_id), attribute->attribute_id);
                printf("\tValue: %.*s\r\n", length, value);
        } else {
                printf("Entity Attribute Read Failed, status: 0x%02X\r\n", status);
#endif
        }

        queue_pop_front(&peer_info.attribute_requests);
        OS_FREE(attribute);
        process_attribute_requests();
}

static const ams_client_callbacks_t ams_callbacks = {
        .set_event_state_completed = ams_set_event_state_cb,
        .remote_commands_update = ams_remote_commands_update_cb,
        .remote_command_completed = ams_remote_command_completed_cb,
        .entity_update_command_completed = ams_entity_update_command_completed_cb,
        .entity_update = ams_entity_update_cb,
        .entity_attribute_read_completed = ams_entity_attribute_read_completed_cb,
        .entity_attribute_write_completed = ams_entity_attribute_write_completed,
};

static ble_client_t *get_stored_client(uint16_t conn_idx, ble_storage_key_t key)
{
        ble_error_t err;
        uint16_t len = 0;
        void *buffer;

        err = ble_storage_get_buffer(conn_idx, key, &len, &buffer);
        if (err) {
                return NULL;
        }

        switch (key) {
        case GATT_CLIENT_STORAGE_ID:
                return gatt_client_init_from_data(conn_idx, &gatt_callbacks, buffer, len);
        case AMS_CLIENT_STORAGE_ID:
                return ams_client_init_from_data(conn_idx, &ams_callbacks, buffer, len);
        default:
                return NULL;
        }
}

static void store_client(uint16_t conn_idx, ble_client_t *client, ble_storage_key_t key)
{
        uint8_t *buffer = NULL;
        size_t length;

        if (!client) {
                return;
        }

        /* Get serialized BLE Client length */
        ble_client_serialize(client, NULL, &length);
        buffer = OS_MALLOC(length);
        /* Serialize BLE Client */
        ble_client_serialize(client, buffer, &length);
        /* Put BLE Client to the storage */
        ble_storage_put_buffer(conn_idx, key, length, buffer, OS_FREE_FUNC, true);
}

static void handle_evt_gap_connected(ble_evt_gap_connected_t *evt)
{
        bool bonded = false;

        printf("Device connected\r\n");
        printf("\tConnection index: %d\r\n", evt->conn_idx);
        printf("\tAddress: %s, %s\r\n", evt->peer_address.addr_type == PRIVATE_ADDRESS ?
                                "private" : "public", ble_address_to_string(&evt->peer_address));

        peer_info.conn_idx = evt->conn_idx;
        queue_init(&peer_info.attribute_requests);
        queue_init(&peer_info.svc_changed_queue);
        peer_info.gatt_client = get_stored_client(peer_info.conn_idx, GATT_CLIENT_STORAGE_ID);
        peer_info.ams_client = get_stored_client(peer_info.conn_idx, AMS_CLIENT_STORAGE_ID);

        if (!peer_info.ams_client && !peer_info.gatt_client) {
                printf("Browsing...\r\n");

                ble_gattc_browse(evt->conn_idx, NULL);
                peer_info.browsing = true;
        } else if (peer_info.ams_client) {
                ams_client_cap_t caps = ams_client_get_capabilities(peer_info.ams_client);
                if (caps & AMS_CLIENT_CAP_REMOTE_COMMAND) {
                        add_pending_action(PENDING_ACTION_AMS_ENABLE_REMOTE_COMMAND_NOTIF);
                }

                if (caps & AMS_CLIENT_CAP_ENTITY_UPDATE) {
                        add_pending_action(PENDING_ACTION_AMS_ENABLE_ENTITY_UPDATE_NOTIF);
                }
        }

        ble_gap_is_bonded(evt->conn_idx, &bonded);
        if (bonded) {
                // Verify bond status
                ble_gap_set_sec_level(evt->conn_idx, GAP_SEC_LEVEL_2);
        }
}

#if CFG_VERBOSE_LOG
static void handle_evt_gap_pair_completed(ble_evt_gap_pair_completed_t *evt)
{
        printf("Pair completed\r\n");
        printf("\tConnection index: %d\r\n", evt->conn_idx);
        printf("\tStatus: 0x%02X\r\n", evt->status);

        if (evt->status == BLE_HCI_ERROR_NO_ERROR) {
                printf("\tBond: %s\r\n", evt->bond ? "true" : "false");
                printf("\tMITM: %s\r\n", evt->mitm ? "true" : "false");
        }
}

static void handle_evt_gap_data_length_changed(ble_evt_gap_data_length_changed_t *evt)
{
        printf("Data length changed\r\n");
        printf("\tConnection index: %d\r\n", evt->conn_idx);
        printf("\tMaximum RX data length: %d\r\n", evt->max_rx_length);
        printf("\tMaximum RX time: %d\r\n", evt->max_rx_time);
        printf("\tMaximum TX data length: %d\r\n", evt->max_tx_length);
        printf("\tMaximum TX time: %d\r\n", evt->max_tx_time);
}

#endif

static void handle_evt_gap_disconnected(ble_evt_gap_disconnected_t *evt)
{
        printf("Device disconnected\r\n");
        printf("\tConnection index: %d\r\n", evt->conn_idx);
        printf("\tBD address of disconnected device: %s, %s\r\n",
                                evt->address.addr_type == PUBLIC_ADDRESS ? "public" : "private",
                                ble_address_to_string(&evt->address));
        printf("\tReason of disconnection: 0x%02x\r\n", evt->reason);

        ble_client_cleanup(peer_info.gatt_client);
        ble_client_cleanup(peer_info.ams_client);
        queue_remove_all(&peer_info.attribute_requests, OS_FREE_FUNC);
        queue_remove_all(&peer_info.svc_changed_queue, OS_FREE_FUNC);
        memset(&peer_info, 0, sizeof(peer_info));
        peer_info.conn_idx = BLE_CONN_IDX_INVALID;

        display_device_disconnected();

        ble_gap_adv_start(GAP_CONN_MODE_UNDIRECTED);
}

static void handle_evt_gap_pair_req(ble_evt_gap_pair_req_t *evt)
{
        bool bonded = false;

        // Discover services again if lost bond information
        ble_gap_is_bonded(evt->conn_idx, &bonded);
        if (bonded && !peer_info.browsing) {
                purge_clients();
                peer_info.busy_init = false;
                peer_info.pending_init = 0;
                peer_info.browsing = true;
                ble_gattc_browse(evt->conn_idx, NULL);
        }

        ble_gap_pair_reply(evt->conn_idx, true, evt->bond);
}

static void handle_evt_gattc_browse_svc(ble_evt_gattc_browse_svc_t *evt)
{
        att_uuid_t uuid;

        ble_uuid_from_string(UUID_AMS, &uuid);
        if (ble_uuid_equal(&uuid, &evt->uuid) && !peer_info.ams_client) {
                peer_info.ams_client = ams_client_init(&ams_callbacks, evt);
                if (!peer_info.ams_client) {
                        return;
                }

                ble_client_add(peer_info.ams_client);
                store_client(evt->conn_idx, peer_info.ams_client, AMS_CLIENT_STORAGE_ID);
                return;
        }

        ble_uuid_create16(UUID_SERVICE_GATT, &uuid);
        if (ble_uuid_equal(&uuid, &evt->uuid) && !peer_info.gatt_client) {
                peer_info.gatt_client = gatt_client_init(&gatt_callbacks, evt);
                if (!peer_info.gatt_client) {
                        return;
                }

                ble_client_add(peer_info.gatt_client);
                store_client(evt->conn_idx, peer_info.gatt_client, GATT_CLIENT_STORAGE_ID);
                return;
        }
}

static void handle_evt_gattc_browse_completed(ble_evt_gattc_browse_completed_t *evt)
{
        ams_client_cap_t caps;

        printf("Browse completed\r\n");
        printf("GATT: %s\r\n", peer_info.gatt_client ? "found" : "not found");
        printf("AMS: %s\r\n", peer_info.ams_client ? "found" : "not found");

        if (queue_length(&peer_info.svc_changed_queue)) {
                browse_req_t *req = queue_pop_front(&peer_info.svc_changed_queue);

                printf("Services changed, browsing at range from 0x%04x to 0x%04x...\r\n",
                                                                         req->start_h, req->end_h);

                if (peer_info.gatt_client && ble_client_in_range(peer_info.gatt_client,
                                                                req->start_h, req->end_h)) {
                        purge_gatt();
                }

                if (peer_info.ams_client && ble_client_in_range(peer_info.ams_client,
                                                                req->start_h, req->end_h)) {
                        purge_ams();
                }

                ble_gattc_browse_range(evt->conn_idx, req->start_h, req->end_h, NULL);
                peer_info.browsing = true;

                OS_FREE(req);
                return;
        }

        peer_info.browsing = false;

        if (peer_info.gatt_client && (gatt_client_get_capabilites(peer_info.gatt_client) &
                                                                GATT_CLIENT_CAP_SERVICE_CHANGED)) {
                add_pending_action(PENDING_ACTION_GATT_ENABLE_SERVICE_CHANGED_IND);
        }

        if (!peer_info.ams_client) {
                return;
        }

        caps = ams_client_get_capabilities(peer_info.ams_client);
        if (caps & AMS_CLIENT_CAP_REMOTE_COMMAND) {
                add_pending_action(PENDING_ACTION_AMS_ENABLE_REMOTE_COMMAND_NOTIF);
        }

        if (caps & AMS_CLIENT_CAP_ENTITY_UPDATE) {
                add_pending_action(PENDING_ACTION_AMS_ENABLE_ENTITY_UPDATE_NOTIF);
        }
}

static void remote_cmd(int argc, const char *argv[], void *user_data)
{
        ams_client_remote_command_t cmd;
        bool status;
        char *end = NULL;
        long arg;

        printf("Remote command\r\n");

        if (argc != 2) {
                printf("\tInvalid count of arguments: cmd <remote_command>\r\n");
                return;
        }

        if (peer_info.conn_idx == BLE_CONN_IDX_INVALID) {
                printf("\tNot connected.\r\n");
                return;
        }

        if (!peer_info.ams_client) {
                printf("\tAMS Client does not exist!\r\n");
                return;
        }

        if (!(ams_client_get_capabilities(peer_info.ams_client) & AMS_CLIENT_CAP_REMOTE_COMMAND)) {
                printf("\tAMS Client does not support remote command characteristic!\r\n");
                return;
        }

        errno = 0;

        arg = strtol(argv[1], &end, 0);
        if (end == argv[1] || *end != 0) {
                printf("\tPlease enter a valid remote command value\r\n");
                return;
        }

        if (errno || arg < AMS_CLIENT_REMOTE_COMMAND_ID_PLAY ||
                                        arg > AMS_CLIENT_REMOTE_COMMAND_ID_BOOKMARK_TRACK) {
                printf("\tValid commands range is 0x%02x - 0x%02x\r\n",
                                                AMS_CLIENT_REMOTE_COMMAND_ID_PLAY,
                                                AMS_CLIENT_REMOTE_COMMAND_ID_BOOKMARK_TRACK);
                return;
        }

        cmd = arg;

        status = ams_client_remote_command(peer_info.ams_client, cmd);
        printf("\tRemote command 0x%02x %s\r\n", cmd, status ? "sent" : "failed");
}

static void fetch_attribute(int argc, const char *argv[], void *user_data)
{
        ams_client_entity_id_t entity_id;
        attribute_request_t *attribute;
        uint8_t attribute_id;

        printf("Fetch attribute\r\n");

        if (argc != 3) {
                printf("\tInvalid count of arguments: fetch <entity_id> <attribute_id>\r\n");
                return;
        }

        if (peer_info.conn_idx == BLE_CONN_IDX_INVALID) {
                printf("\tNot connected.\r\n");
                return;
        }

        if (!peer_info.ams_client) {
                printf("\tAMS Client does not exist!\r\n");
                return;
        }

        if (!(ams_client_get_capabilities(peer_info.ams_client) &
                                                        AMS_CLIENT_CAP_ENTITY_ATTRIBUTE)) {
                printf("\tAMS Client does not support entity attribute characteristic!\r\n");
                return;
        }

        if (!parse_u8(argv[1], &entity_id)) {
                printf("\tInvalid value of <entity_id>.\r\n");
                return;
        }

        if (!parse_u8(argv[2], &attribute_id)) {
                printf("\tInvalid value of <attribute_id>.\r\n");
                return;
        }

        attribute = attribute_request_new(entity_id, attribute_id);
        queue_push_back(&peer_info.attribute_requests, attribute);
        process_attribute_requests();

        printf("\tPushed to attribute requests queue\r\n");
}

static void handle_evt_gap_sec_level_changed(ble_evt_gap_sec_level_changed_t *evt)
{
        printf("Security level changed\r\n");
        printf("\tConnection index: %d\r\n", evt->conn_idx);
        printf("\tSecurity level: %d\r\n", evt->level + 1);

        process_pending_actions();
}

const static cli_command_t debug_handlers[] = {
        { "cmd", remote_cmd, NULL },
        { "fetch", fetch_attribute, NULL },
        {},
};

static void default_handler(int argc, const char *argv[], void *user_data)
{
        printf("Valid commands:\r\n");
        printf("\tcmd <remote_command>\r\n");
        printf("\tfetch <entity_id> <attribute_id>\r\n");
}

static cli_t register_debug(uint32_t notif_mask)
{
        return cli_register(notif_mask, debug_handlers, default_handler);
}

void ams_wkup_handler(void)
{
        if (current_task) {
                OS_TASK_NOTIFY_FROM_ISR(current_task, BUTTON_NOTIF, eSetBits);
        }
}

void ams_task(void *params)
{
        int8_t wdog_id;
        cli_t cli;

        /* register ams task to be monitored by watchdog */
        wdog_id = sys_watchdog_register(false);

        /* Initialize CLI */
        cli = register_debug(CLI_NOTIF);

        current_task = OS_GET_CURRENT_TASK();

        ble_peripheral_start();
        ble_gap_mtu_size_set(128);
        ble_register_app();
        /*
         * Set device name and appearance to be discoverable by iOS devices
         */
        ble_gap_device_name_set("Dialog AMS Demo", ATT_PERM_READ);
        ble_gap_appearance_set(BLE_GAP_APPEARANCE_GENERIC_WATCH, ATT_PERM_READ);

        /*
         * Unbond all devices if user holds button during start
         */
        if (!hw_gpio_get_pin_status(CFG_USER_BUTTON_PORT, CFG_USER_BUTTON_PIN)) {
                size_t bonded_length = defaultBLE_MAX_BONDED;
                static gap_device_t devices[defaultBLE_MAX_BONDED];
                ble_error_t err;

                err = ble_gap_get_devices(GAP_DEVICE_FILTER_BONDED, NULL, &bonded_length, devices);
                if (err == BLE_STATUS_OK && bonded_length > 0) {
                        size_t i;

                        printf("Unbonding %d device%s\r\n", bonded_length,
                                                                bonded_length > 1 ? "s" : "");

                        for (i = 0; i < bonded_length; i++) {
                                printf("\t%s [%u/%u]\r\n",
                                        ble_address_to_string(&devices[i].address), i + 1,
                                        bonded_length);

                                ble_gap_unpair(&devices[i].address);
                        }
                }
        }


        ble_gap_adv_data_set(sizeof(adv_data), adv_data, sizeof(scan_rsp), scan_rsp);
        ble_gap_adv_start(GAP_CONN_MODE_UNDIRECTED);

        printf("Start advertising...\r\n");

        for (;;) {
                OS_BASE_TYPE ret;
                uint32_t notif;

                /* Notify watchdog on each loop */
                sys_watchdog_notify(wdog_id);

                /* Suspend watchdog while blocking on OS_TASK_NOTIFY_WAIT() */
                sys_watchdog_suspend(wdog_id);

                /*
                 * Wait on any of the notification bits, then clear them all
                 */
                ret = OS_TASK_NOTIFY_WAIT(0, OS_TASK_NOTIFY_ALL_BITS, &notif, OS_TASK_NOTIFY_FOREVER);
                OS_ASSERT(ret == OS_OK);

                /* Resume watchdog */
                sys_watchdog_notify_and_resume(wdog_id);

                /* Notified from BLE manager, can get event */
                if (notif & BLE_APP_NOTIFY_MASK) {
                        ble_evt_hdr_t *hdr;

                        hdr = ble_get_event(false);
                        if (!hdr) {
                                goto no_event;
                        }

                        ble_client_handle_event(hdr);

                        if (!ble_service_handle_event(hdr)) {
                                switch (hdr->evt_code) {
                                case BLE_EVT_GAP_CONNECTED:
                                        handle_evt_gap_connected((ble_evt_gap_connected_t *) hdr);
                                        break;
#if CFG_VERBOSE_LOG
                                case BLE_EVT_GAP_DATA_LENGTH_CHANGED:
                                        handle_evt_gap_data_length_changed((ble_evt_gap_data_length_changed_t *) hdr);
                                        break;
                                case BLE_EVT_GAP_PAIR_COMPLETED:
                                        handle_evt_gap_pair_completed((ble_evt_gap_pair_completed_t *) hdr);
                                        break;
#endif
                                case BLE_EVT_GAP_DISCONNECTED:
                                        handle_evt_gap_disconnected((ble_evt_gap_disconnected_t *) hdr);
                                        break;
                                case BLE_EVT_GAP_PAIR_REQ:
                                        handle_evt_gap_pair_req((ble_evt_gap_pair_req_t *) hdr);
                                        break;
                                case BLE_EVT_GAP_SEC_LEVEL_CHANGED:
                                        handle_evt_gap_sec_level_changed((ble_evt_gap_sec_level_changed_t *) hdr);
                                        break;
                                case BLE_EVT_GATTC_BROWSE_SVC:
                                        handle_evt_gattc_browse_svc((ble_evt_gattc_browse_svc_t *) hdr);
                                        break;
                                case BLE_EVT_GATTC_BROWSE_COMPLETED:
                                        handle_evt_gattc_browse_completed((ble_evt_gattc_browse_completed_t *) hdr);
                                        break;
                                default:
                                        ble_handle_event_default(hdr);
                                        break;
                                }
                        }

                        OS_FREE(hdr);

no_event:
                        /* Notify again if there are more events to process in queue */
                        if (ble_has_event()) {
                                OS_TASK_NOTIFY(OS_GET_CURRENT_TASK(), BLE_APP_NOTIFY_MASK, eSetBits);
                        }
                }

                if (notif & CLI_NOTIF) {
                        cli_handle_notified(cli);
                }

                if (notif & BUTTON_NOTIF) {
                        if (peer_info.conn_idx != BLE_CONN_IDX_INVALID && peer_info.ams_client) {
                                ams_client_remote_command(peer_info.ams_client,
                                                AMS_CLIENT_REMOTE_COMMAND_ID_TOGGLE_PLAY_PAUSE);
                        }
                }
        }
}
