/**
 ****************************************************************************************
 *
 * @file bh1750_ucodes.h
 *
 * @brief SNC Definitions of I2C BH1750 ambient light sensor demo application
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BH1750_UCODES_H_
#define BH1750_UCODES_H_

#include "SeNIS.h"

/**
 * \brief Ambient Light Status that results in a CM33 notification
 *
 */
typedef enum {
        NOTIFY_STATUS_NORMAL = 0,
        NOTIFY_STATUS_HIGH   = 1,
        NOTIFY_STATUS_LOW    = 2,
        NOTIFY_STATUS_NONE   = 3
} NOTIFY_STATUS;

#define UCODE_BH1750_MAX_NUM_OF_SAMPLES        16

/**
 * \brief uCode-Block that reads measurements from the BH1750 light sensor
 */
SNC_UCODE_BLOCK_DECL(ucode_bh1750_ambient_light_collect_samples);

/**
 * \brief uCode-Block that reads measurements from the BH1750 light sensor using SNC-queue
 */
SNC_UCODE_BLOCK_DECL(ucode_bh1750_ambient_light_collect_samples_queue);

/**
 * \brief uCode-Block that reads measurements from the BH1750 light sensor , determines the light
 *        state and sends notifications when a state transition has occurred
 */
SNC_UCODE_BLOCK_DECL(ucode_bh1750_ambient_light_notifier);

/**
 * \brief Getter function that returns the last updated local FIFO of the BH1750
 *        collected ambient raw light samples
 *
 * \param [in] fifo_data        pointer to the buffer where the acquired BH1750 data are stored
 */
void ucode_bh1750_get_fifo_samples(uint32_t *fifo_data);

/**
 * \brief Function that returns the last updated local FIFO of the BH1750,
 *        where the collected ambient raw light samples are stored
 *
 * \return uint32_t the number of the local FIFO samples
 */
uint32_t ucode_bh1750_get_fifo_num_of_samples(void);

/**
 * \brief Initialization function for the BH1750 uCode-Block
 */
void ucode_bh1750_init(void);

/**
 * \brief Set  high threshold and high tolerance values
 *
 * \param [in] high_threshold   Threshold value for HIGH_LIGHT state
 * \param [in] tolerance        Tolerance value used to avoid constant state switching
 */
void ucode_bh1750_set_threshold_high(uint16_t high_threshold, uint16_t tolerance);

/**
 * \brief Set  low threshold and low tolerance values
 *
 * \param [in] low_threshold    Threshold value for LOW_LIGHT state
 * \param [in] tolerance        Tolerance value used to avoid constant state switching
 */
void ucode_bh1750_set_threshold_low(uint16_t low_threshold, uint16_t tolerance);

/**
 * \brief Get current ambient light state
 *
 * \param [out] bh1750_sample_value     Light value triggered last state switching
 *
 * \return uint32_t                     Ambient light state
 */
uint32_t ucode_bh1750_get_lux_state(uint32_t * bh1750_sample_value);

/**
 * \brief Set number of samples to collect before notifying CM33
 *
 * \param [in] notify_samples   Notifying samples number
 */
void ucode_bh1750_set_notify_samples(uint32_t notify_samples);

/**
 * \brief Get ambient light value that triggered the last state switching
 *
 * \return uint32_t     Ambient light value (in lux)
 */
uint32_t ucode_bh1750_get_last_notif_value();

/**
 * \brief Get current ambient light state
 *
 * \return uint32_t     Ambient light state
 */
uint32_t ucode_bh1750_get_notif_state();

/**
 * \brief Get last measured ambient light value
 *
 * \return uint32_t     Most recent data sample of sensor (measured in lux)
 */
uint32_t ucode_bh1750_get_last_sensor_value();

#endif /* BH1750_UCODES_H_ */
