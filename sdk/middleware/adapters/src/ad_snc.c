/**
 * \addtogroup MID_SYS_ADAPTERS
 * \{
 * \addtogroup SNC_ADAPTER SNC Adapter
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_snc.c
 *
 * @brief Sensor Node Controller (SNC) Adapter implementation
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#if dg_configSNC_ADAPTER

#include <string.h>
#include "sys_power_mgr.h"
#include "sys_watchdog.h"
#include "osal.h"
#include "ad_snc.h"
#include "hw_pdc.h"
#if dg_configUSE_HW_SENSOR_NODE_EMU
#include "snc_emu.h"
#else
#include "hw_snc.h"
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */

#define AD_SNC_PDC_ENABLE_PD_MASK       (HW_PDC_LUT_ENTRY_EN_COM | HW_PDC_LUT_ENTRY_EN_PER | \
                                         HW_PDC_LUT_ENTRY_EN_TMR | HW_PDC_LUT_ENTRY_EN_XTAL)
#define AD_SNC_UNUSED_UCT_ENTRY_VALUE   (NULL)
#define AD_SNC_PDC_EVT_AR_INDEX_INVALID HW_PDC_INVALID_LUT_INDEX
#define AD_SNC_UCT_INDEX_INVALID        AD_SNC_UCODE_ID_INVALID
#define AD_SNC_UCODE_REG_IN_PROGRESS    (1)
#define OS_MUTEX_GET_COUNT(mutex)       uxSemaphoreGetCount((mutex))

__RETAINED static bool initialized;
__RETAINED static OS_MUTEX ad_snc_mutex;
__RETAINED static OS_EVENT ad_snc_task_event;

#if dg_configUSE_HW_SENSOR_NODE_EMU

#define AD_SNC_EMULATOR_NOTIFICATION    (1 << 0)

#define SNC_EMU_TASK_PRIORITY           (OS_TASK_PRIORITY_NORMAL)

__RETAINED static OS_TASK prvSensorNodeEMUTask_h;

/*
 * Sensor Node emulator Task function
 */
static void prvSensorNodeEMUTask(void *pvParameters);

#endif // dg_configUSE_HW_SENSOR_NODE_EMU

#define AD_SNC_IRQ_NOTIFICATION         (1 << 1)
#define AD_SNC_CMD_NOTIFICATION         (1 << 2)

#define SNC_AD_TASK_PRIORITY            (OS_TASK_PRIORITY_NORMAL)
#define SNC_AD_TASK_IRQ_PRIORITY        (OS_TASK_PRIORITY_NORMAL)

static void prvSensorNodeAdapterTask(void *pvParameters);
static void prvSensorNodeAdapterIRQTask(void *pvParameters);

__RETAINED static OS_TASK prvSensorNodeAdapterIRQTask_h;

/**
 * \brief uCode-Block data structure
 */
typedef struct {
        /**< DO NOT MOVE, needs to be first element. To be used by SNC */
        snc_main_ucode_entry_t snc_ucode;
        uint32_t ucode_id;              /**< uCode id and index to the uCode LUT */
        uint32_t pdc_entry;             /**< PDC LUT entry field, packed by using the macro HW_PDC_LUT_ENTRY_VAL */
        uint32_t pdc_lut_idx;           /**< PDC LUT entry idx */
        uint32_t pdc_array_idx;         /**< PDC array idx */
        AD_SNC_UCODE_PRIORITY ucode_pr; /**< uCode priority, set by application */
        /**< PDC event priority set by application */
        AD_SNC_PDC_EVT_PRIORITY pdc_evt_pr;
        OS_MUTEX mutex;                 /**< Mutex used for the control of uCode access by different tasks */
        ad_snc_interrupt_cb cb;         /**< Application callback to be called as programmed by uCode execution */
} ad_snc_ucode_t;

/**
 * \brief SNC Adapter environment structure
 */
typedef struct {
        /**< uCode table. Holds the registered uCodes */
        ad_snc_ucode_t *ad_snc_uct[AD_SNC_UCT_SIZE];
        /**< PDC events array */
        snc_main_pdc_evt_entry_t ad_snc_pdc_evt_array[AD_SNC_PDC_EVT_AR_SIZE];
        bool snc_irq_registered;        /**< True when at least one uCode has been registered */
        uint32_t snc_cm33_pdc_lut_idx;  /**< Stores the PDC LUT entry for SNC-to-CM33 trigger */
} ad_snc_env_t;

/**
 * \brief Sensor Node Adapter environment
 */
__RETAINED static ad_snc_env_t ad_snc_env;

/**
 * \brief SNC Adapter interface
 */
typedef struct {
        OS_TASK  task;                  /**< SNC Adapter task handle */
        OS_QUEUE task_cmd_msg_q;        /**< SNC Adapter task command message queue */
} ad_snc_interface_t;

__RETAINED static ad_snc_interface_t ad_snc_if;

/**
 * \brief uCode command enumeration type
 */
typedef enum {
        AD_SNC_CMD_UCODE_REG,           /**< uCode registration to PDC event command */
        AD_SNC_CMD_UCODE_UNREG,         /**< uCode un-registration from PDC event command */
} AD_SNC_CMD_TYPE;

/**
 * \brief Definition of SNC Adapter task command message
 */
typedef struct {
        AD_SNC_CMD_TYPE cmd;            /**< Command type */
        void *cmd_attrs;                /**< Pointer to the command attributes */
} ad_snc_cmd_msg_t;

/**
 * \brief Definition of SNC Adapter uCode registration command
 */
typedef struct {
        ad_snc_ucode_cfg_t *cfg;        /**< uCode configuration parameters */
        snc_ucode_context_t *ucode_ctx; /**< uCode context */

        /**< The uCode ID returned after a uCode registration request */
        uint32_t ucode_id;
} ad_snc_ucode_reg_cmd_t;

/**
 * \brief Definition of SNC Adapter uCode un-registration command
 */
typedef struct {
        uint32_t ucode_id;              /**< The uCode ID of the uCode to unregister */
        bool stat;                      /**< The returned uCode unregister status */
} ad_snc_ucode_unreg_cmd_t;

/*
 * Converts SNC Adapter PDC event priority to SNC PDC event priority
 */
static SNC_MAIN_PDC_EVT_PRIORITY ad_snc_convert_pdc_evt_priority(AD_SNC_PDC_EVT_PRIORITY priority)
{
        SNC_MAIN_PDC_EVT_PRIORITY rtn_priority = SNC_MAIN_PDC_EVT_PR_0;

        switch (priority) {
        case AD_SNC_PDC_EVT_PR_1:
                rtn_priority = SNC_MAIN_PDC_EVT_PR_1;
                break;
        case AD_SNC_PDC_EVT_PR_2:
                rtn_priority = SNC_MAIN_PDC_EVT_PR_2;
                break;
        case AD_SNC_PDC_EVT_PR_3:
                rtn_priority = SNC_MAIN_PDC_EVT_PR_3;
                break;
        case AD_SNC_PDC_EVT_PR_0:
                rtn_priority = SNC_MAIN_PDC_EVT_PR_0;
                break;
        default:
                OS_ASSERT(0); // Invalid priority
                break;
        }

        return rtn_priority;
}

