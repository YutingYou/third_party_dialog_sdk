/**
 * \addtogroup MID_SYS_ADAPTERS
 * \{
 * \addtogroup SNC_ADAPTER SNC Adapter
 *
 * \brief Sensor Node Controller (SNC) Adapter
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_snc.h
 *
 * @brief Sensor Node Controller (SNC) Adapter API
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef AD_SNC_H_
#define AD_SNC_H_


#if dg_configSNC_ADAPTER

#include "snc_defs.h"
#include "hw_gpio.h"
#include "hw_timer.h"
#include "snc_main.h"

/** uCode table size */
#define AD_SNC_UCT_SIZE                 (16)
/** PDC events array size */
#define AD_SNC_PDC_EVT_AR_SIZE          (16)
/** Value for invalid uCode ID */
#define AD_SNC_UCODE_ID_INVALID         ((uint32_t)-1)

/**
 * \brief Application interrupt callback function type
 */
typedef void (*ad_snc_interrupt_cb)(void);

/**
 * \brief PDC event priorities
 */
typedef enum {
        AD_SNC_PDC_EVT_PR_1 = 0,        /**< Highest priority */
        AD_SNC_PDC_EVT_PR_2 = 1,        /**< Mid priority */
        AD_SNC_PDC_EVT_PR_3 = 2,        /**< Lowest priority */
        AD_SNC_PDC_EVT_PR_0 = 3,        /**< No priority, can be used for round robin */
} AD_SNC_PDC_EVT_PRIORITY;

/**
 * \brief uCode execution priorities
 */
typedef enum {
        AD_SNC_UCODE_PR_0,              /**< No priority */
        AD_SNC_UCODE_PR_15,             /**< Lowest priority */
        AD_SNC_UCODE_PR_14,
        AD_SNC_UCODE_PR_13,
        AD_SNC_UCODE_PR_12,
        AD_SNC_UCODE_PR_11,
        AD_SNC_UCODE_PR_10,
        AD_SNC_UCODE_PR_9,
        AD_SNC_UCODE_PR_8,
        AD_SNC_UCODE_PR_7,
        AD_SNC_UCODE_PR_6,
        AD_SNC_UCODE_PR_5,
        AD_SNC_UCODE_PR_4,
        AD_SNC_UCODE_PR_3,
        AD_SNC_UCODE_PR_2,
        AD_SNC_UCODE_PR_1,              /**< Highest priority */
} AD_SNC_UCODE_PRIORITY;

/**
 * \brief SNC queues type
 */
typedef enum {
        AD_SNC_QUEUE_TYPE_CM33_SNC,     /**< Queue is implementing communication from CM33 to SNC */
        AD_SNC_QUEUE_TYPE_SNC_CM33      /**< Queue is implementing communication from SNC to CM33 */
} AD_SNC_QUEUE_TYPE;

/**
 * \brief uCode-Block configuration
 *
 * A Global variable of this type must be defined in application space for each uCode-Block.
 */
typedef struct {
        /**< PDC LUT event entry field, packed by using the macro HW_PDC_LUT_ENTRY_VAL */
        uint32_t pdc_entry;
        AD_SNC_PDC_EVT_PRIORITY pdc_evt_pr;             /**< PDC event priority */
        AD_SNC_UCODE_PRIORITY ucode_pr;                 /**< uCode priority */
        /**< Application callback to be called as programmed by uCode execution */
        ad_snc_interrupt_cb cb;
#if dg_configUSE_SNC_QUEUES
        /**< uCode's SNC-to-CM33 data exchange queue configuration */
        snc_queue_config_t snc_to_cm33_queue_cfg;
        /**< uCode's CM33-to-SNC data exchange queue configuration */
        snc_queue_config_t cm33_to_snc_queue_cfg;
#endif /* dg_configUSE_SNC_QUEUES */
} ad_snc_ucode_cfg_t;

/**
 * \brief SNC Adapter initialization function, to be called before uCode registration
 */
void ad_snc_init(void);

/**
 * \brief uCode registration function
 *
 * To be called by the application task to register a uCode-Block to a PDC event
 *
 * \param [in] cfg              uCode configuration parameters
 * \param [in] ucode_ctx        pointer to the uCode context
 *
 * \return uint32_t             the uCode ID that has been assigned to the successfully created
 *                              uCode, otherwise AD_SNC_UCODE_ID_INVALID
 *
 * \sa ad_snc_ucode_unregister
 */
