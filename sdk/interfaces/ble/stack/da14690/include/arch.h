/**
 ****************************************************************************************
 *
 * @file arch.h
 *
 * @brief This file contains the definitions of the macros and functions that are
 * architecture dependent.  The implementation of those is implemented in the
 * appropriate architecture directory.
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 * Copyright (c) 2015-2018 Modified by Dialog Semiconductor
 *
 ****************************************************************************************
 */


#ifndef _ARCH_H_
#define _ARCH_H_

/**
 ****************************************************************************************
 * @defgroup REFIP REFIP
 * @brief Reference IP Platform
 *
 * This module contains reference platform components - REFIP.
 *
 *
 * @{
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @defgroup DRIVERS DRIVERS
 * @ingroup REFIP
 * Reference IP Platform Drivers
 *
 * This module contains the necessary drivers to run the platform with the
 * RW BT SW protocol stack.
 *
 * This has the declaration of the platform architecture API.
 *
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>     // standard integer definition
#include "compiler.h"   // inline functions
#include "user_config_defs.h"
#if (CMAC_CPU) || defined(HOST_LIB)
#include "global_dbg.h"
#include "ll.h"         // define GLOBAL_INT_** macros as inline assembly
#endif

#if (CMAC_CPU)
void rf_reinit_func(void);
void SetSystemVars_func(void);
void crypto_init_func(void);

#define rf_init                                 rf_init_func
#define rf_reinit                               rf_reinit_func
#define lld_sleep_compensate                    lld_sleep_compensate_func
#define lld_sleep_init                          lld_sleep_init_func
#define lld_sleep_us_2_lpcycles                 lld_sleep_us_2_lpcycles_func
#define lld_sleep_lpcycles_2_us                 lld_sleep_lpcycles_2_us_func
#define h4tl_init                               h4tl_init_func
#define h4tl_read_start                         h4tl_read_start_func
#define h4tl_read_hdr                           h4tl_read_hdr_func
#define h4tl_read_payl                          h4tl_read_payl_func
#define h4tl_read_next_out_of_sync              h4tl_read_next_out_of_sync_func
#define h4tl_out_of_sync                        h4tl_out_of_sync_func
#define h4tl_tx_done                            h4tl_tx_done_func
#define h4tl_rx_done                            h4tl_rx_done_func
#define ke_task_init                            ke_task_init_func
#define ke_timer_init                           ke_timer_init_func
#define llm_encryption_done                     llm_encryption_done_func
#define nvds_get                                nvds_get_func
#define nvds_del                                nvds_del_func
#define nvds_put                                nvds_put_func
#define rwip_eif_get                            rwip_eif_get_func
#define platform_reset                          platform_reset_func
#define lld_test_stop                           lld_test_stop_func
#define lld_test_mode_tx                        lld_test_mode_tx_func
#define lld_test_mode_rx                        lld_test_mode_rx_func
#define nvds_init                               nvds_init_func
#define dbg_init                                dbg_init_func
#define dbg_platform_reset_complete             dbg_platform_reset_complete_func
#define hci_rd_local_supp_feats_cmd_handler     hci_rd_local_supp_feats_cmd_handler_func
//GZ 4.2
#define crypto_init                             crypto_init_func
#define llm_le_adv_report_ind                   llm_le_adv_report_ind_func
#define PK_PointMult                            PK_PointMult_func
#define llm_p256_start                          llm_p256_start_func
#define llm_create_p256_key                     llm_create_p256_key_func
#define llm_p256_req_handler                    llm_p256_req_handler_func
#define llc_le_length_effective                 llc_le_length_effective_func
#define llc_le_length_conn_init                 llc_le_length_conn_init_func
#define lld_data_tx_prog                        lld_data_tx_prog_func
#define lld_data_tx_check                       lld_data_tx_check_func
#define llc_pdu_send                            llc_pdu_send_func
#define llc_data_notif                          llc_data_notif_func

#define dia_rand                                dia_rand_func
#define dia_srand                               dia_srand_func
#endif // (CMAC_CPU)

#if (!CMAC_CPU)
#define gtl_init                                gtl_init_func
#define gtl_eif_init                            gtl_eif_init_func
#define gtl_eif_read_hdr                        gtl_eif_read_hdr_func
#define gtl_eif_read_payl                       gtl_eif_read_payl_func
#define gtl_eif_read_start                      gtl_eif_read_start_func
#define gtl_eif_rx_done                         gtl_eif_rx_done_func
#define gtl_eif_tx_done                         gtl_eif_tx_done_func
#define h4tl_init                               h4tl_init_func
#define h4tl_tx_done                            h4tl_tx_done_func
#define h4tl_rx_done                            h4tl_rx_done_func
#define h4tl_read_start                         h4tl_read_start_func
#define h4tl_out_of_sync                        h4tl_out_of_sync_func
#define h4tl_read_hdr                           h4tl_read_hdr_func
#define h4tl_read_payl                          h4tl_read_payl_func
#define h4tl_read_next_out_of_sync              h4tl_read_next_out_of_sync_func
#define attc_l2cc_pdu_recv_handler              attc_l2cc_pdu_recv_handler_func
#define atts_l2cc_pdu_recv_handler              atts_l2cc_pdu_recv_handler_func
#define l2c_process_sdu                         l2c_process_sdu_func
#define l2c_send_lecb_message                   l2c_send_lecb_message_func
#define l2cc_pdu_pack                           l2cc_pdu_pack_func
#define l2cc_pdu_unpack                         l2cc_pdu_unpack_func
#define l2cc_pdu_recv_ind_handler               l2cc_pdu_recv_ind_handler_func
#define gapc_lecb_connect_cfm_handler           gapc_lecb_connect_cfm_handler_func
#define smpc_check_param                        smpc_check_param_func
#define smpc_dhkey_calc_ind                     smpc_dhkey_calc_ind_func
#define smpc_pdu_recv                           smpc_pdu_recv_func
#define smpc_public_key_exchange_start          smpc_public_key_exchange_start_func
#define smpm_ecdh_key_create                    smpm_ecdh_key_create_func
#define prf_cleanup                             prf_cleanup_func
#define prf_create                              prf_create_func
#define prf_get_id_from_task                    prf_get_id_from_task_func
#define prf_get_task_from_id                    prf_get_task_from_id_func
#define prf_init                                prf_init_func
#define prf_add_profile                         prf_add_profile_func
#define nvds_init                               nvds_init_func
#define nvds_get                                nvds_get_func
#define SetSystemVars                           SetSystemVars_func
#define ke_task_init                            ke_task_init_func
#define ke_timer_init                           ke_timer_init_func
#define rwip_eif_get                            rwip_eif_get_func
#define platform_reset                          platform_reset_func
#endif

/*
 * CPU WORD SIZE
 ****************************************************************************************
 */