/*
 * This function is called when a uCode is added to (or removed from) a PDC event entry list of
 * "uCodes-to-execute" and the PDC ENABLE PD flags, for that particular PDC event, need to be
 * rebuild
 */
static uint32_t ad_snc_rebuild_pdc_event_enable_pd_flags(uint32_t pdc_array_idx)
{
        ad_snc_ucode_t *list_el;
        uint32_t pdc_entry = 0;

        if (pdc_array_idx != AD_SNC_PDC_EVT_AR_INDEX_INVALID) {
                if (ad_snc_env.ad_snc_pdc_evt_array[pdc_array_idx].first_ucode) {
                        // Traverse list and rebuild PDC event ENABLE PD mask
                        list_el = (ad_snc_ucode_t*)ad_snc_env.ad_snc_pdc_evt_array[pdc_array_idx].first_ucode;

                        while (list_el) {
                                pdc_entry |= list_el->pdc_entry;
                                list_el = list_el->snc_ucode.next;
                        }
                }
        }
        return pdc_entry;
}

/*
 * Returns the number of PDC event entries that have no priority (AD_SNC_PDC_EVT_PR_0)
 */
static uint8_t ad_snc_get_num_pdc_evt_entries_with_no_pr()
{
        uint32_t pdc_array_idx = AD_SNC_PDC_EVT_PR_0;
        uint8_t cnt = 0;

        while ((pdc_array_idx < AD_SNC_PDC_EVT_AR_SIZE) &&
                ad_snc_env.ad_snc_pdc_evt_array[pdc_array_idx].first_ucode) {
                if (ad_snc_env.ad_snc_pdc_evt_array[pdc_array_idx].first_ucode) {
                        cnt++;
                }
                pdc_array_idx++;
        }

        return cnt;
}

/*
 * Removes PDC event from the PDC events array (ad_snc_pdc_evt_array)
 */
static void ad_snc_remove_pdc_evt_from_array(uint32_t pdc_array_idx, uint32_t pdc_lut_idx)
{
        hw_pdc_acknowledge(pdc_lut_idx);
        hw_pdc_remove_entry(pdc_lut_idx);

        if (pdc_array_idx >= AD_SNC_PDC_EVT_PR_0) {
                /* PDC events with priority AD_SNC_PDC_EVT_PR_0 can have multiple entries in the array.
                 * The other priorities have only one entry and no shorting is needed.
                 */
                if (ad_snc_env.ad_snc_pdc_evt_array[pdc_array_idx + 1].first_ucode) {
                        do {
                                ad_snc_env.ad_snc_pdc_evt_array[pdc_array_idx] =
                                        ad_snc_env.ad_snc_pdc_evt_array[pdc_array_idx + 1];
                                ((ad_snc_ucode_t*)(ad_snc_env.ad_snc_pdc_evt_array[pdc_array_idx].first_ucode))->pdc_array_idx =
                                        pdc_array_idx;

                                snc_main_ucode_entry_t* tmp =
                                        ad_snc_env.ad_snc_pdc_evt_array[pdc_array_idx].first_ucode;

                                while (tmp && tmp->next) {
                                        tmp = tmp->next;
                                        ((ad_snc_ucode_t*)tmp)->pdc_array_idx = pdc_array_idx;
                                }

                                pdc_array_idx++;
                        } while ((pdc_array_idx < AD_SNC_PDC_EVT_AR_SIZE) &&
                                 ad_snc_env.ad_snc_pdc_evt_array[pdc_array_idx].first_ucode);
                }
                ad_snc_env.ad_snc_pdc_evt_array[pdc_array_idx].first_ucode = NULL;

                // Inform SNC about the change.
                snc_main_set_pdc_evnt_entries(ad_snc_convert_pdc_evt_priority(AD_SNC_PDC_EVT_PR_0),
                        &ad_snc_env.ad_snc_pdc_evt_array[AD_SNC_PDC_EVT_PR_0],
                        ad_snc_get_num_pdc_evt_entries_with_no_pr());
        }
}

/*
 * Adds a uCode to the PDC event list of "uCodes-to-execute"
 */
static bool ad_snc_add_ucode_to_pdc_evt_list(uint32_t idx, ad_snc_ucode_t *new_ucode)
{
        ad_snc_ucode_t *list_el;
        ad_snc_ucode_t *prev_el = NULL;

        list_el = (ad_snc_ucode_t*)ad_snc_env.ad_snc_pdc_evt_array[idx].first_ucode;
        if (!list_el || !new_ucode) {
                return false;
        }

        while (list_el && (new_ucode->ucode_pr <= list_el->ucode_pr)) {
                prev_el = list_el;
                list_el = (ad_snc_ucode_t*)list_el->snc_ucode.next;
        }

        if (list_el) {
                if (prev_el) {
                        // insert in the list
                        prev_el->snc_ucode.next = (ad_snc_ucode_t*)new_ucode;
                        new_ucode->snc_ucode.next = (ad_snc_ucode_t*)list_el;
                } else {
                        // add at the top of the list
                        new_ucode->snc_ucode.next =
                                (ad_snc_ucode_t*)ad_snc_env.ad_snc_pdc_evt_array[idx].first_ucode;
                        ad_snc_env.ad_snc_pdc_evt_array[idx].first_ucode =
                                (snc_main_ucode_entry_t*)new_ucode;
                }
        } else {
                // Add at the end of the list
                prev_el->snc_ucode.next = new_ucode;
                prev_el = new_ucode;
                prev_el->snc_ucode.next = NULL;
        }
        return true;
}

/*
 * Removes a uCode from the PDC event list of "uCodes-to-execute"
 */
static bool ad_snc_remove_ucode_from_pdc_evt_list(uint32_t idx, uint32_t ucode_id)
{
        bool rtn = false;
        ad_snc_ucode_t *list_el;
        ad_snc_ucode_t *prev_el = NULL;

        if (ad_snc_env.ad_snc_pdc_evt_array[idx].first_ucode) {
                list_el = (ad_snc_ucode_t*)ad_snc_env.ad_snc_pdc_evt_array[idx].first_ucode;

                while (list_el && (list_el->ucode_id != ucode_id)) {
                        prev_el = list_el;
                        list_el = (ad_snc_ucode_t*)list_el->snc_ucode.next;
                }

                if (list_el) {
                        if (prev_el) {
                                prev_el->snc_ucode.next = list_el->snc_ucode.next;
                        } else {
                                // Remove from top of the list
                                ad_snc_env.ad_snc_pdc_evt_array[idx].first_ucode =
                                        (snc_main_ucode_entry_t*)list_el->snc_ucode.next;
                        }
                        rtn = true;
                }
        }

        return rtn;
}

/*
 * Adds a PDC event to the PDC events array (ad_snc_pdc_evt_array).
 * Events with Priority 1, 2 and 3 are stored in index 0, 1 and 2 respectively.
 * Note, only one PDC event per priority 1, 2 and 3.
 * Events with Priority 0 (no priority), are stored in index 3 and higher.
 * Multiple PDC events with "no-priority" is supported.
 */
