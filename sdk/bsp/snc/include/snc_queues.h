/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SNC_QUEUES
 *
 * \brief SNC queues
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file snc_queues.h
 *
 * @brief SNC-Context SNC Queues API
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SNC_QUEUES_H_
#define SNC_QUEUES_H_


#if dg_configUSE_HW_SENSOR_NODE

#if dg_configUSE_SNC_QUEUES

#include "snc_defs.h"

/*
 * MACRO DEPENDENCIES
 *****************************************************************************************
 */

#include "snc_queues_macros.h"

/*
 * FUNCTION DECLARATIONS
 *****************************************************************************************
 */

/**
 * \brief Function used in SNC context in order to reset the SNC-to-CM33 (write) queue
 *
 * In essence, when calling this function, the chunks that have been marked as "written/pushed"
 * in the SNC-to-CM33 (write) queue, are finally marked as free.
 *
 */
#define SNC_queues_snc_reset_wq()                                                               \
        _SNC_queues_snc_reset_wq()

/**
 * \brief Function used in SNC context in order to push data into the SNC-to-CM33 (write) queue
 *
 * A queue write pointer should first be obtained using the SNC_queues_snc_get_wq() function.
 * After that, the user may write inside the queue using that pointer. After writing the desired
 * data, SNC_queues_snc_push() function has to be called in order to update the contents of the queue.
 *
 * \sa SNC_queues_snc_pop
 * \sa SNC_queues_snc_get_wq
 *
 */
#define SNC_queues_snc_push()                                                                   \
        _SNC_queues_snc_push()

/**
 * \brief Function used in SNC context in order to check if the SNC-to-CM33 (write) queue is full
 *
 * \param [out] q_is_full       (uint32_t*: use da() or ia())
 *                              the returned status (1 if the queue is full, 0 otherwise)
 *
 */
#define SNC_queues_snc_wq_is_full(q_is_full)                                                    \
        _SNC_queues_snc_wq_is_full(q_is_full)

/**
 * \brief Function used in SNC context in order to pop data from the CM33-to-SNC (read) queue
 *
 * A queue read pointer together with the data size and timestamp (if enabled),
 * should first be obtained using the SNC_queues_snc_get_rq() function.
 * After that, the user may read from the queue using that pointer. After reading the desired
 * data, SNC_queues_snc_pop() should be called in order to update the contents of the queue.
 *
 * \sa SNC_queues_snc_push
 * \sa SNC_queues_snc_get_rq
 *
 */
#define SNC_queues_snc_pop()                                                                    \
        _SNC_queues_snc_pop()

/**
 * \brief Function used in SNC context in order to check if the CM33-to-SNC (read) queue is empty
 *
 * \param [out] q_is_empty      (uint32_t*: use da() or ia())
 *                              the returned status (1 if the queue is empty, 0 otherwise)
 *
 */
#define SNC_queues_snc_rq_is_empty(q_is_empty)                                                  \
        _SNC_queues_snc_rq_is_empty(q_is_empty)

/**
 * \brief Function used in SNC context in order to obtain a write pointer to the SNC-to-CM33 (write)
 *        queue, and set the size of the data to be written, as well as a timestamp (if enabled)
 *
 * \param [out] wp              (uint32_t*: use da() or ia())
 *                              the returned write pointer (NULL if the queue is full)
 * \param [in]  size            (uint32_t: use da() or ia() or build-time-only value)
 *                              the size of the data in bytes that is to be written
 * \param [in]  timestamp       (uint32_t: use da() or ia() or build-time-only value)
 *                              the timestamp of the written data (if enabled)
 *
 * \sa SNC_queues_snc_push
 *
 */
#define SNC_queues_snc_get_wq(wp, size, timestamp)                                              \
        _SNC_queues_snc_get_wq(wp, size, timestamp)

/**
 * \brief Function used in SNC context in order to obtain the maximum number of bytes that a
 *        chunk can stores in the SNC-to-CM33 (write) queue
 *
 * \param [out] max_chunk_size_bytes    (uint32_t*: use da() or ia())
 *                                      the returned max chunk size in bytes
 *
 */
#define SNC_queues_snc_get_wq_max_chunk_bytes(max_chunk_size_bytes)                             \
        _SNC_queues_snc_get_wq_max_chunk_bytes(max_chunk_size_bytes)

/**
 * \brief Function used in SNC context in order to obtain a read pointer on the CM33-to-SNC (read)
 *        queue as well as the size and the timestamp (if enabled) of the data to be read/popped
 *
 * \param [out] rp              (uint32_t*: use da() or ia())
 *                              the returned read pointer (NULL if the queue is empty)
 * \param [out] size            (uint32_t*: use da() or ia())
 *                              the size of the data in bytes residing in the current queue position
 * \param [out] timestamp       (uint32_t*: use da() or ia())
 *                              the timestamp accompanying the data (if enabled)
 *
 * \sa SNC_queues_snc_pop
 *
 */
#define SNC_queues_snc_get_rq(rp, size, timestamp)                                              \
        _SNC_queues_snc_get_rq(rp, size, timestamp)

#endif /* dg_configUSE_SNC_QUEUES */

#endif /* dg_configUSE_HW_SENSOR_NODE */


#endif /* SNC_QUEUES_H_ */

/**
 * \}
 * \}
 */
