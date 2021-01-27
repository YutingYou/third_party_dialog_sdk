/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SNC_QUEUES
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file snc_queues.c
 *
 * @brief SNC Queues implementation
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#if dg_configUSE_HW_SENSOR_NODE

#if dg_configUSE_SNC_QUEUES

#include <string.h>

#include "snc_defs.h"
#include "osal.h"
#include "snc_hw_sys.h"

#include "snc_queues.h"

// Queue Entry/Sample Header Size Definitions
#define SIMPLE_HEADER_SIZE              1
#define TIMESTAMPED_HEADER_SIZE         2

/**
 * \brief SNC queue setup masks bit positions
 *
 */
typedef enum {
        SNC_QUEUE_FLAG_BIT_POS_NO_DATA_TIMESTAMP =      0,/**< Queue No-Data-Timestamp Flag Bit  */
        SNC_QUEUE_FLAG_BIT_POS_SWAP_DATA_PUSHED_BYTES = 1,/**< Queue Swap Pushed Bytes Flag Bit  */
        SNC_QUEUE_FLAG_BIT_POS_SWAP_DATA_POPPED_BYTES = 2,/**< Queue Swap Popped Bytes Flag Bit  */
        SNC_QUEUE_FLAG_BIT_POS_ELEMENT_WEIGHT_BYTE =    3,/**< Queue Element Weight Byte Bit     */
        SNC_QUEUE_FLAG_BIT_POS_ELEMENT_WEIGHT_HWORD =   4,/**< Queue Element Weight Half-Word Bit*/
        SNC_QUEUE_FLAG_BIT_POS_ELEMENT_WEIGHT_WORD =    5 /**< Queue Element Weight Word Bit     */
} SNC_QUEUE_FLAG_BIT_POS;

/**
 * \brief SNC queue setup masks
 *
 */
typedef enum {
        /**< Queue No-Data-Timestamp Flag Mask  */
        SNC_QUEUE_FLAG_NO_DATA_TIMESTAMP =      (1 << SNC_QUEUE_FLAG_BIT_POS_NO_DATA_TIMESTAMP),
        /**< Queue Swap Pushed Bytes Flag Mask  */
        SNC_QUEUE_FLAG_SWAP_PUSHED_DATA_BYTES = (1 << SNC_QUEUE_FLAG_BIT_POS_SWAP_DATA_PUSHED_BYTES),
        /**< Queue Swap Popped Bytes Flag Mask  */
        SNC_QUEUE_FLAG_SWAP_POPPED_DATA_BYTES = (1 << SNC_QUEUE_FLAG_BIT_POS_SWAP_DATA_POPPED_BYTES),
        /**< Queue Element Weight Byte Mask     */
        SNC_QUEUE_FLAG_ELEMENT_WEIGHT_BYTE =    (1 << SNC_QUEUE_FLAG_BIT_POS_ELEMENT_WEIGHT_BYTE),
        /**< Queue Element Weight Half-Word Mask*/
        SNC_QUEUE_FLAG_ELEMENT_WEIGHT_HWORD =   (1 << SNC_QUEUE_FLAG_BIT_POS_ELEMENT_WEIGHT_HWORD),
        /**< Queue Element Weight Word Mask     */
        SNC_QUEUE_FLAG_ELEMENT_WEIGHT_WORD =    (1 << SNC_QUEUE_FLAG_BIT_POS_ELEMENT_WEIGHT_WORD)
} SNC_QUEUE_FLAG;

/**
 * \brief SNC queue preamble masks
 *
 */
typedef enum {
        SNC_QUEUE_PREAMBLE_MASK_SIZE = 0x7FFFFFFF,      /**< Queue Preamble's Size Mask         */
        SNC_QUEUE_PREAMBLE_MASK_WBIT = 0x80000000,      /**< Queue Preamble's Write Bit Mask    */
} SNC_QUEUE_PREAMBLE_MASK;

/**
 * \brief SNC queue preamble masks bit positions
 *
 */
typedef enum {
        SNC_QUEUE_PREAMBLE_BIT_POS_WBIT = 31,           /**< Queue Preamble's Write Bit Position*/
} SNC_QUEUE_PREAMBLE_BIT_POS;

/**
 * \brief SNC queue structure
 *
 */
typedef struct {
        uint32_t **pChunks;             /**< Queue Chunk Pointers                       */
        uint32_t flags;                 /**< Queue Setup Flags                          */
        uint32_t **write_chunk_pt;      /**< Queue write chunk pointer                  */
        uint32_t **read_chunk_pt;       /**< Queue read chunk pointer                   */
        uint32_t **chunk_wp_to_push;    /**< SNC's Queue next chunk pointer to push to
                                             Keeps track of the next chunk to be pushed.
                                             Used in SNC context                        */
        uint32_t **chunk_rp_to_pop;     /**< SNC's Queue next chunk pointer to pop from
                                             Keeps track of the next chunk to be popped.
                                             Used in SNC context                        */
        uint32_t max_chunk_size_bytes;  /**< The chunk's maximum size in bytes          */
        uint32_t num_of_chunks;         /**< The total chunk number                     */
        uint32_t **last_chunk_pt;       /**< Queue last chunk pointer                   */
        uint32_t *data;                 /**< Queue Data                                 */
} snc_q_t;

/**
 * \brief SNC queue Header Structure
 *
 */
typedef struct {
        uint32_t preamble;              /**< Preamble header field                      */
        uint32_t timestamp;             /**< Timestamp header field                     */
} snc_queue_c_hdr_t;

SNC_FUNC_DECL(snc_queues_pop_ucode, uint32_t **q_read_chunk_pt_addr, uint32_t **q_chunk_pop_addr,
        uint32_t *first_chunk, uint32_t *last_chunk);

void snc_queues_snc_pop(b_ctx_t* b_ctx)
{
        snc_q_t** dQ;

        // Check argument validity
        ASSERT_WARNING(b_ctx);
        dQ = (snc_q_t**)(&((snc_ucode_context_t*)(b_ctx->ucode_this_ctx))->CM33_to_SNC_data_queue);
        ASSERT_WARNING(*dQ);
        SNC_ASSERT(da(dQ));

        // Call the respective uCode function
        senis_call(b_ctx, SNC_UCODE_CTX(snc_queues_pop_ucode), 4 * 2,
                SENIS_OPER_TYPE_VALUE, &(*dQ)->read_chunk_pt,
                SENIS_OPER_TYPE_VALUE, &(*dQ)->chunk_rp_to_pop,
                SENIS_OPER_TYPE_ADDRESS_DA, &(*dQ)->pChunks,
                SENIS_OPER_TYPE_ADDRESS_DA, &(*dQ)->last_chunk_pt);
}

