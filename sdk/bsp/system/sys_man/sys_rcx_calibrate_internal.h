/**
 ****************************************************************************************
 *
 * @file sys_rcx_calibrate_internal.h
 *
 * @brief RCX calibration internal header file
 *
 * Copyright (C) 2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SYS_RCX_CALIBRATE_INTERNAL_H_
#define SYS_RCX_CALIBRATE_INTERNAL_H_

#if (dg_configUSE_LP_CLK == LP_CLK_RCX && defined(OS_FREERTOS))

#include "ad_gpadc.h"
#include "ad_snc.h"
#include "snc_hw_sys.h"

/* ~1.7 msec for any subsequent calibration. */
#define RCX_CALIBRATION_CYCLES_WUP      25

#define VBAT_VOLTAGE_DRIFT      800 // in mV
#define BANGAP_TEMP_DRIFT       1 // in C
#define RCX_CAL_POLL_INT        1000 // in msec
#define UNCOND_CAL_TIME_IN_SEC  60 // Time in sec for triggering unconditionally RCX calibration

/**
 * \brief Function that configures and registers the RCX uCode, measures the current value of both
 *        bandgap temperature sensor and Vbat and calculates the initial upper and lower bounds.
 *
 * \param [in] _rcx_calibrate_cb        a callback function that should be called
 *                                      whenever the RCX uCode-Block notifies CM33
 */
void sys_rcx_calibrate_config(ad_snc_interrupt_cb _rcx_calibrate_cb);

/**
 * \brief Function that returns the GPADC measurements
 *
 * \param [out] vbat    the GPADC raw value for the Vbat measurement
 *
 * \return uint16_t     the GPADC raw value for the bandgap temperature sensor measurement
 */
uint16_t sys_rcx_calibrate_get_value(uint16_t *vbat);

/**
 * \brief Function that sets the upper, lower bounds for both bandgap and Vbat
 *
 * \param [in] bandgap  the value for bandgap based on which the lower and upper bounds
 *                      are defined
 * \param [in] vbat     the value for Vbat based on which the lower and upper bounds
 *                      are defined
 */
void sys_rcx_calibrate_set_bounds(uint16_t bandgap, uint16_t vbat);

/**
 * \brief uCode-Block that reads measurements from both bandgap and Vbat. If either measurement
 *        exceeds the corresponding boundaries SNC starts RCX calibration and notifies M33.
 *
 */
SNC_UCODE_BLOCK_DECL(ucode_rcx_calibrate_notify);

#endif /* (dg_configUSE_LP_CLK == LP_CLK_RCX && defined(OS_FREERTOS))*/
#endif /* SYS_RCX_CALIBRATE_INTERNAL_H_ */
