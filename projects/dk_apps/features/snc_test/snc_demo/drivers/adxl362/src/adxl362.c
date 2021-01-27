/***************************************************************************//**
 *   @file   ADXL362.c
 *   @brief  Implementation of ADXL362 Driver.
 *   @author DNechita(Dan.Nechita@analog.com)
 ********************************************************************************
 * Copyright 2012(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ********************************************************************************
 *   SVN Revision: $WCREV$
 *******************************************************************************/
#if dg_configSPI_ADAPTER
/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdio.h>
#include <osal.h>
#include <ad_spi.h>

#include "adxl362.h"

/******************************************************************************/
/************************* Variables Declarations *****************************/
/******************************************************************************/
int8_t selectedRange = 0;

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

#define ADXL362_RESET_CODE      0x52
#define ADXL362_FILTER_CTL_CODE 0x51
#define ADXL362_POWER_CTL_CODE  0x22
#define ADXL362_ACC_RANGE       4       /* g */
#define ADXL362_MAX_11_BIT      0x07FF

/***************************************************************************//**
 * @brief Initializes communication with the device and checks if the part is
 *        present by reading the device id.
 *
 * @return  0 - the initialization was successful and the device is present;
 *         -1 - an error occurred.
 *******************************************************************************/
int8_t ADXL362_Init(ad_spi_handle_t dev)
{
        unsigned char regValue = 0;
        int8_t status = 0;
        ADXL362_GetRegisterValue(dev, &regValue, ADXL362_REG_PARTID, 1);
        if ((regValue != ADXL362_PART_ID))
        {
                status = 0;
        }
        selectedRange = 2; // Measurement Range: +/- 2g (reset default).

        return status;
}

/***************************************************************************//**
 * @brief Writes data into a register.
 *
 * @param registerValue   - Data value to write.
 * @param registerAddress - Address of the register.
 * @param bytesNumber     - Number of bytes to write.
 * @return None.
 *******************************************************************************/
void ADXL362_SetRegisterValue(ad_spi_handle_t dev, uint16_t registerValue, uint8_t registerAddress,
        uint8_t bytesNumber)
{
        uint8_t wbuf[2] = { ADXL362_WRITE_REG, registerAddress };

        ad_spi_activate_cs(dev);
        ad_spi_write(dev, wbuf, sizeof(wbuf));
        ad_spi_write(dev, (uint8_t*)&registerValue, bytesNumber);
        ad_spi_deactivate_cs_when_spi_done(dev);
}

/***************************************************************************//**
 * @brief Performs a burst read of a specified number of registers.
 *
 * @param pReadData       - The read values are stored in this buffer.
 * @param registerAddress - The start address of the burst read.
 * @param bytesNumber     - Number of bytes to read.
 *
 * @return None.
 *******************************************************************************/
void ADXL362_GetRegisterValue(ad_spi_handle_t dev,
        uint8_t* pReadData,
        uint8_t registerAddress,
        uint8_t bytesNumber)
{
        uint8_t wbuf[2] = { ADXL362_READ_REG, registerAddress };

        ad_spi_activate_cs(dev);
        ad_spi_write(dev, wbuf, sizeof(wbuf));
        ad_spi_read(dev, pReadData, bytesNumber);
        ad_spi_deactivate_cs_when_spi_done(dev);

        return;
}

/***************************************************************************//**
 * @brief Reads multiple bytes from the device's FIFO buffer.
 *
 * @param pBuffer     - Stores the read bytes.
 * @param bytesNumber - Number of bytes to read.
 *
 * @return None.
 *******************************************************************************/
void ADXL362_GetFifoValue(ad_spi_handle_t dev, uint8_t* pBuffer, uint16_t bytesNumber)
{
        uint8_t wbuf[1] = { ADXL362_READ_FIFO };

        ad_spi_activate_cs(dev);
        ad_spi_write(dev, wbuf, sizeof(wbuf));
        ad_spi_read(dev, pBuffer, bytesNumber - (bytesNumber % 2));
        ad_spi_deactivate_cs_when_spi_done(dev);
}

/***************************************************************************//**
 * @brief Resets the device via SPI communication bus.
 *
 * @return None.
 *******************************************************************************/
void ADXL362_SoftwareReset(ad_spi_handle_t dev)
{
        ADXL362_SetRegisterValue(dev, ADXL362_RESET_KEY, ADXL362_REG_SOFT_RESET, 1);

        OS_DELAY_MS(2);    // delay 2 ms
}

