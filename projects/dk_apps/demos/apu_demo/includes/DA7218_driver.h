/**
 ****************************************************************************************
 *
 * @file DA7218_driver.h
 *
 * @brief Audio codec driver header file.
 *
 * Copyright (C) 2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef INCLUDES_DA7218_DRIVER_H_
#define INCLUDES_DA7218_DRIVER_H_

#define DA7218_MAX_GAIN      (9)

void DA7218_Init(void);
void DA7218_Enable(void);
void DA7218_Disable(void);

extern void DA7218_SetGain(int iLevel);
extern int DA7218_GetGain(void);

#endif /* INCLUDES_DA7218_DRIVER_H_ */
