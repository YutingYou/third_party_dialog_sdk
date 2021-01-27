/**
 ****************************************************************************************
 *
 * @file 24fc256_ucodes.h
 *
 * @brief SNC-Definition of I2C 24FC256 EEPROM demo application
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef _24FC256_UCODES_H_
#define _24FC256_UCODES_H_

#include "SeNIS.h"

/**
 * \brief uCode-Block used to write data to the I2C EEPROM
 */
SNC_UCODE_BLOCK_DECL(ucode_write_eeprom_test_data);

/**
 * \brief uCode-Block used to read data from the I2C EEPROM triggered by TIMER1 interrupts
 */
SNC_UCODE_BLOCK_DECL(ucode_read_eeprom_on_rtc);

#endif /* _24FC256_UCODES_H_ */
