/**
 ****************************************************************************************
 *
 * @file co_version.h
 *
 * @brief Version definitions for BT4.0
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 * Copyright (c) 2015-2018 Modified by Dialog Semiconductor
 *
 *
 ****************************************************************************************
 */


#ifndef _DA14690_CO_VERSION_H_
#define _DA14690_CO_VERSION_H_
/**
 ****************************************************************************************
 * @defgroup CO_VERSION Version Defines
 * @ingroup COMMON
 *
 * @brief Bluetooth Controller Version definitions.
 *
 * @{
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */

/// RWBT SW Major Version
#define RWBLE_SW_VERSION_MAJOR                   11

/// RWBT SW Minor Version
#define RWBLE_SW_VERSION_MINOR                   0

/// RWBT SW Build Version
#define RWBLE_SW_VERSION_BUILD                   1

/// RWBT SW Major Version
#define RWBLE_SW_VERSION_SUB_BUILD               0


/// Combine major and minor version numbers into a single descriptor
/// Assumes that minor numbers never exceed 100
#define RWBLE_SW_VERSION (RWBLE_SW_VERSION_MAJOR * 100 + RWBLE_SW_VERSION_MINOR)

/// Macro that produces version literals as (major, minor) pairs
#define MK_VERSION(maj, min) ((maj) * 100 + (min))

#define VERSION_8_0 MK_VERSION(8, 0)
#define VERSION_8_1 MK_VERSION(8, 1)
#define VERSION_9_0 MK_VERSION(9, 0)
#define VERSION_10_0 MK_VERSION(10, 0)

/// @} CO_VERSION


#endif // _DA14690_CO_VERSION_H_

