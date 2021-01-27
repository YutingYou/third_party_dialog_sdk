/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SNC_HW_UART
 *
 * \brief UART LLD macros for SNC context
 *
 * \{
 */

/****************************************************************************************
 *
 * @file snc_hw_uart_macros.h
 *
 * @brief SNC definitions of UART Low Level Driver Macros
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SNC_HW_UART_MACROS_H_
#define SNC_HW_UART_MACROS_H_


#if dg_configUSE_HW_SENSOR_NODE

#if dg_configUSE_SNC_HW_UART

//==================== Controller Acquisition functions ========================

void snc_uart_open(b_ctx_t* b_ctx, const snc_uart_controller_conf_t* conf);
#define _SNC_uart_open(conf)                                                                    \
        SNC_STEP_BY_STEP();                                                                     \
        snc_uart_open(b_ctx, _SNC_OP_VALUE(const snc_uart_controller_conf_t*, conf));

void snc_uart_close(b_ctx_t* b_ctx, const snc_uart_controller_conf_t* conf);
#define _SNC_uart_close(conf)                                                                   \
        SNC_STEP_BY_STEP();                                                                     \
        snc_uart_close(b_ctx, _SNC_OP_VALUE(const snc_uart_controller_conf_t*, conf));

//==================== Controller Configuration functions ======================

void snc_uart_reconfig(b_ctx_t* b_ctx, const snc_uart_controller_conf_t* conf);
#define _SNC_uart_reconfig(conf)                                                                \
        SNC_STEP_BY_STEP();                                                                     \
        snc_uart_reconfig(b_ctx, _SNC_OP_VALUE(const snc_uart_controller_conf_t*, conf));

//==================== Status Acquisition functions ============================

void snc_uart_is_write_pending(b_ctx_t* b_ctx, const snc_uart_controller_conf_t* conf,
                                               SENIS_OPER_TYPE pending_type, uint32_t* pending);
#define _SNC_uart_is_write_pending(conf, pending)                                               \
        SNC_STEP_BY_STEP();                                                                     \
        snc_uart_is_write_pending(b_ctx, _SNC_OP_VALUE(const snc_uart_controller_conf_t*, conf), \
                                         _SNC_OP_ADDRESS(pending))

void snc_uart_wait_write_pending(b_ctx_t* b_ctx, const snc_uart_controller_conf_t* conf);
#define _SNC_uart_wait_write_pending(conf)                                                      \
        SNC_STEP_BY_STEP();                                                                     \
        snc_uart_wait_write_pending(b_ctx, _SNC_OP_VALUE(const snc_uart_controller_conf_t*, conf))

//==================== Control functions =======================================

void snc_uart_abort_read(b_ctx_t* b_ctx, const snc_uart_controller_conf_t* conf);
#define _SNC_uart_abort_read(conf)                                                              \
        SNC_STEP_BY_STEP();                                                                     \
        snc_uart_abort_read(b_ctx, _SNC_OP_VALUE(const snc_uart_controller_conf_t*, conf))

void snc_uart_set_flow_on(b_ctx_t* b_ctx, const snc_uart_controller_conf_t* conf);
#define _SNC_uart_set_flow_on(conf)                                                             \
        SNC_STEP_BY_STEP();                                                                     \
        snc_uart_set_flow_on(b_ctx, _SNC_OP_VALUE(const snc_uart_controller_conf_t*, conf))

void snc_uart_set_flow_off(b_ctx_t* b_ctx, const snc_uart_controller_conf_t* conf,
        SENIS_OPER_TYPE chars_avail_type, uint32_t* chars_avail, bool forced);
#define _SNC_uart_set_flow_off(conf, chars_avail)                                               \
        SNC_STEP_BY_STEP();                                                                     \
        snc_uart_set_flow_off(b_ctx, _SNC_OP_VALUE(const snc_uart_controller_conf_t*, conf),    \
                                     _SNC_OP_ADDRESS(chars_avail), true)
#define _SNC_uart_try_flow_off(conf, chars_avail)                                               \
        SNC_STEP_BY_STEP();                                                                     \
        snc_uart_set_flow_off(b_ctx, _SNC_OP_VALUE(const snc_uart_controller_conf_t*, conf),    \
                                     _SNC_OP_ADDRESS(chars_avail), false)

//==================== Data Read/Write functions ===============================

void snc_uart_write(b_ctx_t* b_ctx, const snc_uart_controller_conf_t* conf,
                                    SENIS_OPER_TYPE out_buf_type, uint32_t* out_buf,
                                    SENIS_OPER_TYPE out_len_type, uint32_t* out_len);
#define _SNC_uart_write(conf, out_buf, out_len)                                                 \
        SNC_STEP_BY_STEP();                                                                     \
        snc_uart_write(b_ctx, _SNC_OP_VALUE(const snc_uart_controller_conf_t*, conf),           \
                              _SNC_OP_ADDRESS(out_buf), _SNC_OP(out_len))

void snc_uart_read(b_ctx_t* b_ctx, const snc_uart_controller_conf_t* conf,
                                   SENIS_OPER_TYPE in_buf_type, uint32_t* in_buf,
                                   SENIS_OPER_TYPE in_len_type, uint32_t* in_len,
                                   SENIS_OPER_TYPE wait_time_type, uint32_t* wait_time,
                                   SENIS_OPER_TYPE rcv_len_type, uint32_t* rcv_len);
#define _SNC_uart_read(conf, in_buf, in_len, wait_time, rcv_len)                                \
        SNC_STEP_BY_STEP();                                                                     \
        snc_uart_read(b_ctx, _SNC_OP_VALUE(const snc_uart_controller_conf_t*, conf),            \
                             _SNC_OP_ADDRESS(in_buf), _SNC_OP(in_len),                          \
                             _SNC_OP(wait_time), _SNC_OP_ADDRESS(rcv_len))

void snc_uart_read_check_error(b_ctx_t* b_ctx, const snc_uart_controller_conf_t* conf,
                                   SENIS_OPER_TYPE in_buf_type, uint32_t* in_buf,
                                   SENIS_OPER_TYPE in_len_type, uint32_t* in_len,
                                   SENIS_OPER_TYPE wait_time_type, uint32_t* wait_time,
                                   SENIS_OPER_TYPE rcv_len_type, uint32_t* rcv_len,
                                   SENIS_OPER_TYPE error_type, uint32_t* error,
                                   SENIS_OPER_TYPE error_char_type, uint32_t* error_char);
#define _SNC_uart_read_check_error(conf, in_buf, in_len, wait_time, rcv_len, error, error_char) \
        SNC_STEP_BY_STEP();                                                                     \
        snc_uart_read_check_error(b_ctx, _SNC_OP_VALUE(const snc_uart_controller_conf_t*, conf), \
                                         _SNC_OP_ADDRESS(in_buf), _SNC_OP(in_len),              \
                                         _SNC_OP(wait_time), _SNC_OP_ADDRESS(rcv_len),          \
                                         _SNC_OP_ADDRESS(error), _SNC_OP_ADDRESS(error_char))

#endif /* dg_configUSE_SNC_HW_UART */

#endif /* dg_configUSE_HW_SENSOR_NODE */


#endif /* SNC_HW_UART_MACROS_H_ */

/**
 * \}
 * \}
 */
