/**
 * \addtogroup HAPTICS_ALGORITHM_LIBRARY
 * \{
 * \addtogroup CURVE_FITTER Curve Fitter
 *
 * \brief Curve Fitting Algorithm
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file curve_fitting.h
 *
 * @brief Curve Fitting Algorithm
 *
 * Copyright (C) 2019-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef CURVE_FITTING_H
#define CURVE_FITTING_H


#include <stdint.h>


/**
 * 2D vector structure
 */
typedef struct {
    int16_t x;
    int16_t y;
} vect2d16_t;


/**
 * Curve Fitter parameter structure
 */
typedef struct curve_fitter_params_str {

    /* Configuration parameters (initialisation required) */
    uint16_t amplitude_threshold;

    /* Calculated parameters */
    int16_t dc_offset;
    uint16_t amplitude;
    int16_t phi;
    vect2d16_t phasor;

} curve_fitter_params;


/**
 * Function performs Curve Fitter algorithm processing.
 *
 * @param  params  Pointer to the Curve Fitter parameter structure
 * @param  i_data  Pointer to the sampled i-sense data
 */
void curve_fitter_process(curve_fitter_params *params, int16_t *i_data);

#endif /* CURVE_FITTING_H */

/**
 * \}
 * \}
 */
