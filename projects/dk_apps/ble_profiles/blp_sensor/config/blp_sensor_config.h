/**
 ****************************************************************************************
 *
 * @file blp_sensor_config.h
 *
 * @brief Application configuration
 *
 * Copyright (C) 2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BLP_SENSOR_CONFIG_H_
#define BLP_SENSOR_CONFIG_H_

/* Interval between next intermediate blood cuff pressure measurement in [ms] */
#define CFG_INTER_CUFF_TIME_INTERVAL_MS                 (1000)

/* Max number of stored measurements */
#define CFG_MAX_NUMBER_OF_MEASUREMENTS                  (25)

#endif /* BLP_SENSOR_CONFIG_H_ */