/// ARM is a 32-bit CPU
#define CPU_WORD_SIZE                           (4)

/*
 * CPU Endianness
 ****************************************************************************************
 */
/// ARM is little endian
#define CPU_LE                                  (1)

/*
 * DEBUG configuration
 ****************************************************************************************
 */
#if defined(CFG_DBG)
#define PLF_DEBUG                               (1)
#else //CFG_DBG
#define PLF_DEBUG                               (0)
#endif //CFG_DBG

/*
 * NVDS
 ****************************************************************************************
 */

/// NVDS
#ifdef CFG_NVDS
#define PLF_NVDS                                (1)
#else // CFG_NVDS
#define PLF_NVDS                                (0)
#endif // CFG_NVDS

/*
 * LLD ROM defines
 ****************************************************************************************
 */
struct lld_sleep_env_tag
{
        uint32_t irq_mask;
};

/*
 * UART
 ****************************************************************************************
 */

/// UART
#define PLF_UART                                (1)


/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * Deep sleep threshold. Application specific. Control if during deep sleep the system RAM will be
 * powered off and if OTP copy will be required.
 ****************************************************************************************
 */
/// Sleep Duration Value in periodic wake-up mode
#define MAX_SLEEP_DURATION_PERIODIC_WAKEUP_DEF  (0x0320) // 0.5s

/// Sleep Duration Value in external wake-up mode
#define MAX_SLEEP_DURATION_EXTERNAL_WAKEUP_DEF  (0x3E80) // 10s

/// Possible errors detected by FW
#define RESET_NO_ERROR                          (0x00000000)
#define RESET_MEM_ALLOC_FAIL                    (0xF2F2F2F2)

/// Reset platform and stay in ROM
#define RESET_TO_ROM                            (0xA5A5A5A5)

/// Reset platform and reload FW
#define RESET_AND_LOAD_FW                       (0xC3C3C3C3)

/*
 * EXPORTED FUNCTION DECLARATION
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Fast division by 625.
 *
 * This function computes the quotient and the remainder of a division by 625.
 *
 * @param [in] q The number to be divided by 625
 * @param [out] rem The remainder of the division (q / 625)
 *
 * @return The quotient of the division (q / 625)
 ****************************************************************************************
 */
uint32_t fast_div_by_625(uint32_t q, uint32_t *rem);

/**
 ****************************************************************************************
 * @brief Fast division by 100.
 *
 * This function computes the quotient and the remainder of a division by 100.
 *
 * @param [in] q The number to be divided by 100
 * @param [out] rem The remainder of the division (q / 100)
 *
 * @return The quotient of the division (q / 100)
 *
 * @warning The function is accurate up to: MAX_SLEEP_TIME_IN_SLOTS * 48.
 *
 ****************************************************************************************
 */
uint32_t fast_div_by_100(uint32_t q, uint32_t *rem);

/**
 ****************************************************************************************
 * @brief Compute size of SW stack used.
 *
 * This function is compute the maximum size stack used by SW.
 *
 * @return Size of stack used (in bytes)
 ****************************************************************************************
 */
uint16_t get_stack_usage(void);

/**
 ****************************************************************************************
 * @brief Re-boot FW.
 *
 * This function is used to re-boot the FW when error has been detected, it is the end of
 * the current FW execution.
 * After waiting transfers on UART to be finished, and storing the information that
 * FW has re-booted by itself in a non-loaded area, the FW restart by branching at FW
 * entry point.
 *
 * Note: when calling this function, the code after it will not be executed.
 *
 * @param[in] error      Error detected by FW
 ****************************************************************************************
 */
