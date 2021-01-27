/**
 ****************************************************************************************
 *
 * @file 24fc256_snc_demo.h
 *
 * @brief I2C EEPROM - SNC Demo functions header file
 *
 * Copyright (C) 2017-2018 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef _24FC256_SNC_DEMO_H_
#define _24FC256_SNC_DEMO_H_

#include "ad_snc.h"

/**
 * \brief Demo SNC/I2C EEPROM writer application initializations and uCode registration
 *
 * \param [in] _eeprom_write_cb         a callback function that should be called whenever
 *                                      the EEPROM writer uCode notifies CM33
 *
 * \return uint32_t the uCode ID of the created uCode-Block
 */
uint32_t snc_demo_24fc256_writer_init(ad_snc_interrupt_cb _eeprom_write_cb);

/**
 * \brief Demo SNC/I2C EEPROM reader application initializations and uCode registration
 *
 * \param [in] _eeprom_read_cb          a callback function that should be called whenever
 *                                      the EEPROM reader uCode notifies CM33
 *
 * \return uint32_t the uCode ID of the created uCode-Block
 */
uint32_t snc_demo_24fc256_reader_init(ad_snc_interrupt_cb _eeprom_read_cb);

#endif /* _24FC256_SNC_DEMO_H_ */
