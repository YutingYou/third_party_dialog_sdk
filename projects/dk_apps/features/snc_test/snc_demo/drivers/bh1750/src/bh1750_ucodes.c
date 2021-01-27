/**
 ****************************************************************************************
 *
 * @file bh1750_ucodes.c
 *
 * @brief SNC-Implementation of I2C BH1750 ambient light sensor demo application
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <string.h>

#include "SeNIS.h"
#include "snc_queues.h"
#include "snc_hw_gpio.h"
#include "snc_hw_sys.h"
#include "snc_hw_i2c.h"
#include "platform_devices.h"

#include "bh1750_snc_demo.h"
#include "bh1750_ucodes.h"
#include "bh1750.h"

#if dg_configUSE_SNC_HW_I2C

_SNC_RETAINED static snc_cm33_mutex_t bh1750_mutex;

_SNC_RETAINED static uint32_t cmd_var;
_SNC_RETAINED static uint32_t bh1750_num_of_samples = 0;
_SNC_RETAINED static uint32_t bh1750_local_fifo[UCODE_BH1750_MAX_NUM_OF_SAMPLES * BH1750_SAMPLE_SIZE];
_SNC_RETAINED static uint32_t *bh1750_p_fifo = &bh1750_local_fifo[0];

SNC_UCODE_BLOCK_DEF(ucode_bh1750_ambient_light_collect_samples)
{
        // Clear the RTC PDC event
        SNC_hw_sys_clear_rtc_pdc_event();

        // Open the I2C controller based on BH1750 configuration
        SNC_i2c_open(BH1750);

        // Send the continuous command again and collect the last updated measurement
        SENIS_assign(da(&cmd_var), BH1750_CONT_H_RES_MODE_OPCODE);

        SNC_MUTEX_LOCK(&bh1750_mutex);
        SNC_i2c_write(BH1750, da(&cmd_var), 1,
                SNC_HW_I2C_FLAG_ADD_STOP | SNC_HW_I2C_FLAG_ADD_RESTART);
        SNC_i2c_read(BH1750, ia(&bh1750_p_fifo), BH1750_SAMPLE_SIZE,
                SNC_HW_I2C_FLAG_ADD_STOP | SNC_HW_I2C_FLAG_ADD_RESTART);
        SNC_MUTEX_UNLOCK(&bh1750_mutex);

        // Close the I2C controller
        SNC_i2c_close(BH1750);

        // ++num_of_samples;
        SENIS_inc1(da(&bh1750_num_of_samples));

        // If BH1750_MAX_NUM_OF_SAMPLES have been collected
        SENIS_if (da(&bh1750_num_of_samples), EQ, UCODE_BH1750_MAX_NUM_OF_SAMPLES) {
                // Set the temp local FIFO pointer to point at the start of the temporary FIFO
                SENIS_assign(da(&bh1750_p_fifo), &bh1750_local_fifo[0]);

                // Set the number of samples to 0
                SENIS_assign(da(&bh1750_num_of_samples), 0);

                // Notify CM33 that collecting the desired amount of samples has finished
                SNC_CM33_NOTIFY();
        }
}

_SNC_RETAINED static uint32_t *writeQp;
_SNC_RETAINED static uint32_t bh1750_timestamp;
_SNC_RETAINED static uint32_t bh1750_notify_samples;
_SNC_RETAINED static uint32_t queue_is_full;

SNC_UCODE_BLOCK_DEF(ucode_bh1750_ambient_light_collect_samples_queue)
{
        // Clear the RTC PDC event
        SNC_hw_sys_clear_rtc_pdc_event();

        SENIS_assign(da(&queue_is_full), 0);
        SNC_queues_snc_wq_is_full(da(&queue_is_full));
        // If queue is full
        SENIS_if (da(&queue_is_full), EQ, 1) {
                SNC_CM33_NOTIFY();
                SENIS_return;
        }
        SNC_queues_snc_get_wq(da(&writeQp), BH1750_SAMPLE_SIZE, da(&bh1750_timestamp));

        // Send the continuous command again and collect the last updated measurement
        SENIS_assign(da(&cmd_var), BH1750_CONT_H_RES_MODE_OPCODE);

        // Open the I2C controller based on BH1750 configuration
        SNC_i2c_open(BH1750);
        SNC_i2c_write(BH1750, da(&cmd_var), 1,
                SNC_HW_I2C_FLAG_ADD_STOP | SNC_HW_I2C_FLAG_ADD_RESTART);
        SNC_i2c_read(BH1750, ia(&writeQp), BH1750_SAMPLE_SIZE,
                SNC_HW_I2C_FLAG_ADD_STOP | SNC_HW_I2C_FLAG_ADD_RESTART);

        // Close the I2C controller
        SNC_i2c_close(BH1750);

        // Push the recently acquired data into the SNC queue
        SNC_queues_snc_push();

        // ++num_of_samples;
        SENIS_inc1(da(&bh1750_num_of_samples));

        // If BH1750_MAX_NUM_OF_SAMPLES have been collected
        SENIS_if (da(&bh1750_num_of_samples), EQ, da(&bh1750_notify_samples)) {
                // Set the number of samples to 0
                SENIS_assign(da(&bh1750_num_of_samples), 0);

                // Notify CM33 that collecting the desired amount of samples has finished
                SNC_CM33_NOTIFY();
        }

}
// High threshold value
_SNC_RETAINED static uint32_t bh1750_threshold_high;
// Low threshold value
_SNC_RETAINED static uint32_t bh1750_threshold_low;
// tolerance value for high threshold
_SNC_RETAINED static uint32_t bh1750_threshold_high_tolerance;
// Tolerance value for low threshold
_SNC_RETAINED static uint32_t bh1750_threshold_low_tolerance;
// Light State // 0:idle, 1:high lux, 2:low lux
_SNC_RETAINED static uint32_t bh1750_ambient_light_state = 0;
// Sensor data sample
_SNC_RETAINED static uint32_t bh1750_sample[2] = { 0 };
// Notification Status
_SNC_RETAINED static uint32_t bh1750_notif_status;
// Last sensor value
_SNC_RETAINED static uint32_t bh1750_last_sensor_data;
// Last sensor value
_SNC_RETAINED static uint32_t bh1750_last_notif_value;

SNC_UCODE_BLOCK_DEF(ucode_bh1750_ambient_light_notifier)
{
        // Clear the RTC PDC event
        SNC_hw_sys_clear_rtc_pdc_event();

        // Set the continuous command opcode
        SENIS_assign(da(&cmd_var), BH1750_CONT_H_RES_MODE_OPCODE);

        // Open the I2C controller based on BH1750 configuration
        SNC_i2c_open(BH1750);

        // Send the command and collect the last updated measurement
        SNC_i2c_write(BH1750, da(&cmd_var), 1,
                SNC_HW_I2C_FLAG_ADD_STOP | SNC_HW_I2C_FLAG_ADD_RESTART);
        SNC_i2c_read(BH1750, da(&bh1750_sample[0]), BH1750_SAMPLE_SIZE,
                SNC_HW_I2C_FLAG_ADD_STOP | SNC_HW_I2C_FLAG_ADD_RESTART);

        // Close the I2C controller
        SNC_i2c_close(BH1750);

        SENIS_lshift_masked(da(&bh1750_sample[0]), 8, 8);
        SENIS_xor(da(&bh1750_sample[0]),da(&bh1750_sample[1]));

        // Update last sensor measurement
        SENIS_assign(da(&bh1750_last_sensor_data), da(&bh1750_sample[0]));

        // Initially set notify status to NONE
        SENIS_assign(da(&bh1750_notif_status), NOTIFY_STATUS_NONE);
        // Detect Transition from normal light state
        SENIS_if(da(&bh1750_ambient_light_state), EQ, 0) {
                // Compare value with high threshold
                SENIS_if(da(&bh1750_sample[0]), GT, da(&bh1750_threshold_high)) {
                        // Normal to High light state transition detected
                        SENIS_assign(da(&bh1750_notif_status), NOTIFY_STATUS_HIGH);
                SENIS_else {
                        // Compare value with low threshold
                        SENIS_if(da(&bh1750_sample[0]), LT, da(&bh1750_threshold_low)) {
                                SENIS_assign(da(&bh1750_notif_status), NOTIFY_STATUS_LOW);
                        }
                }}
        }
        // Detect Transition from high light state
        SENIS_if(da(&bh1750_ambient_light_state), EQ, 1) {
                // Compare value with low threshold
                SENIS_if(da(&bh1750_sample[0]), LT, da(&bh1750_threshold_low)) {
                        SENIS_assign(da(&bh1750_notif_status), NOTIFY_STATUS_LOW);
                SENIS_else {
                        // Compare value with high threshold minus tolerance
                        SENIS_if(da(&bh1750_sample[0]), LT, da(&bh1750_threshold_high_tolerance)) {
                                SENIS_assign(da(&bh1750_notif_status), NOTIFY_STATUS_NORMAL);
                        }
                }}
        }
        // Detect Transition from low light state
        SENIS_if(da(&bh1750_ambient_light_state), EQ, 2) {
                // Compare value with high threshold
                SENIS_if(da(&bh1750_sample[0]), GT, da(&bh1750_threshold_high)) {
                        SENIS_assign(da(&bh1750_notif_status), NOTIFY_STATUS_HIGH);
                SENIS_else {
                        // Compare value with low threshold plus tolerance
                        SENIS_if(da(&bh1750_sample[0]), GT, da(&bh1750_threshold_low_tolerance)) {
                                SENIS_assign(da(&bh1750_notif_status), NOTIFY_STATUS_NORMAL);
                        }
                }}
        }

        // Send notification for normal light state
        SENIS_if(da(&bh1750_notif_status), EQ, NOTIFY_STATUS_NORMAL) {
                // Lock mutex to update variables
                SNC_MUTEX_LOCK(&bh1750_mutex);
                // Update sensor measurement
                SENIS_assign(da(&bh1750_last_notif_value), da(&bh1750_sample[0]));
                // Update state
                SENIS_assign(da(&bh1750_ambient_light_state), AMBIENT_LIGHT_STATE_NORMAL);
                SNC_MUTEX_UNLOCK(&bh1750_mutex);
                // Send notification to CM33s
                SNC_CM33_NOTIFY();
                SENIS_return;
        }
        // Send notification for high light state
        SENIS_if(da(&bh1750_notif_status), EQ, NOTIFY_STATUS_HIGH) {
                // Lock mutex to update variables
                SNC_MUTEX_LOCK(&bh1750_mutex);
                // Update sensor measurement
                SENIS_assign(da(&bh1750_last_notif_value), da(&bh1750_sample[0]));
                // Update state
                SENIS_assign(da(&bh1750_ambient_light_state), AMBIENT_LIGHT_STATE_HIGH);
                SNC_MUTEX_UNLOCK(&bh1750_mutex);
                // Send notification to CM33
                SNC_CM33_NOTIFY();
                SENIS_return;
        }
        // Send notification for low light state
        SENIS_if(da(&bh1750_notif_status), EQ, NOTIFY_STATUS_LOW) {
                // Lock mutex to update variables
                SNC_MUTEX_LOCK(&bh1750_mutex);
                // Update sensor measurement
                SENIS_assign(da(&bh1750_last_notif_value), da(&bh1750_sample[0]));
                // Update state
                SENIS_assign(da(&bh1750_ambient_light_state), AMBIENT_LIGHT_STATE_LOW);
                SNC_MUTEX_UNLOCK(&bh1750_mutex);
                // Send notification to CM33
                SNC_CM33_NOTIFY();
                SENIS_return;
        }
}

void ucode_bh1750_get_fifo_samples(uint32_t *fifo_data)
{
        snc_mutex_SNC_lock(&bh1750_mutex);
        memcpy(fifo_data, bh1750_local_fifo, sizeof(bh1750_local_fifo));
        snc_mutex_SNC_unlock(&bh1750_mutex);
}

uint32_t ucode_bh1750_get_fifo_num_of_samples(void)
{
        return UCODE_BH1750_MAX_NUM_OF_SAMPLES * BH1750_SAMPLE_SIZE;
}

void ucode_bh1750_init(void)
{
        snc_mutex_SNC_create(&bh1750_mutex);
}

void ucode_bh1750_set_notify_samples(uint32_t notify_samples)
{
        bh1750_notify_samples =  notify_samples;
}

void ucode_bh1750_set_threshold_high(uint16_t high_threshold, uint16_t tolerance)
{
        bh1750_threshold_high  = high_threshold + tolerance;
        bh1750_threshold_high_tolerance = high_threshold - tolerance;
}
void ucode_bh1750_set_threshold_low(uint16_t low_threshold, uint16_t tolerance)
{
        bh1750_threshold_low  = low_threshold - tolerance;
        bh1750_threshold_low_tolerance = low_threshold + tolerance;
}
uint32_t ucode_bh1750_get_lux_state(uint32_t * bh1750_sample_value)
{
        *bh1750_sample_value = bh1750_sample[0];
        return bh1750_ambient_light_state;
}

uint32_t ucode_bh1750_get_last_sensor_value(void)
{
        return bh1750_last_sensor_data;
}

uint32_t ucode_bh1750_get_notif_state(void)
{
        return bh1750_ambient_light_state;
}

uint32_t ucode_bh1750_get_last_notif_value(void)
{
        return  bh1750_last_notif_value;
}

#endif /* dg_configUSE_SNC_HW_I2C */

