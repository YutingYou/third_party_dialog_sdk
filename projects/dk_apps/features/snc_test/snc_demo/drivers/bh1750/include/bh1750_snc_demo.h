/**
 ****************************************************************************************
 *
 * @file bh1750_snc_demo.h
 *
 * @brief BH1750 - SNC Demo functions header file
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BH1750_SNC_DEMO_H_
#define BH1750_SNC_DEMO_H_

#include "ad_snc.h"

/**
 * \brief Enumeration of light states
 * */
typedef enum {
        AMBIENT_LIGHT_STATE_NORMAL =    0,
        AMBIENT_LIGHT_STATE_HIGH =      1,
        AMBIENT_LIGHT_STATE_LOW =       2
} AMBIENT_LIGHT_STATE;

/**
 * \brief SNC/BH1750 demo application initializations and uCode-Block registration
 *
 * \param [in] _bh1750_read_cb          a callback function that should be called
 *                                      whenever the BH1750 uCode-Block notifies CM33
 * \param [in] mode                     sensor demo operation mode. Choose between :
 *                                      0: data collect, 1: data collect with queue, 2: notifier
 * \param [in] notify_samples           Number of sensor samples acquired by Sensor Node before
 *                                      notifying CM33
 *
 * \return uint32_t the uCode ID of the created uCode-Block
 */
uint32_t snc_demo_bh1750_init(ad_snc_interrupt_cb _bh1750_read_cb, uint8_t mode, uint32_t notify_samples);

/**
 * \brief Set  high threshold and high tolerance values
 *
 * \param [in] high_threshold   Threshold value for HIGH_LIGHT state
 * \param [in] tolerance        Tolerance value used to avoid constant state switching
 */
void bh1750_set_threshold_high(uint16_t high_threshold, uint16_t tolerance);

/**
 * \brief Set  low threshold and low tolerance values
 *
 * \param [in] low_threshold    Threshold value for LOW_LIGHT state
 * \param [in] tolerance        Tolerance value used to avoid constant state switching
 */
void bh1750_set_threshold_low(uint16_t low_threshold, uint16_t tolerance);

/**
 * \brief Set number of samples to collect before notifying CM33
 *
 * \param [in] notify_samples    Notifying samples number
 */
void bh1750_set_notify_samples(uint32_t notify_samples);

/**
 * \brief Get current ambient light state
 *
 * \param [out] notif_light_value       Light value triggered last state switching
 *
 * \return uint32_t                     Ambient light state
 */
uint32_t bh1750_get_ambient_light_state(uint32_t *notif_light_value);

/**
 * \brief Get last measured ambient light value
 *
 * \return uint32_t     Most recent data sample of sensor (measured in lux)
 */
uint32_t bh1750_get_ambient_light_last_value();

#endif /* BH1750_SNC_DEMO_H_ */
