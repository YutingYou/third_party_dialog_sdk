/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SNC_MAIN
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file snc_main.c
 *
 * @brief Sensor Node Controller (SNC) main uCode implementation
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#if dg_configUSE_HW_SENSOR_NODE

#include "snc_defs.h"
#include "snc_hw_sys.h"

#include "snc_main.h"

/*
 * DATA TYPE AND STRUCTURE DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief SNC-main-uCode environment structure
 */
typedef struct snc_main_env_t {
        /**< PDC event IDs per priority for the registered arrays of PDC events */
        uint32_t pdc_event_pr_list_id[SNC_MAIN_PDC_EVT_PR_0 + 1];
        /**< Starting address per priority for the registered arrays of PDC events */
        snc_main_pdc_evt_entry_t* pdc_event_pr_list_begin[SNC_MAIN_PDC_EVT_PR_0 + 1];
        /**< Ending address per priority for the registered arrays of PDC events */
        snc_main_pdc_evt_entry_t* pdc_event_pr_list_end[SNC_MAIN_PDC_EVT_PR_0 + 1];
        /**< Address of the PDC event entry to be served next per priority for the registered arrays of PDC events */
        snc_main_pdc_evt_entry_t* pdc_event_pr_list_cur_event[SNC_MAIN_PDC_EVT_PR_0 + 1];
        /**< Address of the uCode entry to be served next per priority for the registered arrays of PDC events */
        snc_main_ucode_entry_t* pdc_event_pr_list_cur_ucode[SNC_MAIN_PDC_EVT_PR_0 + 1];
} snc_main_env_t;

/*
 * SNC-main-uCode environment
 */
_SNC_RETAINED static snc_main_env_t snc_main_env = { 0 };

/*
 * FUNCTION DEFINITIONS
 *****************************************************************************************
 */

void snc_main_set_halt(void)
{
        volatile uint32_t* p_snc_active = &snc_context.snc_active;

        snc_context.snc_halt = 1;

        while (*p_snc_active != 0);
}

void snc_main_reset_halt(void)
{
        snc_context.snc_halt = 0;
}

bool snc_main_is_halt_set(void)
{
        return (snc_context.snc_halt > 0);
}

void snc_main_set_pdc_evnt_entries(SNC_MAIN_PDC_EVT_PRIORITY priority,
        snc_main_pdc_evt_entry_t* pdc_events, uint32_t numEvents)
{
        ASSERT_WARNING(priority <= SNC_MAIN_PDC_EVT_PR_0);

        if (pdc_events == NULL) {
                snc_main_env.pdc_event_pr_list_begin[priority] = NULL;
        } else {
                snc_main_env.pdc_event_pr_list_id[priority] = 1 << priority;
                snc_main_env.pdc_event_pr_list_begin[priority] = &pdc_events[0];
                snc_main_env.pdc_event_pr_list_end[priority] = &pdc_events[numEvents];
                snc_main_env.pdc_event_pr_list_cur_event[priority] = &pdc_events[0];
                snc_main_env.pdc_event_pr_list_cur_ucode[priority] = pdc_events[0].first_ucode;

                for (uint32_t i = 0; i < numEvents; i++) {
                        pdc_events[i].pdc_event_id_shifted = pdc_events[i].pdc_event_id << 23;
                        pdc_events[i].pdc_event_id_idx =
                                ((uint32_t)1) << (pdc_events[i].pdc_event_id);
                }
        }
}

void snc_main_enable_ucode_entry(snc_main_ucode_entry_t* ucode_entry)
{
        snc_enable_SNC_ucode(ucode_entry->ucode_context);
}

void snc_main_disable_ucode_entry(snc_main_ucode_entry_t* ucode_entry)
{
        snc_disable_SNC_ucode(ucode_entry->ucode_context);
}

