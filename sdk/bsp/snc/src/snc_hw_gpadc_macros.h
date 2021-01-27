/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SNC_HW_GPADC
 *
 * \brief GPADC LLD macros for SNC context
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file snc_hw_gpadc_macros.h
 *
 * @brief SNC definitions of GPADC Low Level Driver Macros
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SNC_HW_GPADC_MACROS_H_
#define SNC_HW_GPADC_MACROS_H_


#if dg_configUSE_HW_SENSOR_NODE

#if dg_configUSE_SNC_HW_GPADC

//==================== Peripheral Acquisition functions ========================

void snc_gpadc_open(b_ctx_t* b_ctx, const snc_gpadc_config* conf);
#define _SNC_gpadc_open(conf)                                                                   \
        SNC_STEP_BY_STEP();                                                                     \
        snc_gpadc_open(b_ctx, _SNC_OP_VALUE(const snc_gpadc_config*, conf))

void snc_gpadc_close(b_ctx_t* b_ctx);
#define _SNC_gpadc_close(conf)                                                                  \
        SNC_STEP_BY_STEP();                                                                     \
        snc_gpadc_close(b_ctx)

//==================== Peripheral Access functions =============================

void snc_gpadc_read(b_ctx_t* b_ctx, SENIS_OPER_TYPE value_type, uint32_t* value);
#define _SNC_gpadc_read(conf, value)                                                            \
        SNC_STEP_BY_STEP();                                                                     \
        snc_gpadc_read(b_ctx, _SNC_OP_ADDRESS(value))

#endif /* dg_configUSE_SNC_HW_GPADC */

#endif /* dg_configUSE_HW_SENSOR_NODE */


#endif /* SNC_HW_GPADC_MACROS_H_ */

/**
 * \}
 * \}
 */