SNC_FUNC_DEF(snc_queues_pop_ucode, uint32_t **q_read_chunk_pt_addr, uint32_t **q_chunk_pop_addr,
        uint32_t *first_chunk, uint32_t *last_chunk)
{
        SENIS_labels(mark_chunk_read, no_wrap, invalid_pop);
        _SNC_TMP_ADD(uint32_t*, tmp_pop_chunk_pt, sizeof(uint32_t*));
        _SNC_TMP_ADD(uint32_t*, tmp_var, sizeof(uint32_t*));

        // Get the value of the chunk pop pointer
        SENIS_wadad(da(tmp_pop_chunk_pt), ia(&SNC_ARG(q_chunk_pop_addr)));
        SENIS_rdcgr(da(&snc_const[1]), da(tmp_pop_chunk_pt));
        SENIS_cobr_gr(l(invalid_pop));

        // Get the value of the read chunk pointer
        SENIS_wadad(da(tmp_var), ia(tmp_pop_chunk_pt));

        // Mark the current chunk as read
        SENIS_wadva(ia(tmp_var), 0);

        // Increment the read pointer by one
        SENIS_rdcgr(da(&SNC_ARG(last_chunk)), da(tmp_pop_chunk_pt));
        SENIS_inc4(da(tmp_pop_chunk_pt));
        SENIS_cobr_gr(l(no_wrap));
        SENIS_wadad(da(tmp_pop_chunk_pt), da(&SNC_ARG(first_chunk)));

        SENIS_label(no_wrap);

        // Refresh the read chunk pointer value
        SENIS_wadad(ia(&SNC_ARG(q_read_chunk_pt_addr)), da(tmp_pop_chunk_pt));
        SENIS_wadva(ia(&SNC_ARG(q_chunk_pop_addr)), NULL);

        SENIS_label(invalid_pop);
        _SNC_TMP_RMV(tmp_var);
        _SNC_TMP_RMV(tmp_pop_chunk_pt);
}

SNC_FUNC_DECL(snc_queues_rq_is_empty_ucode, uint32_t* q_header, uint32_t *q_is_empty);

void snc_queues_snc_rq_is_empty(b_ctx_t* b_ctx, SENIS_OPER_TYPE q_empty_type, uint32_t *q_is_empty)
{
        snc_q_t** dQ;

        // Check argument validity
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING((q_empty_type < SENIS_OPER_TYPE_VALUE)&&
                (!b_ctx->upd || !_SNC_ADDR_IS_REG(q_is_empty)));
        dQ = (snc_q_t**)(&((snc_ucode_context_t*)(b_ctx->ucode_this_ctx))->CM33_to_SNC_data_queue);
        ASSERT_WARNING(*dQ);
        SNC_ASSERT(da(dQ));

        // Call the respective uCode function
        senis_call(b_ctx, SNC_UCODE_CTX(snc_queues_rq_is_empty_ucode), 2 * 2,
                SENIS_OPER_TYPE_ADDRESS_IA, &(*dQ)->read_chunk_pt,
                q_empty_type + 1, q_is_empty);
}

SNC_FUNC_DEF(snc_queues_rq_is_empty_ucode, uint32_t* q_header, uint32_t* q_is_empty)
{
        SENIS_labels(queue_not_empty);
        _SNC_TMP_ADD(uint32_t, header_val, sizeof(uint32_t));

        SENIS_wadad(da(header_val), ia(&SNC_ARG(q_header)));
        SENIS_rdcbi(da(header_val), SNC_QUEUE_PREAMBLE_BIT_POS_WBIT);
        SENIS_cobr_eq(l(queue_not_empty));
        SENIS_wadva(ia(&SNC_ARG(q_is_empty)), 1);
        SENIS_return;

        SENIS_label(queue_not_empty);
        SENIS_wadva(ia(&SNC_ARG(q_is_empty)), 0);

        _SNC_TMP_RMV(header_val);
}

SNC_FUNC_DECL(snc_queues_reset_wq_ucode, uint32_t *first_chunk, uint32_t *last_chunk);

void snc_queues_snc_reset_wq(b_ctx_t* b_ctx)
{
        snc_q_t** dQ;

        // Check argument validity
        ASSERT_WARNING(b_ctx);
        dQ = (snc_q_t**)(&((snc_ucode_context_t*)(b_ctx->ucode_this_ctx))->SNC_to_CM33_data_queue);
        ASSERT_WARNING(*dQ);
        SNC_ASSERT(da(dQ));

        // Call the respective uCode function
        senis_call(b_ctx, SNC_UCODE_CTX(snc_queues_reset_wq_ucode), 2 * 2,
                SENIS_OPER_TYPE_ADDRESS_DA, &(*dQ)->pChunks,
                SENIS_OPER_TYPE_ADDRESS_DA, &(*dQ)->last_chunk_pt);

        SENIS_wadva(da(&(*dQ)->read_chunk_pt), &(*dQ)->pChunks[0]);
        SENIS_wadva(da(&(*dQ)->write_chunk_pt), &(*dQ)->pChunks[0]);
        SENIS_wadva(da(&(*dQ)->chunk_wp_to_push), NULL);
        SENIS_wadva(da(&(*dQ)->chunk_rp_to_pop), NULL);
}

SNC_FUNC_DEF(snc_queues_reset_wq_ucode, uint32_t *first_chunk, uint32_t *last_chunk)
{
        uint32_t **cur_chunk = &SNC_ARG(first_chunk);

        SENIS_labels(pt_foreach_chunk, pt_break);

        _SNC_TMP_ADD(uint32_t*, tmp_preamble_p, sizeof(uint32_t*));

        SENIS_label(pt_foreach_chunk);

        // Mark the current chunk as free
        SENIS_wadad(da(tmp_preamble_p), ia(cur_chunk));
        SENIS_wadva(ia(tmp_preamble_p), 0);
        SENIS_inc4(da(cur_chunk));

        SENIS_rdcgr(da(cur_chunk), da(&SNC_ARG(last_chunk)));
        SENIS_cobr_gr(l(pt_break));
        SENIS_goto(l(pt_foreach_chunk));

        SENIS_label(pt_break);

        _SNC_TMP_RMV(tmp_preamble_p);
}

SNC_FUNC_DECL(snc_queues_push_ucode, uint32_t **q_write_chunk_pt_addr, uint32_t **q_chunk_push_addr,
        uint32_t *first_chunk, uint32_t *last_chunk);