static void snc_main_deside_on_xtal32m(b_ctx_t* b_ctx, uint32_t** pp_flags)
{
        SENIS_labels(
                pt_wait_for_xtal32m_to_settle, pt_prepare_dcdc,
                pt_DCDC_V14_ENABLE_HV_is_set, pt_DCDC_V14_ENABLE_HV_is_cleared,
                pt_DCDC_V14_ENABLE_LV_is_set, pt_DCDC_V14_ENABLE_LV_is_cleared,
                pt_wait_for_ldo_to_be_enabled,
                pt_return
        );

        _SNC_TMP_ADD(uint32_t, cur_ucode_flags, sizeof(uint32_t));

        // Check if XTAL32M is required for the uCode, ...
        SENIS_wadad(da(cur_ucode_flags), ia(pp_flags));

        SENIS_rdcbi(da(cur_ucode_flags), SNC_MAIN_FLAG_POS_XTAL32M_REQ);

        // ... and if yes, continue with waiting for XTAL32M to settle
        SENIS_cobr_eq(l(pt_wait_for_xtal32m_to_settle));
        SENIS_goto(l(pt_return));

        SENIS_label(pt_wait_for_xtal32m_to_settle);

        // ... wait for XTAL32M to settle and ...
        _SNC_STATIC(uint32_t, const_xtalrdy_0count_minus_1, sizeof(uint32_t),
                ((uint32_t)1 << REG_POS(CRG_XTAL, XTALRDY_STAT_REG, XTALRDY_COUNT)) - 1
        );

        SENIS_rdcgr(da(&CRG_XTAL->XTALRDY_STAT_REG), da(const_xtalrdy_0count_minus_1));
        SENIS_cobr_gr(l(pt_wait_for_xtal32m_to_settle));

        // ... finally switch to XTAL32M.
        SENIS_wadva(da(&CRG_TOP->CLK_SWITCH2XTAL_REG), 0x1);

        _SNC_TMP_RMV(cur_ucode_flags);

        SENIS_label(pt_return);
}
#define SNC_main_deside_on_xtal32m(pp_flags)                                                    \
        snc_main_deside_on_xtal32m(b_ctx, pp_flags)

static void snc_main_branch_to_ucode(b_ctx_t* b_ctx, uint32_t** pp_cur_ucode, uint32_t** pp_flags)
{
        SENIS_labels(pt_branch_to_ucode_exit);

        _SNC_TMP_ADD(uint32_t*, p_cur_ucode, sizeof(uint32_t*));
        _SNC_TMP_ADD(uint32_t**, pp_cur_lp, sizeof(uint32_t**));
        _SNC_TMP_ADD(uint32_t*, pp_cur_flags, sizeof(uint32_t*));
        _SNC_TMP_ADD(uint32_t, p_cur_flags, sizeof(uint32_t));
        _SNC_TMP_ADD(uint32_t, p_cur_cmd, sizeof(uint32_t));

        SENIS_assign(da(p_cur_ucode), ia(pp_cur_ucode));
        SENIS_inc4(da(pp_cur_ucode));
        SENIS_assign(da(pp_cur_lp), da(pp_cur_ucode));
        SENIS_inc4(da(pp_cur_ucode));
        SENIS_assign(da(pp_cur_flags), da(pp_cur_ucode));
        SENIS_inc4(da(pp_cur_ucode));

        // Try to set SNC uCode as "running"
        {
                // Set SNC_UCODE_BLOCK_FLAG_RUNNING flag
                SENIS_assign(da(p_cur_flags), ia(pp_cur_flags));
                SENIS_tobre(da(p_cur_flags), (1 << SNC_UCODE_BLOCK_FLAG_RUNNING));
                SENIS_assign(ia(pp_cur_flags), da(p_cur_flags));

                // Check if SNC_UCODE_BLOCK_CMD_DISABLE command flag is set
                SENIS_assign(da(p_cur_cmd), ia(pp_cur_ucode));
                SENIS_rdcbi(da(p_cur_cmd), SNC_UCODE_BLOCK_CMD_DISABLE);
                SENIS_cobr_eq(l(pt_branch_to_ucode_exit));
        }

        // Switch to XTAL32M if it is required for the uCode
        SNC_main_deside_on_xtal32m(pp_flags);

        // "SeNIS_call"
        SENIS_wadva(da(&SNC->SNC_STATUS_REG), SENIS_FLAG_EQ);
        SENIS_wadad(ia(pp_cur_lp), da(&SNC->SNC_PC_REG));
        SENIS_cobr_eq(ia(p_cur_ucode));

        SENIS_label(pt_branch_to_ucode_exit);

        // Clear SNC_UCODE_BLOCK_FLAG_RUNNING flag
        SENIS_tobre(da(p_cur_flags), (1 << SNC_UCODE_BLOCK_FLAG_RUNNING));
        SENIS_assign(ia(pp_cur_flags), da(p_cur_flags));

        _SNC_TMP_RMV(p_cur_cmd);
        _SNC_TMP_RMV(p_cur_flags);
        _SNC_TMP_RMV(pp_cur_flags);
        _SNC_TMP_RMV(pp_cur_lp);
        _SNC_TMP_RMV(p_cur_ucode);
}
#define SNC_main_branch_to_ucode(pp_cur_ucode, pp_flags)                                        \
        snc_main_branch_to_ucode(b_ctx, pp_cur_ucode, pp_flags)

