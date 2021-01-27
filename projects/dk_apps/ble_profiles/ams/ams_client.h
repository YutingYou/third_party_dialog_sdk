/**
 ****************************************************************************************
 *
 * @file ams_client.h
 *
 * @brief Apple Media Service client interface
 *
 * Copyright (C) 2018 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef AMS_CLIENT_H_
#define AMS_CLIENT_H_

#include <stdint.h>
#include "ble_client.h"

/**
 * Client capabilities
 *
 * AMS exposes three characteristics: Remote Command, Entity Update and Entity Attribute.
 * The AMS specification defines that any subset of these characteristics may be supported.
 */
typedef enum {
        AMS_CLIENT_CAP_REMOTE_COMMAND = 0x01,           ///< Remote Command characteristic
        AMS_CLIENT_CAP_ENTITY_UPDATE = 0x02,            ///< Entity Update characteristic
        AMS_CLIENT_CAP_ENTITY_ATTRIBUTE = 0x04,         ///< Entity Attribute characteristic
} ams_client_cap_t;

/**
 * Configurable client events
 */
typedef enum {
        AMS_CLIENT_EVENT_REMOTE_COMMAND_NOTIFY,         ///< Remote Command notifications
        AMS_CLIENT_EVENT_ENTITY_UPDATE_NOTIFY,          ///< Entity Update notifications
} ams_client_event_t;

/**
 * AMS Client Remote Commands
 */
typedef enum {
        AMS_CLIENT_REMOTE_COMMAND_ID_PLAY = 0x00,               ///< Remote Command ID Play
        AMS_CLIENT_REMOTE_COMMAND_ID_PAUSE = 0x01,              ///< Remote Command ID Pause
        AMS_CLIENT_REMOTE_COMMAND_ID_TOGGLE_PLAY_PAUSE = 0x02,  ///< Remote Command ID Toggle Play Pause
        AMS_CLIENT_REMOTE_COMMAND_ID_NEXT_TRACK = 0x03,         ///< Remote Command ID Next Track
        AMS_CLIENT_REMOTE_COMMAND_ID_PREVIOUS_TRACK = 0x04,     ///< Remote Command ID Previous Track
        AMS_CLIENT_REMOTE_COMMAND_ID_VOLUME_UP = 0x05,          ///< Remote Command ID Volume Up
        AMS_CLIENT_REMOTE_COMMAND_ID_VOLUME_DOWN = 0x06,        ///< Remote Command ID Volume Down
        AMS_CLIENT_REMOTE_COMMAND_ID_REPEAT_MODE = 0x07,        ///< Remote Command ID Repeat Mode
        AMS_CLIENT_REMOTE_COMMAND_ID_SHUFFLE_MODE = 0x08,       ///< Remote Command ID Shuffle Mode
        AMS_CLIENT_REMOTE_COMMAND_ID_SKIP_FORWARD = 0x09,       ///< Remote Command ID Skip Forward
        AMS_CLIENT_REMOTE_COMMAND_ID_SKIP_BACKWARD = 0x0A,      ///< Remote Command ID Skip Backward
        AMS_CLIENT_REMOTE_COMMAND_ID_LIKE_TRACK = 0x0B,         ///< Remote Command ID Like Track
        AMS_CLIENT_REMOTE_COMMAND_ID_DISLIKE_TRACK = 0x0C,      ///< Remote Command ID Dislike Track
        AMS_CLIENT_REMOTE_COMMAND_ID_BOOKMARK_TRACK = 0x0D,     ///< Remote Command ID Bookmark Track
} ams_client_remote_command_t;

/**
 * AMS Client Error codes
 */
typedef enum {
        AMS_CLIENT_ERROR_INVALID_STATE = 0xA0,          ///< Error Invalid State
        AMS_CLIENT_ERROR_INVALID_COMMAND = 0xA1,        ///< Error Invalid Command
        AMS_CLIENT_ERROR_ABSENT_ATTRIBUTE = 0xA2,       ///< Error Absent Attribute
} ams_client_error_t;

