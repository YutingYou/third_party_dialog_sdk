/**
 ****************************************************************************************
 *
 * @file adxl362_snc_demo.h
 *
 * @brief ADXL362 - SNC Demo functions header file
 *
 * Copyright (C) 2017-2018 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef ADX362_SNC_DEMO_H_
#define ADX362_SNC_DEMO_H_

#include "ad_snc.h"

/**
 * \brief Initialize the ADXL362/SNC demo application
 *
 * Configure the ADXL362 sensor and register a uCode-Block that will handle the ADXL362 interrupt
 *
 * \param [in] _adxl362_int1_cb         the callback that shall be called when the ADXL362
 *                                      uCode-Block notifies CM33
 *
 * \return uint32_t the uCode ID of the created uCode-Block
 */
uint32_t snc_demo_adxl362_init(ad_snc_interrupt_cb _adxl362_int1_cb);

#endif /* ADX362_SNC_DEMO_H_ */
