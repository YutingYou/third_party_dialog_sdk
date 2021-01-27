/**
 ****************************************************************************************
 *
 * @file 24fc256.c
 *
 * @brief 24FC256 EEPROM driver source code
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configI2C_ADAPTER

#include <string.h>
#include <osal.h>
#include <ad_i2c.h>

#include "24fc256.h"

#define MEM_24FC256_write_cycle_delay()         OS_DELAY_MS(EEPROM_MAX_WRITE_CYCLE_DURATION_MS + 1)

static uint32_t _eeprom_24fc256_read(ad_i2c_handle_t dev, uint8_t *addrbuf, uint8_t *data,
        size_t dataLen)
{
        uint8_t ret;

        ret = ad_i2c_write(dev, (uint8_t*)addrbuf, 2, HW_I2C_F_NONE);
        ret += ad_i2c_read(dev, (uint8_t*)data, dataLen, HW_I2C_F_ADD_RESTART | HW_I2C_F_ADD_STOP);

        return ret;
}

static uint32_t _eeprom_24fc256_bytes_until_next_page(uint32_t addr)
{
        return ((addr / EEPROM_24FC256_PAGE_SIZE) + 1) * EEPROM_24FC256_PAGE_SIZE - addr;
}

uint32_t eeprom_24fc256_write_data(ad_i2c_handle_t dev, uint32_t addr, uint8_t *data, size_t len)
{
        uint32_t bytesInPage;
        uint32_t bytesToWrite;
        uint32_t ret = 0;
        uint8_t write_buff[EEPROM_24FC256_PAGE_SIZE + 2];

        if (addr + len > EEPROM_24FC256_TOTAL_SIZE) {
                len = EEPROM_24FC256_TOTAL_SIZE - addr;
        }

        while (len) {
                bytesInPage = _eeprom_24fc256_bytes_until_next_page(addr);
                bytesToWrite = len;

                if (bytesInPage < bytesToWrite) {
                        bytesToWrite = bytesInPage;
                }
                write_buff[0] = (addr >> 8);
                write_buff[1] = (addr & 0xFF);
                memcpy(&write_buff[2], data, bytesToWrite);
                ret = ad_i2c_write(dev, write_buff, bytesToWrite + 2,
                        HW_I2C_F_ADD_STOP | HW_I2C_F_WAIT_FOR_STOP);

                if (ret) {
                        break;
                }

                len -= bytesToWrite;
                data += bytesToWrite;
                addr += bytesToWrite;

                MEM_24FC256_write_cycle_delay();
        }

        return (ret) ? (0) : len;
}

uint32_t eeprom_24fc256_read_data(ad_i2c_handle_t dev, uint32_t addr, uint8_t *data, size_t len)
{
        uint32_t ret;
        uint8_t addrBuf[2];

        if (addr + len > EEPROM_24FC256_TOTAL_SIZE) {
                len = EEPROM_24FC256_TOTAL_SIZE - addr;
        }

        addrBuf[0] = (addr >> 8);
        addrBuf[1] = (addr & 0xFF);

        ret = _eeprom_24fc256_read(dev, addrBuf, data, len);

        return (ret) ? (0) : len;
}

#endif /* dg_configI2C_ADAPTER */
