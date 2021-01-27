/**
 ****************************************************************************************
 *
 * @file bh1750.c
 *
 * @brief I2C BH1750 ambient light sensor driver source code
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configI2C_ADAPTER

#include "ad_i2c.h"
#include "platform_devices.h"

#include "bh1750.h"

static uint8_t _bh1750_mode = 0;

static void _bh1750_write_cmd(ad_i2c_handle_t dev, uint8_t thecmd)
{
        uint8_t cmd = thecmd;

        ad_i2c_write(dev, &cmd, 1, HW_I2C_F_ADD_STOP);
}

static uint16_t _bh1750_meas_access(ad_i2c_handle_t dev)
{
        uint16_t meas;

        ad_i2c_read(dev, (uint8_t*)&meas, sizeof(meas), HW_I2C_F_ADD_STOP);

        return meas;
}

void bh1750_init_mode(ad_i2c_handle_t dev, bh1750_meas_mode_t mode)
{
        _bh1750_mode = (uint8_t)mode;

        _bh1750_write_cmd(dev, BH1750_POWER_ON_OPCODE);
        _bh1750_write_cmd(dev, BH1750_RESET_OPCODE);
        _bh1750_write_cmd(dev, _bh1750_mode);
}

uint16_t bh1750_trigger_measurement(ad_i2c_handle_t dev)
{
        uint16_t meas;

        if (_bh1750_mode & BH1750_SINGLE_CONVERSION_MASK) {
                _bh1750_write_cmd(dev, BH1750_POWER_ON_OPCODE);
                _bh1750_write_cmd(dev, _bh1750_mode);
        }
        meas = _bh1750_meas_access(dev);

        return meas;
}

uint32_t bh1750_raw_meas_to_lx(uint16_t rawmeas)
{
        return (( (uint32_t)rawmeas ) * 10) / 12;
}

#endif /* dg_configI2C_ADAPTER */
