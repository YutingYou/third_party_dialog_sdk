/**
 ****************************************************************************************
 *
 * @file adxl362_ucodes.c
 *
 * @brief SNC-uCode reading ADXL362 FIFO data based on INT1 interrupt
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
#include "snc_hw_sys.h"
#include "snc_hw_gpio.h"
#include "snc_hw_spi.h"
#include "platform_devices.h"
#include "peripheral_setup.h"

#include "adxl362.h"
#include "adxl362_ucodes.h"

#if dg_configUSE_SNC_HW_SPI

#define ADXL362_UCODE_FIFO_FULL_CHECK_RETRIES   5

_SNC_RETAINED static uint32_t accelSamples[2];

_SNC_RETAINED static uint32_t read_len;
_SNC_RETAINED static uint32_t readSampleCount;

_SNC_RETAINED static uint32_t *writeQp;
_SNC_RETAINED static uint32_t adxl362_timestamp;
_SNC_RETAINED static uint32_t queue_max_chunk;
_SNC_RETAINED static uint32_t queue_is_full;
_SNC_RETAINED static uint32_t queue_is_full_cnt;
_SNC_RETAINED static uint32_t timestampNotAcquired;

_SNC_RETAINED static uint32_t read_fifo_cmd[] = { ADXL362_READ_FIFO };

_SNC_RETAINED static uint32_t get_fifo_samples_num[] = { ADXL362_READ_REG, ADXL362_REG_FIFO_L };

_SNC_RETAINED static uint32_t fifo_disable_cmd[] = {
        ADXL362_WRITE_REG, ADXL362_REG_FIFO_CTL,
        ADXL362_FIFO_CTL_FIFO_MODE(ADXL362_FIFO_DISABLE) | ((0x80 > 255) ? ADXL362_FIFO_CTL_AH : 0),
};

_SNC_RETAINED static uint32_t fifo_stream_cmd[] = {
        ADXL362_WRITE_REG, ADXL362_REG_FIFO_CTL,
        ADXL362_FIFO_CTL_FIFO_MODE(ADXL362_FIFO_STREAM) | ((ADXL362_FIFO_WM_LEVEL > 255) ? ADXL362_FIFO_CTL_AH : 0),
};

_SNC_RETAINED static uint32_t inv_fifo_samples[] = {
        ADXL362_WRITE_REG, ADXL362_REG_FIFO_SAMPLES,
        0x80
};

_SNC_RETAINED static uint32_t set_fifo_samples[] = {
        ADXL362_WRITE_REG, ADXL362_REG_FIFO_SAMPLES,
        ADXL362_FIFO_WM_LEVEL
};

SNC_UCODE_BLOCK_DEF(ucode_gpio_adxl362_int1_queue_test)
{
        // Clear the ADLX362 WKUP pin interrupt
        SNC_hw_sys_clear_wkup_status(ADXL362_INT_1_PORT, 1<<ADXL362_INT_1_PIN);

        // Open the SPI controller based on ADXL362 configuration
        SNC_spi_open(ADXL362);

        SENIS_assign(da(&timestampNotAcquired), 1);

        SENIS_while (1) {
                // Check if the SNC-to-CM33 queue is full for a number of times
                SENIS_assign(da(&queue_is_full_cnt), 0);
                SENIS_while (da(&queue_is_full_cnt), NEQ, ADXL362_UCODE_FIFO_FULL_CHECK_RETRIES) {
                        SNC_queues_snc_wq_is_full(da(&queue_is_full));
                        SENIS_if (da(&queue_is_full)) {
                                SENIS_inc1(da(&queue_is_full_cnt));
                        SENIS_else {
                                SENIS_break;
                        }}
                }

                // If the max number of checks for full queue is reached,
                // flush the ADXL362 FIFO and break the while (1)
                SENIS_if (da(&queue_is_full_cnt), EQ, ADXL362_UCODE_FIFO_FULL_CHECK_RETRIES) {

                        // ADXL362 FIFO Flush
                        // _adxl362_fifo_setup(ADXL362_FIFO_DISABLE, 0x80, 0);
                        SNC_spi_activate_cs(ADXL362);   // CS low
                        SNC_spi_write(ADXL362, da(fifo_disable_cmd), 3);
                        SNC_spi_deactivate_cs(ADXL362); // CS high

                        SNC_spi_activate_cs(ADXL362);   // CS low
                        SNC_spi_write(ADXL362, da(inv_fifo_samples), 3);
                        SNC_spi_deactivate_cs(ADXL362); // CS high

                        // ADXL362 FIFO Re-Setup
                        // _adxl362_fifo_setup(ADXL362_FIFO_STREAM, ADXL362_FIFO_WM_LEVEL, 0);
                        SNC_spi_activate_cs(ADXL362);   // CS low
                        SNC_spi_write(ADXL362, da(fifo_stream_cmd), 3);
                        SNC_spi_deactivate_cs(ADXL362); // CS high

                        SNC_spi_activate_cs(ADXL362);   // CS low
                        SNC_spi_write(ADXL362, da(set_fifo_samples), 3);
                        SNC_spi_deactivate_cs(ADXL362); // CS high

                        // Break the loop
                        SENIS_break;
                }

                SNC_spi_activate_cs(ADXL362);   // CS low
                SNC_spi_write(ADXL362, da(get_fifo_samples_num), 2);
                SNC_spi_read(ADXL362, da(accelSamples), 2);
                SNC_spi_deactivate_cs(ADXL362); // CS high

                // Calculate the total amount of samples residing in the FIFO
                SENIS_assign(da(&readSampleCount), da(&accelSamples[0]));
                SENIS_if (da(&accelSamples[1]), EQ, 1) {
                        SENIS_xor(da(&readSampleCount), 0x0100);
                        SENIS_else {
                                SENIS_if (da(&accelSamples[1]), EQ, 2) {
                                        SENIS_xor(da(&readSampleCount), 0x0200);
                                }
                        }
                }

                // If there are no samples to get then break the loop
                SENIS_if (da(&readSampleCount), EQ, 0) {
                        SENIS_break;
                }

                // Prepare the READ FIFO command
                SENIS_assign(da(&read_len), 0);

                // Calculate the bytes based on the samples ( 1 sample = 2 bytes )
                SENIS_xor(da(&readSampleCount), 0xFFFFFFFF);
                SENIS_inc1(da(&readSampleCount));
                SENIS_while (da(&readSampleCount), NEQ, 0) {
                        SENIS_inc1(da(&read_len));
                        SENIS_inc1(da(&read_len));
                        SENIS_inc1(da(&readSampleCount));
                }

                // If the calculated length is more than the max chunk size
                // read only the maximum allowed
                SNC_queues_snc_get_wq_max_chunk_bytes(da(&queue_max_chunk));
                SENIS_if (da(&read_len), GT, da(&queue_max_chunk)) {
                        SENIS_assign(da(&read_len), da(&queue_max_chunk));
                }

                // Get the timer capture register value in order to use it as a timestamp
                // to the acquired accelerometer samples
                // If we have already acquired the timestamp, then if there are
                // more samples to get give them an invalid timestamp
                SENIS_if (da(&timestampNotAcquired)) {
                        SENIS_assign(da(&timestampNotAcquired), 0);
                        SNC_hw_sys_timer_get_capture1(HW_TIMER, da(&adxl362_timestamp));
                SENIS_else {
                        SENIS_assign(da(&adxl362_timestamp), 0xFFFFFFFF);
                }}

                SNC_queues_snc_get_wq(da(&writeQp), da(&read_len), da(&adxl362_timestamp));

                // Read the ADXL362 FIFO samples
                SNC_spi_activate_cs(ADXL362);   // CS low
                SNC_spi_write_advanced(ADXL362, HW_SPI_WORD_8BIT, da(read_fifo_cmd), 1);
                SNC_spi_read_advanced(ADXL362, HW_SPI_WORD_32BIT, ia(&writeQp), da(&read_len));
                SNC_spi_deactivate_cs(ADXL362); // CS high

                // Push the recently acquired data into the SNC queue
                SNC_queues_snc_push();
        }

        // Close the SPI controller
        SNC_spi_close(ADXL362);

        // Send a notification back to CM33
        SNC_CM33_NOTIFY();
}

#endif /* dg_configUSE_SNC_HW_SPI */
