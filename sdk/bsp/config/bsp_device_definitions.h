/**
 * \addtogroup BSP_CONFIG_DEFINITIONS
 * \{
 * \addtogroup BSP_CFG_DEF_DEVICE_MAP Device-Map Definitions
 *
 * \brief Device-Map Definitions. Macros for all the devices supported by SDK10.
 *
 *\{
 */
/**
 ****************************************************************************************
 *
 * @file bsp_device_definitions.h
 *
 * @brief Board Support Package. Device-Map definitions.
 *
 * Copyright (C) 2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BSP_DEVICE_DEFINITIONS_H_
#define BSP_DEVICE_DEFINITIONS_H_


#include "bsp_device_definitions_internal.h"

/*
 * Available public macros characterizing each product supported by SDK10.
 * The variable dg_configDEVICE *MUST* take one of the following values
 * i.e. dg_configDEVICE=DA14680_01
 * The variable *MUST* be visible to both the compiler and the assembler.
 */
#define DA14680_01                      (DA14680 | _DEVICE_MK_IC_VER(A, E))
#define DA14681_01                      (DA14681 | _DEVICE_MK_IC_VER(A, E))
#define DA14682_00                      (DA14682 | _DEVICE_MK_IC_VER(B, B))
#define DA14683_00                      (DA14683 | _DEVICE_MK_IC_VER(B, B))

#define DA14691_00                      (DA14691 | _DEVICE_MK_IC_VER(A, B))
#define DA14693_00                      (DA14693 | _DEVICE_MK_IC_VER(A, B))
#define DA14695_00                      (DA14695 | _DEVICE_MK_IC_VER(A, B))
#define DA14697_00                      (DA14697 | _DEVICE_MK_IC_VER(A, B))
#define DA14699_00                      (DA14699 | _DEVICE_MK_IC_VER(A, B))

/*
 * Backward compatibility macros.
 * Useful for applications developed with older SDK versions.
 * DEVICE_DA146XX can be assigned the desired device.
 */
/* DA14680 family substitution (with the exception of uartboot loader) */
#define DEVICE_DA14680                  DA14683_00
/* DA14690 family substitution */
#define DEVICE_DA1469x                  DA14699_00


#endif /* BSP_DEVICE_DEFINITIONS_H_ */
/**
\}
\}
*/