void snc_queues_snc_push(b_ctx_t* b_ctx)
{
        snc_q_t** dQ;

        // Check argument validity
        ASSERT_WARNING(b_ctx);
        dQ = (snc_q_t**)(&((snc_ucode_context_t*)(b_ctx->ucode_this_ctx))->SNC_to_CM33_data_queue);
        ASSERT_WARNING(*dQ);
        SNC_ASSERT(da(dQ));

        // Call the respective uCode function
        senis_call(b_ctx, SNC_UCODE_CTX(snc_queues_push_ucode), 4 * 2,
                SENIS_OPER_TYPE_VALUE, &(*dQ)->write_chunk_pt,
                SENIS_OPER_TYPE_VALUE, &(*dQ)->chunk_wp_to_push,
                SENIS_OPER_TYPE_ADDRESS_DA, &(*dQ)->pChunks,
                SENIS_OPER_TYPE_ADDRESS_DA, &(*dQ)->last_chunk_pt);
}

SNC_FUNC_DEF(snc_queues_push_ucode, uint32_t **q_write_chunk_pt_addr, uint32_t **q_chunk_push_addr,
        uint32_t *first_chunk, uint32_t *last_chunk)
{
        SENIS_labels(check_chunk_wp_reached, mark_chunk_written, no_wrap, invalid_push);

        _SNC_TMP_ADD(uint32_t*, tmp_push_chunk_pt, sizeof(uint32_t*));
        _SNC_TMP_ADD(uint32_t*, tmp_var, sizeof(uint32_t*));

        SENIS_wadad(da(tmp_push_chunk_pt), ia(&SNC_ARG(q_chunk_push_addr)));
        SENIS_rdcgr(da(&snc_const[1]), da(tmp_push_chunk_pt));
        SENIS_cobr_gr(l(invalid_push));

        SENIS_wadad(da(tmp_var), ia(tmp_push_chunk_pt));

        // Mark the current chunk as written
        SENIS_xor(ia(tmp_var), SNC_QUEUE_PREAMBLE_MASK_WBIT);

        // Increment the write pointer by one
        SENIS_rdcgr(da(&SNC_ARG(last_chunk)), da(tmp_push_chunk_pt));
        SENIS_inc4(da(tmp_push_chunk_pt));
        SENIS_cobr_gr(l(no_wrap));
        SENIS_wadad(da(tmp_push_chunk_pt), da(&SNC_ARG(first_chunk)));

        SENIS_label(no_wrap);

        // Refresh the write chunk pointer value
        SENIS_wadad(ia(&SNC_ARG(q_write_chunk_pt_addr)), da(tmp_push_chunk_pt));

        // Set the temp push chunk pointer value to NULL
        SENIS_wadva(ia(&SNC_ARG(q_chunk_push_addr)), NULL);

        SENIS_label(invalid_push);
        _SNC_TMP_RMV(tmp_var);
        _SNC_TMP_RMV(tmp_push_chunk_pt);
}

SNC_FUNC_DECL(snc_queues_wq_is_full_ucode, uint32_t* q_chunk_header, uint32_t *q_is_full);

void snc_queues_snc_wq_is_full(b_ctx_t* b_ctx, SENIS_OPER_TYPE q_full_type, uint32_t *q_is_full)
{
        snc_q_t** dQ;

        // Check argument validity
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING((q_full_type < SENIS_OPER_TYPE_VALUE)&&
                (!b_ctx->upd || !_SNC_ADDR_IS_REG(q_is_full)));
        dQ = (snc_q_t**)(&((snc_ucode_context_t*)(b_ctx->ucode_this_ctx))->SNC_to_CM33_data_queue);
        ASSERT_WARNING(*dQ);
        SNC_ASSERT(da(dQ));

        // Call the respective uCode function
        senis_call(b_ctx, SNC_UCODE_CTX(snc_queues_wq_is_full_ucode), 2 * 2,
                SENIS_OPER_TYPE_ADDRESS_IA, &(*dQ)->write_chunk_pt,
                q_full_type + 1, q_is_full);
}

SNC_FUNC_DEF(snc_queues_wq_is_full_ucode, uint32_t *q_chunk_header, uint32_t* q_is_full)
{
        SENIS_labels(queue_is_full);
        _SNC_TMP_ADD(uint32_t, header_val, sizeof(uint32_t));

        SENIS_wadad(da(header_val), ia(&SNC_ARG(q_chunk_header)));
        SENIS_rdcbi(da(header_val), SNC_QUEUE_PREAMBLE_BIT_POS_WBIT);
        SENIS_cobr_eq(l(queue_is_full));
        SENIS_wadva(ia(&SNC_ARG(q_is_full)), 0);
        SENIS_return;

        SENIS_label(queue_is_full);
        SENIS_wadva(ia(&SNC_ARG(q_is_full)), 1);

        _SNC_TMP_RMV(header_val);
}

SNC_FUNC_DECL(snc_queues_get_wq_ucode,
        uint32_t *q_write_chunk_pt, uint32_t **q_chunk_push_addr, uint32_t flags,
        uint32_t **wp, uint32_t *size, uint32_t *timestamp);

void snc_queues_snc_get_wq(b_ctx_t* b_ctx, SENIS_OPER_TYPE wp_type, uint32_t *wp,
        SENIS_OPER_TYPE size_type, uint32_t *size, SENIS_OPER_TYPE ts_type, uint32_t *timestamp)
{
        snc_q_t** dQ;

        // Check argument validity
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING((wp_type < SENIS_OPER_TYPE_VALUE)&&
                (!b_ctx->upd || !_SNC_ADDR_IS_REG(wp)));
        ASSERT_WARNING((!b_ctx->upd || !_SNC_ADDR_IS_REG(size)));
        ASSERT_WARNING((!b_ctx->upd || !_SNC_ADDR_IS_REG(timestamp)));
        dQ = (snc_q_t**)(&((snc_ucode_context_t*)(b_ctx->ucode_this_ctx))->SNC_to_CM33_data_queue);
        ASSERT_WARNING(*dQ);
        SNC_ASSERT(da(dQ));

        // Call the respective uCode function
        senis_call(b_ctx, SNC_UCODE_CTX(snc_queues_get_wq_ucode), 6 * 2,
                SENIS_OPER_TYPE_ADDRESS_DA, &(*dQ)->write_chunk_pt,
                SENIS_OPER_TYPE_VALUE, &(*dQ)->chunk_wp_to_push,
                SENIS_OPER_TYPE_ADDRESS_DA, &(*dQ)->flags,
                wp_type + 1, wp,
                size_type, size,
                ts_type, timestamp);
}

