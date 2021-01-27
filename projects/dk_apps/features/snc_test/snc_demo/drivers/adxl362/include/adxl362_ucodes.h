/**
 ****************************************************************************************
 *
 * @file adxl362_ucodes.h
 *
 * @brief SNC-uCode reading ADXL362 FIFO data based on INT1 interrupt header file
 *
 * Copyright (C) 2017-2018 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef ADXL362_UCODES_H_
#define ADXL362_UCODES_H_

#include "SeNIS.h"

#define ADXL362_FIFO_WM_LEVEL   32

/**
 * \brief An SNC uCode-Block triggered whenever the INT1 of the ADXL362 sensor occurs
 *
 * It will check how many samples are residing in the ADXL362 FIFO and read them to a local buffer
 */
SNC_UCODE_BLOCK_DECL(ucode_gpio_adxl362_int1_queue_test);

#endif /* ADXL362_UCODES_H_ */
