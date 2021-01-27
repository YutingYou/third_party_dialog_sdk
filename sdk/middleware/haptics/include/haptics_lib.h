/**
 * \addtogroup HAPTICS_ALGORITHM_LIBRARY
 * \{
 * \addtogroup HAPTICS_LIB Haptics Algorithm Library Wrapper
 *
 * \brief Wrapper module for the Curve Fitter, SmartDrive, and Automatic Frequency Control algorithms
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file haptics_lib.h
 *
 * @brief Haptics Algorithm Library Wrapper
 *
 * Copyright (C) 2019-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HAPTICS_LIB_H
#define HAPTICS_LIB_H


#include <stdbool.h>
#include "curve_fitting.h"
#include "lra_frequency_control.h"
#include "smart_drive.h"


/**
 * Haptics Algorithm Library Wrapper control/state flags structure
 */
typedef struct haptics_lib_flags_str {

    /* Control flags */
    bool curve_fitter_enabled : 1;
    bool lra_afc_enabled : 1;
    bool smart_drive_enabled : 1;
    bool start_polarity : 1;
    bool rsvd_4 : 1;
    bool rsvd_5 : 1;
    bool rsvd_6 : 1;
    bool rsvd_7 : 1;

    /* State flags */
    bool rsvd_8 : 1;
    bool rsvd_9 : 1;
    bool rsvd_10 : 1;
    bool rsvd_11 : 1;
    bool rsvd_12 : 1;
    bool rsvd_13 : 1;
    bool rsvd_14 : 1;
    bool braking : 1;

} haptics_lib_flags;


/**
 * Haptics Algorithm Library Wrapper parameter structure
 */
typedef struct haptics_lib_params_str {
    haptics_lib_flags flags;
    int16_t i_data_threshold;
    curve_fitter_params curve_fitter;
    lra_afc_params lra_afc;
    smart_drive_params smart_drive;
} haptics_lib_params;


/**
 * Haptics Algorithm Library Wrapper hardware interface state structure
 */
typedef struct haptics_lib_if_state_str {
    bool drive_polarity : 1;
    bool drive_polarity_inverted : 1;
    bool i_data_valid : 1;
    bool rsvd_3 : 1;
    bool rsvd_4 : 1;
    bool rsvd_5 : 1;
    bool rsvd_6 : 1;
    bool rsvd_7 : 1;
    bool rsvd_8 : 1;
    bool rsvd_9 : 1;
    bool rsvd_10 : 1;
    bool rsvd_11 : 1;
    bool rsvd_12 : 1;
    bool rsvd_13 : 1;
    bool rsvd_14 : 1;
    bool rsvd_15 : 1;
} haptics_lib_if_state;


/**
 * Function initializes the Haptics Algorithm Library.
 *
 * @param  params        Pointer to the haptics algorithm library parameter
 *                       structure
 */
void haptics_lib_init(haptics_lib_params *params);


/**
 * Function performs Haptics Algorithm Library processing.
 *
 * @param  params        Pointer to the haptics algorithm library parameter
 *                       structure
 * @param  i_data        Pointer to array of sampled i-sense data from
 *                       previous drive cycle
 * @param  half_period   Half period of previous drive cycle
 * @param  drive_level   Drive level of previous drive cycle
 * @param  state         State of hardware interface
 * @param  target_level  Target drive level
 */
void haptics_lib_process(haptics_lib_params *params, int16_t *i_data, uint16_t *half_period, uint16_t *drive_level, haptics_lib_if_state *state, uint16_t target_level);

#endif /* HAPTICS_LIB_H */

/**
 * \}
 * \}
 */
