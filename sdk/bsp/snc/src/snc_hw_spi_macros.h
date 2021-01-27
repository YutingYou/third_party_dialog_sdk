/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SNC_HW_SPI
 *
 * \brief SPI LLD macros for SNC context
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file snc_hw_spi_macros.h
 *
 * @brief SNC definitions of SPI Low Level Driver Macros
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SNC_HW_SPI_MACROS_H_
#define SNC_HW_SPI_MACROS_H_


#if dg_configUSE_HW_SENSOR_NODE

#if dg_configUSE_SNC_HW_SPI

//==================== Controller Acquisition functions ========================

void snc_spi_open(b_ctx_t* b_ctx, const snc_spi_controller_conf_t* conf);
#define _SNC_spi_open(conf)                                                                     \
        SNC_STEP_BY_STEP();                                                                     \
        snc_spi_open(b_ctx, _SNC_OP_VALUE(const snc_spi_controller_conf_t*, conf))

void snc_spi_close(b_ctx_t* b_ctx, const snc_spi_controller_conf_t* conf);
#define _SNC_spi_close(conf)                                                                    \
        SNC_STEP_BY_STEP();                                                                     \
        snc_spi_close(b_ctx, _SNC_OP_VALUE(const snc_spi_controller_conf_t*, conf))

//==================== Controller Configuration functions ======================

#if CONFIG_SNC_SPI_ENABLE_RECONFIG
void snc_spi_reconfig(b_ctx_t* b_ctx, const snc_spi_controller_conf_t* conf);
#define _SNC_spi_reconfig(conf)                                                                 \
        SNC_STEP_BY_STEP();                                                                     \
        snc_spi_reconfig(b_ctx, _SNC_OP_VALUE(const snc_spi_controller_conf_t*, conf))
#endif /* CONFIG_SNC_SPI_ENABLE_RECONFIG */

//==================== CS handling functions ===================================

void snc_spi_activate_cs(b_ctx_t* b_ctx, const snc_spi_controller_conf_t* conf);
#define _SNC_spi_activate_cs(conf)                                                              \
        SNC_STEP_BY_STEP();                                                                     \
        snc_spi_activate_cs(b_ctx, _SNC_OP_VALUE(const snc_spi_controller_conf_t*, conf))

void snc_spi_deactivate_cs(b_ctx_t* b_ctx, const snc_spi_controller_conf_t* conf);
#define _SNC_spi_deactivate_cs(conf)                                                            \
        SNC_STEP_BY_STEP();                                                                     \
        snc_spi_deactivate_cs(b_ctx, _SNC_OP_VALUE(const snc_spi_controller_conf_t*, conf))

//==================== Data Read/Write functions ===============================

void snc_spi_write(b_ctx_t* b_ctx, const snc_spi_controller_conf_t* conf,
                                   SENIS_OPER_TYPE out_buf_type, uint32_t* out_buf,
                                   SENIS_OPER_TYPE out_len_type, uint32_t* out_len);
#define _SNC_spi_write(conf, out_buf, out_len)                                                  \
        SNC_STEP_BY_STEP();                                                                     \
        snc_spi_write(b_ctx, _SNC_OP_VALUE(const snc_spi_controller_conf_t*, conf),             \
                             _SNC_OP_ADDRESS(out_buf), _SNC_OP(out_len))

void snc_spi_read(b_ctx_t* b_ctx, const snc_spi_controller_conf_t* conf,
                                  SENIS_OPER_TYPE in_buf_type, uint32_t* in_buf,
                                  SENIS_OPER_TYPE in_len_type, uint32_t* in_len);
#define _SNC_spi_read(conf, in_buf, in_len)                                                     \
        SNC_STEP_BY_STEP();                                                                     \
        snc_spi_read(b_ctx, _SNC_OP_VALUE(const snc_spi_controller_conf_t*, conf),              \
                            _SNC_OP_ADDRESS(in_buf), _SNC_OP(in_len))

#if CONFIG_SNC_SPI_USE_ADVANCED_WRITE
void snc_spi_write_advanced(b_ctx_t* b_ctx, const snc_spi_controller_conf_t* conf,
                                   SENIS_OPER_TYPE out_word_mode_type, uint32_t* out_word_mode,
                                   SENIS_OPER_TYPE out_buf_type, uint32_t* out_buf,
                                   SENIS_OPER_TYPE out_len_type, uint32_t* out_len);
#define _SNC_spi_write_advanced(conf, out_word_mode, out_buf, out_len)                          \
        SNC_STEP_BY_STEP();                                                                     \
        snc_spi_write_advanced(b_ctx, _SNC_OP_VALUE(const snc_spi_controller_conf_t*, conf),    \
                _SNC_OP(out_word_mode), _SNC_OP_ADDRESS(out_buf), _SNC_OP(out_len))
#endif /* CONFIG_SNC_SPI_USE_ADVANCED_WRITE */

#if CONFIG_SNC_SPI_USE_ADVANCED_READ
void snc_spi_read_advanced(b_ctx_t* b_ctx, const snc_spi_controller_conf_t* conf,
                                  SENIS_OPER_TYPE in_word_mode_type, uint32_t* in_word_mode,
                                  SENIS_OPER_TYPE in_buf_type, uint32_t* in_buf,
                                  SENIS_OPER_TYPE in_len_type, uint32_t* in_len);
#define _SNC_spi_read_advanced(conf, in_word_mode, in_buf, in_len)                              \
        SNC_STEP_BY_STEP();                                                                     \
        snc_spi_read_advanced(b_ctx, _SNC_OP_VALUE(const snc_spi_controller_conf_t*, conf),     \
                _SNC_OP(in_word_mode), _SNC_OP_ADDRESS(in_buf), _SNC_OP(in_len))
#endif /* CONFIG_SNC_SPI_USE_ADVANCED_READ */

void snc_spi_writeread_buf(b_ctx_t* b_ctx, const snc_spi_controller_conf_t* conf,
                                           SENIS_OPER_TYPE out_buf_type, uint32_t* out_buf,
                                           SENIS_OPER_TYPE out_len_type, uint32_t* out_len,
                                           SENIS_OPER_TYPE in_buf_type, uint32_t* in_buf,
                                           SENIS_OPER_TYPE in_len_type, uint32_t* in_len);
#define _SNC_spi_writeread_buf(conf, out_buf, out_len, in_buf, in_len)                          \
        SNC_STEP_BY_STEP();                                                                     \
        snc_spi_writeread_buf(b_ctx, _SNC_OP_VALUE(const snc_spi_controller_conf_t*, conf),     \
                                     _SNC_OP_ADDRESS(out_buf), _SNC_OP(out_len),                \
                                     _SNC_OP_ADDRESS(in_buf), _SNC_OP(in_len))

#endif /* dg_configUSE_SNC_HW_SPI */

#endif /* dg_configUSE_HW_SENSOR_NODE */


#endif /* SNC_HW_SPI_MACROS_H_ */

/**
 * \}
 * \}
 */