uint32_t ad_snc_ucode_register(ad_snc_ucode_cfg_t *cfg, snc_ucode_context_t *ucode_ctx);

/**
 * \brief uCode un-registration function
 *
 * To be called by the application task to un-register a uCode from a PDC event
 *
 * \param [in] ucode_id         uCode ID of the registered uCode
 *
 * \return bool                 true if the uCode has been unregistered successfully; false if not
 *
 * \sa ad_snc_ucode_register
 */
bool ad_snc_ucode_unregister(uint32_t ucode_id);

/**
 * \brief Set a PDC LUT entry where a uCode has been registered as pending
 *
 * It will force-trigger the execution of all uCodes that have been registered to the PDC event
 * being set as pending. SNC_CHECK_CM33_NOTIF_PENDING() macro function can be used in SNC
 * execution context in order to check if the implied notification targets the particular uCode.
 *
 * \param [in] ucode_id         uCode ID of the registered uCode
 *
 * \return bool                 true if the PDC event has been set as pending successfully
 *
 * \sa ad_snc_ucode_register
 * \sa SNC_CHECK_CM33_NOTIF_PENDING
 */
bool ad_snc_pdc_set_pending(uint32_t ucode_id);

/**
 * \brief Enable uCode execution
 *
 * \param [in] ucode_id         uCode ID of the registered uCode
 *
 * \return bool                 true if the uCode has been indicated as enabled to execute successfully
 *
 * \sa ad_snc_ucode_disable
 */
bool ad_snc_ucode_enable(uint32_t ucode_id);

/**
 * \brief Disable uCode execution
 *
 * \param [in] ucode_id         uCode ID of the registered uCode
 *
 * \return bool                 true if the uCode has been indicated as disabled to execute successfully
 *
 * \sa ad_snc_ucode_enable
 */
bool ad_snc_ucode_disable(uint32_t ucode_id);

#if dg_configUSE_SNC_QUEUES
/**
 * \brief Update an SNC queue configuration for a registered uCode-Block
 *
 * \param [in] ucode_id         uCode ID of the registered uCode
 * \param [in] qType            type of the queue to be created
 * \param [in] cfg              queue configuration parameters
 *
 * \return bool                 true if the SNC queue has been successfully updated
 */
bool ad_snc_queue_update(uint32_t ucode_id, AD_SNC_QUEUE_TYPE qType, const snc_queue_config_t *cfg);

/**
 * \brief Reset uCode-Block's SNC-to-CM33 queue or CM33-to-SNC queue data
 *
 * \param [in] ucode_id         uCode ID of the registered uCode
 * \param [in] qType            type of the queue
 *
 * \return bool                 true if the SNC queue has been successfully reset
 */
bool ad_snc_queue_reset(uint32_t ucode_id, AD_SNC_QUEUE_TYPE qType);

/**
 * \brief Push data into a uCode-Block's CM33-to-SNC queue
 *
 * \param [in] ucode_id         uCode ID of the registered uCode
 * \param [in] data             pointer to the data to be pushed
 * \param [in] size             size of the data in bytes
 * \param [in] timestamp        timestamp accompanying the pushed data
 *                              (if the queue supports timestamping)
 *
 * \return bool                 true if the data has been pushed successfully into the queue
 *
 * \sa ad_snc_queue_pop
 */
bool ad_snc_queue_push(uint32_t ucode_id, const uint8_t *data, size_t size, uint32_t timestamp);

/**
 * \brief Pop data from a uCode-Block's SNC-to-CM33 queue
 *
 * \param [in] ucode_id         uCode ID of the registered uCode
 * \param [out] data            pointer to where the popped data will be stored
 * \param [out] size            size of the popped data in bytes
 * \param [out] timestamp       timestamp accompanying the popped data
 *                              (if the queue supports timestamping)
 *
 * \return bool                 true if the data has been popped successfully from the queue
 *
 * \sa ad_snc_queue_push
 */
bool ad_snc_queue_pop(uint32_t ucode_id, uint8_t *data, uint32_t *size, uint32_t *timestamp);

