/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SNC_UTILS
 *
 * \brief Sensor Node Controller (SNC) utility/general purpose functions
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file snc_utils.h
 *
 * @brief SNC Utility/General Purpose functions header file
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SNC_UTILS_H_
#define SNC_UTILS_H_


#if dg_configUSE_HW_SENSOR_NODE

#include "SeNIS.h"

/*
 * ENUMERATION, DATA TYPE AND STRUCTURE DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief SNC-main-uCode context structure
 */
typedef struct snc_main_context_t {
        uint32_t* ucode;                /**< Pointer to the uCode starting address */

#if dg_configUSE_SNC_DEBUGGER
        uint32_t size;                  /**< uCode size in 32bit words */
#if dg_configUSE_HW_SENSOR_NODE_EMU
        snc_emu_dbg_attrs_t dbg_attrs;  /**< SNC emulator debugging attributes of the uCode */
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */
#endif /* dg_configUSE_SNC_DEBUGGER */

        /**< Function pointer initialized upon uCode definition (i.e. SNC_MAIN_DEF()),
         * used for calling the constructor function which creates the uCode
         */
        void (*ucode_create)(void);
        /**< Function pointer initialized upon uCode definition (i.e. SNC_MAIN_DEF()),
         * used for calling the destructor function which deletes the uCode and frees its
         * allocated resources
         */
        void (*ucode_delete)(void);
} snc_main_context_t;

/*
 * FUNCTION DECLARATIONS
 *****************************************************************************************
 */

//==================== uCode declaration macros ================================

/**
 * \brief SNC Function-uCode declaration macro with configurable number of attributes
 *
 * \param [in] name             the name of the Function-uCode
 * \param [in] ... args         (optional) arguments (e.g. uint32_t arg1, uint32_t* arg2)
 *
 */
#define SNC_FUNC_DECL(name, ...)                                                                \
        _SNC_FUNC_DECL(func, name, __VA_ARGS__)

/**
 * \brief SNC-main-uCode declaration macro
 *
 * \param [in] name             the name of the SNC-main-uCode
 *
 */
#define SNC_MAIN_DECL(name)                                                                     \
        _SNC_FUNC_DECL(main, name, void)

//==================== uCode definition macros =================================

/**
 * \brief SNC Function-uCode definition macro with configurable number of attributes
 *
 * \param [in] name             the name of the Function-uCode
 * \param [in] ... args         (optional) arguments (e.g. uint32_t arg1, uint32_t* arg2)
 *
 */
#define SNC_FUNC_DEF(name, ...)                                                                 \
        _SNC_FUNC_DEF(func, name, __VA_ARGS__)

/**
 * \brief SNC-main-uCode definition macro
 *
 * \param [in] name             the name of the SNC-main-uCode
 *
 */
#define SNC_MAIN_DEF(name)                                                                      \
        _SNC_FUNC_DEF(main, name)

//==================== Macros used for reference to special uCode structures ===

/**
 * \brief Macro used in SNC context for creating reference to Function-uCode arguments in order
 *        to be used inside SeNIS constructs
 *
 * \param [in] arg              the name of the argument
 *
 */
#define SNC_ARG(arg)                                                                            \
        _SNC_ARG(arg)

/**
 * \brief Macro used in SNC context for creating reference to uCode 32-bit word in order to be used
 *        inside SeNIS constructs
 *
 * \param [in] index            the index of the 32-bit word being referenced with respect to the
 *                              uCode body starting position
 *
 */
#define SNC_UCODE(index)                                                                        \
        _SNC_UCODE(index)

//==================== SNC uCode resources access manipulation macros ==========

/**
 * \brief Macro used in SNC context to acquire access over a uCode resource that is also accessed
 *        in SYSCPU (CM33) context
 *
 * \param [in] name             the name of the uCode-Block
 *
 * \sa SNC_UCODE_RSRC_RELEASE
 * \sa snc_acquire_SNC_ucode_rsrc
 * \sa snc_release_SNC_ucode_rsrc
 *
 */
#define SNC_UCODE_RSRC_ACQUIRE(name)                                                            \
        _SNC_UCODE_RSRC_ACQUIRE(name)

/**
 * \brief Macro used in SNC context to release access over a uCode resource that is also accessed
 *        in SYSCPU (CM33) context
 *
 * \param [in] name             the name of the uCode-Block
 *
 * \sa SNC_UCODE_RSRC_ACQUIRE
 * \sa snc_acquire_SNC_ucode_rsrc
 * \sa snc_release_SNC_ucode_rsrc
 *
 */
#define SNC_UCODE_RSRC_RELEASE(name)                                                            \
        _SNC_UCODE_RSRC_RELEASE(name)

//==================== Calling/returning-from-an-SNC-function functions ========

/**
 * \brief Function used in SNC context for implementing a jump from uCode to uCode as a function
 *        call, taking as arguments, apart from the name of the called Function-uCode, the values
 *        to be passed as parameters
 *
 * \param [in] ... func_name    the name of the Function-uCode
 * \param [in] ... params       (optional) list of parameter values separated by commas
 *
 */