static uint32_t ad_snc_add_pdc_evt_to_array(ad_snc_ucode_t *ucode)
{
        uint32_t idx = AD_SNC_PDC_EVT_AR_INDEX_INVALID;

        // If highest priority, then add at the front of the array
        if (ucode->pdc_evt_pr < AD_SNC_PDC_EVT_PR_0) {
                if (ad_snc_env.ad_snc_pdc_evt_array[ucode->pdc_evt_pr].first_ucode == NULL) {
                        ad_snc_env.ad_snc_pdc_evt_array[ucode->pdc_evt_pr].first_ucode =
                                (snc_main_ucode_entry_t*)ucode;
                        ad_snc_env.ad_snc_pdc_evt_array[ucode->pdc_evt_pr].first_ucode->next = NULL;
                        idx = ucode->pdc_evt_pr;
                        // pdc_event_id must be set before calling snc_main_set_pdc_evnt_entries()
                        ad_snc_env.ad_snc_pdc_evt_array[idx].pdc_event_id = ucode->pdc_lut_idx;
                        snc_main_set_pdc_evnt_entries(ad_snc_convert_pdc_evt_priority(ucode->pdc_evt_pr),
                                &ad_snc_env.ad_snc_pdc_evt_array[idx], 1);
                }
        } else if (ucode->pdc_evt_pr == AD_SNC_PDC_EVT_PR_0) { // Multiple PDC entries with no-priority
                for (int i = AD_SNC_PDC_EVT_PR_0; i < AD_SNC_PDC_EVT_AR_SIZE; i++) {
                        if (!ad_snc_env.ad_snc_pdc_evt_array[i].first_ucode) {
                                ad_snc_env.ad_snc_pdc_evt_array[i].first_ucode =
                                        (snc_main_ucode_entry_t*)ucode;
                                ad_snc_env.ad_snc_pdc_evt_array[i].first_ucode->next = NULL;
                                idx = i;
                                // pdc_event_id must be set before calling snc_main_set_pdc_evnt_entries()
                                ad_snc_env.ad_snc_pdc_evt_array[idx].pdc_event_id = ucode->pdc_lut_idx;
                                snc_main_set_pdc_evnt_entries(ad_snc_convert_pdc_evt_priority(AD_SNC_PDC_EVT_PR_0),
                                        &ad_snc_env.ad_snc_pdc_evt_array[AD_SNC_PDC_EVT_PR_0],
                                        ad_snc_get_num_pdc_evt_entries_with_no_pr());
                                break;
                        }
                }
        } else {
                OS_ASSERT(0); // Invalid priority
        }

        return idx;
}

/*
 * Initializes the uCode table, i.e. the array that stores the registered uCodes
 */
static void ad_snc_uct_init(void)
{
        for (int i = 0; i < AD_SNC_UCT_SIZE; ++i) {
                *(ad_snc_env.ad_snc_uct + i) = AD_SNC_UNUSED_UCT_ENTRY_VALUE;
        }
}

/*
 * Checks whether the PDC event of the uCode to be registered already exists in the array of
 * the PDC events
 * Note that PDC events with different ENABLE PD fields are considered to be the same.
 */
static uint32_t ad_snc_check_ucode_pdc_entry(uint32 pdc_entry)
{
        uint32_t idx = AD_SNC_UCT_INDEX_INVALID;

        // Check for PDC entry equality but ignore the ENABLE PD fields
        for (uint8_t i = 0; i < AD_SNC_UCT_SIZE; ++i) {
                if (ad_snc_env.ad_snc_uct[i] != AD_SNC_UNUSED_UCT_ENTRY_VALUE) {
                        if ((((*(ad_snc_env.ad_snc_uct + i))->pdc_entry) | AD_SNC_PDC_ENABLE_PD_MASK)
                                == (pdc_entry | AD_SNC_PDC_ENABLE_PD_MASK)) {
                                idx = i;
                                break;
                        }
                }
        }

        return idx;
}

/*
 * Returns the first empty location of the table of registered uCodes
 */
static uint32_t ad_snc_get_free_ucode_lut_idx(void)
{
        uint32_t idx = AD_SNC_UCT_INDEX_INVALID;
        for (uint8_t i = 0; i < AD_SNC_UCT_SIZE; ++i) {
                if (ad_snc_env.ad_snc_uct[i] == AD_SNC_UNUSED_UCT_ENTRY_VALUE) {
                        idx = i;
                        break;
                }
        }
        return idx;
}

/*
 * Checks whether the table of registered uCodes is empty
 */
static bool ad_snc_ucode_lut_empty(void)
{
        uint32_t rtn = true;
        for (uint8_t i = 0; i < AD_SNC_UCT_SIZE; ++i) {
                if (ad_snc_env.ad_snc_uct[i] != AD_SNC_UNUSED_UCT_ENTRY_VALUE) {
                        rtn = false;
                        break;
                }
        }
        return rtn;
}

#if dg_configUSE_HW_SENSOR_NODE_EMU
/*
 * Delay function to be used by the SNC Emulator in order to emulate the SNC delay command
 */
static void ad_snc_emu_delay(uint8_t ticks)
{
        hw_clk_delay_usec(1000000 * ticks / dg_configXTAL32K_FREQ);
}
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */

#if dg_configUSE_SNC_QUEUES
/*
 * Validates the SNC queue configuration parameters
 */
static bool ad_snc_validate_queue(const snc_queue_config_t *cfg)
{
        return (!cfg->num_of_chunks || cfg->max_chunk_bytes);
}

/*
 * Acquires access to a uCode's SNC queue
 */
static snc_queue_t ad_snc_queue_acquire(uint32_t ucode_id, AD_SNC_QUEUE_TYPE qType)
{
        OS_ASSERT(ucode_id < AD_SNC_UCT_SIZE);
        OS_ASSERT(qType <= AD_SNC_QUEUE_TYPE_SNC_CM33);

        ad_snc_ucode_t *ucode_ptr = ad_snc_env.ad_snc_uct[ucode_id];
        snc_queue_t sncQ = NULL;

        if (ucode_ptr != AD_SNC_UNUSED_UCT_ENTRY_VALUE) {
                if (qType == AD_SNC_QUEUE_TYPE_SNC_CM33) {
                        sncQ = ucode_ptr->snc_ucode.ucode_context->SNC_to_CM33_data_queue;
                } else {
                        sncQ = ucode_ptr->snc_ucode.ucode_context->CM33_to_SNC_data_queue;
                }

                // Acquire control over uCode environment
                OS_MUTEX_GET(ucode_ptr->mutex, OS_MUTEX_FOREVER);
        }

        return sncQ;
}

/*
 * Releases access to a uCode's SNC queue
 */
static void ad_snc_queue_release(uint32_t ucode_id)
{
        ad_snc_ucode_t *ucode_ptr = ad_snc_env.ad_snc_uct[ucode_id];

        if (ucode_ptr != AD_SNC_UNUSED_UCT_ENTRY_VALUE) {
                // Release control over uCode environment
                OS_MUTEX_PUT(ucode_ptr->mutex);
        }
}

/*
 * Creates an SNC queue of a specific type for a uCode
 */
