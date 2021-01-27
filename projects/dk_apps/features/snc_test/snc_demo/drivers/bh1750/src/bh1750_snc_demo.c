/**
 ****************************************************************************************
 *
 * @file bh1750_snc_demo.c
 *
 * @brief BH1750 - SNC Demo functions source code
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include "hw_pdc.h"
#include "SeNIS.h"
#include "platform_devices.h"

#include "bh1750.h"
#include "bh1750_ucodes.h"
#include "bh1750_snc_demo.h"

#if dg_configI2C_ADAPTER

uint32_t snc_demo_bh1750_init(ad_snc_interrupt_cb _bh1750_read_cb, uint8_t mode, uint32_t notify_samples)
{
        uint32_t ucode_id;
        ad_snc_ucode_cfg_t cfg = { 0 };
        snc_ucode_context_t *ucode_ctx;

        ad_i2c_handle_t bh1750_hdl;

        // Configure the PDC event and uCode priorities
        cfg.pdc_evt_pr = AD_SNC_PDC_EVT_PR_0;
        cfg.ucode_pr = AD_SNC_UCODE_PR_2;

        // Set the SNC to CM33 notification callback
        cfg.cb = _bh1750_read_cb;

        // The BH1750 uCode is executed on RTC-to-PDC event interrupt
        cfg.pdc_entry = HW_PDC_LUT_ENTRY_VAL(HW_PDC_TRIG_SELECT_PERIPHERAL,
                HW_PDC_PERIPH_TRIG_ID_RTC_TIMER, HW_PDC_MASTER_SNC, 0);

        // Open the I2C controller based on BH1750 configuration
        bh1750_hdl = ad_i2c_open(BH1750);
        if (bh1750_hdl == NULL) {
                return AD_SNC_UCODE_ID_INVALID;
        }

        // Initialize BH1750 in continuous h-res modes
        bh1750_init_mode(bh1750_hdl, BH1750_HIGH_RES_CONT_MODE);

        // Close the I2C controller
        while (ad_i2c_close(bh1750_hdl, false) != AD_I2C_ERROR_NONE);

        // Configure uCode & SNC parameters depending on the demo mode
        switch (mode) {
        case 0:
                ucode_ctx = SNC_UCODE_CTX(ucode_bh1750_ambient_light_collect_samples);
                ucode_bh1750_init();
                ucode_bh1750_set_notify_samples(notify_samples);
                break;
        case 1:
                cfg.snc_to_cm33_queue_cfg.max_chunk_bytes = BH1750_SAMPLE_SIZE;
                cfg.snc_to_cm33_queue_cfg.element_weight = SNC_QUEUE_ELEMENT_SIZE_BYTE;
                cfg.snc_to_cm33_queue_cfg.num_of_chunks = notify_samples;
                ucode_bh1750_set_notify_samples(notify_samples);
                ucode_ctx = SNC_UCODE_CTX(ucode_bh1750_ambient_light_collect_samples_queue );
                break;
        case 2:
                ucode_bh1750_init();
                ucode_ctx = SNC_UCODE_CTX(ucode_bh1750_ambient_light_notifier );
                break;
        default:
                ucode_ctx = SNC_UCODE_CTX(ucode_bh1750_ambient_light_collect_samples );
        }

        // Register uCode
        ucode_id = ad_snc_ucode_register(&cfg, ucode_ctx);

        // Enable uCode
        ad_snc_ucode_enable(ucode_id);

        return ucode_id;
}

void bh1750_set_notify_samples(uint32_t notify_samples)
{
        ucode_bh1750_set_notify_samples(notify_samples);
}

void bh1750_set_threshold_high(uint16_t high_threshold, uint16_t tolerance)
{
        ucode_bh1750_set_threshold_high(high_threshold,tolerance);
}

void bh1750_set_threshold_low(uint16_t low_threshold, uint16_t tolerance)
{
        ucode_bh1750_set_threshold_low(low_threshold,tolerance);
}

uint32_t bh1750_get_ambient_light_state(uint32_t *notif_light_value)
{
        *notif_light_value = ucode_bh1750_get_last_notif_value();
        return  ucode_bh1750_get_notif_state();
}

uint32_t bh1750_get_ambient_light_last_value()
{
        return  ucode_bh1750_get_last_sensor_value();
}

#endif /* dg_configI2C_ADAPTER */