SNC_FUNC_DEF(snc_main_wait_if_halt_req_ucode, uint32_t* p_was_halted)
{
        SENIS_labels(pt_check_enter_leave_halt, pt_enter_halt, pt_halt);

        SENIS_assign(ia(&SNC_ARG(p_was_halted)), 0);

        SENIS_label(pt_check_enter_leave_halt);

        SENIS_rdcbi(da(&snc_context.snc_halt), 0);
        SENIS_cobr_eq(l(pt_enter_halt));

        SENIS_return;

        // enter_halt
        SENIS_label(pt_enter_halt);

        SENIS_assign(da(&snc_context.snc_active), 0);

        SENIS_label(pt_halt);

        SENIS_rdcbi(da(&snc_context.snc_halt), 0);
        SENIS_cobr_eq(l(pt_halt));

        SENIS_assign(da(&snc_context.snc_active), 1);

        SENIS_assign(ia(&SNC_ARG(p_was_halted)), 1);
        SENIS_goto(l(pt_check_enter_leave_halt));
}
static void snc_main_wait_if_halt_req(b_ctx_t* b_ctx, uint32_t* p_was_halted)
{
        _SNC_TMP_ADD(uint32_t, temp_was_halted, sizeof(uint32_t));

        if (p_was_halted == NULL) {
                p_was_halted = temp_was_halted;
        }

        senis_call(b_ctx, SNC_UCODE_CTX(snc_main_wait_if_halt_req_ucode), 2 * 1,
                _SNC_OP(p_was_halted));

        _SNC_TMP_RMV(temp_was_halted);
}
#define SNC_main_wait_if_halt_req(p_was_halted)                                                 \
        SNC_STEP_BY_STEP();                                                                     \
        snc_main_wait_if_halt_req(b_ctx, p_was_halted)

