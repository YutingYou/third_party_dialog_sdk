/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SNC_MAIN
 *
 * \brief Sensor Node Controller (SNC) main uCode
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file snc_main.h
 *
 * @brief Sensor Node Controller (SNC) main uCode API
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SNC_MAIN_H_
#define SNC_MAIN_H_


#if dg_configUSE_HW_SENSOR_NODE

#include "snc_defs.h"

/*
 * DATA TYPE DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief PDC event priorities.
 *
 */
typedef enum {
        SNC_MAIN_PDC_EVT_PR_1 = 0,      /**< Highest priority */
        SNC_MAIN_PDC_EVT_PR_2 = 1,      /**< Mid priority */
        SNC_MAIN_PDC_EVT_PR_3 = 2,      /**< Lowest priority */

        SNC_MAIN_PDC_EVT_PR_0           /**< No priority */
} SNC_MAIN_PDC_EVT_PRIORITY;

/**
 * \brief Bitmap position of the uCode operation flags tracked by SNC-main-uCode.
 *
 */
typedef enum {
        SNC_MAIN_FLAG_POS_XTAL32M_REQ = 0, /**< XTAL32M is required */
} SNC_MAIN_FLAG_POS;

/**
 * \brief uCode operation flags tracked by SNC-main-uCode.
 *
 */
typedef enum {
        /**< XTAL32M is required */
        SNC_MAIN_FLAG_XTAL32M_REQ = (1 << SNC_MAIN_FLAG_POS_XTAL32M_REQ),
} SNC_MAIN_FLAG;

/**
 * \brief SNC-main-uCode entry.
 *
 * \sa ad_snc_ucode_t
 *
 */
typedef struct snc_main_ucode_entry_t {
        /**< Pointer to the uCode context addressed by the entry in the SNC-main-uCode list */
        snc_ucode_context_t *ucode_context;

        /**< uCode operation flags (of type SNC_MAIN_FLAGS):
         *   bit0 - Flag indicating requirement for operation with XTAL32M enabled
         *
         * \parblock
         *         Bit:      |  31 - 1  |     0     |
         *                   +----------+-----------+
         *         Event     | reserved |  xtal_req |
         *                   +----------+-----------+
         * \endparblock
         */
        uint32_t flags;

        /**< Pointer to the next uCode entry in the SNC-main-uCode list */
        void *next;
} snc_main_ucode_entry_t;

/**
 * \brief PDC event element. It includes its ID and the starting address of the registered for the
 *        PDC event uCode-Blocks list
 *
 */
typedef struct snc_main_pdc_evt_entry_t {
        /**< Used by the SNC-main-uCode to perform RDCBI and check PDC pending register.
         * It is evaluated when snc_main_set_pdc_evnt_entries() is called
         */
        uint32_t pdc_event_id_shifted;

        /**< Used by the SNC-main-uCode to perform TOBRE and clear PDC pending entries value.
         * It is evaluated when snc_main_set_pdc_evnt_entries() is called
         */
        uint32_t pdc_event_id_idx;

        /**< The ID of the PDC event, used by the SNC-main-uCode to acknowledge the event when it is pending */
        uint32_t pdc_event_id;

        /**< The first uCode to be executed when the PDC event is triggered */
        snc_main_ucode_entry_t *first_ucode;
} snc_main_pdc_evt_entry_t;

/*
 * FUNCTION DECLARATIONS
 *****************************************************************************************
 */

/**
 * \brief SNC-main-uCode set-halt function
 *
 * Halt the SNC-main-uCode, so that PDC events or uCodes can be registered or unregistered
 *
 */
void snc_main_set_halt(void);

/**
 * \brief SNC-main-uCode reset-halt function
 *
 * Reset the SNC-main-uCode halt, so that the SNC-main-uCode can continue execution
 *
 */
void snc_main_reset_halt(void);

/**
 * \brief SNC-main-uCode check halt function
 *
 * \return true if halt is requested for the SNC-main-uCode
 *
 */
bool snc_main_is_halt_set(void);

/**
 * \brief SNC-main-uCode registered PDC event entries update function
 *
 * Register to the SNC-main-uCode an array of PDC events of specific priority
 *
 * \param [in] priority         PDC events priority
 * \param [in] pdc_events       PDC events for the given priority
 * \param [in] numEvents        number of PDC events
 *
 */
void snc_main_set_pdc_evnt_entries(SNC_MAIN_PDC_EVT_PRIORITY priority,
        snc_main_pdc_evt_entry_t* pdc_events, uint32_t numEvents);

/**
 * \brief Function used in order to enable the execution of an SNC uCode
 *
 * When this function is called, the execution of the given uCode is enabled, that is,
 * the SNC-main-uCode will branch to the starting address of the uCode once the corresponding PDC
 * event is triggered.
 *
 * \param [in] ucode_entry      the uCode entry registered to the SNC-main-uCode
 *
 * \sa snc_main_disable_ucode_entry
 *
 */
void snc_main_enable_ucode_entry(snc_main_ucode_entry_t* ucode_entry);

/**
 * \brief Function used in order to disable the execution of an SNC uCode
 *
 * When this function is called, the execution of the given uCode is disabled, that is,
 * the SNC-main-uCode will NOT branch to the starting address of the uCode once the corresponding PDC
 * event is triggered.
 *
 * \param [in] ucode_entry      the uCode entry registered to the SNC-main-uCode
 *
 * \sa snc_main_enable_ucode_entry
 *
 */
void snc_main_disable_ucode_entry(snc_main_ucode_entry_t* ucode_entry);

/**
 * \brief SNC-main-uCode declaration
 *
 * Use SNC_UCODE_CTX(snc_main) macro to get the context of the SNC-main-uCode
 *
 * \return pointer to the context of the SNC-main-uCode (snc_main_context_t*)
 *
 * \sa ad_snc_ucode_register
 * \sa SNC_UCODE_CTX
 *
 */
SNC_MAIN_DECL(snc_main);

#endif /* dg_configUSE_HW_SENSOR_NODE */


#endif /* SNC_MAIN_H_ */

/**
 * \}
 * \}
 */
