/**
 ****************************************************************************************
 *
 * @file sdk_crc16.h
 *
 * @brief CRC16 calculation library API
 *
 * Copyright (C) 2015-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SDK_CRC16_
#define SDK_CRC16_

#include <stddef.h>
#include <stdint.h>

/*
 * \brief Initialize CRC16
 *
 * \param [in] crc16 pointer to CRC16 value
 *
 */
void crc16_init(uint16_t *crc16);

/*
 * \brief Updates CRC16
 *
 * \param [in] crc16 pointer to CRC16 value
 * \param [in] buf data buffer
 * \param [in] len length of data
 *
 */
void crc16_update(uint16_t *crc16, const uint8_t *buf, size_t len);

/*
 * \brief Calculate CRC16
 *
 * \param [in] buf data buffer
 * \param [in] len length of data
 *
 * \return CRC16 value
 *
 */
uint16_t crc16_calculate(const uint8_t *buf, size_t len);

#endif /* SDK_CRC16_ */