SNC_MAIN_DEF(snc_main)
{
        SENIS_labels(
                pt_snc_main_start, pt_check_pdc_pending,

                pt_pdc_entry_list_handling,

                pt_handle_pdc_entry_list,

                blk_dowhile_pdc_evnt_exec_begin, blk_dowhile_pdc_evnt_exec_end,

                ph_setting_cur_event_exec_pending, ph_resetting_cur_event_exec_pending,

                blk_while_pdc_evnt_begin, blk_while_pdc_evnt_cond,

                pt_handle_cur_pdc_entry, pt_handle_ucode_list,

                ph_check_pdc_entry,

                pt_move_to_next_event,

                blk_while_ucode_begin, blk_while_ucode_cond,

                ph_clear_pdc_pending_reg_val, ph_check_pdc_pending_change
        );

        _SNC_TMP_ADD(uint32_t, temp_was_halted, sizeof(uint32_t));
        _SNC_TMP_ADD(uint32_t, pending_pdc_event_execution, sizeof(uint32_t));

        SNC_hw_sys_init();

        SENIS_assign(da(&snc_context.snc_active), 1);
        SENIS_assign(da(pending_pdc_event_execution), 0);

        SENIS_label(pt_snc_main_start);

        SNC_main_wait_if_halt_req(NULL);

        SENIS_label(pt_check_pdc_pending);

        SENIS_rdcgr_z(da(&PDC->PDC_PENDING_SNC_REG));
        SENIS_cobr_gr(l(pt_pdc_entry_list_handling));                  // jmp to PDC-entry-list handling

        SENIS_rdcbi(da(&SNC->SNC_CTRL_REG), REG_POS(SNC, SNC_CTRL_REG, SNC_IRQ_EN));
        SENIS_cobr_eq(l(pt_snc_main_start));

        SENIS_rdcgr_z(da(&snc_context.prevent_slp_cnt));
        SENIS_cobr_gr(l(pt_snc_main_start));

        SENIS_assign(da(&snc_context.snc_active), 0);

        SENIS_slp;

        _SNC_TMP_ADD(uint32_t**, pp_list_id, sizeof(uint32_t**));
        _SNC_TMP_ADD(snc_main_pdc_evt_entry_t**, pp_list_begin, sizeof(snc_main_pdc_evt_entry_t**));
        _SNC_TMP_ADD(snc_main_pdc_evt_entry_t**, pp_list_end, sizeof(snc_main_pdc_evt_entry_t**));
        _SNC_TMP_ADD(snc_main_pdc_evt_entry_t**, pp_cur_event, sizeof(snc_main_pdc_evt_entry_t**));
        _SNC_TMP_ADD(snc_main_ucode_entry_t**, pp_cur_ucode, sizeof(snc_main_ucode_entry_t**));

        _SNC_TMP_ADD(uint32_t, pdc_pending_reg_val, sizeof(uint32_t));

        _SNC_TMP_ADD(snc_main_ucode_entry_t*, p_cur_ucode, sizeof(snc_main_ucode_entry_t*));

        _SNC_TMP_ADD(snc_ucode_context_t*, cur_ucode_ctx, sizeof(snc_ucode_context_t*));

        _SNC_TMP_ADD(snc_main_pdc_evt_entry_t*, p_list_begin, sizeof(snc_main_pdc_evt_entry_t*));
        _SNC_TMP_ADD(snc_main_pdc_evt_entry_t*, p_list_end, sizeof(snc_main_pdc_evt_entry_t*));

        _SNC_TMP_ADD(uint32_t, no_ucode_has_exec, sizeof(uint32_t));
        _SNC_TMP_ADD(snc_main_pdc_evt_entry_t*, p_cur_event, sizeof(snc_main_pdc_evt_entry_t*));

        // PDC-entry-list-handling
        SENIS_label(pt_pdc_entry_list_handling); {

                SENIS_assign(da(pp_list_id), &snc_main_env.pdc_event_pr_list_id[SNC_MAIN_PDC_EVT_PR_1]);
                SENIS_assign(da(pp_list_begin), &snc_main_env.pdc_event_pr_list_begin[SNC_MAIN_PDC_EVT_PR_1]);
                SENIS_assign(da(pp_list_end), &snc_main_env.pdc_event_pr_list_end[SNC_MAIN_PDC_EVT_PR_1]);
                SENIS_assign(da(pp_cur_event), &snc_main_env.pdc_event_pr_list_cur_event[SNC_MAIN_PDC_EVT_PR_1]);
                SENIS_assign(da(pp_cur_ucode), &snc_main_env.pdc_event_pr_list_cur_ucode[SNC_MAIN_PDC_EVT_PR_1]);

                SENIS_assign(da(pdc_pending_reg_val), da(&PDC->PDC_PENDING_SNC_REG));

                // Handling a PDC entry list of a specific priority each time
                SENIS_label(pt_handle_pdc_entry_list); {

                        SENIS_assign(da(p_list_begin), ia(pp_list_begin));
                        SENIS_assign(da(p_list_end), ia(pp_list_end));

                        // Executing uCodes of a specific PDC entry
                        SENIS_label(blk_dowhile_pdc_evnt_exec_begin); {

                                SENIS_rdcgr(da(&snc_const[1]), da(p_list_begin));
                                SENIS_cobr_gr(l(blk_dowhile_pdc_evnt_exec_end)); // break;

                                SENIS_assign(da(p_cur_event), ia(pp_cur_event));

                                SENIS_rdcgr(da(p_cur_event), da(p_list_begin));
                                SENIS_cobr_gr(l(pt_handle_ucode_list));

                                SENIS_assign(l(ph_setting_cur_event_exec_pending) + 1, ia(pp_list_id));
                                SENIS_label(ph_setting_cur_event_exec_pending);
                                senis_tobre(b_ctx,
                                        _SNC_OP_DIRECT_ADDRESS(da(pending_pdc_event_execution)),
                                        SNC_PLACE_HOLDER); // set pending execution

                                SENIS_assign(da(no_ucode_has_exec), 1); // no uCode has executed yet

                                SENIS_goto(l(blk_while_pdc_evnt_cond));
                                SENIS_label(blk_while_pdc_evnt_begin); {

                                        SENIS_assign(l(ph_check_pdc_entry), ia(p_cur_event));                           //snc_main_pdc_evt_entry_t->pdc_event_id_shifted
                                        SENIS_tobre(l(ph_check_pdc_entry), _SENIS_B_RDCBI_D(pdc_pending_reg_val, 0));

                                        SENIS_inc4(da(p_cur_event));
                                        SENIS_assign(ia(pp_cur_event), da(p_cur_event));

                                        SENIS_label(ph_check_pdc_entry);
                                        senis_rdcbi(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(SNC_PLACE_HOLDER)),
                                                SNC_PLACE_HOLDER);
                                        SENIS_cobr_eq(l(pt_handle_cur_pdc_entry));
                                        SENIS_goto(l(pt_move_to_next_event));

                                        SENIS_label(pt_handle_cur_pdc_entry); {
                                                _SNC_TMP_ADD(uint32_t*, cur_pdc_event_index, sizeof(uint32_t*));

                                                SENIS_assign(da(cur_pdc_event_index), da(p_cur_event));                         //snc_main_pdc_evt_entry_t->pdc_event_id_idx

                                                SENIS_assign(l(ph_clear_pdc_pending_reg_val) + 1, ia(cur_pdc_event_index));
                                                SENIS_label(ph_clear_pdc_pending_reg_val);
                                                senis_tobre(b_ctx,
                                                        _SNC_OP_DIRECT_ADDRESS(da(pdc_pending_reg_val)),
                                                        SNC_PLACE_HOLDER);

                                                SENIS_inc4(da(cur_pdc_event_index));
                                                SNC_hw_sys_pdc_acknowledge(ia(cur_pdc_event_index));                            //snc_main_pdc_evt_entry_t->pdc_event_id

                                                SENIS_inc4(da(cur_pdc_event_index));
                                                SENIS_assign(ia(pp_cur_ucode), ia(cur_pdc_event_index));                        //snc_main_pdc_evt_entry_t->first_ucode

                                                _SNC_TMP_RMV(cur_pdc_event_index);

                                                SENIS_label(pt_handle_ucode_list); {

                                                        SENIS_assign(da(p_cur_ucode), ia(pp_cur_ucode));

                                                        // While there is any uCode in the uCode list of the particular PDC entry
                                                        SENIS_goto(l(blk_while_ucode_cond));
                                                        SENIS_label(blk_while_ucode_begin); {

                                                                SENIS_assign(da(cur_ucode_ctx), ia(p_cur_ucode));               //snc_main_ucode_entry_t->ucode_context
                                                                SENIS_rdcgr(da(&snc_const[1]), da(cur_ucode_ctx));
                                                                SENIS_cobr_gr(l(pt_move_to_next_event));

                                                                SENIS_inc4(da(p_cur_ucode));                                    //snc_main_ucode_entry_t->flags

                                                                // Call uCode
                                                                SNC_main_branch_to_ucode((uint32_t**)cur_ucode_ctx, (uint32_t**)p_cur_ucode);

                                                                SENIS_inc4(da(p_cur_ucode));
                                                                SENIS_assign(da(p_cur_ucode), ia(p_cur_ucode));                 //snc_main_ucode_entry_t->next
                                                                SENIS_assign(ia(pp_cur_ucode), da(p_cur_ucode));

                                                                SENIS_assign(da(no_ucode_has_exec), 0);

                                                                // Check if a new event has been triggered, ...
                                                                _SNC_TMP_ADD(uint32_t, tmp_pdc_pending_reg_val, sizeof(uint32_t));

                                                                SENIS_assign(da(tmp_pdc_pending_reg_val),
                                                                        da(&PDC->PDC_PENDING_SNC_REG));
                                                                SENIS_assign(l(ph_check_pdc_pending_change) + 1,
                                                                        da(pdc_pending_reg_val));
                                                                SENIS_label(ph_check_pdc_pending_change);
                                                                senis_tobre(b_ctx,
                                                                        _SNC_OP_DIRECT_ADDRESS(da(tmp_pdc_pending_reg_val)),
                                                                        SNC_PLACE_HOLDER);
                                                                SENIS_rdcgr_z(da(tmp_pdc_pending_reg_val));

                                                                _SNC_TMP_RMV(tmp_pdc_pending_reg_val);
                                                                // ... and if yes, branch accordingly to execute
                                                                // all registered uCodes of higher priority
                                                                SENIS_cobr_gr(l(pt_pdc_entry_list_handling)); // goto to pt_pdc_entry_list_handling

                                                                SENIS_label(blk_while_ucode_cond);

                                                                SENIS_rdcgr_z(da(p_cur_ucode));
                                                                SENIS_cobr_gr(l(blk_while_ucode_begin));
                                                        } // blk_while_ucode
                                                } // pt_handle_ucode_list
                                        } // pt_handle_cur_pdc_entry

                                        SENIS_label(pt_move_to_next_event);

                                        SENIS_inc4(da(p_cur_event));
                                        SENIS_inc4(da(p_cur_event));
                                        SENIS_inc4(da(p_cur_event));

                                        SENIS_label(blk_while_pdc_evnt_cond);

                                        SENIS_rdcgr(da(p_list_end), da(p_cur_event));
                                        SENIS_cobr_gr(l(blk_while_pdc_evnt_begin));
                                } // blk_while_pdc_evnt

                                SENIS_assign(ia(pp_cur_event), da(p_list_begin));

                                SENIS_assign(l(ph_resetting_cur_event_exec_pending) + 1, ia(pp_list_id));
                                SENIS_label(ph_resetting_cur_event_exec_pending);
                                senis_tobre(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(pending_pdc_event_execution)),
                                        SNC_PLACE_HOLDER);

                                // If no uCode has been executed for the current event priority list,
                                // change event priority list and do not check for SNC-halt request
                                SENIS_rdcbi(da(no_ucode_has_exec), 0);
                                SENIS_cobr_eq(l(blk_dowhile_pdc_evnt_exec_end));

                                // Check fast if there is an SNC-halt request (since snc_active = 1)
                                SENIS_rdcgr(da(&snc_const[1]), da(&snc_context.snc_halt));
                                SENIS_cobr_gr(l(pt_check_pdc_pending));
                                // If there is a pending execution of a PDC event,
                                // do not check if there is an SNC-halt request
                                SENIS_rdcgr_z(da(pending_pdc_event_execution));
                                SENIS_cobr_gr(l(pt_check_pdc_pending));

                                SNC_main_wait_if_halt_req(temp_was_halted);

                                // If SNC was halted, then check again PDC pending register for SNC
                                SENIS_rdcbi(da(temp_was_halted), 0);
                                SENIS_cobr_eq(l(pt_check_pdc_pending));

                                SENIS_label(blk_dowhile_pdc_evnt_exec_end);
                        } // blk_dowhile_pdc_evnt_exec

                        _SNC_STATIC(snc_main_pdc_evt_entry_t**, pp_list_begin_to_cmp,
                                sizeof(snc_main_pdc_evt_entry_t**),
                                &snc_main_env.pdc_event_pr_list_begin[SNC_MAIN_PDC_EVT_PR_0] - 1);

                        SENIS_rdcgr(da(pp_list_begin), da(pp_list_begin_to_cmp));
                        SENIS_cobr_gr(l(pt_snc_main_start));

                        SENIS_inc4(da(pp_list_id));
                        SENIS_inc4(da(pp_list_begin));
                        SENIS_inc4(da(pp_list_end));
                        SENIS_inc4(da(pp_cur_event));
                        SENIS_inc4(da(pp_cur_ucode));

                        SENIS_goto(l(pt_handle_pdc_entry_list));
                } // pt_handle_pdc_entry_list
        } // pt_pdc_entry_list_handling

        _SNC_TMP_RMV(p_cur_event);
        _SNC_TMP_RMV(no_ucode_has_exec);

        _SNC_TMP_RMV(p_list_end);
        _SNC_TMP_RMV(p_list_begin);

        _SNC_TMP_RMV(cur_ucode_ctx);

        _SNC_TMP_RMV(p_cur_ucode);

        _SNC_TMP_RMV(pdc_pending_reg_val);

        _SNC_TMP_RMV(pp_cur_ucode);
        _SNC_TMP_RMV(pp_cur_event);
        _SNC_TMP_RMV(pp_list_end);
        _SNC_TMP_RMV(pp_list_begin);
        _SNC_TMP_RMV(pp_list_id);

        _SNC_TMP_RMV(pending_pdc_event_execution);
        _SNC_TMP_RMV(temp_was_halted);
}

#endif /* dg_configUSE_HW_SENSOR_NODE */


/**
 * \}
 * \}
 */