/***************************************************************************//**
 * @brief Places the device into standby/measure mode.
 *
 * @param pwrMode - Power mode.
 *                  Example: 0 - ADXL362_MODE_STANDBY
 *                           1 - ADXL362_MODE_NORMAL
 *                           2 - ADXL362_MODE_LOW_NOISE
 *                           3 - ADXL362_MODE_ULTRA_LOW_NOISE
 *
 * @return None.
 *******************************************************************************/
void ADXL362_SetPowerMode(ad_spi_handle_t dev, uint8_t pwrMode)
{
        uint8_t oldPowerCtl = 0;
        uint8_t newPowerCtl = 0;

        ADXL362_GetRegisterValue(dev, &oldPowerCtl, ADXL362_REG_POWER_CTL, 1);
        newPowerCtl = oldPowerCtl
                & (~(ADXL362_POWER_CTL_MEASURE(0x3) | ADXL362_POWER_CTL_LOW_NOISE(0x3)));

        switch (pwrMode) {
        case ADXL362_MODE_STANDBY:
                newPowerCtl = newPowerCtl | (ADXL362_POWER_CTL_MEASURE(ADXL362_MEASURE_STANDBY));
                break;
        case ADXL362_MODE_NORMAL:
                newPowerCtl = newPowerCtl | (ADXL362_POWER_CTL_MEASURE(ADXL362_MEASURE_ON))
                        | ADXL362_POWER_CTL_LOW_NOISE(ADXL362_NOISE_MODE_NORMAL);
                break;
        case ADXL362_MODE_LOW_NOISE:
                newPowerCtl = newPowerCtl | (ADXL362_POWER_CTL_MEASURE(ADXL362_MEASURE_ON))
                        | ADXL362_POWER_CTL_LOW_NOISE(ADXL362_NOISE_MODE_LOW);
                break;
        case ADXL362_MODE_ULTRA_LOW_NOISE:
                newPowerCtl = newPowerCtl | (ADXL362_POWER_CTL_MEASURE(ADXL362_MEASURE_ON))
                        | ADXL362_POWER_CTL_LOW_NOISE(ADXL362_NOISE_MODE_ULTRALOW);
                break;
        }
        ADXL362_SetRegisterValue(dev, newPowerCtl, ADXL362_REG_POWER_CTL, 1);
}

/***************************************************************************//**
 * @brief Selects the measurement range.
 *
 * @param gRange - Range option.
 *                  Example: ADXL362_RANGE_2G  -  +-2 g
 *                           ADXL362_RANGE_4G  -  +-4 g
 *                           ADXL362_RANGE_8G  -  +-8 g
 *
 * @return None.
 *******************************************************************************/
void ADXL362_SetRange(ad_spi_handle_t dev, uint8_t gRange)
{
        uint8_t oldFilterCtl = 0;
        uint8_t newFilterCtl = 0;

        ADXL362_GetRegisterValue(dev, &oldFilterCtl, ADXL362_REG_FILTER_CTL, 1);
        newFilterCtl = oldFilterCtl & ~ADXL362_FILTER_CTL_RANGE(0x3);
        newFilterCtl = newFilterCtl | ADXL362_FILTER_CTL_RANGE(gRange);
        ADXL362_SetRegisterValue(dev, newFilterCtl, ADXL362_REG_FILTER_CTL, 1);
        selectedRange = (1 << gRange) * 2;
}

/***************************************************************************//**
 * @brief Selects the Output Data Rate of the device.
 *
 * @param outRate - Output Data Rate option.
 *                  Example: ADXL362_ODR_12_5_HZ  -  12.5Hz
 *                           ADXL362_ODR_25_HZ    -  25Hz
 *                           ADXL362_ODR_50_HZ    -  50Hz
 *                           ADXL362_ODR_100_HZ   -  100Hz
 *                           ADXL362_ODR_200_HZ   -  200Hz
 *                           ADXL362_ODR_400_HZ   -  400Hz
 *
 * @return None.
 *******************************************************************************/
void ADXL362_SetOutputRate(ad_spi_handle_t dev, uint8_t outRate)
{
        unsigned char oldFilterCtl = 0;
        unsigned char newFilterCtl = 0;

        ADXL362_GetRegisterValue(dev, &oldFilterCtl, ADXL362_REG_FILTER_CTL, 1);
        newFilterCtl = oldFilterCtl & ~ADXL362_FILTER_CTL_ODR(0x7);
        newFilterCtl = newFilterCtl | ADXL362_FILTER_CTL_ODR(outRate);
        ADXL362_SetRegisterValue(dev, newFilterCtl, ADXL362_REG_FILTER_CTL, 1);
}

/***************************************************************************//**
 * @brief Reads the 3-axis raw data from the accelerometer.
 *
 * @param x - Stores the X-axis data(as two's complement).
 * @param y - Stores the Y-axis data(as two's complement).
 * @param z - Stores the Z-axis data(as two's complement).
 *
 * @return None.
 *******************************************************************************/
