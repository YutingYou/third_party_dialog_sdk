/**
 * \addtogroup MID_SYS_SERVICES
 * \{
 * \addtogroup SYS_ADC Analog Digital Converter Service
 *
 * \brief Functions for ADC Service
 *
 * \{
 */
/**
 ****************************************************************************************
 *
 * @file sys_adc.h
 *
 * @brief sys_adc header file.
 *
 * Copyright (C) 2018-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if (dg_configUSE_SYS_ADC == 1)

#ifndef SYS_ADC_H_
#define SYS_ADC_H_

#include "ad_gpadc.h"
#include "ad_snc.h"
#include "snc_hw_sys.h"
///@cond INTERNAL
#include "../adapters/src/sys_platform_devices_internal.h"
///@endcond

/**
 * \brief Monitor parameters configuration
 *
 */
typedef struct {
        uint16_t        drift;          /**<  the drift to be monitored */
        uint16_t        pollIntervalms; /**<  time period for the GPADC uCode-Block execution */
} sys_adc_monitor_par_t;

/**
 * \brief Function that configures the GPADC device, measures the current value and
 *      calculates the initial upper and lower bounds
 *
 * \param [in] conf            the GPADC device controller to be configured
 * \param [in] _gpadc_cb        a callback function that should be called
 *                              whenever the GPADC uCode-Block notifies CM33
 * \param [in] monitor_par      pointer to monitor par struct
 */
void sys_adc_config(const ad_gpadc_controller_conf_t * conf, ad_snc_interrupt_cb _gpadc_cb, sys_adc_monitor_par_t* monitor_par);

/**
 * \brief Function that enables the GPADC uCode-Block
 *
 */
void sys_adc_enable();

/**
 * \brief Function that disables the GPADC uCode-Block
 *
 */
void sys_adc_disable();

/**
 * \brief Function that returns the GPADC measurement
 *
 * \return uint32_t     the GPADC raw value
 *
 */
uint32_t sys_adc_get_value();

/**
 * \brief Function that sets the upper, lower bounds
 *
 * \param [in] value    the value based on which the lower and upper bounds
 *                      are defined
 */
void sys_adc_set_bounds(uint16_t value);

/**
 * \brief uCode-Block that reads measurements from the on-chip radio temperature sensor and notifies M33
 *
 */
SNC_UCODE_BLOCK_DECL(ucode_gpadc_rf_temp_notify);


#endif /* SYS_ADC_H_ */

#endif /* (dg_configUSE_SYS_ADC == 1) */

/**
 \}
 \}
 */
