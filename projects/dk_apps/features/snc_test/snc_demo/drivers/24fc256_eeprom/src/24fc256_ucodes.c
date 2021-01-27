/**
 ****************************************************************************************
 *
 * @file 24fc256_ucodes.c
 *
 * @brief SNC-Implementation of demo application for the 24FC256 I2C EEPROM
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <string.h>

#include "platform_devices.h"
#include "peripheral_setup.h"
#include "SeNIS.h"
#include "snc_hw_sys.h"
#include "snc_hw_gpio.h"
#include "snc_queues.h"
#include "snc_hw_i2c.h"

#include "24fc256.h"
#include "24fc256_ucodes.h"

#if dg_configUSE_SNC_HW_I2C

_SNC_RETAINED static uint32_t is_pending;
_SNC_RETAINED static uint32_t *qEEPROM_writer_data;
_SNC_RETAINED static uint32_t qEEPROM_write_data_len;

SNC_UCODE_BLOCK_DEF(ucode_write_eeprom_test_data)
{
        // Check if there is a pending notification for this uCode-Block
        SNC_CHECK_CM33_NOTIF_PENDING(da(&is_pending));

        // If its not, return immediately
        SENIS_if (1, GT, da(&is_pending)) {
                SENIS_return;
        }

        // Pop pages and write them into the EEPROM if the
        // CM33-to-SNC queue is NOT empty
        SENIS_while (1) {
                SNC_queues_snc_get_rq(da(&qEEPROM_writer_data), da(&qEEPROM_write_data_len), da(NULL));
                SENIS_if (da(&qEEPROM_writer_data), NEQ, 0) {

                        // Open the I2C controller based on MEM. 24FC256 configuration
                        SNC_i2c_open(MEM_24FC256);

                        // Write the test data
                        SNC_i2c_write(MEM_24FC256, ia(&qEEPROM_writer_data), da(&qEEPROM_write_data_len),
                                SNC_HW_I2C_FLAG_ADD_STOP);

                        // Perform the queue pop, since we acquired the data
                        SNC_queues_snc_pop();

                        // After writing on the 24FC256 in order to perform another transaction we need
                        // to poll the EEPROM until its ready (i.e answers with ACK)
                        SNC_i2c_poll_for_ack(MEM_24FC256);

                        // Close the I2C controller
                        SNC_i2c_close(MEM_24FC256);

                        // Notify CM33 to check the SNC-to-CM33 queue
                        SNC_CM33_NOTIFY();

                SENIS_else {
                        SENIS_break;
                }}
        }
}

// A pointer that is used to push the read data from the EEPROM
// into the SNC-to-CM33 queue
_SNC_RETAINED static uint32_t *qEEPROM_read_data;

// A pointer that is used to pop the next EEPROM address to read from
_SNC_RETAINED static uint32_t *qEEPROM_next_raddr;

// A helper variable holding the read offset / address
_SNC_RETAINED static uint32_t read_addr_size;

SNC_UCODE_BLOCK_DEF(ucode_read_eeprom_on_rtc)
{
        // Clear the RTC PDC event
        SNC_hw_sys_clear_rtc_pdc_event();

        // Check the CM33-to-SNC queue for any EEPROM address to read from
        SNC_queues_snc_get_rq(da(&qEEPROM_next_raddr), da(&read_addr_size), da(NULL));
        SENIS_if (da(&qEEPROM_next_raddr), NEQ, 0) {

                // Check if we have space to push the read data
                SNC_queues_snc_get_wq(da(&qEEPROM_read_data), EEPROM_24FC256_PAGE_SIZE, 0);
                SENIS_if (da(&qEEPROM_read_data), NEQ, 0) {

                        // Open the I2C controller based on MEM. 24FC256 configuration
                        SNC_i2c_open(MEM_24FC256);

                        // Read back the previously written data
                        SNC_i2c_write(MEM_24FC256, ia(&qEEPROM_next_raddr), da(&read_addr_size),
                                SNC_HW_I2C_FLAG_ADD_RESTART | SNC_HW_I2C_FLAG_ADD_STOP);
                        SNC_i2c_read(MEM_24FC256, ia(&qEEPROM_read_data), EEPROM_24FC256_PAGE_SIZE,
                                SNC_HW_I2C_FLAG_ADD_RESTART | SNC_HW_I2C_FLAG_ADD_STOP);

                        // Close the I2C controller
                        SNC_i2c_close(MEM_24FC256);

                        // Update the CM33-to-SNC queue
                        SNC_queues_snc_pop();

                        // Update the SNC-to-CM33 queue
                        SNC_queues_snc_push();

                        // Notify CM33 to check the SNC-to-CM33 queue
                        SNC_CM33_NOTIFY();
                }
        }
}

#endif /* dg_configUSE_SNC_HW_I2C */