#define SENIS_call(...)                                                                         \
        _SENIS_call(__VA_ARGS__)

//==================== Functions used for SYSCPU (CM33) to SNC uCodes communication =

/**
 * \brief Function used in SYSCPU context to get the status of the notified events to
 *        SYSCPU (CM33) by uCodes
 *
 * When a uCode sends an event to SYSCPU using SNC_CM33_NOTIFY() macro, it sets a bit in SNC
 * context, the position of which is equal to its ID.
 *
 * \return the status of the notified events to SYSCPU (CM33) by uCodes
 *
 * \sa SNC_CM33_NOTIFY
 *
 */
uint32_t snc_get_SNC_to_CM33_trigger(void);

/**
 * \brief Function used in SYSCPU context to clear the bit indicating an event from an SNC uCode to
 *        SYSCPU (CM33)
 *
 * \param [in] ucode_ctx        the context of the targeted uCode
 *
 * \sa SNC_CM33_NOTIFY
 * \sa snc_get_SNC_to_CM33_trigger
 *
 */
void snc_clear_SNC_to_CM33_trigger(snc_ucode_context_t* ucode_ctx);

/**
 * \brief Function used in SYSCPU context to indicate an event from SYSCPU (CM33) to an SNC uCode
 *
 * When this function is called, the CM33_to_SNC_triggered flag in the context of the targeted
 * uCode is set, indicating an event.
 *
 * \param [in] ucode_ctx        the context of the targeted uCode
 *
 * \sa SNC_CHECK_CM33_NOTIF_PENDING
 *
 */
void snc_notify_SNC_ucode(snc_ucode_context_t* ucode_ctx);

//==================== Functions used for SNC uCodes control ===================

/**
 * \brief Function used in SYSCPU context to acquire access over a resource that is also accessed
 *        by an SNC uCode
 *
 * \param [in] ucode_ctx        the context of the uCode
 *
 * \sa snc_release_SNC_ucode_rsrc
 * \sa SNC_UCODE_RSRC_ACQUIRE
 * \sa SNC_UCODE_RSRC_RELEASE
 *
 */
void snc_acquire_SNC_ucode_rsrc(snc_ucode_context_t* ucode_ctx);

/**
 * \brief Function used in SYSCPU context to release access over a resource that is also accessed
 *        by an SNC uCode
 *
 * \param [in] ucode_ctx        the context of the uCode
 *
 * \sa snc_acquire_SNC_ucode_rsrc
 * \sa SNC_UCODE_RSRC_ACQUIRE
 * \sa SNC_UCODE_RSRC_RELEASE
 *
 */
void snc_release_SNC_ucode_rsrc(snc_ucode_context_t* ucode_ctx);

/**
 * \brief Function used in SYSCPU context to enable the execution of an SNC uCode
 *
 * When this function is called, the SNC_UCODE_BLOCK_CMD_DISABLE command flag in the context of
 * the targeted uCode is cleared, indicating that the uCode can be executed (e.g. when the PDC
 * event to which it is registered is triggered).
 *
 * \param [in] ucode_ctx        the context of the uCode
 *
 * \sa snc_disable_SNC_ucode
 * \sa SNC_UCODE_BLOCK_FLAG
 * \sa SNC_UCODE_BLOCK_CMD
 *
 * \note Enabling/disabling an SNC uCode and setting appropriately the "RUNNING" flag
 *       must be handled in SNC-main-uCode.
 */
void snc_enable_SNC_ucode(snc_ucode_context_t* ucode_ctx);

/**
 * \brief Function used in SYSCPU context to disable the execution of an SNC uCode
 *
 * When this function is called, the SNC_UCODE_BLOCK_CMD_DISABLE command flag in the context of
 * the targeted uCode is set, indicating that the uCode execution is suspended (e.g. when the PDC
 * event to which it is registered is triggered).
 *
 * \param [in] ucode_ctx        the context of the uCode
 *
 * \sa snc_enable_SNC_ucode
 * \sa SNC_UCODE_BLOCK_FLAG
 * \sa SNC_UCODE_BLOCK_CMD
 *
 * \note Enabling/disabling an SNC uCode and setting appropriately the "RUNNING" flag
 *       must be handled in SNC-main-uCode.
 *
 */
void snc_disable_SNC_ucode(snc_ucode_context_t* ucode_ctx);

#if dg_configUSE_SNC_DEBUGGER
//==================== Functions used for SNC breakpoints ======================

/**
 * \brief Function used in SYSCPU context to process an SNC breakpoint
 *
 * This function should be called inside the Sensor Node Handler. It will check if an
 * SNC breakpoint occurred. If it did, then it will un-halt the SNC and clear snc_bkpt.
 *
 */
void snc_process_bkpt(void);