static bool ad_snc_queue_create_req(uint32_t ucode_id, AD_SNC_QUEUE_TYPE qType,
        const snc_queue_config_t *cfg)
{
        ad_snc_ucode_t *ucode_ptr = ad_snc_env.ad_snc_uct[ucode_id];
        snc_queue_t *qP;

        // Select SNC queue type
        if (qType == AD_SNC_QUEUE_TYPE_SNC_CM33) {
                qP = &ucode_ptr->snc_ucode.ucode_context->SNC_to_CM33_data_queue;
        } else {
                qP = &ucode_ptr->snc_ucode.ucode_context->CM33_to_SNC_data_queue;
        }

        // Create the SNC queue if the number of chunks is not zero
        if (cfg->num_of_chunks) {
                *qP = snc_queues_cm33_create(cfg);
                // If queue creation fails, then return false
                if (*qP == NULL) {
                        return false;
                }
        } else {
                *qP = NULL;
        }

        return true;
}

/*
 * Deletes an SNC queue of a specific type for a uCode
 */
static void ad_snc_queue_delete_req(uint32_t ucode_id, AD_SNC_QUEUE_TYPE qType)
{
        ad_snc_ucode_t *ucode_ptr = ad_snc_env.ad_snc_uct[ucode_id];
        snc_queue_t *qP;

        // Select SNC queue type
        if (qType == AD_SNC_QUEUE_TYPE_SNC_CM33) {
                qP = &ucode_ptr->snc_ucode.ucode_context->SNC_to_CM33_data_queue;
        } else {
                qP = &ucode_ptr->snc_ucode.ucode_context->CM33_to_SNC_data_queue;
        }

        // Delete the SNC queue
        if (*qP) {
                snc_queues_cm33_destroy(*qP);
        }
}
#endif /* dg_configUSE_SNC_QUEUES */

/*
 * Validates the uCode configuration parameters before proceeding to uCode registration
 */
static uint32_t ad_snc_validate_ucode(const ad_snc_ucode_cfg_t *cfg)
{
        uint32_t ucode_lut_idx = ad_snc_get_free_ucode_lut_idx();

        if (ucode_lut_idx != AD_SNC_UCT_INDEX_INVALID) {
#if dg_configUSE_SNC_QUEUES
                // registered uCode already reached max allowed number
                if (!ad_snc_validate_queue(&cfg->snc_to_cm33_queue_cfg) ||
                    !ad_snc_validate_queue(&cfg->cm33_to_snc_queue_cfg)) {
                        OS_ASSERT(0); // Invalid queue configuration
                        ucode_lut_idx = AD_SNC_UCT_INDEX_INVALID;
                } else
#endif /* dg_configUSE_SNC_QUEUES */
                if (cfg->ucode_pr > AD_SNC_UCODE_PR_1) {
                        OS_ASSERT(0); // Invalid uCode priority
                        ucode_lut_idx = AD_SNC_UCT_INDEX_INVALID;
                } else {
                        // Check if uCode has a PDC entry that has been registered already
                        uint32_t tmp_idx = ad_snc_check_ucode_pdc_entry(cfg->pdc_entry);

                        if (tmp_idx == AD_SNC_UCT_INDEX_INVALID) { // New entry, make sure it uses the right priority.
                                // Check if registered uCode has PDC event priority 1,2,
                                // or 3 and this priority has been used by another entry
                                if (cfg->pdc_evt_pr < AD_SNC_PDC_EVT_PR_0) {
                                        if (ad_snc_env.ad_snc_pdc_evt_array[cfg->pdc_evt_pr].first_ucode != NULL) {
                                                ucode_lut_idx = AD_SNC_UCT_INDEX_INVALID;
                                        }
                                } else if (cfg->pdc_evt_pr == AD_SNC_PDC_EVT_PR_0) {
                                        // check if there is an empty entry for "no-priority"
                                        for (int i = AD_SNC_PDC_EVT_PR_0; i < AD_SNC_PDC_EVT_AR_SIZE; i++) {
                                                if (!ad_snc_env.ad_snc_pdc_evt_array[i].first_ucode) {
                                                        break;
                                                } else {
                                                        // PDC events array is full
                                                        if (i == (AD_SNC_PDC_EVT_AR_SIZE - 1)) {
                                                                return AD_SNC_UCT_INDEX_INVALID;
                                                        }
                                                }
                                        }
                                } else {
                                        OS_ASSERT(0); // Invalid PDC event priority
                                        ucode_lut_idx = AD_SNC_UCT_INDEX_INVALID;
                                }
                        }
                }
        }

        return ucode_lut_idx;
}

/*
 * Creates the uCode environment
 */
static bool ad_snc_create_ucode_env(uint32_t ucode_id, const ad_snc_ucode_cfg_t *cfg)
{
#if dg_configUSE_SNC_QUEUES
        // Create the CM33-to-SNC queue
        if (!ad_snc_queue_create_req(ucode_id, AD_SNC_QUEUE_TYPE_CM33_SNC,
                &cfg->cm33_to_snc_queue_cfg)) {
                return false;
        }

        // Create the SNC-to-CM33 queue
        if (!ad_snc_queue_create_req(ucode_id, AD_SNC_QUEUE_TYPE_SNC_CM33,
                &cfg->snc_to_cm33_queue_cfg)) {
                // If queue creation fails but we have created the CM33-to-SNC queue
                // then destroy it and return false
                if (ad_snc_env.ad_snc_uct[ucode_id]->snc_ucode.ucode_context->CM33_to_SNC_data_queue) {
                        ad_snc_queue_delete_req(ucode_id, AD_SNC_QUEUE_TYPE_CM33_SNC);
                }
                return false;
        }
#endif /* dg_configUSE_SNC_QUEUES */
        return true;
}

/*
 * Clears the uCode environment
 */
static void ad_snc_destroy_ucode_env(uint32_t ucode_id)
{
#if dg_configUSE_SNC_QUEUES
        // Delete SNC queues
        ad_snc_queue_delete_req(ucode_id, AD_SNC_QUEUE_TYPE_CM33_SNC);
        ad_snc_queue_delete_req(ucode_id, AD_SNC_QUEUE_TYPE_SNC_CM33);
#endif /* dg_configUSE_SNC_QUEUES */
}

/*
 * MUTEX get function, to protect SNC Adapter critical sections to be
 * accessed simultaneously by different application tasks with different priorities.
 */
static void ad_snc_lock(void)
{
        OS_MUTEX_GET(ad_snc_mutex, OS_MUTEX_FOREVER);
}

/*
 * MUTEX put function, to protect SNC Adapter critical sections to be
 * accessed simultaneously by different application tasks with different priorities.
 */
static void ad_snc_unlock(void)
{
        OS_MUTEX_PUT(ad_snc_mutex);
}

/*
 * SNC Adapter callback function to be called upon execution of the SNC_IRQn handler
 */
static void ad_snc_irq_cb(void)
{
#if dg_configUSE_SNC_DEBUGGER
        snc_process_bkpt();
#endif /* dg_configUSE_SNC_DEBUGGER */

#if dg_configUSE_HW_SENSOR_NODE_EMU
        // If the irq comes from the SNC emulator
        if (snc_context.snc_emu) {
                snc_context.snc_emu = 0;
                // send notification to emu task
                OS_TASK_NOTIFY_FROM_ISR(prvSensorNodeEMUTask_h, AD_SNC_EMULATOR_NOTIFICATION, OS_NOTIFY_SET_BITS);
        }
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */

        OS_TASK_NOTIFY_FROM_ISR(prvSensorNodeAdapterIRQTask_h, AD_SNC_IRQ_NOTIFICATION, OS_NOTIFY_SET_BITS);
}

