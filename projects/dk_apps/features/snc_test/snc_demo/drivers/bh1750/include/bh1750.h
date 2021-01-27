/**
 ****************************************************************************************
 *
 * @file bh1750.h
 *
 * @brief BH1750 Ambient Light Sensor Header File
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BH1750_H_
#define BH1750_H_

#include "ad_i2c.h"
#include "sdk_defs.h"

/*
 * MACRO DEPENDENCIES
 *****************************************************************************************
 */

/* Measurement Conversion Masks */
#define BH1750_SINGLE_CONVERSION_MASK           (0x10)
#define BH1750_CONTINUOUS_CONVERSION_MASK       (0x20)

/* Measurement Resolution */
#define BH1750_HRES_MODE                        (0x00)
#define BH1750_HRES_MODE2                       (0x01)
#define BH1750_LRES_MODE                        (0x03)

/* BH1750 Instruction Set */
#define BH1750_POWER_DOWN_OPCODE                0x00    // Power Down
#define BH1750_POWER_ON_OPCODE                  0x01    // Power On
#define BH1750_RESET_OPCODE                     0x07    // Reset

#define BH1750_CONT_H_RES_MODE_OPCODE           \
        (BH1750_CONTINUOUS_CONVERSION_MASK | BH1750_HRES_MODE)  // Continuous H-Resolution Mode

#define BH1750_CONT_H_RES_MODE2_OPCODE          \
        (BH1750_CONTINUOUS_CONVERSION_MASK | BH1750_HRES_MODE2) // Continuous H-Resolution Mode2

#define BH1750_CONT_L_RES_MODE_OPCODE           \
        (BH1750_CONTINUOUS_CONVERSION_MASK | BH1750_LRES_MODE)  // Continuous L-Resolution Mode

#define BH1750_SING_H_RES_MODE_OPCODE           \
        (BH1750_SINGLE_CONVERSION_MASK | BH1750_HRES_MODE)      // One Time H-Resolution Mode

#define BH1750_SING_H_RES_MODE2_OPCODE          \
        (BH1750_SINGLE_CONVERSION_MASK | BH1750_HRES_MODE2)     // One Time H-Resolution Mode2

#define BH1750_SING_L_RES_MODE_OPCODE           \
        (BH1750_SINGLE_CONVERSION_MASK | BH1750_LRES_MODE)      // One Time L-Resolution Mode

#define BH1750_CHANGE_MEAS_TIMEH_OPCODE         0x40    // Change measurement time (high bits) [7:5]
#define BH1750_CHANGE_MEAS_TIMEL_OPCODE         0xC0    // Change measurement time (low bits)  [4:0]

#define BH1750_SAMPLE_SIZE                      2

/*
 * ENUMERATION, DATA TYPE AND STRUCTURE DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief BH1750 measurement modes
 */
typedef enum {
        BH1750_NO_MODE,
        BH1750_HIGH_RES_CONT_MODE = BH1750_CONT_H_RES_MODE_OPCODE,
        BH1750_HIGH_RES2_CONT_MODE = BH1750_CONT_H_RES_MODE2_OPCODE,
        BH1750_LOW_RES_CONT_MODE = BH1750_CONT_L_RES_MODE_OPCODE,
        BH1750_HIGH_RES_SINGLE_MODE = BH1750_SING_H_RES_MODE2_OPCODE,
        BH1750_HIGH_RES2_SINGLE_MODE = BH1750_SING_H_RES_MODE2_OPCODE,
        BH1750_LOW_RES_SINGLE_MODE = BH1750_SING_L_RES_MODE_OPCODE,
} bh1750_meas_mode_t;

/*
 * FUNCTION DECLARATIONS
 *****************************************************************************************
 */

/**
 * \brief Initialize BH1750 and set measurement mode
 *
 * \param [in] dev      the device handle
 * \param [in] mode     the measurement mode
 */
void bh1750_init_mode(ad_i2c_handle_t dev, bh1750_meas_mode_t mode);

/**
 * \brief Trigger the BH1750 to measure and get the last measurement
 *
 * \param [in] dev      the device handle
 *
 * \return the last measurement acquired from BH1750
 */
uint16_t bh1750_trigger_measurement(ad_i2c_handle_t dev);

/**
 * \brief Convert a BH1750 raw measurement to actual lx measurement
 *
 * \param [in] meas     the raw measurement
 *
 * \return uint32_t the ambient light measurement in lx
 */
uint32_t bh1750_raw_meas_to_lx(uint16_t rawmeas);

#endif /* BH1750_H_ */
