/**
 ****************************************************************************************
 *
 * @file platform_devices.h
 *
 * @brief Configuration of devices connected to board
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef PLATFORM_DEVICES_H_
#define PLATFORM_DEVICES_H_

#define CFG_24FC256
#define CFG_BH1750
#define CFG_ADXL362

/**
 * \brief SPI device handle
 */
typedef const void* spi_device;

/**
 * \brief I2C device handle
 */
typedef const void* i2c_device;


#if dg_configSPI_ADAPTER || dg_configUSE_SNC_HW_SPI
/*
 * SPI DEVICES
 *****************************************************************************************
 */
#ifdef CFG_ADXL362
/**
 * \brief ADXL362 device
 */
extern const spi_device ADXL362;
#endif /* ADXL362 */

#endif /* dg_configSPI_ADAPTER || dg_configUSE_SNC_HW_SPI */

/*
 * I2C DEVICES
 *****************************************************************************************
 */
#if dg_configI2C_ADAPTER || dg_configUSE_SNC_HW_I2C

#ifdef CFG_BH1750
/**
 * \brief BH1750 device
 */
extern const i2c_device BH1750;
#endif /* CFG_BH1750 */

#ifdef CFG_24FC256
/**
 * \brief MEM. 24FC256 device
 */
extern const i2c_device MEM_24FC256;
#endif /* CFG_24FC256 */

#endif /* dg_configI2C_ADAPTER || dg_configUSE_SNC_HW_I2C */

#endif /* PLATFORM_DEVICES_H_ */