SNC_FUNC_DEF(snc_queues_get_wq_ucode,
        uint32_t* q_write_chunk_pt, uint32_t **q_chunk_push_addr, uint32_t flags,
        uint32_t **wp, uint32_t *size, uint32_t *timestamp)
{
        SENIS_labels(timestamp_disabled, queue_is_full);

        _SNC_TMP_ADD(uint32_t*, temp_push_chunk_pt, sizeof(uint32_t*));
        _SNC_TMP_ADD(uint32_t*, temp_wp, sizeof(uint32_t*));
        _SNC_TMP_ADD(uint32_t, temp_preamble, sizeof(uint32_t));

        // Set the returned write pointer to NULL
        SENIS_wadva(ia(&SNC_ARG(wp)), NULL);
        SENIS_wadad(da(temp_push_chunk_pt), da(&SNC_ARG(q_write_chunk_pt)));

        // Check if the queue is full by checking the write bit in the header preamble
        SENIS_wadad(da(temp_wp), ia(temp_push_chunk_pt));
        SENIS_wadad(da(temp_preamble), ia(temp_wp));
        SENIS_rdcbi(da(temp_preamble), SNC_QUEUE_PREAMBLE_BIT_POS_WBIT);
        SENIS_cobr_eq(l(queue_is_full));

        // If the queue is NOT full, write the size field in the chunk header
        SENIS_wadad(ia(temp_wp), da(&SNC_ARG(size)));
        SENIS_inc4(da(temp_wp));

        SENIS_rdcbi(da(&SNC_ARG(flags)), SNC_QUEUE_FLAG_BIT_POS_NO_DATA_TIMESTAMP);
        SENIS_cobr_eq(l(timestamp_disabled));

        // as well as the timestamp (if not disabled)
        SENIS_wadad(ia(temp_wp), da(&SNC_ARG(timestamp)));
        SENIS_inc4(da(temp_wp));

        SENIS_label(timestamp_disabled);

        // Update the chunk push pointer value
        SENIS_wadad(ia(&SNC_ARG(q_chunk_push_addr)), da(temp_push_chunk_pt));

        // Return the write pointer
        SENIS_wadad(ia(&SNC_ARG(wp)), da(temp_wp));

        SENIS_label(queue_is_full);

        _SNC_TMP_RMV(temp_preamble);
        _SNC_TMP_RMV(temp_wp);
        _SNC_TMP_RMV(temp_push_chunk_pt);
}

void snc_queues_snc_get_wq_max_chunk_bytes(b_ctx_t* b_ctx,
        SENIS_OPER_TYPE max_c_type, uint32_t *max_chunk_size_bytes)
{
        snc_q_t** dQ;

        // Check argument validity
        ASSERT_WARNING(b_ctx);
        dQ = (snc_q_t**)(&((snc_ucode_context_t*)(b_ctx->ucode_this_ctx))->SNC_to_CM33_data_queue);
        ASSERT_WARNING(*dQ);
        SNC_ASSERT(da(dQ));
        ASSERT_WARNING((max_c_type < SENIS_OPER_TYPE_VALUE)&&
                (!b_ctx->upd || !_SNC_ADDR_IS_REG(max_chunk_size_bytes)));
        ASSERT_WARNING(max_chunk_size_bytes);

        senis_assign(b_ctx, max_c_type, max_chunk_size_bytes, _SNC_OP(da(&(*dQ)->max_chunk_size_bytes)));
}

SNC_FUNC_DECL(snc_queues_get_rq_ucode,
        uint32_t *q_read_pt, uint32_t **q_chunk_pop_addr, uint32_t flags,
        uint32_t **rp, uint32_t *size, uint32_t *timestamp);

void snc_queues_snc_get_rq(b_ctx_t* b_ctx, SENIS_OPER_TYPE rp_type, uint32_t *rp,
        SENIS_OPER_TYPE size_type, uint32_t *size, SENIS_OPER_TYPE ts_type, uint32_t *timestamp)
{
        snc_q_t** dQ;

        // Check argument validity
        ASSERT_WARNING(b_ctx);
        dQ = (snc_q_t**)(&((snc_ucode_context_t*)(b_ctx->ucode_this_ctx))->CM33_to_SNC_data_queue);
        ASSERT_WARNING(*dQ);
        SNC_ASSERT(da(dQ));
        ASSERT_WARNING((rp_type < SENIS_OPER_TYPE_VALUE)&&
                (!b_ctx->upd || !_SNC_ADDR_IS_REG(rp)));
        ASSERT_WARNING((size_type < SENIS_OPER_TYPE_VALUE) &&
                (!b_ctx->upd || !_SNC_ADDR_IS_REG(size)));
        ASSERT_WARNING((ts_type < SENIS_OPER_TYPE_VALUE) &&
                (!b_ctx->upd || !_SNC_ADDR_IS_REG(timestamp)));
        ASSERT_WARNING(rp);
        ASSERT_WARNING(size);

        // Call the respective uCode function
        senis_call(b_ctx, SNC_UCODE_CTX(snc_queues_get_rq_ucode), 6 * 2,
                SENIS_OPER_TYPE_ADDRESS_DA, &(*dQ)->read_chunk_pt,
                SENIS_OPER_TYPE_VALUE, &(*dQ)->chunk_rp_to_pop,
                SENIS_OPER_TYPE_ADDRESS_DA, &(*dQ)->flags,
                rp_type + 1, rp,
                size_type + 1, size,
                ts_type + 1, timestamp);
}

SNC_FUNC_DEF(snc_queues_get_rq_ucode,
        uint32_t *q_read_pt, uint32_t **q_chunk_pop_addr, uint32_t flags,
        uint32_t **rp, uint32_t *size, uint32_t *timestamp)
{
        SENIS_labels(timestamp_disabled, queue_not_empty, queue_is_empty);

        _SNC_TMP_ADD(uint32_t *, temp_pop_chunk_pt, sizeof(uint32_t*));
        _SNC_TMP_ADD(uint32_t *, temp_rp, sizeof(uint32_t*));
        _SNC_TMP_ADD(uint32_t, temp_preamble, sizeof(uint32_t));

        // Set the returned read pointer to NULL
        SENIS_wadva(ia(&SNC_ARG(rp)), NULL);

        SENIS_wadad(da(temp_pop_chunk_pt), da(&SNC_ARG(q_read_pt)));

        // Check if the queue is empty by checking the write bit in the header preamble
        SENIS_wadad(da(temp_rp), ia(temp_pop_chunk_pt));
        SENIS_wadad(da(temp_preamble), ia(temp_rp));
        SENIS_rdcbi(da(temp_preamble), SNC_QUEUE_PREAMBLE_BIT_POS_WBIT);
        SENIS_cobr_eq(l(queue_not_empty));
        SENIS_goto(l(queue_is_empty));

        // If the queue is NOT empty, extract the size
        // and timestamp (if enabled) from the chunk header
        SENIS_label(queue_not_empty);
        SENIS_xor(da(temp_preamble), SNC_QUEUE_PREAMBLE_MASK_WBIT);
        SENIS_wadad(ia(&SNC_ARG(size)), da(temp_preamble));
        SENIS_inc4(da(temp_rp));
        SENIS_rdcbi(da(&SNC_ARG(flags)), SNC_QUEUE_FLAG_BIT_POS_NO_DATA_TIMESTAMP);
        SENIS_cobr_eq(l(timestamp_disabled));
        SENIS_wadad(ia(&SNC_ARG(timestamp)), ia(temp_rp));
        SENIS_inc4(da(temp_rp));

        SENIS_label(timestamp_disabled);

        // Refresh the chunk_rp_to_pop pointer
        SENIS_wadad(ia(&SNC_ARG(q_chunk_pop_addr)), da(temp_pop_chunk_pt));

        // Return the read pointer value
        SENIS_wadad(ia(&SNC_ARG(rp)), da(temp_rp));

        SENIS_label(queue_is_empty);

        _SNC_TMP_RMV(temp_preamble);
        _SNC_TMP_RMV(temp_rp);
        _SNC_TMP_RMV(temp_pop_chunk_pt);
}