/**
 * AMS Client Entity ID
 */
typedef enum {
        AMS_CLIENT_ENTITY_ID_PLAYER = 0x00,     ///< Entity ID Player
        AMS_CLIENT_ENTITY_ID_QUEUE = 0x01,      ///< Entity ID Queue
        AMS_CLIENT_ENTITY_ID_TRACK = 0x02,      ///< Entity ID Track
} ams_client_entity_id_t;

/**
 * AMS Client Player Attribute ID
 */
typedef enum {
        AMS_CLIENT_PLAYER_ATTRIBUTE_ID_NAME = 0x00,             ///< Player Attribute ID Name
        AMS_CLIENT_PLAYER_ATTRIBUTE_ID_PLAYBACK_INFO = 0x01,    ///< Player Attribute ID Playback Info
        AMS_CLIENT_PLAYER_ATTRIBUTE_ID_VOLUME = 0x02,           ///< Player Attribute ID Volume
} ams_client_player_attribute_id_t;

/**
 * AMS Client Queue Attribute ID
 */
typedef enum {
        AMS_CLIENT_QUEUE_ATTRIBUTE_ID_INDEX = 0x00,             ///< Queue Attribute ID Index
        AMS_CLIENT_QUEUE_ATTRIBUTE_ID_COUNT = 0x01,             ///< Queue Attribute ID Count
        AMS_CLIENT_QUEUE_ATTRIBUTE_ID_SHUFFLE_MODE = 0x02,      ///< Queue Attribute ID Shuffle Mode
        AMS_CLIENT_QUEUE_ATTRIBUTE_ID_REPEAT_MODE = 0x03,       ///< Queue Attribute ID Repeat Mode
} ams_client_queue_attribute_id_t;

/**
 * AMS Client Track Attribute ID
 */
typedef enum {
        AMS_CLIENT_TRACK_ATTRIBUTE_ID_ARTIST = 0x00,            ///< Track Attribute ID Artist
        AMS_CLIENT_TRACK_ATTRIBUTE_ID_ALBUM = 0x01,             ///< Track Attribute ID Album
        AMS_CLIENT_TRACK_ATTRIBUTE_ID_TITLE = 0x02,             ///< Track Attribute ID Title
        AMS_CLIENT_TRACK_ATTRIBUTE_ID_DURATION = 0x03,          ///< Track Attribute ID Duration
} ams_client_track_attribute_id_t;

/**
 * AMS Client Entity Update Flags
 */
typedef enum {
        AMS_CLIENT_ENTITY_UPDATE_FLAG_TRUNCATED = 0x01,         ///< Entity Update Flag Truncated
} ams_client_entity_update_flag_t;

/**
 * AMS CLient Playback State
 */
typedef enum {
        AMS_CLIENT_PLAYBACK_STATE_PAUSED = 0x00,                ///< Playback State Paused
        AMS_CLIENT_PLAYBACK_STATE_PLAYING = 0x01,               ///< Playback State Playing
        AMS_CLIENT_PLAYBACK_STATE_REWINDING = 0x02,             ///< Playback State Rewinding
        AMS_CLIENT_PLAYBACK_STATE_FAST_FORWARDING = 0x03,       ///< Playback State Fast Forwarding
} ams_client_playback_state_t;

/**
 * AMS Client Shuffle Mode
 */
typedef enum {
        AMS_CLIENT_SHUFFLE_MODE_OFF = 0x00,     ///< Shuffle Mode Off
        AMS_CLIENT_SHUFFLE_MODE_ONE = 0x01,     ///< Shuffle Mode One
        AMS_CLIENT_SHUFFLE_MODE_ALL = 0x02,     ///< Shuffle Mode All
} ams_client_shuffle_mode_t;

/**
 * AMS Client Repeat Mode
 */