/*
 * Registers a uCode to a PDC event
 */
static uint32_t ad_snc_ucode_register_req(const ad_snc_ucode_cfg_t *cfg,
        snc_ucode_context_t *ucode_ctx)
{
        uint32_t ucode_lut_idx = AD_SNC_UCT_INDEX_INVALID;

        ucode_lut_idx = ad_snc_validate_ucode(cfg);

        if (ucode_lut_idx != AD_SNC_UCT_INDEX_INVALID) {
                uint32_t pdc_lut_idx;

                // Check if uCode uses a PDC event that has been registered already
                uint32_t tmp_idx = ad_snc_check_ucode_pdc_entry(cfg->pdc_entry);

                if (tmp_idx == AD_SNC_UCT_INDEX_INVALID) {
                        // New uCode with new PDC entry
                        // Add PDC event entry for SNC master
                        pdc_lut_idx = hw_pdc_add_entry(cfg->pdc_entry);
                        if (pdc_lut_idx == HW_PDC_INVALID_LUT_INDEX) {
                                ucode_lut_idx = AD_SNC_UCT_INDEX_INVALID;
                        }
                } else {
                        // uCode with the same PDC event already exists, update ENABLE PD flags if needed
                        pdc_lut_idx = ad_snc_env.ad_snc_uct[tmp_idx]->pdc_lut_idx;
                        if (ad_snc_env.ad_snc_uct[tmp_idx]->pdc_entry != cfg->pdc_entry) {
                                hw_pdc_write_entry(ad_snc_env.ad_snc_uct[tmp_idx]->pdc_lut_idx,
                                        (cfg->pdc_entry | ad_snc_rebuild_pdc_event_enable_pd_flags(
                                                ad_snc_env.ad_snc_uct[tmp_idx]->pdc_array_idx)));
                        }
                }

                if (ucode_lut_idx != AD_SNC_UCT_INDEX_INVALID) {
                        ad_snc_ucode_t *ad_ucode_ptr;

                        // Allocate memory for SNC Adapter uCode structure, and initialize uCode fields
                        ad_ucode_ptr = (ad_snc_ucode_t*)OS_MALLOC(sizeof(ad_snc_ucode_t));
                        OS_ASSERT(ad_ucode_ptr);

                        ad_ucode_ptr->cb = cfg->cb;
                        ad_ucode_ptr->pdc_entry = cfg->pdc_entry;
                        ad_ucode_ptr->pdc_evt_pr = cfg->pdc_evt_pr;
                        ad_ucode_ptr->ucode_pr = cfg->ucode_pr;
                        ad_ucode_ptr->snc_ucode.ucode_context = ucode_ctx;
                        ad_ucode_ptr->snc_ucode.flags =
                                ((ad_ucode_ptr->pdc_entry & HW_PDC_LUT_ENTRY_EN_XTAL) ? SNC_MAIN_FLAG_XTAL32M_REQ : 0);
                        ad_ucode_ptr->snc_ucode.next = NULL;

                        // Add uCode pointer to the uCode LUT
                        ad_snc_env.ad_snc_uct[ucode_lut_idx] = ad_ucode_ptr;
                        ad_ucode_ptr->ucode_id = ucode_lut_idx;

                        // Create the mutex to control uCode access by different tasks
                        OS_MUTEX_CREATE(ad_ucode_ptr->mutex);
                        OS_ASSERT(ad_ucode_ptr->mutex);

                        // Create SNC queues and queues timestamp related environment (if enabled)
                        bool ucode_env_created;
                        ucode_env_created = ad_snc_create_ucode_env(ucode_lut_idx, cfg);
                        OS_ASSERT(ucode_env_created);

                        // Create uCode
                        ucode_ctx->ucode_create(ucode_lut_idx);

                        // In case a registration has started and interrupted
                        while (snc_main_is_halt_set());

                        // Halt SNC while updating the uCode table and the PDC events list
                        snc_main_set_halt();

                        // If new PDC entry, then add PDC event to the array of PDC events. Otherwise, just link uCodes
                        ad_ucode_ptr->pdc_lut_idx = pdc_lut_idx;
                        if (tmp_idx == AD_SNC_UCT_INDEX_INVALID) {
                                // Store PDC event array index to link uCode with PDC event
                                ad_ucode_ptr->pdc_array_idx = ad_snc_add_pdc_evt_to_array(ad_ucode_ptr);
                        } else {
                                // Since uCode will be linked to an existing PDC event, ignore PDC event priority
                                ad_ucode_ptr->pdc_evt_pr = ad_snc_env.ad_snc_uct[tmp_idx]->pdc_evt_pr;
                                ad_ucode_ptr->pdc_array_idx = ad_snc_env.ad_snc_uct[tmp_idx]->pdc_array_idx;

                                // Link uCode to the list of uCodes that will be executed on the same PDC event
                                ad_snc_add_ucode_to_pdc_evt_list(ad_snc_env.ad_snc_uct[tmp_idx]->pdc_array_idx, ad_ucode_ptr);
                        }

                        // If this is the first uCode, register SNC IRQ callback
                        if (!ad_snc_env.snc_irq_registered) {
                                hw_snc_register_int(ad_snc_irq_cb);
                                ad_snc_env.snc_irq_registered = true;
                        }

                        // Initially disable uCode
                        snc_disable_SNC_ucode(ucode_ctx);

                        snc_main_reset_halt();
                }
        }

        return ucode_lut_idx;
}

/*
 * Unregisters a uCode from a PDC event
 */
static bool ad_snc_ucode_unregister_req(uint32_t ucode_id)
{
        uint32_t pdc_entry;
        ad_snc_ucode_t *ucode_ptr;

        if (ucode_id >= AD_SNC_UCT_SIZE) {
                return false;
        }

        ucode_ptr = ad_snc_env.ad_snc_uct[ucode_id];
        if (ucode_ptr == AD_SNC_UNUSED_UCT_ENTRY_VALUE) {
                return false;
        }

        // In case a registration has started and interrupted
        while (snc_main_is_halt_set());

        // Halt SNC while updating the uCode table and the PDC events list
        snc_main_set_halt();

        // Delete uCode mutex
        {
                OS_MUTEX cur_mutex = ucode_ptr->mutex;

                // Acquire control over uCode environment
                OS_MUTEX_GET(ucode_ptr->mutex, OS_MUTEX_FOREVER);

                // Provoking assertions when the particular uCode environment mutex
                // is to be acquired
                ucode_ptr->mutex = NULL;

                /* The assertion is here to indicate that there are pending
                 * accesses on the particular uCode environment at the time of its
                 * un-registration.
                 */
                OS_ASSERT(OS_MUTEX_GET_COUNT(cur_mutex) == 0);

                OS_MUTEX_PUT(cur_mutex);
                OS_MUTEX_DELETE(cur_mutex);
        }

        // Remove from list of PDC events
        if (ad_snc_remove_ucode_from_pdc_evt_list(ucode_ptr->pdc_array_idx, ucode_ptr->ucode_id))
        {
                if (!ad_snc_env.ad_snc_pdc_evt_array[ucode_ptr->pdc_array_idx].first_ucode) {
                        // This is the last uCode in the list. Remove PDC event from array and PDC
                        ad_snc_remove_pdc_evt_from_array(ucode_ptr->pdc_array_idx, ucode_ptr->pdc_lut_idx);
                } else if (ucode_ptr->pdc_entry & AD_SNC_PDC_ENABLE_PD_MASK) {
                        // a uCode has been removed from the list with PDC ENABLE flags set.
                        // Rebuild PDC ENABLE PD flags
                        pdc_entry = ad_snc_rebuild_pdc_event_enable_pd_flags(ucode_ptr->pdc_array_idx);
                        if (pdc_entry) {
                                hw_pdc_write_entry(ucode_ptr->pdc_lut_idx, pdc_entry);
                        }
                }
        }

        // Clear any triggers generated by the uCode that is currently being unregistered
        snc_clear_SNC_to_CM33_trigger(ucode_ptr->snc_ucode.ucode_context);

        snc_main_reset_halt();

        // Call delete uCode function
        ucode_ptr->snc_ucode.ucode_context->ucode_delete();

        // Destroy SNC queues and queues timestamp related environment (if enabled)
        ad_snc_destroy_ucode_env(ucode_id);

        // Free uCode memory
        OS_FREE(ad_snc_env.ad_snc_uct[ucode_id]);
        ad_snc_env.ad_snc_uct[ucode_id] = AD_SNC_UNUSED_UCT_ENTRY_VALUE;

        // If this is the last registered uCode, unregister SNC IRQ callback
        if (ad_snc_ucode_lut_empty()) {
                hw_snc_unregister_int();
                ad_snc_env.snc_irq_registered = false;
        }

        return true;
}

