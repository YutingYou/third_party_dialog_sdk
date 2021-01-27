/**
 ****************************************************************************************
 *
 * @file 24fc256_snc_demo.c
 *
 * @brief I2C EEPROM - SNC Demo functions source code
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include "hw_pdc.h"
#include "SeNIS.h"

#include "24fc256.h"
#include "24fc256_ucodes.h"
#include "24fc256_snc_demo.h"

#if dg_configI2C_ADAPTER

uint32_t snc_demo_24fc256_writer_init(ad_snc_interrupt_cb _eeprom_write_cb)
{
        uint32_t ucode_id;
        ad_snc_ucode_cfg_t cfg = { 0 };

        // Configure the PDC event and uCode priorities
        cfg.pdc_evt_pr = AD_SNC_PDC_EVT_PR_0;
        cfg.ucode_pr = AD_SNC_UCODE_PR_3;

        // Set the SNC to CM33 notification callback
        cfg.cb = _eeprom_write_cb;

        // The EEPROM writer uCode is executed on CM33 SW trigger
        cfg.pdc_entry = HW_PDC_LUT_ENTRY_VAL(HW_PDC_TRIG_SELECT_MASTER,
                HW_PDC_PERIPH_TRIG_ID_MASTERONLY, HW_PDC_MASTER_SNC, 0);

        // Create a CM33-to-SNC queue for the EEPROM writer uCode-Block
        // This queue will be used by CM33 to send data that are required to be written
        // in the EEPROM memory

        // Do not use the queue header timestamp field
        cfg.cm33_to_snc_queue_cfg.enable_data_timestamp = false;

        // The element weight is one byte
        cfg.cm33_to_snc_queue_cfg.element_weight = SNC_QUEUE_ELEMENT_SIZE_BYTE;

        // The chunk size is the EEPROM's page size plus 2 bytes for the address bytes
        cfg.cm33_to_snc_queue_cfg.max_chunk_bytes = EEPROM_24FC256_PAGE_SIZE + 2;

        // Maximum number of chunks is 4
        cfg.cm33_to_snc_queue_cfg.num_of_chunks = 4;

        // Register uCode
        ucode_id = ad_snc_ucode_register(&cfg, SNC_UCODE_CTX(ucode_write_eeprom_test_data));

        // Enable uCode
        ad_snc_ucode_enable(ucode_id);

        return ucode_id;
}

uint32_t snc_demo_24fc256_reader_init(ad_snc_interrupt_cb _eeprom_read_cb)
{
        uint32_t ucode_id;
        ad_snc_ucode_cfg_t cfg = { 0 };

        // Configure the PDC event and uCode priorities
        cfg.pdc_evt_pr = AD_SNC_PDC_EVT_PR_0;
        cfg.ucode_pr = AD_SNC_UCODE_PR_4;

        // Set the SNC to CM33 notification callback
        cfg.cb = _eeprom_read_cb;

        // The EEPROM reader uCode is executed on RTC pdc event trigger
        cfg.pdc_entry = HW_PDC_LUT_ENTRY_VAL(HW_PDC_TRIG_SELECT_PERIPHERAL,
                HW_PDC_PERIPH_TRIG_ID_RTC_TIMER, HW_PDC_MASTER_SNC, 0);

        // Create an SNC-to-CM33 queue for the EEPROM reader uCode-Block
        // This queue will be used by SNC's EEPROM reader uCode-Block
        // in order to read pages from the EEPROM and push them into the queue

        // The element weight is 1 byte
        cfg.snc_to_cm33_queue_cfg.element_weight = SNC_QUEUE_ELEMENT_SIZE_BYTE;

        // The chunk size is the EEPROM's page size
        cfg.snc_to_cm33_queue_cfg.max_chunk_bytes = EEPROM_24FC256_PAGE_SIZE;

        // Do not use the queue header timestamp field
        cfg.snc_to_cm33_queue_cfg.enable_data_timestamp = false;

        // Maximum number of chunks is 2
        cfg.snc_to_cm33_queue_cfg.num_of_chunks = 2;

        // Create a CM33-to-SNC queue.
        // This queue will be used by CM33 to send to the EEPROM reader uCode
        // page base addresses that it wants the uCode to read from

        // The element weight is 1 byte
        cfg.cm33_to_snc_queue_cfg.element_weight = SNC_QUEUE_ELEMENT_SIZE_BYTE;

        // The chunk size is the EEPROM's address size
        cfg.cm33_to_snc_queue_cfg.max_chunk_bytes = 2;

        // Do not use the queue header timestamp field
        cfg.cm33_to_snc_queue_cfg.enable_data_timestamp = false;

        // Maximum number of chunks is 16
        cfg.cm33_to_snc_queue_cfg.num_of_chunks = 16;

        // Register uCode
        ucode_id = ad_snc_ucode_register(&cfg, SNC_UCODE_CTX(ucode_read_eeprom_on_rtc));

        // Enable uCode
        ad_snc_ucode_enable(ucode_id);

        return ucode_id;
}

#endif /* dg_configI2C_ADAPTER */
