/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SNC_HW_GPADC
 *
 * \brief GPADC LLD for SNC context
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file snc_hw_gpadc.h
 *
 * @brief SNC-Definition of GPADC Low Level Driver API
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SNC_HW_GPADC_H_
#define SNC_HW_GPADC_H_


#if dg_configUSE_HW_SENSOR_NODE

#if dg_configUSE_SNC_HW_GPADC

#include "snc_defs.h"
#include "ad_gpadc.h"
#include "hw_gpadc.h"

/*
 * ENUMERATION, DATA TYPE AND STRUCTURE DEFINITIONS
 *****************************************************************************************
 */
typedef ad_gpadc_controller_conf_t snc_gpadc_config;

/*
 * MACRO DEPENDENCIES
 *****************************************************************************************
 */

#include "snc_hw_gpadc_macros.h"

/*
 * FUNCTION DECLARATIONS
 *****************************************************************************************
 */

//==================== Peripheral Acquisition functions ========================

/**
 * \brief Function used in SNC context to initialize and acquire the GPADC peripheral
 *        depending on a GPADC configuration setup
 *
 * \param [in] conf             (const snc_gpadc_config* : build-time-only value)
 *                              handle of GPADC source
 *
 */
#define SNC_gpadc_open(conf)                                                                   \
        _SNC_gpadc_open(conf)

/**
 * \brief Function used in SNC context to de-initialize and release the GPADC peripheral
 *        depending on a GPADC configuration setup
 *
 * \param [in] conf             (const snc_gpadc_config* : build-time-only value)
 *                              handle of GPADC source
 *
 */
#define SNC_gpadc_close(conf)                                                                  \
        _SNC_gpadc_close(conf)

//==================== Peripheral Access functions =============================

/**
 * \brief Function used in SNC context to read the raw value of the measurement from GPADC
 *
 * \param [in] conf             (const snc_gpadc_config* : build-time-only value)
 *                              handle of GPADC source
 * \param [out] value           (uint32_t*: use da() or ia())
 *                              pointer to the variable where the GPADC value will be returned
 *
 */
#define SNC_gpadc_read(conf, value)                                                            \
        _SNC_gpadc_read(conf, value)

#endif /* dg_configUSE_SNC_HW_GPADC */

#endif /* dg_configUSE_HW_SENSOR_NODE */


#endif /* SNC_HW_GPADC_H_ */

/**
 * \}
 * \}
 */
