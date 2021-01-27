/**
 ****************************************************************************************
 *
 * @file adxl362_snc_demo.c
 *
 * @brief ADXL362 - SNC Demo functions source code
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include "hw_pdc.h"
#include "SeNIS.h"
#include "snc_queues.h"
#include "platform_devices.h"
#include "peripheral_setup.h"

#include "adxl362_snc_demo.h"
#include "adxl362_ucodes.h"
#include "adxl362.h"

#if dg_configSPI_ADAPTER

/* ADXL362 Configuration Macros */
#define SNC_DEMO_ADXL362_ACC_RATE       ADXL362_ACC_RATE_100HZ
#define SNC_DEMO_ADXL362_FIFO_WM_LEVEL  (ADXL362_FIFO_WM_LEVEL + 1)

/* ADXL362 Helper Macros*/
#define _adxl362_init()                                                                 \
        ADXL362_Init(adxl362_hdl)

#define _adxl362_set_register(regvalue, regAddr, bytesNum)                              \
        ADXL362_SetRegisterValue(adxl362_hdl, regvalue, regAddr, bytesNum)

#define _adxl362_get_register(regData, regAddr, bytesNum)                               \
        ADXL362_GetRegisterValue(adxl362_hdl, regData, regAddr, bytesNum)

#define _adxl362_get_fifo(pbuf, bytesNum)                                               \
        ADXL362_GetFifoValue(adxl362_hdl, pbuf, bytesNum)

#define _adxl362_sw_reset()                                                             \
        ADXL362_SoftwareReset(adxl362_hdl)

#define _adxl362_set_power_mode(pwrmode)                                                \
        ADXL362_SetPowerMode(adxl362_hdl, pwrmode)

#define _adxl362_set_range(grange)                                                      \
        ADXL362_SetRange(adxl362_hdl, grange)

#define _adxl362_set_output_rate(outrate)                                               \
        ADXL362_SetOutputRate(adxl362_hdl, outrate)

#define _adxl362_get_xyz(x, y, z)                                                       \
        ADXL362_GetXyz(adxl362_hdl, x, y, z)

#define _adxl362_get_G_xyz(x, y, z)                                                     \
        ADXL362_GetGxyz(adxl362_hdl, x, y, z)

#define _adxl362_read_temperature()                                                     \
        ADXL362_ReadTemperature(adxl362_hdl)

#define _adxl362_fifo_setup(mode, watermark_LVL, enTempRead)                            \
        ADXL362_FifoSetup(adxl362_hdl, mode, watermark_LVL, enTempRead)

#define _adxl362_setup_activity_detection(refOrArbs, thresh, time)                      \
        ADXL362_SetupActivityDetection(adxl362_hdl, refOrArbs, thresh, time)

#define _adxl362_setup_inactivity_detection(refOrArbs, thresh, time)                    \
        ADXL362_SetupInactivityDetection(adxl362_hdl, refOrArbs, thresh, time)

#define _adxl362_get_status()                                                           \
        ADXL362_GetStatus(adxl362_hdl)

#define _adxl362_get_fifo_samples()                                                     \
        ADXL362_GetFifoSamples(adxl362_hdl)

uint32_t snc_demo_adxl362_init(ad_snc_interrupt_cb _adxl362_int1_cb)
{
        uint32_t ucode_id;
        ad_snc_ucode_cfg_t cfg = { 0 };

        ad_spi_handle_t adxl362_hdl;

        // Open the SPI Controller based on ADXL362 configuration
        adxl362_hdl = ad_spi_open(ADXL362);
        if (adxl362_hdl == NULL) {
                return AD_SNC_UCODE_ID_INVALID;
        }

        // Initialize ADXL362
        _adxl362_init();

        // Send a soft reset command
        _adxl362_sw_reset();

        // Enable INT1 interrupt and associate it with the FIFO watermark
        _adxl362_set_register(ADXL362_INTMAP1_INT_LOW, ADXL362_REG_INTMAP1, 1);

        // Set the accelerometer range
        _adxl362_set_range(ADXL362_RANGE_2G);

        // Set the accelerometer output rate
        _adxl362_set_output_rate(SNC_DEMO_ADXL362_ACC_RATE);

        // Set the accelerometer power mode to standby
        _adxl362_set_power_mode(ADXL362_MODE_STANDBY);

        // Accelerometer FIFO Flush
        _adxl362_fifo_setup(ADXL362_FIFO_DISABLE, 0x80, 0);

        // Accelerometer FIFO Setup
        _adxl362_fifo_setup(ADXL362_FIFO_STREAM, ADXL362_FIFO_WM_LEVEL, 0);

        // Set the accelerometer power mode
        _adxl362_set_power_mode(ADXL362_MODE_NORMAL);

        // Setup the FIFO interrupt
        _adxl362_set_register(ADXL362_INTMAP1_INT_LOW | ADXL362_INTMAP1_FIFO_WATERMARK,
                ADXL362_REG_INTMAP1, 1);

        // Close the SPI controller
        while (ad_spi_close(adxl362_hdl, false) != AD_SPI_ERROR_NONE);

        // Register the ADXL362 INT1 uCode
        // Configure the PDC event and uCode priorities
        cfg.pdc_evt_pr = AD_SNC_PDC_EVT_PR_0;
        cfg.ucode_pr = AD_SNC_UCODE_PR_1;

        // Set the SNC notification callback
        cfg.cb = _adxl362_int1_cb;

        // The ADXL362 uCode is executed on INT1 pin interrupt
        cfg.pdc_entry = HW_PDC_LUT_ENTRY_VAL(ADXL362_INT_1_PORT, ADXL362_INT_1_PIN,
                HW_PDC_MASTER_SNC, 0);

        cfg.snc_to_cm33_queue_cfg.max_chunk_bytes =
                (SNC_DEMO_ADXL362_FIFO_WM_LEVEL % 3 == 0) ?
                        (SNC_DEMO_ADXL362_FIFO_WM_LEVEL * 2) :
                        ((SNC_DEMO_ADXL362_FIFO_WM_LEVEL + 1 + (SNC_DEMO_ADXL362_FIFO_WM_LEVEL % 2))
                                * 2);

        cfg.snc_to_cm33_queue_cfg.enable_data_timestamp = true;
        cfg.snc_to_cm33_queue_cfg.element_weight = SNC_QUEUE_ELEMENT_SIZE_WORD;
        cfg.snc_to_cm33_queue_cfg.num_of_chunks = 4;
        cfg.snc_to_cm33_queue_cfg.swap_popped_data_bytes = true;

        // Register uCode
        ucode_id = ad_snc_ucode_register(&cfg, SNC_UCODE_CTX(ucode_gpio_adxl362_int1_queue_test));

        // Enable uCode
        ad_snc_ucode_enable(ucode_id);

        return ucode_id;
}

#endif /* dg_configSPI_ADAPTER */
