/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SNC_HW_I2C
 *
 * \brief I2C LLD macros for SNC context
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file snc_hw_i2c_macros.h
 *
 * @brief SNC definitions of I2C Low Level Driver Macros
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SNC_HW_I2C_MACROS_H_
#define SNC_HW_I2C_MACROS_H_


#if dg_configUSE_HW_SENSOR_NODE

#if dg_configUSE_SNC_HW_I2C

//==================== Controller Acquisition functions ========================

void snc_i2c_open(b_ctx_t* b_ctx, const snc_i2c_controller_conf_t* conf);
#define _SNC_i2c_open(conf)                                                                     \
        SNC_STEP_BY_STEP();                                                                     \
        snc_i2c_open(b_ctx, _SNC_OP_VALUE(const snc_i2c_controller_conf_t*, conf))

void snc_i2c_close(b_ctx_t* b_ctx, const snc_i2c_controller_conf_t* conf);
#define _SNC_i2c_close(conf)                                                                    \
        SNC_STEP_BY_STEP();                                                                     \
        snc_i2c_close(b_ctx, _SNC_OP_VALUE(const snc_i2c_controller_conf_t*, conf))

//==================== Controller Configuration functions ======================

void snc_i2c_reconfig(b_ctx_t* b_ctx, const snc_i2c_controller_conf_t* conf);
#define _SNC_i2c_reconfig(conf)                                                                 \
        SNC_STEP_BY_STEP();                                                                     \
        snc_i2c_reconfig(b_ctx, _SNC_OP_VALUE(const snc_i2c_controller_conf_t*, conf))

//==================== Status Acquisition functions ============================

void snc_i2c_poll_for_ack(b_ctx_t* b_ctx, const snc_i2c_controller_conf_t* conf);
#define _SNC_i2c_poll_for_ack(conf)                                                             \
        SNC_STEP_BY_STEP();                                                                     \
        snc_i2c_poll_for_ack(b_ctx, _SNC_OP_VALUE(const snc_i2c_controller_conf_t*, conf))

void snc_i2c_check_abort_source(b_ctx_t* b_ctx, const snc_i2c_controller_conf_t* conf,
        SENIS_OPER_TYPE abort_source_type, uint32_t* abort_source);
#define _SNC_i2c_check_abort_source(conf, abort_source)                                         \
        SNC_STEP_BY_STEP();                                                                     \
        snc_i2c_check_abort_source(b_ctx, _SNC_OP_VALUE(const snc_i2c_controller_conf_t*, conf), \
                                          _SNC_OP_ADDRESS(abort_source))

//==================== Data Read/Write functions ===============================

void snc_i2c_write(b_ctx_t* b_ctx, const snc_i2c_controller_conf_t* conf,
                                   SENIS_OPER_TYPE out_buf_type, uint32_t* out_buf,
                                   SENIS_OPER_TYPE out_len_type, uint32_t* out_len,
                                   uint32_t flags);
#define _SNC_i2c_write(conf, out_buf, out_len, flags)                                           \
        SNC_STEP_BY_STEP();                                                                     \
        snc_i2c_write(b_ctx, _SNC_OP_VALUE(const snc_i2c_controller_conf_t*, conf),             \
                             _SNC_OP_ADDRESS(out_buf), _SNC_OP(out_len),                        \
                             _SNC_OP_VALUE(uint32_t, flags))

void snc_i2c_read(b_ctx_t* b_ctx, const snc_i2c_controller_conf_t* conf,
                                  SENIS_OPER_TYPE in_buf_type, uint32_t* in_buf,
                                  SENIS_OPER_TYPE in_len_type, uint32_t* in_len,
                                  uint32_t flags);
#define _SNC_i2c_read(conf, in_buf, in_len, flags)                                              \
        SNC_STEP_BY_STEP();                                                                     \
        snc_i2c_read(b_ctx, _SNC_OP_VALUE(const snc_i2c_controller_conf_t*, conf),              \
                            _SNC_OP_ADDRESS(in_buf), _SNC_OP(in_len),                           \
                            _SNC_OP_VALUE(uint32_t, flags))

#endif /* dg_configUSE_SNC_HW_I2C */

#endif /* dg_configUSE_HW_SENSOR_NODE */


#endif /* SNC_HW_I2C_MACROS_H_ */

/**
 * \}
 * \}
 */
