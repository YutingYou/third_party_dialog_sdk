/**
 ****************************************************************************************
 *
 * @file 24fc256.h
 *
 * @brief 24FC256 EEPROM driver header file
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef _24FC256_H_
#define _24FC256_H_

#include "sdk_defs.h"
#include "ad_i2c.h"

#define EEPROM_24FC256_PAGE_SIZE                (64)
#define EEPROM_24FC256_NUM_OF_PAGES             (512)
#define EEPROM_24FC256_TOTAL_SIZE               (EEPROM_24FC256_NUM_OF_PAGES * EEPROM_24FC256_PAGE_SIZE)

#define EEPROM_24FC256_MAX_WRITE_SIZE           (EEPROM_24FC256_PAGE_SIZE)
#define EEPROM_24FC256_MAX_READ_SIZE            (EEPROM_24FC256_TOTAL_SIZE)

#define EEPROM_WRAP_MULTIPLE                    (EEPROM_24FC256_PAGE_SIZE)
#define EEPROM_MAX_WRITE_CYCLE_DURATION_MS      (5)

/**
 * \brief Write data to the EEPROM
 *
 * \param [in] addr     the starting memory address to write
 * \param [in] data     a pointer to the data to be written
 * \param [in] len      the length of the data to be written
 *
 * \return uint32_t the actual size of the data that was written
 *      (if 0, there was an error during the write transfer)
 *
 */
uint32_t eeprom_24fc256_write_data(ad_i2c_handle_t dev, uint32_t addr, uint8_t *data, size_t len);

/**
 * \brief Read data from the EEPROM
 *
 * \param [in] addr     the starting memory address to read from
 * \param [out] data    pointer where the read data will be placed
 * \param [in] len      the length of the data to read
 *
 * \return uint32_t the actual size of the data that was read
 *      (if 0, there was an error during the read transfer)
 *
 */
uint32_t eeprom_24fc256_read_data(ad_i2c_handle_t dev, uint32_t addr, uint8_t *data, size_t len);

#endif /* _24FC256_H_ */
