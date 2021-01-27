/**
 ****************************************************************************************
 *
 * @file ble_mgr_common.c
 *
 * @brief BLE manager handlers for common API
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <string.h>
#include "ble_stack_config.h"
#include "ad_ble_msg.h"
#include "ble_mgr.h"
#include "ble_mgr_ad_msg.h"
#include "ble_mgr_gtl.h"
#include "ble_mgr_cmd.h"
#include "ble_mgr_common.h"
#include "ble_mgr_helper.h"
#include "ble_common.h"
#include "storage.h"
#include "gapm_task.h"
#if defined(CONFIG_USE_BLE_CLIENTS)
#include "ble_client.h"
#endif
#if defined(CONFIG_USE_BLE_SERVICES)
#include "ble_service.h"
#endif

void ble_mgr_common_stack_msg_handler(void *param)
{
        /* Send message directly to BLE adapter */
        ad_ble_command_queue_send(&param, OS_QUEUE_FOREVER);
}

void ble_mgr_common_register_cmd_handler(void *param)
{
        const ble_mgr_common_register_cmd_t *cmd = param;
        ble_mgr_common_register_rsp_t *rsp;

        ble_mgr_register_application(cmd->task);

        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_COMMON_REGISTER_CMD, sizeof(*rsp));

        rsp->status = BLE_STATUS_OK;

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_adapter_cmp_evt_init(ad_ble_msg_t *ad_msg, void *param)
{
        /* Event received from BLE adapter -- NOT GTL */
        ad_ble_cmp_evt_t *ad_evt = (void *) ad_msg;
        ble_mgr_common_enable_rsp_t *rsp;
        ble_dev_params_t *ble_dev_params;

        OS_ASSERT(ad_evt->status == AD_BLE_STATUS_NO_ERROR);

        /* Set status to BLE_IS_ENABLED */
        ble_mgr_set_status(BLE_IS_ENABLED);

        ble_msg_free(param);

        /* Create response */
        rsp = ble_msg_init(BLE_MGR_COMMON_ENABLE_CMD, sizeof(*rsp));
        rsp->status = (ad_evt->status == 0 ? BLE_STATUS_OK : BLE_ERROR_FAILED);

        /*
         * We now know that BLE adapter is up and running which means it already have proper address
         * set. Now it's a good time to update ble_dev_params - we always start with public static
         * address and application can change this after BLE is enabled.
         */
        ble_dev_params = ble_mgr_dev_params_acquire();
        ble_dev_params->own_addr.addr_type = PUBLIC_STATIC_ADDRESS;
        /* Update own public BD address with the one stored in NVPARAM */
        ad_ble_get_public_address(ble_dev_params->own_addr.addr);
        /* Update own public IRK with the one stored in NVPARAM */
        ad_ble_get_irk(ble_dev_params->irk.key);
        ble_mgr_dev_params_release();

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}

void ble_mgr_common_enable_cmd_handler(void *param)
{
        ad_ble_msg_t *ad_cmd;

        storage_init();

        /* Allocate buffer for BLE adapter message */
        ad_cmd = ble_ad_msg_alloc(AD_BLE_OP_INIT_CMD, sizeof(ad_ble_msg_t));

        /* Add expected response on the waitqueue -- NOT GTL */
        ble_ad_msg_wqueue_add(AD_BLE_OP_CMP_EVT, AD_BLE_OP_INIT_CMD, ble_adapter_cmp_evt_init, param);

        /* Send BLE adapter message -- NOT GTL */
        ble_ad_msg_send(ad_cmd);
}

static void ble_adapter_cmp_evt_reset(ad_ble_msg_t *ad_msg, void *param)
{
        /* Event received from BLE adapter -- not GTL */
        ad_ble_cmp_evt_t *ad_evt = (void *) ad_msg;
        ble_dev_params_t *dev_params;
        ble_mgr_common_reset_rsp_t *rsp;
        ble_evt_reset_completed_t *evt;

        /* Free command buffer */
        ble_msg_free(param);

        /* Create response and event (the event receiving task may be different than the API caller
         * task) */
        rsp = ble_msg_init(BLE_MGR_COMMON_RESET_CMD, sizeof(*rsp));
        evt = ble_evt_init(BLE_EVT_RESET_COMPLETED, sizeof(*evt));

        if (ad_evt->status != AD_BLE_STATUS_NO_ERROR) {
                rsp->status = BLE_ERROR_FAILED;
                evt->status = BLE_ERROR_FAILED;
                goto done;
        }

        rsp->status = BLE_STATUS_OK;
        evt->status = BLE_STATUS_OK;

        /* Cleanup and initialize storage */
        storage_acquire();
        storage_cleanup();
        storage_init();
        storage_release();

        /* Clear waitqueue (does not call waitqueue callback functions) */
        ble_gtl_waitqueue_flush_all();

        /* Set default device parameters */
        dev_params = ble_mgr_dev_params_acquire();
        ble_mgr_dev_params_set_default();
        /* Update own public BD address with the one stored in NVPARAM */
        ad_ble_get_public_address(dev_params->own_addr.addr);
        /* Update own IRK with the one stored in NVPARAM */
        ad_ble_get_irk(dev_params->irk.key);
        ble_mgr_dev_params_release();

        /* Cleanup clients and services */
        #if defined(CONFIG_USE_BLE_CLIENTS)
        ble_clients_cleanup();
        #endif
        #if defined(CONFIG_USE_BLE_SERVICES)
        ble_services_cleanup();
        #endif

done:
        ble_mgr_set_status(BLE_IS_ENABLED);
        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
        ble_mgr_event_queue_send(&evt, OS_QUEUE_FOREVER);
}

void ble_mgr_common_reset_cmd_handler(void *param)
{
        ad_ble_msg_t *ad_cmd;

        /* Set BLE in reset status */
        ble_mgr_set_status(BLE_IS_RESET);

        /* Flush BLE event queue */
        ble_mgr_event_queue_flush();

        /* Allocate buffer for BLE adapter message */
        ad_cmd = ble_ad_msg_alloc(AD_BLE_OP_RESET_CMD, sizeof(ad_ble_msg_t));

        /* Add expected response on the waitqueue -- not GTL */
        ble_ad_msg_wqueue_add(AD_BLE_OP_CMP_EVT, AD_BLE_OP_RESET_CMD, ble_adapter_cmp_evt_reset, param);

        /* Send BLE adapter message */
        ble_ad_msg_send(ad_cmd);
}

void ble_mgr_common_read_tx_power_cmd_handler(void *param)
{
        ble_mgr_common_read_tx_power_rsp_t *rsp;

        ble_msg_free(param);
        rsp = ble_msg_init(BLE_MGR_COMMON_READ_TX_POWER_CMD, sizeof(*rsp));

        rsp->tx_power_level = 0x00;
        rsp->status = BLE_STATUS_OK;

        ble_mgr_response_queue_send(&rsp, OS_QUEUE_FOREVER);
}