void ADXL362_GetXyz(ad_spi_handle_t dev, int16_t* x, int16_t* y, int16_t* z)
{
        uint8_t xyzValues[6] = { 0, 0, 0, 0, 0, 0 };

        ADXL362_GetRegisterValue(dev, xyzValues, ADXL362_REG_XDATA_L, 6);
        *x = (int16_t)(((uint16_t)xyzValues[1] << 8) + xyzValues[0]);
        *y = (int16_t)(((uint16_t)xyzValues[3] << 8) + xyzValues[2]);
        *z = (int16_t)(((uint16_t)xyzValues[5] << 8) + xyzValues[4]);
}

/***************************************************************************//**
 * @brief Reads the 3-axis raw data from the accelerometer and converts it to g.
 *
 * @param x - Stores the X-axis data.
 * @param y - Stores the Y-axis data.
 * @param z - Stores the Z-axis data.
 *
 * @return None.
 *******************************************************************************/
void ADXL362_GetGxyz(ad_spi_handle_t dev, float* x, float* y, float* z)
{
        uint8_t xyzValues[6] = { 0, 0, 0, 0, 0, 0 };

        ADXL362_GetRegisterValue(dev, xyzValues, ADXL362_REG_XDATA_L, 6);
        *x = (int16_t)(((uint16_t)xyzValues[1] << 8) + xyzValues[0]);
        *x /= (1000 / (selectedRange / 2));
        *y = (int16_t)(((uint16_t)xyzValues[3] << 8) + xyzValues[2]);
        *y /= (1000 / (selectedRange / 2));
        *z = (int16_t)(((uint16_t)xyzValues[5] << 8) + xyzValues[4]);
        *z /= (1000 / (selectedRange / 2));
}

/***************************************************************************//**
 * @brief Reads the temperature of the device.
 *
 * @return tempCelsius - The value of the temperature(degrees Celsius).
 *******************************************************************************/
float ADXL362_ReadTemperature(ad_spi_handle_t dev)
{
        uint8_t rawTempData[2] = { 0, 0 };
        int16_t signedTemp = 0;
        float tempCelsius = 0;

        ADXL362_GetRegisterValue(dev, rawTempData, ADXL362_REG_TEMP_L, 2);
        signedTemp = (int16_t)(rawTempData[1] << 8) + rawTempData[0];
        tempCelsius = (float)signedTemp * 0.065;

        return tempCelsius;
}

/***************************************************************************//**
 * @brief Configures the FIFO feature.
 *
 * @param mode         - Mode selection.
 *                       Example: ADXL362_FIFO_DISABLE      -  FIFO is disabled.
 *                                ADXL362_FIFO_OLDEST_SAVED -  Oldest saved mode.
 *                                ADXL362_FIFO_STREAM       -  Stream mode.
 *                                ADXL362_FIFO_TRIGGERED    -  Triggered mode.
 * @param waterMarkLvl - Specifies the number of samples to store in the FIFO.
 * @param enTempRead   - Store Temperature Data to FIFO.
 *                       Example: 1 - temperature data is stored in the FIFO
 *                                    together with x-, y- and x-axis data.
 *                                0 - temperature data is skipped.
 *
 * @return None.
 *******************************************************************************/
void ADXL362_FifoSetup(ad_spi_handle_t dev,
        uint8_t mode,
        uint16_t waterMarkLvl,
        uint8_t enTempRead)
{
        uint8_t writeVal = 0;

        writeVal = ADXL362_FIFO_CTL_FIFO_MODE(mode) |
                (enTempRead * ADXL362_FIFO_CTL_FIFO_TEMP) |
                ((waterMarkLvl > 255) ? ADXL362_FIFO_CTL_AH : 0);
        ADXL362_SetRegisterValue(dev, writeVal, ADXL362_REG_FIFO_CTL, 1);
        ADXL362_SetRegisterValue(dev, waterMarkLvl, ADXL362_REG_FIFO_SAMPLES, 1);
}

/***************************************************************************//**
 * @brief Configures activity detection.
 *
 * @param refOrAbs  - Referenced/Absolute Activity Select.
 *                    Example: 0 - absolute mode.
 *                             1 - referenced mode.
 * @param threshold - 11-bit unsigned value that the adxl362 samples are
 *                    compared to.
 * @param time      - 8-bit value written to the activity timer register. The
 *                    amount of time (in seconds) is: time / ODR, where ODR - is
 *                    the output data rate.
 *
 * @return None.
 *******************************************************************************/