/**
 * Sends a notification with command for uCode configuration to Sensor Node Adapter Task
 *
 * \param [in] cmd              the command type
 * \param [in] cmd_attrs        the pointer to the command attributes
 *
 * \return bool true if the command has been successfully sent and executed
 */
static bool ad_snc_send_cmd(AD_SNC_CMD_TYPE cmd, void *cmd_attrs)
{
        bool stat = false;

        if (initialized) {
                ad_snc_cmd_msg_t cmd_msg = {.cmd = cmd, .cmd_attrs = cmd_attrs};
                ad_snc_cmd_msg_t *cmd_msg_p = &cmd_msg;

                if (OS_QUEUE_PUT(ad_snc_if.task_cmd_msg_q, &cmd_msg_p, OS_QUEUE_FOREVER) == OS_OK) {
                        OS_TASK_NOTIFY(ad_snc_if.task, AD_SNC_CMD_NOTIFICATION, OS_NOTIFY_SET_BITS);
                        // wait for uCode command to be executed
                        OS_EVENT_WAIT(ad_snc_task_event, OS_EVENT_FOREVER);
                        stat = true;
                }
        }

        return stat;
}

/*
 * SNC Adapter task to handle IRQs from PDC
 */
static void prvSensorNodeAdapterIRQTask(void *pvParameters)
{
        uint32_t notif, rtn;
        int8_t wdog_id;

        /* register task to be monitored by watchdog */
        wdog_id = sys_watchdog_register(false);

        for (;;) {
                /* notify watchdog on each loop */
                sys_watchdog_notify(wdog_id);

                /* suspend watchdog while blocking on OS_TASK_NOTIFY_WAIT() */
                sys_watchdog_suspend(wdog_id);

                // Event / Notification Loop
                rtn = OS_TASK_NOTIFY_WAIT(0x0, OS_TASK_NOTIFY_ALL_BITS, &notif,
                        OS_TASK_NOTIFY_FOREVER);
                OS_ASSERT(rtn == OS_TASK_NOTIFY_SUCCESS);

                /* resume watchdog */
                sys_watchdog_notify_and_resume(wdog_id);

                if (notif & AD_SNC_IRQ_NOTIFICATION) {

                        ad_snc_lock();

                        // Call SNC API to read snc_context_t->snc_to_CM33_trigger
                        uint32_t ucode_idx = snc_get_SNC_to_CM33_trigger();

                        hw_pdc_acknowledge(ad_snc_env.snc_cm33_pdc_lut_idx);

                        for (int i = 0; i < AD_SNC_UCT_SIZE; ++i) {
                                if (ucode_idx & (1 << i)) {
                                        if (ad_snc_env.ad_snc_uct[i] == NULL ||
                                            ad_snc_env.ad_snc_uct[i]->cb == NULL) {
                                                OS_ASSERT(0);
                                        }

                                        snc_clear_SNC_to_CM33_trigger(ad_snc_env.ad_snc_uct[i]->snc_ucode.ucode_context);
                                        ad_snc_env.ad_snc_uct[i]->cb();
                                }
                        }

                        ad_snc_unlock();
                }
        }
}

/*
 * SNC Adapter task function
 */
static void prvSensorNodeAdapterTask(void *pvParameters)
{
        uint32_t notif, rtn;
        int8_t wdog_id;

        /* register task to be monitored by watchdog */
        wdog_id = sys_watchdog_register(false);

#if dg_configUSE_HW_SENSOR_NODE_EMU
        hw_snc_set_emu_delay_func(ad_snc_emu_delay);
#if dg_configUSE_SNC_DEBUGGER
        hw_snc_set_emu_exec_indication(snc_process_bkpt_emu);
#endif /* dg_configUSE_SNC_DEBUGGER */
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */

        // Register SNC-main-uCode
        snc_main_context_t* snc_main_ucode_ctx = SNC_UCODE_CTX(snc_main);
        snc_main_ucode_ctx->ucode_create();
        hw_snc_set_base_address((uint32_t)snc_main_ucode_ctx->ucode);

        ad_snc_env.snc_cm33_pdc_lut_idx =
                hw_pdc_add_entry(HW_PDC_LUT_ENTRY_VAL(HW_PDC_TRIG_SELECT_MASTER,
                        HW_PDC_PERIPH_TRIG_ID_SNC, HW_PDC_MASTER_CM33,
                        (dg_configENABLE_XTAL32M_ON_WAKEUP ? HW_PDC_LUT_ENTRY_EN_XTAL : 0)));

        /* Check if there are messages waiting in the queue. */
        if (OS_QUEUE_MESSAGES_WAITING(ad_snc_if.task_cmd_msg_q)) {
                OS_TASK_NOTIFY(ad_snc_if.task, AD_SNC_CMD_NOTIFICATION, OS_NOTIFY_SET_BITS);
        }

        for (;;) {
                ad_snc_cmd_msg_t* cmd_msg_p;

                /* notify watchdog on each loop */
                sys_watchdog_notify(wdog_id);

                /* suspend watchdog while blocking on OS_TASK_NOTIFY_WAIT() */
                sys_watchdog_suspend(wdog_id);

                // Event / Notification Loop
                rtn = OS_TASK_NOTIFY_WAIT(0x0, OS_TASK_NOTIFY_ALL_BITS, &notif,
                        OS_TASK_NOTIFY_FOREVER);
                OS_ASSERT(rtn == OS_TASK_NOTIFY_SUCCESS);

                /* resume watchdog */
                sys_watchdog_notify_and_resume(wdog_id);

                ad_snc_lock();

                if (notif & AD_SNC_CMD_NOTIFICATION) {
                        /* Get message from the queue. */
                        if (OS_QUEUE_PEEK(ad_snc_if.task_cmd_msg_q, &cmd_msg_p, 0)) {
                                AD_SNC_CMD_TYPE cmd = cmd_msg_p->cmd;

                                switch (cmd) {
                                case AD_SNC_CMD_UCODE_REG:
                                {
                                        ad_snc_ucode_reg_cmd_t* reg_cmd_p = cmd_msg_p->cmd_attrs;

                                        // Perform uCode registration
                                        reg_cmd_p->ucode_id =
                                                ad_snc_ucode_register_req(reg_cmd_p->cfg,
                                                        reg_cmd_p->ucode_ctx);
                                        break;
                                }
                                case AD_SNC_CMD_UCODE_UNREG:
                                {
                                        ad_snc_ucode_unreg_cmd_t* unreg_cmd_p = cmd_msg_p->cmd_attrs;

                                        // Perform uCode un-registration
                                        unreg_cmd_p->stat =
                                                ad_snc_ucode_unregister_req(unreg_cmd_p->ucode_id);
                                        break;
                                }
                                default:
                                        // Invalid command
                                        OS_ASSERT(0);
                                }

                                OS_EVENT_SIGNAL(ad_snc_task_event);
                                OS_QUEUE_GET(ad_snc_if.task_cmd_msg_q, &cmd_msg_p, 0);
                        }
                }

                ad_snc_unlock();
        }
}

