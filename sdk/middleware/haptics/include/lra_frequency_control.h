/**
 * \addtogroup HAPTICS_ALGORITHM_LIBRARY
 * \{
 * \addtogroup AFC Automatic Frequency Control
 *
 * \brief LRA Automatic Frequency Control Algorithm
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file lra_frequency_control.h
 *
 * @brief LRA Automatic Frequency Controller Algorithm
 *
 * Copyright (C) 2019-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef LRA_FREQUENCY_CONTROL_H
#define LRA_FREQUENCY_CONTROL_H


#include <stdint.h>


/**
 * LRA Automatic Frequency Control algorithm parameter structure
 */
typedef struct lra_afc_params_str {
    uint16_t half_period;      /**< Current half period (updated by algorithm) */
    uint16_t half_period_min;  /**< Minimum half period */
    uint16_t half_period_max;  /**< Maximum half period */
    uint16_t zeta;             /**< Damping factor */
} lra_afc_params;


/**
 * Function performs LRA Automatic Frequency Control algorithm processing.
 *
 * @param  params        Pointer to the LRA automatic frequency controller
 *                       parameter structure
 * @param  half_period   Half period of previous drive cycle
 * @param  phase_offset  Phase offset of sampled i-sense data from previous
 *                       drive cycle
 *
 * @return               Updated half period (to be used to configure hardware)
 */
uint16_t lra_afc_process(lra_afc_params *params, uint16_t half_period, int16_t phase_offset);

#endif /* LRA_FREQUENCY_CONTROL_H */

/**
 * \}
 * \}
 */
