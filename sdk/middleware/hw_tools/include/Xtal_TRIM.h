/**
 ****************************************************************************************
 *
 * @file Xtal_TRIM.h
 *
 * @brief Xtal trim API
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef XTAL_TRIM_H_
#define XTAL_TRIM_H_

#define AUTO_XTAL_TEST_DBG_EN           0                       // Enable/Disable debug parameters.

int16_t auto_trim(HW_GPIO_PORT port, HW_GPIO_PIN pin);

void delay(uint32_t dd);
void Setting_Trim(uint32_t Trim_Value);

uint32_t pulse_counter(void);                                   // counting pulses during 500 msec
uint32_t MEASURE_PULSE(int32_t datareg1, int32_t shift_bit1);   // see assembly code

#if AUTO_XTAL_TEST_DBG_EN
void TRIM_test (int S1, int S2);                                // testing
#endif

#endif /* XTAL_TRIM_H_ */