typedef enum {
        AMS_CLIENT_REPEAT_MODE_OFF = 0x00,      ///< Repeat Mode Off
        AMS_CLIENT_REPEAT_MODE_ONE = 0x01,      ///< Repeat Mode One
        AMS_CLIENT_REPEAT_MODE_ALL = 0x02,      ///< Repeat Mode All
} ams_client_repeat_mode_t;

/**
 * Application callbacks
 */
typedef struct {
        /**
         * Get event state completed
         *
         * Called when ams_client_get_event_state() action is completed.
         *
         * \param [in] client   AMS client instance
         * \param [in] status   operation status
         * \param [in] event    requested event
         * \param [in] enabled  event state (true if enabled, false otherwise)
         *
         */
        void (* get_event_state_completed) (ble_client_t *client, att_error_t status,
                                                        ams_client_event_t event, bool enabled);

        /**
         * Set event state completed
         *
         * Called when ams_client_set_event_state() action is completed.
         *
         * \param [in] client   AMS client instance
         * \param [in] status   operation status
         * \param [in] event    requested event
         *
         */
        void (* set_event_state_completed) (ble_client_t *client, att_error_t status,
                                                                        ams_client_event_t event);

        /**
         * Remote Command completed
         *
         * Called when ams_client_remote_command() action is completed.
         *
         * \param [in] client   AMS client instance
         * \param [in] status   operation status
         *
         */
        void (* remote_command_completed) (ble_client_t *client, att_error_t status);

        /**
         * Supported Remote Commands update
         *
         * Called when list of commands supported by the media player changes.
         *
         * \param [in] client   AMS client instance
         * \param [in] length   number of supported remote commands
         * \param [in] commands list of supported commands
         *
         */
        void (* remote_commands_update) (ble_client_t *client, uint16_t length,
                                                                        const uint8_t *commands);

        /**
         * Entity Update Command completed
         *
         * Called when ams_client_entity_update_command() action is completed.
         *
         * \param [in] client   AMS client instance
         * \param [in] status   operation status
         *
         */
        void (* entity_update_command_completed) (ble_client_t *client, att_error_t status);

        /**
         * Entity Update
         *
         * Called when notification from Entity Update characteristic is received.
         *
         * \param [in] client           AMS client instance
         * \param [in] entity_id        entity id
         * \param [in] attribute_id     attribute id
         * \param [in] flags            entity update flags
         * \param [in] length           value length
         * \param [in] value            value of given attribute
         */
        void (* entity_update) (ble_client_t *client, ams_client_entity_id_t entity_id,
                                uint8_t attribute_id, ams_client_entity_update_flag_t flags,
                                uint16_t length, const uint8_t *value);

        /**
         * Entity Attribute write completed
         *
         * Called when ams_client_entity_attribute_write() action is completed.
         *
         * \param [in] client   AMS client instance
         * \param [in] status   operation status
         *
         */
        void (* entity_attribute_write_completed) (ble_client_t *client, att_error_t status);

        /**
         * Entity Attribute read completed
         *
         * Called when ams_client_entity_attribute_read() action is completed.
         *
         * \param [in] client   AMS client instance
         * \param [in] status   operation status
         * \param [in] length   value length
         * \param [in] value    attribute value
         *
         */
        void (* entity_attribute_read_completed) (ble_client_t *client, att_error_t status,
                                                        uint16_t length, const uint8_t *value);
} ams_client_callbacks_t;

/**
 * \brief Initialize AMS client instance
 *
 * This should be called by application when the AMS service discovery is completed. An AMS client
 * instance will be created and initialized as a result which will identify the client in all other
 * API calls.
 *
 * \param [in] cb       AMS client callbacks
 * \param [in] evt      browse event instance
 *
 * \return client instance or NULL if failed
 *
 */
ble_client_t *ams_client_init(const ams_client_callbacks_t *cb,
                                                        const ble_evt_gattc_browse_svc_t *evt);