#if dg_configUSE_HW_SENSOR_NODE_EMU
/*
 * SNC emulator task function
 */
static void prvSensorNodeEMUTask(void* pvParameters)
{
        uint32_t notif, rtn;
        int8_t wdog_id;

        /* register task to be monitored by watchdog */
        wdog_id = sys_watchdog_register(false);

        for (;;) {
                /* notify watchdog on each loop */
                sys_watchdog_notify(wdog_id);

                /* suspend watchdog while blocking on OS_TASK_NOTIFY_WAIT() */
                sys_watchdog_suspend(wdog_id);

                // Event / Notification Loop
                rtn = OS_TASK_NOTIFY_WAIT(0x0, OS_TASK_NOTIFY_ALL_BITS, &notif,
                        OS_TASK_NOTIFY_FOREVER);
                OS_ASSERT(rtn == OS_TASK_NOTIFY_SUCCESS);

                /* resume watchdog */
                sys_watchdog_notify_and_resume(wdog_id);

                if (notif & AD_SNC_EMULATOR_NOTIFICATION) {
                        hw_snc_emu_run();
                }
        }
}
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */

uint32_t ad_snc_ucode_register(ad_snc_ucode_cfg_t *cfg, snc_ucode_context_t *ucode_ctx)
{
        OS_ASSERT(cfg);
        OS_ASSERT(ucode_ctx);

        ad_snc_ucode_reg_cmd_t reg_cmd = {
                .cfg = cfg,
                .ucode_ctx = ucode_ctx,
                .ucode_id = AD_SNC_UCODE_ID_INVALID
        };

        ad_snc_send_cmd(AD_SNC_CMD_UCODE_REG, &reg_cmd);

        return reg_cmd.ucode_id;
}

bool ad_snc_ucode_unregister(uint32_t ucode_id)
{
        OS_ASSERT(ucode_id < AD_SNC_UCT_SIZE);

        ad_snc_ucode_unreg_cmd_t unreg_cmd = {
                .ucode_id = ucode_id,
                .stat = false
        };

        ad_snc_send_cmd(AD_SNC_CMD_UCODE_UNREG, &unreg_cmd);

        return unreg_cmd.stat;
}

void ad_snc_init(void)
{
        OS_BASE_TYPE status;

        if (initialized) {
                return;
        }

        OS_MUTEX_CREATE(ad_snc_mutex);
        OS_ASSERT(ad_snc_mutex);

        OS_EVENT_CREATE(ad_snc_task_event);
        OS_ASSERT(ad_snc_task_event);
        OS_EVENT_CHECK(ad_snc_task_event); // clears the event

        OS_QUEUE_CREATE(ad_snc_if.task_cmd_msg_q, sizeof(ad_snc_cmd_msg_t*), 1);

        ad_snc_lock();

        ad_snc_uct_init();

        for (int i = 0; i < AD_SNC_PDC_EVT_AR_SIZE; i++) {
                ad_snc_env.ad_snc_pdc_evt_array[i].first_ucode = NULL;
        }

        ad_snc_env.snc_irq_registered = false;

        ad_snc_unlock();

        initialized = true;

        // Create SNC Adapter task to handle SNC IRQs
        status = OS_TASK_CREATE( "AD_IRQ_SNC",          /* The text name assigned to the task, for
                                                           debug only; not used by the kernel. */
                        prvSensorNodeAdapterIRQTask,    /* The function that implements the task. */
                        NULL,                           /* The parameter passed to the task. */
                        64 * OS_STACK_WORD_SIZE,        /* Stack size allocated for the task in bytes. */
                        SNC_AD_TASK_IRQ_PRIORITY,       /* The priority assigned to the task. */
                        prvSensorNodeAdapterIRQTask_h );/* The task handle */
        OS_ASSERT(status == OS_TASK_CREATE_SUCCESS);

#if dg_configUSE_HW_SENSOR_NODE_EMU
        // Create SNC Emulator task
        status = OS_TASK_CREATE( "AD_EMU_SNC",          /* The text name assigned to the task, for
                                                           debug only; not used by the kernel. */
                        prvSensorNodeEMUTask,           /* The function that implements the task. */
                        NULL,                           /* The parameter passed to the task. */
                        64 * OS_STACK_WORD_SIZE,        /* Stack size allocated for the task in bytes. */
                        SNC_EMU_TASK_PRIORITY,          /* The priority assigned to the task. */
                        prvSensorNodeEMUTask_h );       /* The task handle */
        OS_ASSERT(status == OS_TASK_CREATE_SUCCESS);
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */

        // Create SNC Adapter task to handle uCode registration
        status = OS_TASK_CREATE( "AD_SNC",              /* The text name assigned to the task, for
                                                           debug only; not used by the kernel. */
                        prvSensorNodeAdapterTask,       /* The function that implements the task. */
                        NULL,                           /* The parameter passed to the task. */
                        512 * OS_STACK_WORD_SIZE,       /* Stack size allocated for the task in bytes. */
                        SNC_AD_TASK_PRIORITY,           /* The priority assigned to the task. */
                        ad_snc_if.task );               /* The task handle */
        OS_ASSERT(status == OS_TASK_CREATE_SUCCESS);
}

bool ad_snc_pdc_set_pending(uint32_t ucode_id)
{
        OS_ASSERT(ucode_id < AD_SNC_UCT_SIZE);

        bool rtn = false;

        if (initialized) {
                ad_snc_ucode_t *ucode_ptr = ad_snc_env.ad_snc_uct[ucode_id];

                if (ucode_ptr != AD_SNC_UNUSED_UCT_ENTRY_VALUE) {
                        // Acquire control over uCode environment
                        OS_MUTEX_GET(ucode_ptr->mutex, OS_MUTEX_FOREVER);

                        snc_notify_SNC_ucode(ucode_ptr->snc_ucode.ucode_context);

                        // Release control over uCode environment
                        OS_MUTEX_PUT(ucode_ptr->mutex);

                        hw_pdc_set_pending(ucode_ptr->pdc_lut_idx);

                        rtn = true;
                }
        }

        return rtn;
}