void platform_reset(uint32_t error);



/*
 * WEAK Library functions that can be exported by the SDK
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief CMAC Platform reset hook.
 *
 * This function is provided to the SDK to execute all the required operations when the
 * Host stack reports an error that cannot be handled (i.e. memory overflow).
 *
 * Note that when the function is called, the interrupts are already disabled.
 ****************************************************************************************
 */
void platform_reset_sdk(uint32_t error);

/**
 ****************************************************************************************
 * @brief CMAC Host Crypto init hook.
 *
 * This function is provided to the SDK for any initialization required for the Crypto
 * of the Host.
 *
 ****************************************************************************************
 */
void crypto_init_func(void);

/**
 ****************************************************************************************
 * @brief CMAC Host rand() init hook.
 *
 * This function is provided to the SDK for any initialization required for the random
 * numbers engine.
 *
 ****************************************************************************************
 */
int dia_rand_func(void);

/**
 ****************************************************************************************
 * @brief CMAC Host srand() init hook.
 *
 * This function is provided to the SDK for any initialization required for the random
 * numbers engine.
 *
 ****************************************************************************************
 */
void dia_srand_func(unsigned int seed);

/**
 ****************************************************************************************
 * @brief CMAC to System event hook.
 *
 * This function is provided to the SDK for any additional actions required in the 
 * CMAC2SYS_Handler().
 ****************************************************************************************
 */
void cmac2sys_notify(void);

/**
 ****************************************************************************************
 * @brief CMAC to System interrupt entry hook.
 *
 * This function is provided to the SDK for any additional actions required in the 
 * CMAC2SYS_Handler().
 ****************************************************************************************
 */
void cmac2sys_isr_enter(void);

/**
 ****************************************************************************************
 * @brief CMAC to System interrupt exit hook.
 *
 * This function is provided to the SDK for any additional actions required in the 
 * CMAC2SYS_Handler().
 ****************************************************************************************
 */
void cmac2sys_isr_exit(void);

/**
 ****************************************************************************************
 * @brief CMAC On Error critical event hook.
 *
 * This function is provided to the SDK to handle blocking errors reported by CMAC 
 * (e.g. Hardfault or Watchdog or hardware errors). In its default implementation that is
 * included in the library, the function will issue a BKPT, which will lead to a halt, if
 * a debugger is attached to the Host CPU, or a hardfault.
 ****************************************************************************************
 */
void sys_cmac_on_error_handler(void);

/**
 ****************************************************************************************
 * @brief CMAC internal Debug Event handler hook.
 *
 * This function is provided to the SDK in case it needs to override the default debug 
 * event handling that is provided by the library. It is not clear yet under which 
 * conditions this feature could be useful.
 *
 ****************************************************************************************
 */
bool internal_dbg_evt_handling(uint32_t code, uint32_t subcode);

/**
 ****************************************************************************************
 * @brief CMAC Direct Test Report Event handler hook.
 *
 * This function is provided to the SDK to process the Direct Test report that is sent by
 * CMAC in certain cases and if it has been requested to do so explicitly.
 ****************************************************************************************
 */
void hci_dbg_report_evt_process(uint8_t *payload);

/**
 ****************************************************************************************
 * @brief CMAC Wakeup-time fix event hook.
 *
 * This function is provided to the SDK to execute the required operations when CMAC 
 * determines that an update of the wakeup_time is needed.
 ****************************************************************************************
 */
void sys_proc_handler(void);

/**
 ****************************************************************************************
 * @brief CMAC "enable temperature measurement" event handler.
 *
 * This function is provided to the SDK to execute the required operations when CMAC 
 * instructs the Host to enable the temperature monitoring function.
 ****************************************************************************************
 */
void sys_temp_meas_enable(void);

/**
 ****************************************************************************************
 * @brief CMAC "disable temperature measurement" event handler.
 *
 * This function is provided to the SDK to execute the required operations when CMAC 
 * instructs the Host to disable the temperature monitoring function.
 ****************************************************************************************
 */
void sys_temp_meas_disable(void);

/**
 ****************************************************************************************
 * @brief CMAC "start RF calibration" trigger handler
 *
 * This function is provided to the SDK in order to trigger the CMAC to start the RF
 * calibration.
 ****************************************************************************************
 */
void sys_rf_calibration_start(void);



/*
 * ASSERTION CHECK
 ****************************************************************************************
 */

#if (CMAC_CPU) || defined(HOST_LIB)
/// Assertions showing a critical error that could require a full system reset
#define ASSERT_ERR(cond)                        ASSERT_ERROR(cond)

/// Assertions showing a critical error that could require a full system reset
#define ASSERT_INFO(cond, param0, param1)       ASSERT(cond)

/// Assertions showing a non-critical problem that has to be fixed by the SW
#define ASSERT_WARN(cond)                       ASSERT(cond)
#endif

#define CHECK_AND_CALL(func_ptr)                do { } while(0)

/// @} DRIVERS
/// @} REFIP
#endif // _ARCH_H_
