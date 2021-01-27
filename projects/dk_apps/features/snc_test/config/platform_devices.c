/**
 ****************************************************************************************
 *
 * @file platform_devices.c
 *
 * @brief Configuration of devices connected to board data structures
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <ad_spi.h>
#include <ad_i2c.h>
#include "peripheral_setup.h"

#include "platform_devices.h"

/*
 * PLATFORM PERIPHERALS GPIO CONFIGURATION
 *****************************************************************************************
 */

#if dg_configSPI_ADAPTER || dg_configUSE_SNC_HW_SPI

#ifdef CFG_ADXL362
static const ad_io_conf_t io_cs_SPI1[] = {
        {
                .port = ADXL362_SPI_CS_PORT, .pin = ADXL362_SPI_CS_PIN,
                .on =  { HW_GPIO_MODE_OUTPUT_PUSH_PULL, HW_GPIO_FUNC_SPI_EN,  true  },
                .off = { HW_GPIO_MODE_INPUT,            HW_GPIO_FUNC_GPIO ,   true  }
        },
};

/* SPI1 I/O configuration */
const ad_spi_io_conf_t io_SPI1 = {
        .spi_do = {
                .port = SPI1_DO_GPIO_PORT, .pin = SPI1_DO_GPIO_PIN,
                .on =  { HW_GPIO_MODE_OUTPUT_PUSH_PULL, HW_GPIO_FUNC_SPI_DO,  false },
                .off = { HW_GPIO_MODE_INPUT,            HW_GPIO_FUNC_GPIO,    true  },
        },
        .spi_di = {
                .port = SPI1_DI_GPIO_PORT, .pin = SPI1_DI_GPIO_PIN,
                .on =  { HW_GPIO_MODE_INPUT,            HW_GPIO_FUNC_SPI_DI,  false },
                .off = { HW_GPIO_MODE_INPUT,            HW_GPIO_FUNC_GPIO,    true  },
        },
        .spi_clk = {
                .port = SPI1_CLK_GPIO_PORT, .pin = SPI1_CLK_GPIO_PIN,
                .on =  { HW_GPIO_MODE_OUTPUT_PUSH_PULL, HW_GPIO_FUNC_SPI_CLK, false },
                .off = { HW_GPIO_MODE_INPUT,            HW_GPIO_FUNC_GPIO,    true  },
        },
        .cs_cnt = 1,
        .spi_cs = io_cs_SPI1,
        .voltage_level = HW_GPIO_POWER_V33
};
#endif /* CFG_ADXL362 */

#endif /* dg_configSPI_ADAPTER || dg_configUSE_SNC_HW_SPI */

#if dg_configI2C_ADAPTER || dg_configUSE_SNC_HW_I2C

#if defined(CFG_BH1750) || defined(CFG_24FC256)
/* I2C1 I/O configuration */
const ad_i2c_io_conf_t io_I2C1 = {
        .scl = {
                .port = I2C1_SCL_PORT, .pin = I2C1_SCL_PIN,
                .on =  { HW_GPIO_MODE_OUTPUT_OPEN_DRAIN, HW_GPIO_FUNC_I2C_SCL, false },
                .off = { HW_GPIO_MODE_INPUT,             HW_GPIO_FUNC_GPIO,    true  },
        },
        .sda = {
                .port = I2C1_SDA_PORT, .pin = I2C1_SDA_PIN,
                .on =  { HW_GPIO_MODE_OUTPUT_OPEN_DRAIN, HW_GPIO_FUNC_I2C_SDA, false },
                .off = { HW_GPIO_MODE_INPUT,             HW_GPIO_FUNC_GPIO,    true  },
        },
        .voltage_level = HW_GPIO_POWER_V33
};
#endif /* defined(CFG_BH1750) || defined(CFG_24FC256) */

#endif /* dg_configI2C_ADAPTER || dg_configUSE_SNC_HW_I2C */


/*
 * PLATFORM PERIPHERALS CONTROLLER CONFIGURATION
 *****************************************************************************************
 */

#if dg_configSPI_ADAPTER || dg_configUSE_SNC_HW_SPI

#ifdef CFG_ADXL362
/* ADXL362 SPI driver configuration */
const ad_spi_driver_conf_t drv_ADXL362 = {
        .spi.cs_pad             = { ADXL362_SPI_CS_PORT, ADXL362_SPI_CS_PIN },
        .spi.word_mode          = HW_SPI_WORD_8BIT,
        .spi.smn_role           = HW_SPI_MODE_MASTER,
        .spi.polarity_mode      = HW_SPI_POL_LOW,
        .spi.phase_mode         = HW_SPI_PHA_MODE_0,
        .spi.mint_mode          = HW_SPI_MINT_DISABLE,
        .spi.xtal_freq          = HW_SPI_FREQ_DIV_8,
        .spi.fifo_mode          = HW_SPI_FIFO_RX_TX,
        .spi.disabled           = 0,
        .spi.ignore_cs          = 0,
        .spi.use_dma            = true,
        .spi.rx_dma_channel     = HW_DMA_CHANNEL_0,
        .spi.tx_dma_channel     = HW_DMA_CHANNEL_1
};
/* ADXL362 SPI controller configuration */
const ad_spi_controller_conf_t dev_ADXL362 = {
        .id     = HW_SPI1,
        .io     = &io_SPI1,
        .drv    = &drv_ADXL362
};
/* ADXL362 device */
const spi_device ADXL362 = &dev_ADXL362;
#endif /* CFG_ADXL362 */

#endif /* dg_configSPI_ADAPTER || dg_configUSE_SNC_HW_SPI */

#if dg_configI2C_ADAPTER || dg_configUSE_SNC_HW_I2C

#ifdef CFG_BH1750
/* BH1750 I2C driver configuration */
const ad_i2c_driver_conf_t drv_BH1750 = {
        I2C_DEFAULT_CLK_CFG,
        .i2c.speed              = HW_I2C_SPEED_STANDARD,
        .i2c.mode               = HW_I2C_MODE_MASTER,
        .i2c.addr_mode          = HW_I2C_ADDRESSING_7B,
        .i2c.address            = BH1750_I2C_ADDRESS,
        .dma_channel            = HW_DMA_CHANNEL_2
};
/* BH1750 I2C controller configuration */
const ad_i2c_controller_conf_t dev_BH1750 = {
        .id     = HW_I2C1,
        .io     = &io_I2C1,
        .drv    = &drv_BH1750
};
const i2c_device BH1750 = &dev_BH1750;
#endif /* CFG_BH1750 */

#ifdef CFG_24FC256
/* Mem. 24FC256 I2C driver configuration */
const ad_i2c_driver_conf_t drv_MEM_24FC256 = {
        I2C_DEFAULT_CLK_CFG,
        .i2c.speed              = HW_I2C_SPEED_STANDARD,
        .i2c.mode               = HW_I2C_MODE_MASTER,
        .i2c.addr_mode          = HW_I2C_ADDRESSING_7B,
        .i2c.address            = EEPROM_24FC256_ADDRESS,
        .dma_channel            = HW_DMA_CHANNEL_2
};
/* Mem. 24FC256 I2C controller configuration */
const ad_i2c_controller_conf_t dev_MEM_24FC256 = {
        .id     = HW_I2C1,
        .io     = &io_I2C1,
        .drv    = &drv_MEM_24FC256
};
const i2c_device MEM_24FC256 = &dev_MEM_24FC256;
#endif /* CFG_24FC256 */

#endif /* dg_configI2C_ADAPTER || dg_configUSE_SNC_HW_I2C */