/**
 * SNC queue configuration function
 *
 * \param [in/out] snc_queue    pointer to the SNC queue to be configured
 * \param [in] cfg              pointer to the SNC queue configuration
 * \param [in] new_queue        indication whether a new SNC queue is to be created or the already
 *                              created one to be updated in terms of its configuration
 */
static void snc_queues_cm33_config(snc_q_t** snc_queue, const snc_queue_config_t *cfg,
        bool new_queue)
{
        snc_q_t* sncQ;
        uint32_t size_of_queue_data;
        uint32_t chunk_header_size;
        uint32_t *pTemp;
        uint32_t actual_chunk_elements;

        ASSERT_WARNING(cfg != NULL);

        // There is no meaning in creating a queue with 1 chunk (not a queue)
        ASSERT_WARNING(cfg->num_of_chunks > 1);

        // Checking element size validity
        ASSERT_WARNING((cfg->element_weight == SNC_QUEUE_ELEMENT_SIZE_BYTE) ||
                (cfg->element_weight == SNC_QUEUE_ELEMENT_SIZE_HWORD) ||
                (cfg->element_weight == SNC_QUEUE_ELEMENT_SIZE_WORD));

        if (new_queue) {
                // Allocate space for the SNC queue header structure
                *snc_queue = OS_MALLOC(sizeof(snc_q_t));
                ASSERT_WARNING(*snc_queue != NULL);

                sncQ = *snc_queue;
        } else {
                sncQ = *snc_queue;

                ASSERT_WARNING(sncQ != NULL);
                ASSERT_WARNING(sncQ->pChunks != NULL);
                ASSERT_WARNING(sncQ->data != NULL);

                OS_FREE(sncQ->pChunks);
                OS_FREE(sncQ->data);
        }

        // Calculate queue data size
        chunk_header_size =
                (cfg->enable_data_timestamp) ? TIMESTAMPED_HEADER_SIZE : SIMPLE_HEADER_SIZE;

        actual_chunk_elements =
                ((cfg->max_chunk_bytes + 1) / cfg->element_weight) + cfg->element_weight - 2;
        size_of_queue_data = (chunk_header_size + actual_chunk_elements) *
                cfg->num_of_chunks * sizeof(uint32_t);

        // Allocate space for the SNC queue data
        sncQ->data = OS_MALLOC(size_of_queue_data);
        ASSERT_WARNING(sncQ->data != NULL);

        sncQ->flags =
                ((cfg->element_weight == SNC_QUEUE_ELEMENT_SIZE_BYTE)
                        * (SNC_QUEUE_FLAG_ELEMENT_WEIGHT_BYTE)) |
                        ((cfg->element_weight == SNC_QUEUE_ELEMENT_SIZE_HWORD)
                                * (SNC_QUEUE_FLAG_ELEMENT_WEIGHT_HWORD)) |
                        ((cfg->element_weight == SNC_QUEUE_ELEMENT_SIZE_WORD)
                                * (SNC_QUEUE_FLAG_ELEMENT_WEIGHT_WORD)) |
                        ((cfg->enable_data_timestamp == false) & SNC_QUEUE_FLAG_NO_DATA_TIMESTAMP) |
                        ((cfg->swap_pushed_data_bytes != false)
                                * SNC_QUEUE_FLAG_SWAP_PUSHED_DATA_BYTES) |
                        ((cfg->swap_popped_data_bytes != false)
                                * SNC_QUEUE_FLAG_SWAP_POPPED_DATA_BYTES);

        sncQ->max_chunk_size_bytes = cfg->max_chunk_bytes;

        // Allocate space for the chunk pointers
        sncQ->pChunks = OS_MALLOC(cfg->num_of_chunks * sizeof(uint32_t*));
        ASSERT_WARNING(sncQ->pChunks != NULL);

        // Initialize the chunk pointers
        pTemp = sncQ->data;

        for (uint32_t i = 0; i < cfg->num_of_chunks; i++) {
                sncQ->pChunks[i] = pTemp;

                // Also clear the header field in every chunk
                *sncQ->pChunks[i] = 0;

                pTemp = pTemp + chunk_header_size + actual_chunk_elements;
        }

        sncQ->num_of_chunks = cfg->num_of_chunks;
        sncQ->last_chunk_pt = &sncQ->pChunks[cfg->num_of_chunks - 1];
        sncQ->read_chunk_pt = &sncQ->pChunks[0];
        sncQ->write_chunk_pt = &sncQ->pChunks[0];
        sncQ->chunk_wp_to_push = NULL;
        sncQ->chunk_rp_to_pop = NULL;
}

snc_queue_t snc_queues_cm33_create(const snc_queue_config_t *snc_queue_cfg)
{
        snc_queue_t snc_queue;

        snc_queues_cm33_config((snc_q_t**)&snc_queue, snc_queue_cfg, true);

        return snc_queue;
}

void snc_queues_cm33_update(snc_queue_t snc_queue, const snc_queue_config_t *snc_queue_cfg)
{
        snc_queues_cm33_config((snc_q_t**)&snc_queue, snc_queue_cfg, false);
}

void snc_queues_cm33_destroy(snc_queue_t snc_queue)
{
        snc_q_t *sncQ;

        ASSERT_WARNING(snc_queue);
        sncQ = (snc_q_t*)snc_queue;

        // First deallocate the chunks pointer array
        ASSERT_WARNING(sncQ->pChunks);
        OS_FREE(sncQ->pChunks);

        // Then deallocate the SNC queue data
        ASSERT_WARNING(sncQ->data);
        OS_FREE(sncQ->data);

        // Then deallocate the SNC queue header structure
        OS_FREE(sncQ);
}

