/**
 ****************************************************************************************
 *
 * @file display.c
 *
 * @brief Media Remote display implementation
 *
 * Copyright (C) 2018-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdint.h>
#include <stdio.h>
#include "string.h"
#include "osal.h"
#include "hw_gpio.h"
#include "sys_power_mgr.h"
#include "ams_config.h"
#include "display.h"

typedef struct {
        char *name;
        char *volume;
        int playback_state;

        int index;
        int count;
        int shuffle_mode;
        int repeat_mode;

        char *artist;
        char *album;
        char *title;
        char *duration;
} display_info_t;

__RETAINED_RW static display_info_t display_info = {
        .playback_state = -1,
        .index = -1,
        .count = -1,
        .shuffle_mode = -1,
        .repeat_mode = -1,
};

static const char *playback_state2str(ams_client_playback_state_t state)
{
        switch (state) {
        case AMS_CLIENT_PLAYBACK_STATE_PAUSED:
                return "Paused";
        case AMS_CLIENT_PLAYBACK_STATE_PLAYING:
                return "Playing";
        case AMS_CLIENT_PLAYBACK_STATE_REWINDING:
                return "Rewinding";
        case AMS_CLIENT_PLAYBACK_STATE_FAST_FORWARDING:
                return "Fast Forwarding";
        }

        return "<unknown>";
}

static const char *shuffle_mode2str(ams_client_shuffle_mode_t mode)
{
        switch (mode) {
        case AMS_CLIENT_SHUFFLE_MODE_OFF:
                return "Off";
        case AMS_CLIENT_SHUFFLE_MODE_ONE:
                return "One";
        case AMS_CLIENT_SHUFFLE_MODE_ALL:
                return "All";
        }

        return "<unknown>";
}

static const char *repeat_mode2str(ams_client_repeat_mode_t mode)
{
        switch (mode) {
        case AMS_CLIENT_REPEAT_MODE_OFF:
                return "Off";
        case AMS_CLIENT_REPEAT_MODE_ONE:
                return "One";
        case AMS_CLIENT_REPEAT_MODE_ALL:
                return "All";
        }

        return "<unknown>";
}

static void queue_info_reset(void)
{
        display_info.count = -1;
        display_info.index = -1;
        display_info.repeat_mode = -1;
        display_info.shuffle_mode = -1;
}

static void track_info_reset(void)
{
        OS_FREE(display_info.album);
        display_info.album = NULL;

        OS_FREE(display_info.artist);
        display_info.artist = NULL;

        OS_FREE(display_info.duration);
        display_info.duration = NULL;

        OS_FREE(display_info.title);
        display_info.title = NULL;
}

static void set_playback_state(int state)
{
        display_info.playback_state = state;

        switch (state) {
        case AMS_CLIENT_PLAYBACK_STATE_PLAYING:
                hw_gpio_set_active(LED1_PORT, LED1_PIN);
                hw_gpio_pad_latch_enable(LED1_PORT, LED1_PIN);
                hw_gpio_pad_latch_disable(LED1_PORT, LED1_PIN);
                break;
        default:
                hw_gpio_set_inactive(LED1_PORT, LED1_PIN);
                hw_gpio_pad_latch_enable(LED1_PORT, LED1_PIN);
                hw_gpio_pad_latch_disable(LED1_PORT, LED1_PIN);
                break;
        }
}

static void player_info_reset(void)
{
        OS_FREE(display_info.name);
        display_info.name = NULL;

        OS_FREE(display_info.volume);
        display_info.volume = NULL;

        set_playback_state(-1);
}

static void replace_string_value(char **value, uint16_t length, char *new_value)
{
        // free previous value
        OS_FREE(*value);

        if (!length) {
                *value = NULL;
                return;
        }

        if (new_value[length - 1]) {
                // extend value with NULL character
                *value = OS_MALLOC(length + 1);
        } else {
                *value = OS_MALLOC(length);
        }

        sprintf(*value, "%.*s", length, new_value);
}

static int get_number_default(uint16_t length, const uint8_t *value, int default_value)
{
        int temp_value = 0;
        uint16_t i = 0;

        // skip whitespaces
        while (i < length && value[i] == ' ') {
                i++;
        }

        if (i == length || value[i] < '0' || value[i] > '9') {
                return default_value;
        }

        while (i < length && value[i] >= '0' && value[i] <= '9') {
                temp_value = temp_value * 10 + (value[i] - '0');
                i++;
        }

        return temp_value;
}

void display_device_disconnected(void)
{
        player_info_reset();
        track_info_reset();
        queue_info_reset();
}

void display_update(ams_client_entity_id_t entity_id, uint8_t attribute_id, uint16_t length,
                                                                        const uint8_t *value)
{
        switch (entity_id) {
        case AMS_CLIENT_ENTITY_ID_PLAYER:
                switch (attribute_id) {
                case AMS_CLIENT_PLAYER_ATTRIBUTE_ID_NAME:
                        replace_string_value(&display_info.name, length, (char *) value);

                        // Reset also queue info
                        queue_info_reset();
                        track_info_reset();

                        if (display_info.name) {
                                printf("Player name: %s\r\n", display_info.name);
                        }
                        break;
                case AMS_CLIENT_PLAYER_ATTRIBUTE_ID_PLAYBACK_INFO:
                        // Parse only playback state for now
                        set_playback_state(get_number_default(length, value, -1));
                        if (display_info.playback_state >= 0) {
                                printf("Playback state: %s\r\n",
                                                playback_state2str(display_info.playback_state));
                        }
                        break;
                case AMS_CLIENT_PLAYER_ATTRIBUTE_ID_VOLUME:
                        replace_string_value(&display_info.volume, length, (char *) value);

                        if (display_info.volume) {
                                printf("Volume: %s\r\n", display_info.volume);
                        }
                        break;
                }
                break;
        case AMS_CLIENT_ENTITY_ID_QUEUE:
                switch (attribute_id) {
                case AMS_CLIENT_QUEUE_ATTRIBUTE_ID_COUNT:
                        display_info.count = get_number_default(length, value, -1);
                        if (display_info.count >= 0) {
                                printf("Count: %d\r\n", display_info.count);
                        }
                        break;
                case AMS_CLIENT_QUEUE_ATTRIBUTE_ID_INDEX:
                        display_info.index = get_number_default(length, value, -1);
                        if (display_info.index >= 0) {
                                printf("Index: %d\r\n", display_info.index);
                        }
                        break;
                case AMS_CLIENT_QUEUE_ATTRIBUTE_ID_REPEAT_MODE:
                        display_info.repeat_mode = get_number_default(length, value, -1);
                        if (display_info.repeat_mode >= 0) {
                                printf("Repeat Mode: %s\r\n", repeat_mode2str(display_info.repeat_mode));
                        }
                        break;
                case AMS_CLIENT_QUEUE_ATTRIBUTE_ID_SHUFFLE_MODE:
                        display_info.shuffle_mode = get_number_default(length, value, -1);
                        if (display_info.shuffle_mode >= 0) {
                                printf("Shuffle Mode: %s\r\n", shuffle_mode2str(display_info.shuffle_mode));
                        }
                        break;
                }
                break;
        case AMS_CLIENT_ENTITY_ID_TRACK:
                switch (attribute_id) {
                case AMS_CLIENT_TRACK_ATTRIBUTE_ID_ALBUM:
                        replace_string_value(&display_info.album, length, (char *) value);
                        if (display_info.album) {
                                printf("Album: %s\r\n", display_info.album);
                        }
                        break;
                case AMS_CLIENT_TRACK_ATTRIBUTE_ID_ARTIST:
                        replace_string_value(&display_info.artist, length, (char *) value);
                        if (display_info.artist) {
                                printf("Artist: %s\r\n", display_info.artist);
                        }
                        break;
                case AMS_CLIENT_TRACK_ATTRIBUTE_ID_DURATION:
                        replace_string_value(&display_info.duration, length, (char *) value);
                        if (display_info.duration) {
                                printf("Duration: %s\r\n", display_info.duration);
                        }
                        break;
                case AMS_CLIENT_TRACK_ATTRIBUTE_ID_TITLE:
                        replace_string_value(&display_info.title, length, (char *) value);
                        if (display_info.title) {
                                printf("Title: %s\r\n", display_info.title);
                        }
                        break;
                }
                break;
        default:
                return;
        }
}