/**
 * \brief Checks if a uCode-Block's SNC-to-CM33 queue or CM33-to-SNC queue is empty
 *
 * \param [in] ucode_id         uCode ID of the registered uCode
 * \param [in] qType            type of the queue to be checked
 *
 * \return bool                 true if the queue is empty, false otherwise
 *
 * \sa ad_snc_queue_is_full
 */
bool ad_snc_queue_is_empty(uint32_t ucode_id, AD_SNC_QUEUE_TYPE qType);

/**
 * \brief Checks if a uCode-Block's SNC-to-CM33 queue or CM33-to-SNC queue is full
 *
 * \param [in] ucode_id         uCode ID of the registered uCode
 * \param [in] qType            type of the queue
 *
 * \return bool                 true if the queue is full, false otherwise
 *
 * \sa ad_snc_queue_is_empty
 */
bool ad_snc_queue_is_full(uint32_t ucode_id, AD_SNC_QUEUE_TYPE qType);

/**
 * \brief Returns the number of free (not written/allocated) chunks in a uCode-Block's
 *        SNC-to-CM33 queue or CM33-to-SNC queue
 *
 * \param [in] ucode_id         uCode ID of the registered uCode
 * \param [in] qType            type of the queue
 *
 * \return uint32_t             the number of free chunks
 *
 * \sa ad_snc_queue_get_alloc_chunks
 */
uint32_t ad_snc_queue_get_free_chunks(uint32_t ucode_id, AD_SNC_QUEUE_TYPE qType);

/**
 * \brief Returns the number of allocated (written) chunks in a uCode-Block's
 *        SNC-to-CM33 queue or CM33-to-SNC queue
 *
 * \param [in] ucode_id         uCode ID of the registered uCode
 * \param [in] qType            type of the queue
 *
 * \return uint32_t             the number of allocated chunks
 *
 * \sa ad_snc_queue_get_free_chunks
 */
uint32_t ad_snc_queue_get_alloc_chunks(uint32_t ucode_id, AD_SNC_QUEUE_TYPE qType);

/**
 * \brief Returns the number of bytes in the current chunk (i.e. the next chunk to be popped)
 *        of a uCode-Block's SNC-to-CM33 queue or CM33-to-SNC queue
 *
 * \param [in] ucode_id         uCode ID of the registered uCode
 * \param [in] qType            type of the queue
 *
 * \return uint32_t             the number of bytes in the current chunk
 */
uint32_t ad_snc_queue_get_cur_chunk_bytes(uint32_t ucode_id, AD_SNC_QUEUE_TYPE qType);
#endif /* dg_configUSE_SNC_QUEUES */

#if dg_configUSE_HW_TIMER
/**
 * \brief Converts a system timer SNC uptime ticks to CM33 uptime ticks.
 *
 * \param [in] snc_uptime_ticks    the system timer uptime_ticks acquired in SNC context
 *
 * \return The uptime ticks. Each tick will be 1000000 / configSYSTICK_CLOCK_HZ
 *         (e.g. 30.5us if XTAL32K is set as LP clock)
 *
 */
uint64_t ad_snc_sys_timer_get_uptime_ticks(snc_hw_sys_uptime_ticks_t* snc_uptime_ticks);

/**
 * \brief Converts a system timer SNC timestamp to CM33 timestamp
 *
 * This function converts a system timer SNC timestamp to CM33 timestamp.
 * This is expressed in ticks of the clock that clocks OS timer (e.g. XTAL32K). For example,
 * if XTAL32K drives OS timer, each timestamp tick will be 1000000 / 32768 = 30.5uS
 *
 * \param [in] snc_timestamp    the system timer timestamp acquired in SNC context
 *
 * \return uint64_t the current timestamp
 * \deprecated This function is deprecated. User shall call ad_snc_sys_timer_get_uptime_ticks() instead
 */
DEPRECATED_MSG("API no longer supported, use ad_snc_sys_timer_get_uptime_ticks() instead.")
__STATIC_INLINE uint64_t ad_snc_sys_timer_get_timestamp(void* snc_timestamp)
{
        return ad_snc_sys_timer_get_uptime_ticks((snc_hw_sys_uptime_ticks_t *)snc_timestamp);
}
#endif /* dg_configUSE_HW_TIMER */

#endif /* dg_configSNC_ADAPTER */


#endif /* AD_SNC_H_ */

/**
 * \}
 * \}
 */