void snc_queues_cm33_reset(snc_queue_t snc_queue)
{
        snc_q_t *sncQ;

        ASSERT_WARNING(snc_queue);
        sncQ = (snc_q_t *)snc_queue;

        for (uint32_t i = 0; i < sncQ->num_of_chunks; i++) {
                // Clear the header field in every chunk
                *sncQ->pChunks[i] = 0;
        }

        sncQ->read_chunk_pt = &sncQ->pChunks[0];
        sncQ->write_chunk_pt = &sncQ->pChunks[0];
        sncQ->chunk_wp_to_push = NULL;
        sncQ->chunk_rp_to_pop = NULL;
}

bool snc_queues_cm33_push(const snc_queue_t snc_queue, const uint8_t *pData, size_t size,
        uint32_t timestamp)
{
        snc_q_t *sncQ;
        uint32_t i;
        uint32_t *pData32;
        uint16_t *pData16;
        uint32_t *pWrite;
        uint32_t hdr_size, int_size;
        uint32_t **temp_write_pt;

        ASSERT_WARNING(snc_queue);

        sncQ = (snc_q_t *)snc_queue;
        temp_write_pt = sncQ->write_chunk_pt;
        pWrite = *temp_write_pt;
        // Return immediately if the queue is full (cannot push data)
        if (((((snc_queue_c_hdr_t*)pWrite)->preamble & SNC_QUEUE_PREAMBLE_MASK_WBIT) != 0)
                || (size > sncQ->max_chunk_size_bytes)) {
                return false;
        }

        // Calculate the header size depending on the configuration flags
        hdr_size = (!(sncQ->flags & SNC_QUEUE_FLAG_NO_DATA_TIMESTAMP)) ?
                        TIMESTAMPED_HEADER_SIZE : SIMPLE_HEADER_SIZE;

        /*
         * The SNC works with 32-bit aligned data and addresses. Due to this fact,
         * when pushing data into the queue, the given data need to be unpacked into
         * 32-bit elements. The unpacking is done depending on the value of the element_weight
         * during the initial queue configuration.
         * If the chosen element weight is byte, it means that elements inside
         * the queue's chunks are to be treated by the SNC as bytes.
         * If the chosen element weight is half word, it means that elements inside
         * the queue's chunks are to be treated by the SNC as 2-byte values.
         * If the chosen element weight is word, it means that elements inside
         * the queue's chunks are to be treated by the SNC as words.
         * For half word and word element weights, byte swapping is supported. It can be
         * enabled/ disabled during the queue's configuration by the swap_pushed_data_bytes field
         * of the snc_queue_config_t configuration structure.
         *
         *   ### Pushing 4 bytes of data ###
         *
         *   +-------+-------+-------+-------+
         *   |       |       |       |       |
         *   | byte0 | byte1 | byte2 | byte3 |
         *   |       |       |       |       |
         *   +-------+-------+-------+-------+
         *
         *   #### Unpacking ####
         *
         *   %%%%%% Element weight byte %%%%%%
         *   +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
         *   |       *       *       *       |       *       *       *       |       *       *       *       |       *       *       *       |
         *   | byte0 *       *       *       | byte1 *       *       *       | byte2 *       *       *       | byte3 *       *       *       |
         *   |       *       *       *       |       *       *       *       |       *       *       *       |       *       *       *       |
         *   +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
         *
         *   %%%%%% Element weight half word %%%%%%
         *   $ Pushed data byte swapping DISABLED
         *   +-------+-------+-------+-------+-------+-------+-------+-------+
         *   |       *       *       *       |       *       *       *       |
         *   | byte0 * byte1 *       *       | byte2 * byte3 *       *       |
         *   |       *       *       *       |       *       *       *       |
         *   +-------+-------+-------+-------+-------+-------+-------+-------+
         *
         *   $ Pushed data byte swapping ENABLED
         *   +-------+-------+-------+-------+-------+-------+-------+-------+
         *   |       *       *       *       |       *       *       *       |
         *   | byte1 * byte0 *       *       | byte3 * byte2 *       *       |
         *   |       *       *       *       |       *       *       *       |
         *   +-------+-------+-------+-------+-------+-------+-------+-------+
         *
         *   %%%%%% Element weight word %%%%%%
         *   $ Pushed data byte swapping DISABLED
         *   +-------+-------+-------+-------+
         *   |       *       *       *       |
         *   | byte0 * byte1 * byte2 * byte3 |
         *   |       *       *       *       |
         *   +-------+-------+-------+-------+
         *
         *   $ Pushed data byte swapping ENABLED
         *   +-------+-------+-------+-------+
         *   |       *       *       *       |
         *   | byte3 * byte2 * byte1 * byte0 |
         *   |       *       *       *       |
         *   +-------+-------+-------+-------+
         *
         *
         */

        // If the element weight is byte, unpack the data from uint8_t
        // elements to uint32_t elements
        if (sncQ->flags & SNC_QUEUE_FLAG_ELEMENT_WEIGHT_BYTE) {
                pData32 = pWrite + hdr_size;
                for (i = 0; i < size; i++) {
                        *pData32 = *pData;
                        ++pData32;
                        ++pData;
                }
        }
        // If the element weight is 2 bytes, unpack the data from uint16_t
        // elements to uint32_t elements
        else if (sncQ->flags & SNC_QUEUE_FLAG_ELEMENT_WEIGHT_HWORD) {
                pData16 = (uint16_t*)(pWrite + hdr_size);
                int_size = ((size / 2) * 2);

                // If the swap bytes configuration flag is set then also
                // reverse the acquired bytes during the unpacking
                if (sncQ->flags & SNC_QUEUE_FLAG_SWAP_PUSHED_DATA_BYTES) {
                        for (i = 0; i < int_size; i += sizeof(uint16_t)) {
                                *pData16 = SWAP16(*((uint16* )pData));
                                ++pData16;
                                ++pData16;
                                ++pData;
                                ++pData;
                        }
                }
                // Else just unpack the bytes without doing any additional operation
                else {
                        for (i = 0; i < int_size; i += sizeof(uint16_t)) {
                                *pData16 = *((uint16*)pData);
                                ++pData16;
                                ++pData16;
                                ++pData;
                                ++pData;
                        }
                }

                // If the pushed size was not "round", then append the last byte
                // to the next position of the destination buffer
                if (size % 2) {
                        *((uint8_t*)pData16) = *pData;
                }
        }
        // If the element weight is 4 bytes(word), there is no need for unpacking
        // Just copy the data as is
        else {
                int_size = ((size / 4) * 4);
                pData32 = pWrite + hdr_size;
                // If the swap bytes configuration flag is enabled then also revert
                // the pushed bytes while copying to the destination buffer
                if (sncQ->flags & SNC_QUEUE_FLAG_SWAP_PUSHED_DATA_BYTES) {
                        for (i = 0; i < int_size; i += sizeof(uint32_t)) {
                                *pData32 = SWAP32(*((uint32_t* )pData));
                                ++pData32;
                                pData += sizeof(uint32_t);
                        }
                }
                // Else perform a one-by-one memcpy
                else {
                        memcpy((void*)pData32, (void *)pData, size);
                }

                // If the bytes where not a "round" multiple of 4
                // copy the rest of them to the end of the destination buffer
                int_size = size % 4;
                for (i = 0; i < int_size; i++) {
                        *((uint8_t*)pData32) = pData[i];
                        ++pData32;
                }
        }

        // Write the header preamble of the chunk to mark the chunk as written
        ((snc_queue_c_hdr_t*)pWrite)->preamble = size | SNC_QUEUE_PREAMBLE_MASK_WBIT;

        // If timestamp is enabled also write the timestamp
        if (!(sncQ->flags & SNC_QUEUE_FLAG_NO_DATA_TIMESTAMP)) {
                ((snc_queue_c_hdr_t*)pWrite)->timestamp = timestamp;
        }

        // Calculate the next write pointer value
        if (temp_write_pt == &sncQ->pChunks[sncQ->num_of_chunks - 1]) {
                temp_write_pt = sncQ->pChunks;
        } else {
                ++temp_write_pt;
        }

        // Update the write pointer with the new value
        sncQ->write_chunk_pt = temp_write_pt;

        return true;
}

