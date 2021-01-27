/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SNC_QUEUES
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file snc_queues_macros.h
 *
 * @brief SNC definitions of SNC queues macros
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SNC_QUEUES_MACROS_H_
#define SNC_QUEUES_MACROS_H_


#if dg_configUSE_HW_SENSOR_NODE

#if dg_configUSE_SNC_QUEUES

/*
 * FUNCTION DECLARATIONS
 *****************************************************************************************
 */

void snc_queues_snc_reset_wq(b_ctx_t* b_ctx);
#define _SNC_queues_snc_reset_wq()                                                              \
        SNC_STEP_BY_STEP();                                                                     \
        snc_queues_snc_reset_wq(b_ctx)

void snc_queues_snc_push(b_ctx_t* b_ctx);
#define _SNC_queues_snc_push()                                                                  \
        SNC_STEP_BY_STEP();                                                                     \
        snc_queues_snc_push(b_ctx)

void snc_queues_snc_wq_is_full(b_ctx_t* b_ctx, SENIS_OPER_TYPE q_full_type, uint32_t *q_is_full);
#define _SNC_queues_snc_wq_is_full(q_is_full)                                                   \
        SNC_STEP_BY_STEP();                                                                     \
        snc_queues_snc_wq_is_full(b_ctx, _SNC_OP_ADDRESS(q_is_full))

void snc_queues_snc_pop(b_ctx_t* b_ctx);
#define _SNC_queues_snc_pop()                                                                   \
        SNC_STEP_BY_STEP();                                                                     \
        snc_queues_snc_pop(b_ctx)

void snc_queues_snc_rq_is_empty(b_ctx_t* b_ctx, SENIS_OPER_TYPE q_empty_type, uint32_t *q_is_empty);
#define _SNC_queues_snc_rq_is_empty(q_is_empty)                                                 \
        SNC_STEP_BY_STEP();                                                                     \
        snc_queues_snc_rq_is_empty(b_ctx, _SNC_OP_ADDRESS(q_is_empty));

void snc_queues_snc_get_wq(b_ctx_t* b_ctx, SENIS_OPER_TYPE wp_type, uint32_t *wp,
        SENIS_OPER_TYPE len_type, uint32_t *len, SENIS_OPER_TYPE ts_type, uint32_t *timestamp);
#define _SNC_queues_snc_get_wq(wp, len, timestamp)                                              \
        SNC_STEP_BY_STEP();                                                                     \
        snc_queues_snc_get_wq(b_ctx, _SNC_OP_ADDRESS(wp), _SNC_OP(len), _SNC_OP(timestamp))

void snc_queues_snc_get_wq_max_chunk_bytes(b_ctx_t* b_ctx,
        SENIS_OPER_TYPE max_c_type, uint32_t *max_chunk_size_bytes);
#define _SNC_queues_snc_get_wq_max_chunk_bytes(max_chunk_size_bytes)                            \
        SNC_STEP_BY_STEP();                                                                     \
        snc_queues_snc_get_wq_max_chunk_bytes(b_ctx, _SNC_OP_ADDRESS(max_chunk_size_bytes))

void snc_queues_snc_get_rq(b_ctx_t* b_ctx, SENIS_OPER_TYPE rp_type, uint32_t *rp,
        SENIS_OPER_TYPE len_type, uint32_t *len, SENIS_OPER_TYPE ts_type, uint32_t *timestamp);
#define _SNC_queues_snc_get_rq(rp, len, timestamp)                                              \
        SNC_STEP_BY_STEP();                                                                     \
        snc_queues_snc_get_rq(b_ctx,                                                            \
                _SNC_OP_ADDRESS(rp), _SNC_OP_ADDRESS(len), _SNC_OP_ADDRESS(timestamp))

#endif /* dg_configUSE_SNC_QUEUES */

#endif /* dg_configUSE_HW_SENSOR_NODE */


#endif /* SNC_QUEUES_MACROS_H_ */

/**
 * \}
 * \}
 */