bool ad_snc_ucode_enable(uint32_t ucode_id)
{
        OS_ASSERT(ucode_id < AD_SNC_UCT_SIZE);

        bool rtn = false;

        if (initialized) {
                ad_snc_ucode_t *ucode_ptr = ad_snc_env.ad_snc_uct[ucode_id];

                if (ucode_ptr != AD_SNC_UNUSED_UCT_ENTRY_VALUE) {
                        // Acquire control over uCode environment
                        OS_MUTEX_GET(ucode_ptr->mutex, OS_MUTEX_FOREVER);

                        snc_main_enable_ucode_entry(&ucode_ptr->snc_ucode);

                        // Release control over uCode environment
                        OS_MUTEX_PUT(ucode_ptr->mutex);

                        rtn = true;
                }
        }

        return rtn;
}

bool ad_snc_ucode_disable(uint32_t ucode_id)
{
        OS_ASSERT(ucode_id < AD_SNC_UCT_SIZE);

        bool rtn = false;

        if (initialized) {
                ad_snc_ucode_t *ucode_ptr = ad_snc_env.ad_snc_uct[ucode_id];

                if (ucode_ptr != AD_SNC_UNUSED_UCT_ENTRY_VALUE) {
                        // Acquire control over uCode environment
                        OS_MUTEX_GET(ucode_ptr->mutex, OS_MUTEX_FOREVER);

                        snc_main_disable_ucode_entry(&ucode_ptr->snc_ucode);

                        // Release control over uCode environment
                        OS_MUTEX_PUT(ucode_ptr->mutex);

                        rtn = true;
                }
        }

        return rtn;
}

#if dg_configUSE_SNC_QUEUES
bool ad_snc_queue_update(uint32_t ucode_id, AD_SNC_QUEUE_TYPE qType, const snc_queue_config_t *cfg)
{
        OS_ASSERT(cfg);

        bool rtn = false;

        // Validate SNC queue configuration
        if (!ad_snc_validate_queue(cfg)) {
                return false;
        }

        // Acquire control over uCode queue
        snc_queue_t sncQ = ad_snc_queue_acquire(ucode_id, qType);

        if (sncQ) {
                snc_queues_cm33_update(sncQ, cfg);

                // Release control over uCode queue
                ad_snc_queue_release(ucode_id);

                rtn = true;
        }

        return rtn;
}

bool ad_snc_queue_reset(uint32_t ucode_id, AD_SNC_QUEUE_TYPE qType)
{
        bool rtn = false;

        // Acquire control over uCode queue
        snc_queue_t sncQ = ad_snc_queue_acquire(ucode_id, qType);

        if (sncQ) {
                snc_queues_cm33_reset(sncQ);

                // Release control over uCode queue
                ad_snc_queue_release(ucode_id);

                rtn = true;
        }

        return rtn;
}

bool ad_snc_queue_push(uint32_t ucode_id, const uint8_t *data, size_t size, uint32_t timestamp)
{
        bool rtn = false;

        // Acquire control over uCode CM33-to-SNC queue
        snc_queue_t sncQ = ad_snc_queue_acquire(ucode_id, AD_SNC_QUEUE_TYPE_CM33_SNC);

        if (sncQ) {
                rtn = snc_queues_cm33_push(sncQ, data, size, timestamp);

                // Release control over uCode queue
                ad_snc_queue_release(ucode_id);
        }

        return rtn;
}

bool ad_snc_queue_pop(uint32_t ucode_id, uint8_t *data, uint32_t *size, uint32_t *timestamp)
{
        bool rtn = false;

        // Acquire control over uCode CM33-to-SNC queue
        snc_queue_t sncQ = ad_snc_queue_acquire(ucode_id, AD_SNC_QUEUE_TYPE_SNC_CM33);

        if (sncQ) {
                rtn = snc_queues_cm33_pop(sncQ, data, size, timestamp);

                // Release control over uCode queue
                ad_snc_queue_release(ucode_id);
        }

        return rtn;
}

bool ad_snc_queue_is_empty(uint32_t ucode_id, AD_SNC_QUEUE_TYPE qType)
{
        bool rtn = true;

        // Acquire control over uCode queue
        snc_queue_t sncQ = ad_snc_queue_acquire(ucode_id, qType);

        if (sncQ) {
                rtn = snc_queues_cm33_queue_is_empty(sncQ);

                // Release control over uCode queue
                ad_snc_queue_release(ucode_id);
        }

        return rtn;
}

bool ad_snc_queue_is_full(uint32_t ucode_id, AD_SNC_QUEUE_TYPE qType)
{
        bool rtn = false;

        // Acquire control over uCode queue
        snc_queue_t sncQ = ad_snc_queue_acquire(ucode_id, qType);

        if (sncQ) {
                rtn = snc_queues_cm33_queue_is_full(sncQ);

                // Release control over uCode queue
                ad_snc_queue_release(ucode_id);
        }

        return rtn;
}

uint32_t ad_snc_queue_get_free_chunks(uint32_t ucode_id, AD_SNC_QUEUE_TYPE qType)
{
        uint32_t rtn = 0;

        // Acquire control over uCode queue
        snc_queue_t sncQ = ad_snc_queue_acquire(ucode_id, qType);

        if (sncQ) {
                rtn = snc_queues_cm33_get_free_chunks(sncQ);

                // Release control over uCode queue
                ad_snc_queue_release(ucode_id);
        }

        return rtn;
}

uint32_t ad_snc_queue_get_alloc_chunks(uint32_t ucode_id, AD_SNC_QUEUE_TYPE qType)
{
        uint32_t rtn = 0;

        // Acquire control over uCode queue
        snc_queue_t sncQ = ad_snc_queue_acquire(ucode_id, qType);

        if (sncQ) {
                rtn = snc_queues_cm33_get_alloc_chunks(sncQ);

                // Release control over uCode queue
                ad_snc_queue_release(ucode_id);
        }

        return rtn;
}

uint32_t ad_snc_queue_get_cur_chunk_bytes(uint32_t ucode_id, AD_SNC_QUEUE_TYPE qType)
{
        uint32_t rtn = 0;

        // Acquire control over uCode queue
        snc_queue_t sncQ = ad_snc_queue_acquire(ucode_id, qType);

        if (sncQ) {
                rtn = snc_queues_cm33_get_cur_chunk_bytes(sncQ);

                // Release control over uCode queue
                ad_snc_queue_release(ucode_id);
        }

        return rtn;
}
#endif /* dg_configUSE_SNC_QUEUES */

#if dg_configUSE_HW_TIMER
uint64_t ad_snc_sys_timer_get_uptime_ticks(snc_hw_sys_uptime_ticks_t* snc_uptime_ticks)
{
        return snc_uptime_ticks->rtc_time +
               ((snc_uptime_ticks->current_time - snc_uptime_ticks->prev_time) & SNC_HW_SYS_UPTIME_TICKS_RES);
}
#endif /* dg_configUSE_HW_TIMER */

#ifndef OS_BAREMETAL
ADAPTER_INIT(ad_snc_adapter, ad_snc_init);
#endif /* OS_BAREMETAL */

#endif /* dg_configSNC_ADAPTER */


/**
 * \}
 * \}
 */