bool snc_queues_cm33_pop(const snc_queue_t snc_queue, uint8_t *pData, uint32_t *rsize,
        uint32_t *timestamp)
{
        snc_q_t *sncQ;
        uint32_t *pRead;
        uint32_t hdr_size;
        uint32_t i;
        uint32_t *pData32;
        uint16_t *pData16;
        uint32_t** temp_read_pt;
        uint32_t size, int_size;

        ASSERT_WARNING(snc_queue);

        sncQ = (snc_q_t *)snc_queue;

        temp_read_pt = sncQ->read_chunk_pt;
        pRead = *temp_read_pt;

        // Return immediately if the queue is empty (nothing to pop)
        if ((((snc_queue_c_hdr_t*)pRead)->preamble & SNC_QUEUE_PREAMBLE_MASK_WBIT) == 0) {
                return false;
        }

        // Get the size from the chunk's header preamble
        size = ((snc_queue_c_hdr_t*)pRead)->preamble & SNC_QUEUE_PREAMBLE_MASK_SIZE;

        // Calculate the header size depending on the configuration flags
        hdr_size = (!(sncQ->flags & SNC_QUEUE_FLAG_NO_DATA_TIMESTAMP)) ?
                        TIMESTAMPED_HEADER_SIZE : SIMPLE_HEADER_SIZE;

        /*
         * The SNC works with 32-bit aligned data and addresses. Due to this fact,
         * when popping data from a queue chunk, the read data need to be packed into successive
         * 8-bit data. The packing is done depending on the value of the element_weight field as chosen
         * during the initial queue configuration.
         * If the chosen element weight is byte, it means that elements inside
         * the queue's chunks are to be treated by the SNC as bytes. So when popping the data
         * each word is treated as a byte.
         * If the chosen element weight is half word, it means that elements inside
         * the queue's chunks are to be treated by the SNC as 2-byte values. So when popping the data
         * each word is treated as a 2 byte value.
         * If the chosen element weight is word, it means that elements inside the
         * queue's chunks are to be treated by the SNC as words. So when popping the data it is assumed
         * that it is already packed (each word is a word indeed).
         * For half word and word element weights, byte swapping is supported. It can be
         * enabled/ disabled during the queue's configuration by the swap_popped_data_bytes field
         * of the snc_queue_config_t configuration structure.
         *
         *  ### Popping 4 bytes of data from the SNC ###
         *
         *  %%%%%% Element weight byte %%%%%%
         *  +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
         *  |       *       *       *       |       *       *       *       |       *       *       *       |       *       *       *       |
         *  | byte0 *       *       *       | byte1 *       *       *       | byte2 *       *       *       | byte3 *       *       *       |
         *  |       *       *       *       |       *       *       *       |       *       *       *       |       *       *       *       |
         *  +-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+-------+
         *
         *  ### Packing ###
         *  +-------+-------+-------+-------+
         *  |       *       *       *       |
         *  | byte0 * byte1 * byte2 * byte3 |
         *  |       *       *       *       |
         *  +-------+-------+-------+-------+
         *
         *
         *  %%%%%% Element weight half word %%%%%%
         *  +-------+-------+-------+-------+-------+-------+-------+-------+
         *  |       *       *       *       |       *       *       *       |
         *  | byte0 * byte1 *       *       | byte2 * byte3 *       *       |
         *  |       *       *       *       |       *       *       *       |
         *  +-------+-------+-------+-------+-------+-------+-------+-------+
         *
         *  ### Packing ###
         *  $ Popped bytes swapping DISABLED
         *  +-------+-------+-------+-------+
         *  |       *       *       *       |
         *  | byte0 * byte1 * byte2 * byte3 |
         *  |       *       *       *       |
         *  +-------+-------+-------+-------+
         *
         *  $ Popped bytes swapping ENABLED
         *  +-------+-------+-------+-------+
         *  |       *       *       *       |
         *  | byte1 * byte0 * byte3 * byte2 |
         *  |       *       *       *       |
         *  +-------+-------+-------+-------+
         *
         *
         *  %%%%%% Element weight word %%%%%%
         *  +-------+-------+-------+-------+
         *  |       *       *       *       |
         *  | byte0 * byte1 * byte2 * byte3 |
         *  |       *       *       *       |
         *  +-------+-------+-------+-------+
         *
         *  ## Packing ##
         *  $ Popped bytes swapping DISABLED
         *  +-------+-------+-------+-------+
         *  |       *       *       *       |
         *  | byte0 * byte1 * byte2 * byte3 |
         *  |       *       *       *       |
         *  +-------+-------+-------+-------+
         *
         *  $ Popped bytes swapping ENABLED
         *  +-------+-------+-------+-------+
         *  |       *       *       *       |
         *  | byte3 * byte2 * byte1 * byte0 |
         *  |       *       *       *       |
         *  +-------+-------+-------+-------+
         *
         */

        // If the element weight is byte, pack the data from uint32_t
        // elements to uint8_t elements
        if (sncQ->flags & SNC_QUEUE_FLAG_ELEMENT_WEIGHT_BYTE) {
                pData32 = pRead + hdr_size;
                for (i = 0; i < size; i++) {
                        *pData = (uint8_t)*pData32;
                        ++pData32;
                        ++pData;
                }
        }
        // If the element weight is 2 bytes, pack the data from uint32_t
        // elements to uint16_t elements
        else if (sncQ->flags & SNC_QUEUE_FLAG_ELEMENT_WEIGHT_HWORD) {
                pData16 = (uint16_t*)(pRead + hdr_size);
                int_size = ((size / 2) * 2);

                // If the swap bytes configuration flag is set then also
                // reverse the acquired bytes during the packing
                if (sncQ->flags & SNC_QUEUE_FLAG_SWAP_POPPED_DATA_BYTES) {
                        for (i = 0; i < int_size; i += 2) {
                                *((uint16*)pData) = SWAP16(*pData16);
                                ++pData16;
                                ++pData16;
                                ++pData;
                                ++pData;
                        }
                }
                // Else just pack the bytes without doing any additional operation
                else {
                        for (i = 0; i < int_size; i += 2) {
                                *((uint16*)pData) = *pData16;
                                ++pData16;
                                ++pData16;
                                ++pData;
                                ++pData;
                        }
                }

                // If the popped size was not "round", then append the last byte
                // to the next position of the destination buffer
                if (size % 2) {
                        *pData = *((uint8_t*)pData16);
                }
        }
        // If the element weight is 4 bytes(word), there is no need for packing
        // Just copy the data one by one
        else {
                pData32 = pRead + hdr_size;
                int_size = ((size / 4) * 4);
                // If the swap bytes configuration flag is enabled then also revert
                // the acquired bytes while copying to the destination buffer
                if (sncQ->flags & SNC_QUEUE_FLAG_SWAP_POPPED_DATA_BYTES) {
                        for (i = 0; i < int_size; i += 4) {
                                *((uint32_t*)pData) = SWAP32(*pData32);
                                ++pData32;
                                pData += 4;
                        }
                }
                // Else perform a one-by-one memcpy
                else {
                        memcpy((void*)pData, (void *)pData32, int_size);
                }
                // If the bytes where not a "round" multiple of 4
                // copy the rest of them to the end of the destination buffer
                for (i = 0; i < (size % 4); i++) {
                        pData[i] = *((uint8_t*)pData32);
                        ++pData32;
                }
        }

        // If the queue supports data timestamp, then return the data timestamp
        // that was written during pushing
        if ((!(sncQ->flags & SNC_QUEUE_FLAG_NO_DATA_TIMESTAMP)) && timestamp) {
                *timestamp = ((snc_queue_c_hdr_t*)pRead)->timestamp;
        }

        // Return the size that was popped
        if (rsize) {
                *rsize = size;
        }

        // Clear the preamble to mark the chunk as read
        ((snc_queue_c_hdr_t*)pRead)->preamble = 0;

        // Calculate the next value of queue read pointer
        if (temp_read_pt == &sncQ->pChunks[sncQ->num_of_chunks - 1]) {
                temp_read_pt = sncQ->pChunks;
        } else {
                ++temp_read_pt;
        }

        // Update the value of the queue read pointer
        sncQ->read_chunk_pt = temp_read_pt;

        return true;
}