#if dg_configUSE_HW_SENSOR_NODE_EMU
/**
 * \brief Function used in SYSCPU context to process an SNC Emulator step-by-step debugging
 *        breakpoint given the current SNC Emulator Program Counter (PC) value
 *
 * \param [in] pc       the SNC program counter
 *
 * \sa SNC_BKPT_GROUP_EMU()
 *
 */
void snc_process_bkpt_emu(uint32_t pc);
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */
#endif /* dg_configUSE_SNC_DEBUGGER */

#if dg_configUSE_SNC_QUEUES
//==================== Functions used for SNC queues ===========================

/**
 * \brief Function used in SYSCPU context to create an SNC queue
 *
 * \param [in] snc_queue_cfg    the desired queue configuration
 *
 * \return snc_queue_t          a pointer to the created queue (NULL if queue was not created)
 */
snc_queue_t snc_queues_cm33_create(const snc_queue_config_t *snc_queue_cfg);

/**
 * \brief Function used in SYSCPU context to update the configuration of an SNC queue
 *
 * \param [in] snc_queue        a pointer to the queue to update
 * \param [in] snc_queue_cfg    the desired queue configuration
 */
void snc_queues_cm33_update(snc_queue_t snc_queue, const snc_queue_config_t *snc_queue_cfg);

/**
 * \brief Function used in SYSCPU context to destroy an SNC queue
 *
 * \param [in] snc_queue        a pointer to the queue to destroy
 */
void snc_queues_cm33_destroy(snc_queue_t snc_queue);

/**
 * \brief Function used in SYSCPU context to reset an SNC queue
 *
 * \param [in] snc_queue        a pointer to the queue to reset
 */
void snc_queues_cm33_reset(snc_queue_t snc_queue);

/**
 * \brief Function used in SYSCPU context to push data to an SNC queue
 *
 * \param [in] snc_queue        a pointer to the queue to push data
 * \param [in] pData            a pointer to the data to push
 * \param [in] size             the size of the pushed data in bytes
 * \param [in] timestamp        a data timestamp (if the queue supports data timestamping)
 *
 * \return bool                 true if the data has been pushed successfully,
 *                              false if the queue is full
 */
bool snc_queues_cm33_push(snc_queue_t snc_queue, const uint8_t *pData, size_t size, uint32_t timestamp);

/**
 * \brief Function used in SYSCPU context to pop data from an SNC queue
 *
 * This function will pop to a destination buffer the amount of bytes residing
 * in the current chunk of an SNC queue. In order to obtain the number of bytes
 * residing in the chunk, the user may call the snc_queues_cm33_get_cur_chunk_bytes() function.
 *
 * \param [in] snc_queue        a pointer to the queue to pop data from
 * \param [out] pData           a pointer to where the popped data will be placed
 * \param [out] rsize           the size of the popped data in bytes
 * \param [out] timestamp       the data timestamp (if the queue supports data timestamping)
 *
 * \return bool                 true if the data has been popped successfully,
 *                              false if the queue is empty
 *
 * \sa snc_queues_cm33_get_cur_chunk_bytes
 * \sa snc_queues_cm33_push
 */
bool snc_queues_cm33_pop(snc_queue_t snc_queue, uint8_t *pData, uint32_t *rsize, uint32_t *timestamp);

/**
 * \brief Function used in SYSCPU context to check if an SNC queue is empty
 *
 * \param [in] snc_queue        a pointer to the queue to check
 *
 * \return bool                 true if the queue is empty, false otherwise
 */
bool snc_queues_cm33_queue_is_empty(snc_queue_t snc_queue);

/**
 * \brief Function used in SYSCPU context to check if an SNC queue is full
 *
 * \param [in] snc_queue        a pointer to the queue to check
 *
 * \return bool                 true if the queue is full, false otherwise
 */
bool snc_queues_cm33_queue_is_full(snc_queue_t snc_queue);

/**
 * \brief Function used in SYSCPU context to get the number of free (not written/allocated) chunks
 *        of an SNC queue
 *
 * \param [in] snc_queue        a pointer to the queue
 *
 * \return uint32_t             the number of free chunks
 */
uint32_t snc_queues_cm33_get_free_chunks(snc_queue_t snc_queue);

/**
 * \brief Function used in SYSCPU context to get the number of allocated (written) chunks
 *        of an SNC queue
 *
 * \param [in] snc_queue        a pointer to the queue to check
 *
 * \return uint32_t             the number of the allocated chunks
 */
uint32_t snc_queues_cm33_get_alloc_chunks(snc_queue_t snc_queue);

/**
 * \brief Function used in SYSCPU context to get the number of bytes in the current chunk
 *        (i.e. the next chunk to be popped) of an SNC queue
 *
 * \param [in] snc_queue        a pointer to the queue to check
 *
 * \return uint32_t             the number of bytes in the chunk
 */
uint32_t snc_queues_cm33_get_cur_chunk_bytes(snc_queue_t snc_queue);
#endif /* dg_configUSE_SNC_QUEUES */

#endif /* dg_configUSE_HW_SENSOR_NODE */


#endif /* SNC_UTILS_H_ */

/**
 * \}
 * \}
 */