void ADXL362_SetupActivityDetection(ad_spi_handle_t dev,
        uint8_t refOrAbs,
        uint16_t threshold,
        uint8_t time)
{
        uint8_t oldActInactReg = 0;
        uint8_t newActInactReg = 0;

        /* Configure motion threshold and activity timer. */
        ADXL362_SetRegisterValue(dev, (threshold & 0x7FF), ADXL362_REG_THRESH_ACT_L, 2);
        ADXL362_SetRegisterValue(dev, time, ADXL362_REG_TIME_ACT, 1);
        /* Enable activity interrupt and select a referenced or absolute
         configuration. */
        ADXL362_GetRegisterValue(dev, &oldActInactReg, ADXL362_REG_ACT_INACT_CTL, 1);
        newActInactReg = oldActInactReg & ~ADXL362_ACT_INACT_CTL_ACT_REF;
        newActInactReg |= ADXL362_ACT_INACT_CTL_ACT_EN |
                (refOrAbs * ADXL362_ACT_INACT_CTL_ACT_REF);
        ADXL362_SetRegisterValue(dev, newActInactReg, ADXL362_REG_ACT_INACT_CTL, 1);
}

/***************************************************************************//**
 * @brief Configures inactivity detection.
 *
 * @param refOrAbs  - Referenced/Absolute Inactivity Select.
 *                    Example: 0 - absolute mode.
 *                             1 - referenced mode.
 * @param threshold - 11-bit unsigned value that the adxl362 samples are
 *                    compared to.
 * @param time      - 16-bit value written to the inactivity timer register. The
 *                    amount of time (in seconds) is: time / ODR, where ODR - is
 *                    the output data rate.
 *
 * @return None.
 *******************************************************************************/
void ADXL362_SetupInactivityDetection(ad_spi_handle_t dev,
        uint8_t refOrAbs,
        uint16_t threshold,
        uint16_t time)
{
        uint8_t oldActInactReg = 0;
        uint8_t newActInactReg = 0;

        /* Configure motion threshold and inactivity timer. */
        ADXL362_SetRegisterValue(dev, (threshold & 0x7FF),
        ADXL362_REG_THRESH_INACT_L,
                2);
        ADXL362_SetRegisterValue(dev, time, ADXL362_REG_TIME_INACT_L, 2);
        /* Enable inactivity interrupt and select a referenced or absolute
         configuration. */
        ADXL362_GetRegisterValue(dev, &oldActInactReg, ADXL362_REG_ACT_INACT_CTL, 1);
        newActInactReg = oldActInactReg & ~ADXL362_ACT_INACT_CTL_INACT_REF;
        newActInactReg |= ADXL362_ACT_INACT_CTL_INACT_EN |
                (refOrAbs * ADXL362_ACT_INACT_CTL_INACT_REF);
        ADXL362_SetRegisterValue(dev, newActInactReg, ADXL362_REG_ACT_INACT_CTL, 1);
}

uint8_t ADXL362_GetStatus(ad_spi_handle_t dev)
{
        uint8_t status = 0;

        ADXL362_GetRegisterValue(dev, &status, ADXL362_REG_STATUS, 1);

        return status;
}

uint16_t ADXL362_GetFifoSamples(ad_spi_handle_t dev)
{
        uint8_t samples[2] = { 0, };

        ADXL362_GetRegisterValue(dev, samples, ADXL362_REG_FIFO_L, 2);

        return (((uint16_t)(samples[1] & 0x03)) << 8) + samples[0];
}

/***************************************************************************//**
 * @brief Converts FIFO raw samples to actual x,y,z coordinates
 * @param[in] sDat a pointer to the samples/coordinates buffer
 * @return adxl362_meas_t the x,y,z values
 *******************************************************************************/
adxl362_meas_t ADXL362_ConvertXYZ(uint8_t *sDat)
{
        adxl362_meas_t ret = {0};
        uint32_t i = 1;

        if ((sDat[i] & 0xC0) == 0) {
                ret.accX = (int16_t)((((uint16_t)((sDat[i] & 0x3f) |
                        ((sDat[i] & 0x30) << 2))) << 8) |
                        sDat[i - 1]);
        }

        i += 2;

        if ((sDat[i] & 0xC0) == 0x40) {
                ret.accY = (int16_t)((((uint16_t)((sDat[i] & 0x3f) |
                        ((sDat[i] & 0x30) << 2))) << 8) |
                        sDat[i - 1]);
        }

        i += 2;

        if ((sDat[i] & 0xC0) == 0x80) {
                ret.accZ = (int16_t)((((uint16_t)((sDat[i] & 0x3f) |
                        ((sDat[i] & 0x30) << 2))) << 8) |
                        sDat[i - 1]);
        }

        return ret;
}
#endif /* dg_configSPI_ADAPTER */