/**
 * \brief Initialize and register AMS Client instance from data buffer
 *
 * Function initializes AMS Client from data buffer.
 *
 * \param [in] conn_idx         connection index
 * \param [in] cb               client callbacks
 * \param [in] data             data buffer
 * \param [in] length           data buffer's length
 *
 * \return client instance when initialized properly, NULL otherwise
 */
ble_client_t *ams_client_init_from_data(uint16_t conn_idx, const ams_client_callbacks_t *cb,
                                                                const void *data, size_t length);

/**
 * \brief Get AMS client capabilities
 *
 * Function returns bit mask with AMS client capabilities
 *
 * \param [in] ams_client       AMS client instance
 *
 * \return bit mask with AMS client capabilities
 *
 */
ams_client_cap_t ams_client_get_capabilities(const ble_client_t *ams_client);

/**
 * \brief Set AMS client event state
 *
 * This function writes the CCC descriptor of the respective characteristic. New value is passed
 * using enable value. set_event_state_completed() callback will be called when this operation
 * completes.
 *
 * \param [in] ams_client       AMS client instance
 * \param [in] event            requested event
 * \param [in] enabled          event state (true if enabled, false otherwise)
 *
 * \return true if operation was executed successfully, false otherwise
 *
 */
bool ams_client_set_event_state(const ble_client_t *ams_client, ams_client_event_t event,
                                                                                bool enable);

/**
 * \brief Get AMS client event state
 *
 * This function triggers read operation of the CCC descriptor of the respective characteristic.
 * get_event_state_completed() callback will be called when this operation completes.
 *
 * \param [in] ams_client       AMS client instance
 * \param [in] event            requested event
 *
 * \return true if operation was executed successfully, false otherwise
 *
 */
bool ams_client_get_event_state(const ble_client_t *ams_client, ams_client_event_t event);

/**
 * \brief AMS Client remote command
 *
 * This function writes the specified command to Remote command characteristic.
 * remote_command_completed() callback will be called when the operation is completed.
 *
 * \param [in] ams_client       AMS client instance
 * \param [in] command          requested command
 *
 * \return true if operation was executed successfully, false otherwise
 *
 */
bool ams_client_remote_command(ble_client_t *ams_client, ams_client_remote_command_t command);

/**
 * \brief AMS Client entity update command
 *
 * This function will trigger write operation to Entity Update characteristic.
 * User may use it to enable notifications with given entity ID and attribute IDs.
 * Entity_update_command_completed callback will be called when this operation completes.
 *
 * \param [in] ams_client               AMS client instance
 * \param [in] entity_id                entity ID
 * \param [in] num_of_attributes        number of attribute elements within attributes array
 * \param [in] attribute_ids            array of attribute IDs
 *
 */
bool ams_client_entity_update_command(ble_client_t *ams_client, ams_client_entity_id_t entity_id,
                                        uint16_t num_of_attributes, const uint8_t *attribute_ids);

/**
 * \brief AMS Client entity attribute write
 *
 * This function will trigger write operation to Entity Attribute characteristic.
 * Entity_attribute_write_completed callback will be called when this operation completes.
 *
 * \param [in] ams_client       AMS client instance
 * \param [in] entity_id        requested entity ID
 * \param [in] attribute_id     requested attribute ID
 *
 * \return true if operation was executed successfully, false otherwise
 *
 */
bool ams_client_entity_attribute_write(ble_client_t *ams_client, ams_client_entity_id_t entity_id,
                                                                        uint8_t attribute_id);

/**
 * AMS Client entity attribute read
 *
 * Function reads Entity Attribute characteristic. According to specification, it should be done
 * after previous write to the same characteristic.
 *
 * \p entity_attribute_read_completed callback will be called when this operation completes.
 *
 * \param [in] ams_client       AMS client instance
 *
 * \return true if operation was executed successfully, false otherwise
 *
 */
bool ams_client_entity_attribute_read(ble_client_t *ams_client);

#endif /* AMS_CLIENT_H_ */
