/**
 * \addtogroup HAPTICS_ALGORITHM_LIBRARY
 * \{
 * \addtogroup SMARTDRIVE SmartDrive
 *
 * \brief SmartDrive Algorithm
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file smart_drive.h
 *
 * @brief SmartDrive Algorithm
 *
 * Copyright (C) 2019-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SMART_DRIVE_H
#define SMART_DRIVE_H


#include <stdbool.h>
#include <stdint.h>


/**
 * SmartDrive algorithm control flags structure
 */
typedef struct smart_drive_flags_str {
    bool apply : 1;
    bool update : 1;
    bool rsvd_2 : 1;
    bool rsvd_3 : 1;
    bool rsvd_4 : 1;
    bool rsvd_5 : 1;
    bool rsvd_6 : 1;
    bool rsvd_7 : 1;
    bool amplitude_valid : 1;
    bool rsvd_9 : 1;
    bool rsvd_10 : 1;
    bool rsvd_11 : 1;
    bool rsvd_12 : 1;
    bool rsvd_13 : 1;
    bool rsvd_14 : 1;
    bool rsvd_15 : 1;
} smart_drive_flags;


/**
 * SmartDrive algorithm parameter structure
 */
typedef struct smart_drive_params_str {

    /* Configuration parameters (initialisation required) */
    smart_drive_flags flags;
    uint16_t peak_amplitude;
    uint16_t tau;
    uint16_t lambda;
    uint16_t delta;

    /* State parameters */
    uint16_t alpha;
    uint16_t beta;
    uint16_t prev_amplitude;
    int16_t prev_pred_amplitude;
    int16_t drive_level;
    int16_t prev_drive_level;
    uint16_t _padding;
    float P[2][2];

} smart_drive_params;


/**
 * Function initialises SmartDrive algorithm.
 *
 * @param  params          Pointer to the SmartDrive parameter structure
 */
void smart_drive_init(smart_drive_params *params);


/**
 * Function performs SmartDrive algorithm processing.
 *
 * @param  params          Pointer to the SmartDrive parameter structure
 * @param  target_level    Target drive level
 * @param  amplitude       Amplitude of actuator back-EMF signal
 * @param  drive_level     Current drive level
 *
 * @return                 Updated drive level (to be used to configure
 *                         hardware)
 */
int16_t smart_drive_process(smart_drive_params *params, uint16_t target_level, uint16_t amplitude, int16_t drive_level);

#endif /* SMART_DRIVE_H */

/**
 * \}
 * \}
 */