bool snc_queues_cm33_queue_is_empty(const snc_queue_t snc_queue)
{
        snc_q_t *sncQ;
        uint32_t *pPop;

        ASSERT_WARNING(snc_queue);

        sncQ = (snc_q_t *)snc_queue;
        pPop = *sncQ->read_chunk_pt;

        // Queue is EMPTY if the current chunk header preamble that the
        // read pointer points to has its write bit CLEARED
        return ((((snc_queue_c_hdr_t*)pPop)->preamble & SNC_QUEUE_PREAMBLE_MASK_WBIT)==0);
}

bool snc_queues_cm33_queue_is_full(const snc_queue_t snc_queue)
{
        snc_q_t *sncQ;
        uint32_t *pPush;

        ASSERT_WARNING(snc_queue);

        sncQ = (snc_q_t *)snc_queue;
        pPush = *sncQ->write_chunk_pt;
        // Queue is FULL if the current chunk header preamble that the
        // write pointer points to has its write bit SET
        return ((((snc_queue_c_hdr_t*)pPush)->preamble & SNC_QUEUE_PREAMBLE_MASK_WBIT)!=0);
}

uint32_t snc_queues_cm33_get_free_chunks(const snc_queue_t snc_queue)
{
        uint32_t i;
        snc_q_t *sncQ;
        uint32_t ret = 0;

        ASSERT_WARNING(snc_queue);
        sncQ = (snc_q_t *)snc_queue;

        // Count the number of free chunks
        for (i = 0; i < sncQ->num_of_chunks; i++) {
                if ((((snc_queue_c_hdr_t*)sncQ->pChunks[i])->preamble
                        & SNC_QUEUE_PREAMBLE_MASK_WBIT) == 0) {
                        ++ret;
                }
        }

        // Return the number of free chunks
        return ret;
}

uint32_t snc_queues_cm33_get_alloc_chunks(const snc_queue_t snc_queue)
{
        uint32_t i;
        snc_q_t *sncQ;
        uint32_t ret = 0;

        ASSERT_WARNING(snc_queue);
        sncQ = (snc_q_t *)snc_queue;

        // Count the number of allocated chunks
        for (i = 0; i < sncQ->num_of_chunks; i++) {
                if ((((snc_queue_c_hdr_t*)sncQ->pChunks[i])->preamble
                        & SNC_QUEUE_PREAMBLE_MASK_WBIT) != 0) {
                        ++ret;
                }
        }

        // Return the number of allocated chunks
        return ret;
}

uint32_t snc_queues_cm33_get_cur_chunk_bytes(const snc_queue_t snc_queue)
{
        snc_q_t *sncQ;
        uint32_t *pQ;

        ASSERT_WARNING(snc_queue);
        sncQ = (snc_q_t *)snc_queue;
        pQ = *sncQ->read_chunk_pt;

        // Return the number of bytes residing on the current chunk
        return (((snc_queue_c_hdr_t*)pQ)->preamble & SNC_QUEUE_PREAMBLE_MASK_SIZE);
}

#endif /* dg_configUSE_SNC_QUEUES */

#endif /* dg_configUSE_HW_SENSOR_NODE */


/**
 * \}
 * \}
 */
