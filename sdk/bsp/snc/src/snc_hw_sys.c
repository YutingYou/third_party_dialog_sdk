/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SNC_SYS
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file snc_hw_sys.c
 *
 * @brief SNC-Implementation of Sensor Node Controller system Low Level Driver
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#if dg_configUSE_HW_SENSOR_NODE

#include "snc_defs.h"
#include "hw_gpio.h"
#include "sys_tcs.h"

#if (defined(OS_FREERTOS) && dg_configUSE_HW_TIMER)
#include "../../sys_man/sys_timer_internal.h"
#endif /* defined(OS_FREERTOS) && dg_configUSE_HW_TIMER */

#include "snc_hw_sys.h"

/*
 * DATA STRUCTURE DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief Variable used for maintaining the number of nested critical sections the SNC
 *        execution flow has entered.
 *
 */
_SNC_RETAINED static uint32_t snc_critical_cnt = 0;

/*
 * FUNCTION DEFINITIONS
 *****************************************************************************************
 */

static void snc_hw_sys_bsr_lock(b_ctx_t* b_ctx);
/**
 * \brief Function used in SNC context to acquire exclusive access to a shared resource.
 *        This function will block until access is granted.
 *
 * \note Performing multiple calls of snc_hw_sys_bsr_lock() without having called each time
 *       snc_hw_sys_bsr_unlock() first, must not occur.
 *
 * \sa snc_hw_sys_bsr_try_lock
 * \sa snc_hw_sys_bsr_unlock
 *
 */
#define SNC_hw_sys_bsr_lock()                                                                   \
        snc_hw_sys_bsr_lock(b_ctx)

static void snc_hw_sys_bsr_unlock(b_ctx_t* b_ctx);
/**
 * \brief Function used in SNC context to release exclusive access to a shared resource,
 *        so that it can be also used by other masters (SYSCPU or CMAC).
 *
 * \sa snc_hw_sys_bsr_try_lock
 * \sa snc_hw_sys_bsr_lock
 *
 */
#define SNC_hw_sys_bsr_unlock()                                                                 \
        snc_hw_sys_bsr_unlock(b_ctx)

#if dg_configUSE_HW_SYS
static void snc_hw_sys_reg_apply_config(b_ctx_t* b_ctx,
        SENIS_OPER_TYPE sys_reg_config_table_type, uint32_t* sys_reg_config_table,
        SENIS_OPER_TYPE num_of_entries_type, uint32_t* num_of_entries);
/**
 * \brief Apply system register configuration in SNC context
 *
 * Configure system registers using the entries in a given system register configuration table.
 *
 * \param [in] sys_reg_config_table     (uint32_t*: use da() or ia())
 *                                      address of the system register configuration table
 *                                      (i.e. hw_sys_reg_config_t*)
 * \param [in] num_of_entries           (uint32_t: use da() or ia() or build-time-only value)
 *                                      the number of the table entries
 *
 */
#define SNC_hw_sys_reg_apply_config(sys_reg_config_table, num_of_entries)                       \
        SNC_STEP_BY_STEP();                                                                     \
        snc_hw_sys_reg_apply_config(b_ctx, _SNC_OP_ADDRESS(sys_reg_config_table),               \
                _SNC_OP(num_of_entries))
#endif /* dg_configUSE_HW_SYS */

//==================== SNC critical section definition =========================

void snc_critical_section_enter(b_ctx_t* b_ctx)
{
        ASSERT_WARNING(b_ctx);

        SENIS_labels(pt_skip_bsr_acquisition);

        // if (snc_critical_cnt == 0) {
        SENIS_rdcgr_z(da(&snc_critical_cnt));
        SENIS_cobr_gr(l(pt_skip_bsr_acquisition));

        //         Perform acquisition of SNC through BSR
        SNC_hw_sys_bsr_acquire(BSR_PERIPH_ID_SNC);

        SENIS_label(pt_skip_bsr_acquisition);
        // }

        // snc_critical_cnt++;
        SENIS_inc1(da(&snc_critical_cnt));
}

void snc_critical_section_leave(b_ctx_t* b_ctx)
{
        ASSERT_WARNING(b_ctx);

        SENIS_labels(pt_skip_cnt_dec, pt_skip_release);

        // if (snc_critical_cnt >= 2) {
        SENIS_rdcgr(da(&snc_const[2]), da(&snc_critical_cnt));
        SENIS_cobr_gr(l(pt_skip_cnt_dec));

        //         snc_critical_cnt--;
        SENIS_tobre(da(&snc_critical_cnt), 0xFFFFFFFF);
        SENIS_inc1(da(&snc_critical_cnt));
        SENIS_tobre(da(&snc_critical_cnt), 0xFFFFFFFF);

        SENIS_goto(l(pt_skip_release));
        // } else {
        SENIS_label(pt_skip_cnt_dec);

        //         snc_critical_cnt = 0;
        SENIS_wadva(da(&snc_critical_cnt), 0);

        //         Perform release of SNC through BSR, since no nested critical sections left
        SNC_hw_sys_bsr_release(BSR_PERIPH_ID_SNC);

        SENIS_label(pt_skip_release);
        // }
}

//==================== SNC mutex manipulation functions ========================

void snc_mutex_lock(b_ctx_t* b_ctx, snc_cm33_mutex_t* mutex)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(mutex);

        SENIS_labels(pt_wait_for_mutex);

        // Set the snc_semph to 1
        SENIS_wadva(da(&mutex->snc_semph), 1);

        SENIS_label(pt_wait_for_mutex);

        SENIS_rdcbi(da(&mutex->cm33_semph), 0);
        SENIS_cobr_eq(l(pt_wait_for_mutex));
}

void snc_mutex_unlock(b_ctx_t* b_ctx, snc_cm33_mutex_t* mutex)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(mutex);

        SENIS_wadva(da(&mutex->snc_semph), 0x0);
}

//==================== SNC uCodes manipulating shared resources with SYSCPU functions =

void snc_ucode_rsrc_acquire(b_ctx_t* b_ctx, SNC_UCODE_TYPE ucode_type,
        snc_ucode_context_t* ucode_ctx)
{
        ASSERT_WARNING(b_ctx);

        /* The assertion is here to indicate that uCode resource acquisition has been requested for
         * a uCode type other than uCode-Block, which is not permitted.
         */
        ASSERT_WARNING(ucode_type == SNC_UCODE_TYPE_UCODE_BLOCK);

        SENIS_labels(pt_wait_for_resource);

        // Set the uCode flag used for the resource
        SENIS_tobre(da(&ucode_ctx->flags), (1 << SNC_UCODE_BLOCK_FLAG_SNC_CTRL));

        SENIS_label(pt_wait_for_resource);

        SENIS_rdcbi(da(&ucode_ctx->cmd), SNC_UCODE_BLOCK_CMD_CM33_CTRL);
        SENIS_cobr_eq(l(pt_wait_for_resource));
}

void snc_ucode_rsrc_release(b_ctx_t* b_ctx, SNC_UCODE_TYPE ucode_type,
        snc_ucode_context_t* ucode_ctx)
{
        ASSERT_WARNING(b_ctx);

        /* The assertion is here to indicate that uCode resource release has been requested for
         * a uCode type other than uCode-Block, which is not permitted.
         */
        ASSERT_WARNING(ucode_type == SNC_UCODE_TYPE_UCODE_BLOCK);

        // Clear the uCode flag used for the resource
        SENIS_tobre(da(&ucode_ctx->flags), (1 << SNC_UCODE_BLOCK_FLAG_SNC_CTRL));
}

//==================== SNC uCodes exchanging events with SYSCPU functions ======

SNC_FUNC_DECL(snc_cm33_notify_ucode, uint32_t ucode_id_bitmask);

void snc_cm33_notify(b_ctx_t* b_ctx)
{
        ASSERT_WARNING(b_ctx);

        if (b_ctx->ucode_type == SNC_UCODE_TYPE_UCODE_BLOCK) {
                snc_ucode_context_t* ucode_this_ctx = b_ctx->ucode_this_ctx;

                senis_call(b_ctx, SNC_UCODE_CTX(snc_cm33_notify_ucode),
                        2 * 1, _SNC_OP(1 << (ucode_this_ctx->ucode_id)));
        }
}

SNC_FUNC_DEF(snc_cm33_notify_ucode, uint32_t ucode_id_bitmask)
{
        SENIS_labels(notif_sent, ph_tobre_ucode_id_mask);

        _SNC_TMP_ADD(uint32_t, temp_snc_to_CM33_trigger, sizeof(uint32_t));

        SNC_ENTER_CRITICAL_SECTION();

        SENIS_wadad(da(temp_snc_to_CM33_trigger), da(&snc_context.snc_to_CM33_trigger));

        SENIS_wadad(l(ph_tobre_ucode_id_mask) + 1, da(&SNC_ARG(ucode_id_bitmask)));
        SENIS_label(ph_tobre_ucode_id_mask);
        senis_tobre(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(temp_snc_to_CM33_trigger)), SNC_PLACE_HOLDER);

        SENIS_rdcgr(da(&snc_context.snc_to_CM33_trigger), da(temp_snc_to_CM33_trigger));
        SENIS_cobr_gr(l(notif_sent));
        SENIS_wadad(da(&snc_context.snc_to_CM33_trigger), da(temp_snc_to_CM33_trigger));
        SENIS_assign(da(&SNC->SNC_CTRL_REG),
                (REG_MSK(SNC, SNC_CTRL_REG, SNC_IRQ_CONFIG)) |
                (REG_MSK(SNC, SNC_CTRL_REG, SNC_IRQ_EN)));

        SENIS_label(notif_sent);

        SNC_LEAVE_CRITICAL_SECTION();

        _SNC_TMP_RMV(temp_snc_to_CM33_trigger);
}

void snc_check_cm33_notif_pending(b_ctx_t* b_ctx, SENIS_OPER_TYPE is_pending_type,
        uint32_t* is_pending)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(is_pending_type < SENIS_OPER_TYPE_VALUE);

        if (b_ctx->ucode_type == SNC_UCODE_TYPE_UCODE_BLOCK) {
                snc_ucode_context_t* ucode_this_ctx = b_ctx->ucode_this_ctx;

                // Acquire access to the resource of the uCode
                snc_ucode_rsrc_acquire(b_ctx, b_ctx->ucode_type, ucode_this_ctx);

                if (is_pending != NULL) {
                        senis_wadad(b_ctx, is_pending_type, is_pending,
                                _SNC_OP(da(&ucode_this_ctx->CM33_to_SNC_triggered)));
                }

                senis_wadva(b_ctx, _SNC_OP(da(&ucode_this_ctx->CM33_to_SNC_triggered)), 0); // clear flag

                snc_ucode_rsrc_release(b_ctx, b_ctx->ucode_type, ucode_this_ctx);
        }
}

//==================== SNC-main-uCode control functions ========================

#if dg_configUSE_HW_SYS

SNC_FUNC_DECL(snc_hw_sys_reg_apply_config_ucode, uint32_t* sys_reg_config_table,
        uint32_t num_of_entries);

static void snc_hw_sys_reg_apply_config(b_ctx_t* b_ctx,
        SENIS_OPER_TYPE sys_reg_config_table_type, uint32_t* sys_reg_config_table,
        SENIS_OPER_TYPE num_of_entries_type, uint32_t* num_of_entries)
{
        senis_call(b_ctx, SNC_UCODE_CTX(snc_hw_sys_reg_apply_config_ucode), 2 * 2,
                sys_reg_config_table_type + 1, sys_reg_config_table,
                num_of_entries_type, num_of_entries);
}

SNC_FUNC_DEF(snc_hw_sys_reg_apply_config_ucode, uint32_t* sys_reg_config_table,
        uint32_t num_of_entries)
{
        uint32_t** cur_sys_reg_config_p = &SNC_ARG(sys_reg_config_table);
        uint32_t* num_of_entries_p = &SNC_ARG(num_of_entries);

        SENIS_labels(
                pt_blk_cfg_system_regs_begin, pt_blk_cfg_system_regs_cond,
                ph_cfg_system_reg_assign
        );

        // Initially cur_sys_reg_config_p points to the first register address

        SENIS_tobre(da(num_of_entries_p), 0xffffffff);

        SENIS_goto(l(pt_blk_cfg_system_regs_cond));

        SENIS_label(pt_blk_cfg_system_regs_begin); {
                SENIS_wadad(l(ph_cfg_system_reg_assign), ia(cur_sys_reg_config_p));
                SENIS_tobre(l(ph_cfg_system_reg_assign),
                        _SENIS_1ST_OPERAND(_SENIS_B_WADAD_DI(SENIS_REGS_BASE_ADDRESS,
                                cur_sys_reg_config)) ^
                        SENIS_REGS_BASE_ADDRESS);

                // cur_sys_reg_config_p points to the next &hw_sys_reg_config_t->value
                SENIS_inc4(da(cur_sys_reg_config_p));

                SENIS_label(ph_cfg_system_reg_assign);
                senis_wadad(b_ctx, _SNC_OP(da(SNC_PLACE_HOLDER)),
                        _SNC_OP(ia(cur_sys_reg_config_p)));    // Write value to register

                // cur_sys_reg_config_p points to the next &hw_sys_reg_config_t->addr
                SENIS_inc4(da(cur_sys_reg_config_p));

                SENIS_label(pt_blk_cfg_system_regs_cond);

                SENIS_inc1(da(num_of_entries_p));
                SENIS_rdcbi(da(num_of_entries_p), 31);
                SENIS_cobr_eq(l(pt_blk_cfg_system_regs_begin));
        }
}
#endif /* dg_configUSE_HW_SYS */

void snc_hw_sys_init(b_ctx_t* b_ctx)
{
        ASSERT_WARNING(b_ctx);

        /* Initialize GPIO power rails configuration */
        extern uint32_t io_static_power_configuration[HW_GPIO_NUM_PORTS];
        SENIS_assign(da(&GPIO->P0_PADPWR_CTRL_REG), da(&io_static_power_configuration[HW_GPIO_PORT_0]));
        SENIS_assign(da(&GPIO->P1_PADPWR_CTRL_REG), da(&io_static_power_configuration[HW_GPIO_PORT_1]));

#if dg_configUSE_HW_SYS
        // Apply DCDC Converter registers configuration
        SNC_hw_sys_reg_apply_config(da(hw_sys_reg_get_config(0)),
                da(hw_sys_reg_get_num_of_config_entries()));

        // Apply TCS registers group configuration for PD_COMM
        SNC_hw_sys_reg_apply_config(da(sys_tcs_snc_get_reg_pair(SYS_TCS_GROUP_PD_COMM)),
                sys_tcs_snc_get_reg_pair_num_of_entries(SYS_TCS_GROUP_PD_COMM));

        // Apply TCS registers group configuration for PD_PER
        SNC_hw_sys_reg_apply_config(da(sys_tcs_snc_get_reg_pair(SYS_TCS_GROUP_PD_PER)),
                sys_tcs_snc_get_reg_pair_num_of_entries(SYS_TCS_GROUP_PD_PER));
#endif /* dg_configUSE_HW_SYS */
}

void snc_hw_sys_prevent_sleep(b_ctx_t* b_ctx)
{
        ASSERT_WARNING(b_ctx);

        // snc_context.prevent_slp_cnt++; // Preventing SNC from going to sleep
        SENIS_inc1(da(&snc_context.prevent_slp_cnt));
}

void snc_hw_sys_allow_sleep(b_ctx_t* b_ctx)
{
        ASSERT_WARNING(b_ctx);

        SENIS_labels(pt_skip_dec);

        // if (snc_context.prevent_slp_cnt > 0) {
        SENIS_rdcgr(da(&snc_const[1]), da(&snc_context.prevent_slp_cnt));
        SENIS_cobr_gr(l(pt_skip_dec));

        //         snc_context.prevent_slp_cnt--; // Allowing SNC to go to sleep
        SENIS_tobre(da(&snc_context.prevent_slp_cnt), 0xFFFFFFFF);
        SENIS_inc1(da(&snc_context.prevent_slp_cnt));
        SENIS_tobre(da(&snc_context.prevent_slp_cnt), 0xFFFFFFFF);

        SENIS_label(pt_skip_dec);
        // }
}

//==================== Peripheral Acquisition functions ========================

static void snc_hw_sys_bsr_lock(b_ctx_t* b_ctx)
{
        SENIS_labels(blk_while_begin, blk_while_end);

        /* Use HW BSR bits [0:1] to access SW BSR */
        uint32_t hw_bsr_master_id = HW_BSR_MASTER_SNC;

        // uint32_t hw_bsr_mask =  SW_BSR_HW_BSR_MASK;
        //
        // do {
        SENIS_label(blk_while_begin);

        //         MEMCTRL->BUSY_SET_REG = hw_bsr_master_id;
        SENIS_wadva(da(&MEMCTRL->BUSY_SET_REG), hw_bsr_master_id);

        SENIS_wadad(da(&SNC->SNC_STATUS_REG), da(&MEMCTRL->BUSY_STAT_REG));
        SENIS_cobr_gr(l(blk_while_begin));
        SENIS_cobr_eq(l(blk_while_end));
        SENIS_goto(l(blk_while_begin));
        //
        SENIS_label(blk_while_end);
        // } while ((MEMCTRL->BUSY_STAT_REG & hw_bsr_mask) != hw_bsr_master_id);
}

static void snc_hw_sys_bsr_unlock(b_ctx_t* b_ctx)
{
        // MEMCTRL->BUSY_RESET_REG = HW_BSR_MASTER_SNC;
        SENIS_wadva(da(&MEMCTRL->BUSY_RESET_REG), HW_BSR_MASTER_SNC);
}

SNC_FUNC_DECL(snc_hw_sys_bsr_try_acquire_ucode, uint32_t* sw_bsr_addr, uint32_t* is_acquired);

void snc_hw_sys_bsr_try_acquire(b_ctx_t* b_ctx, HW_SYS_BSR_PERIPH_ID perif_id,
        SENIS_OPER_TYPE is_acquired_type, uint32_t* is_acquired)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(perif_id < BSR_PERIPH_ID_MAX);
        ASSERT_WARNING((is_acquired_type < SENIS_OPER_TYPE_VALUE) &&
                (!b_ctx->upd || !_SNC_ADDR_IS_REG(is_acquired)));

        _SNC_TMP_ADD(uint32_t, temp_is_acquired, sizeof(uint32_t));

        if (is_acquired == NULL) {
                is_acquired_type = SENIS_OPER_TYPE_ADDRESS_DA;
                is_acquired = temp_is_acquired;
        }

        senis_call(b_ctx, SNC_UCODE_CTX(snc_hw_sys_bsr_try_acquire_ucode),
                2 * 2, _SNC_OP(&hw_sys_sw_bsr[perif_id]), is_acquired_type + 1, is_acquired);

        _SNC_TMP_RMV(temp_is_acquired);
}

SNC_FUNC_DEF(snc_hw_sys_bsr_try_acquire_ucode, uint32_t* sw_bsr_addr, uint32_t* is_acquired)
{
        SENIS_labels(pt_fail);

        _SNC_TMP_ADD(uint32_t, temp_sw_bsr_val, sizeof(uint32_t));

        // *is_acquired = false;
        SENIS_wadva(ia(&SNC_ARG(is_acquired)), 0);

        // Acquire exclusive access to peripheral's SW BSR
        SNC_hw_sys_bsr_lock();

        // if (hw_sys_sw_bsr[perif_id] > SW_BSR_MASTER_NONE) {
        SENIS_wadad(da(temp_sw_bsr_val), ia(&SNC_ARG(sw_bsr_addr)));
        SENIS_rdcgr(da(temp_sw_bsr_val), da(&snc_const[SW_BSR_MASTER_NONE]));
        SENIS_cobr_gr(l(pt_fail));

        //         Update SW BSR with SNC master ID
        SENIS_wadva(ia(&SNC_ARG(sw_bsr_addr)), SW_BSR_MASTER_SNC);
        //         *is_acquired = true;
        SENIS_wadva(ia(&SNC_ARG(is_acquired)), 1);

        SENIS_label(pt_fail);
        // }

        // Release exclusive access to peripheral's SW BSR
        SNC_hw_sys_bsr_unlock();

        _SNC_TMP_RMV(temp_sw_bsr_val);
}

void snc_hw_sys_bsr_acquire(b_ctx_t* b_ctx, HW_SYS_BSR_PERIPH_ID perif_id)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(perif_id < BSR_PERIPH_ID_MAX);

        SENIS_labels(blk_while_begin);

        _SNC_TMP_ADD(uint32_t, tmp_is_acquired, sizeof(uint32_t));

        // while (!hw_sys_bsr_try_acquire(perif_id)) {
        SENIS_label(blk_while_begin);
        //
        SNC_hw_sys_bsr_try_acquire(perif_id, da(tmp_is_acquired));
        SENIS_rdcgr(da(&snc_const[1]), da(tmp_is_acquired));
        SENIS_cobr_gr(l(blk_while_begin));
        // }

        _SNC_TMP_RMV(tmp_is_acquired);
}

SNC_FUNC_DECL(snc_hw_sys_bsr_release_ucode, uint32_t* sw_bsr_addr);

void snc_hw_sys_bsr_release(b_ctx_t* b_ctx, HW_SYS_BSR_PERIPH_ID perif_id)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(perif_id < BSR_PERIPH_ID_MAX);

        senis_call(b_ctx, SNC_UCODE_CTX(snc_hw_sys_bsr_release_ucode),
                2 * 1, _SNC_OP(&hw_sys_sw_bsr[perif_id]));
}

SNC_FUNC_DEF(snc_hw_sys_bsr_release_ucode, uint32_t* sw_bsr_addr)
{
        // Acquire exclusive access to peripheral's SW BSR
        SNC_hw_sys_bsr_lock();

        SNC_ASSERT(ia(&SNC_ARG(sw_bsr_addr)), BIT, HW_BSR_MASTER_SNC);

        // Update SW BSR with NONE-master-ID
        SENIS_wadva(ia(&SNC_ARG(sw_bsr_addr)), SW_BSR_MASTER_NONE);

        // Release exclusive access to peripheral's SW BSR
        SNC_hw_sys_bsr_unlock();
}

//==================== PDC event entry acknowledgment functions ================

void snc_hw_sys_pdc_acknowledge(b_ctx_t* b_ctx, SENIS_OPER_TYPE idx_type, uint32_t* idx)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(idx_type <= SENIS_OPER_TYPE_VALUE);

        senis_assign(b_ctx, _SNC_OP(da(&PDC->PDC_ACKNOWLEDGE_REG)), idx_type, idx);
}

void snc_hw_sys_get_pdc_pending(b_ctx_t* b_ctx, SENIS_OPER_TYPE pending_type, uint32_t* pending)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(pending_type < SENIS_OPER_TYPE_VALUE);

        senis_assign(b_ctx, pending_type, pending, _SNC_OP(da(&PDC->PDC_PENDING_SNC_REG)));
}

//==================== Clearing peripheral events functions ====================

void snc_hw_sys_clear_wkup_status(b_ctx_t* b_ctx, HW_GPIO_PORT port, uint32_t status)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(port <= HW_GPIO_PORT_1);

        SENIS_wadva(da((&WAKEUP->WKUP_CLEAR_P0_REG) + port), status);
}

void snc_hw_sys_clear_timer_gpio_event(b_ctx_t* b_ctx, uint32_t mask)
{
        ASSERT_WARNING(b_ctx);

        SENIS_wadva(da(&TIMER->TIMER_CLEAR_GPIO_EVENT_REG), mask);
}

void snc_hw_sys_clear_timer_interrupt(b_ctx_t* b_ctx, HW_TIMER_ID id)
{
        ASSERT_WARNING(b_ctx);

        if (id == HW_TIMER) {
                SENIS_wadva(da(&((TIMER_Type *)id)->TIMER_CLEAR_IRQ_REG), 0);
        } else if (id == HW_TIMER2) {
                SENIS_wadva(da(&((TIMER2_Type *)id)->TIMER2_CLEAR_IRQ_REG), 0);
        } else if (id == HW_TIMER3) {
                SENIS_wadva(da(&((TIMER3_Type *)id)->TIMER3_CLEAR_IRQ_REG), 0);
        } else if (id == HW_TIMER4) {
                SENIS_wadva(da(&((TIMER4_Type *)id)->TIMER4_CLEAR_IRQ_REG), 0);
        } else {
                ASSERT_WARNING(0);
        }
}

void snc_hw_sys_get_rtc_event_flags(b_ctx_t* b_ctx, SENIS_OPER_TYPE flags_type, uint32_t* flags)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(flags_type < SENIS_OPER_TYPE_VALUE);

        senis_assign(b_ctx, flags_type, flags, _SNC_OP(da(&RTC->RTC_EVENT_FLAGS_REG)));
}

void snc_hw_sys_clear_rtc_pdc_event(b_ctx_t* b_ctx)
{
        ASSERT_WARNING(b_ctx);

        SENIS_rdcbi(da(&RTC->RTC_PDC_EVENT_CLEAR_REG), 0);
}

//==================== RTC Time/Calendar Getter functions ======================

void snc_hw_sys_rtc_get_time_bcd(b_ctx_t* b_ctx, SENIS_OPER_TYPE time_type, uint32_t* time)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(time_type < SENIS_OPER_TYPE_VALUE);

        senis_assign(b_ctx, time_type, time, _SNC_OP(da(&RTC->RTC_TIME_REG)));
}

void snc_hw_sys_rtc_get_clndr_bcd(b_ctx_t* b_ctx, SENIS_OPER_TYPE clndr_type, uint32_t* clndr)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(clndr_type < SENIS_OPER_TYPE_VALUE);

        senis_assign(b_ctx, clndr_type, clndr, _SNC_OP(da(&RTC->RTC_CALENDAR_REG)));
}

//==================== Timer Capture Getter functions ==========================

void snc_hw_sys_timer_get_capture1(b_ctx_t* b_ctx, HW_TIMER_ID id, SENIS_OPER_TYPE cap_type,
        uint32_t* cap)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(cap_type < SENIS_OPER_TYPE_VALUE);

        if (id == HW_TIMER) {
                senis_assign(b_ctx, cap_type, cap,
                        _SNC_OP(da(&((TIMER_Type *)id)->TIMER_CAPTURE_GPIO1_REG)));
        } else if (id == HW_TIMER2) {
                senis_assign(b_ctx, cap_type, cap,
                        _SNC_OP(da(&((TIMER2_Type *)id)->TIMER2_CAPTURE_GPIO1_REG)));
        } else if (id == HW_TIMER3) {
                senis_assign(b_ctx, cap_type, cap,
                        _SNC_OP(da(&((TIMER3_Type *)id)->TIMER3_CAPTURE_GPIO1_REG)));
        } else if (id == HW_TIMER4) {
                senis_assign(b_ctx, cap_type, cap,
                        _SNC_OP(da(&((TIMER4_Type *)id)->TIMER4_CAPTURE_GPIO1_REG)));
        } else {
                ASSERT_WARNING(id);
        }
}

void snc_hw_sys_timer_get_capture2(b_ctx_t* b_ctx, HW_TIMER_ID id, SENIS_OPER_TYPE cap_type,
        uint32_t* cap)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(cap_type < SENIS_OPER_TYPE_VALUE);

        if (id == HW_TIMER) {
                senis_assign(b_ctx, cap_type, cap,
                        _SNC_OP(da(&((TIMER_Type *)id)->TIMER_CAPTURE_GPIO2_REG)));
        } else if (id == HW_TIMER2) {
                senis_assign(b_ctx, cap_type, cap,
                        _SNC_OP(da(&((TIMER2_Type *)id)->TIMER2_CAPTURE_GPIO2_REG)));
        } else if (id == HW_TIMER3) {
                senis_assign(b_ctx, cap_type, cap,
                        _SNC_OP(da(&((TIMER3_Type *)id)->TIMER3_CAPTURE_GPIO2_REG)));
        } else if (id == HW_TIMER4) {
                senis_assign(b_ctx, cap_type, cap,
                        _SNC_OP(da(&((TIMER4_Type *)id)->TIMER4_CAPTURE_GPIO2_REG)));
        } else {
                ASSERT_WARNING(id);
        }
}

void snc_hw_sys_timer_get_capture3(b_ctx_t* b_ctx, SENIS_OPER_TYPE cap_type, uint32_t* cap)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(cap_type < SENIS_OPER_TYPE_VALUE);

        senis_assign(b_ctx, cap_type, cap,
                _SNC_OP(da(&TBA(HW_TIMER)->TIMER_CAPTURE_GPIO3_REG)));
}

void snc_hw_sys_timer_get_capture4(b_ctx_t* b_ctx, SENIS_OPER_TYPE cap_type, uint32_t* cap)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(cap_type < SENIS_OPER_TYPE_VALUE);

        senis_assign(b_ctx, cap_type, cap,
                _SNC_OP(da(&TBA(HW_TIMER)->TIMER_CAPTURE_GPIO4_REG)));
}

//==================== System Timer Getter functions ===========================

#if (defined(OS_FREERTOS) && dg_configUSE_HW_TIMER)
SNC_FUNC_DECL(snc_hw_sys_timer_get_uptime_ticks_ucode, snc_hw_sys_uptime_ticks_t* uptime_ticks);

void snc_hw_sys_timer_get_uptime_ticks(b_ctx_t* b_ctx, SENIS_OPER_TYPE uptime_ticks_type,
        uint32_t* uptime_ticks)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(uptime_ticks_type < SENIS_OPER_TYPE_VALUE);

        uint32_t** applied_uptime_ticks_ind;

        _SNC_TMP_ADD(uint32_t*, temp_uptime_ticks, sizeof(uint32_t*));

        if (uptime_ticks_type == SENIS_OPER_TYPE_ADDRESS_DA) {
                senis_assign(b_ctx, _SNC_OP(da(temp_uptime_ticks)), _SNC_OP(uptime_ticks));
                applied_uptime_ticks_ind = temp_uptime_ticks;
        } else {
                applied_uptime_ticks_ind = (uint32_t**)uptime_ticks;
        }

        senis_call(b_ctx, SNC_UCODE_CTX(snc_hw_sys_timer_get_uptime_ticks_ucode), 2 * 1,
                _SNC_OP(applied_uptime_ticks_ind));

        _SNC_TMP_RMV(temp_uptime_ticks);
}

SNC_FUNC_DEF(snc_hw_sys_timer_get_uptime_ticks_ucode, snc_hw_sys_uptime_ticks_t** uptime_ticks_ind)
{
        uint64_t* p_rtc_time = sys_timer_get_rtc_time();
        uint32_t* p_current_time = sys_timer_get_current_time();

        SENIS_labels(pt_update_uptime_ticks_values)

        _SNC_TMP_ADD(snc_hw_sys_uptime_ticks_t*, p_uptime_ticks_ind, sizeof(snc_hw_sys_uptime_ticks_t*));
        _SNC_TMP_ADD(uint32_t, prev_current_time, sizeof(uint32_t));
        _SNC_TMP_ADD(uint32_t, new_time_val, sizeof(uint32_t));

        SENIS_label(pt_update_uptime_ticks_values); {
                senis_assign(b_ctx, _SNC_OP(da(p_uptime_ticks_ind)),
                        _SNC_OP(ia(&SNC_ARG(uptime_ticks_ind))));

                // Acquire system timer uptime ticks related variables values
                senis_assign(b_ctx, _SNC_OP(ia(p_uptime_ticks_ind)),
                        _SNC_OP(da(&TBA(HW_TIMER2)->TIMER_TIMER_VAL_REG)));
                senis_inc4(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(p_uptime_ticks_ind)));
                senis_assign(b_ctx, _SNC_OP(da(prev_current_time)), _SNC_OP(da(p_current_time)));
                senis_assign(b_ctx, _SNC_OP(ia(p_uptime_ticks_ind)), _SNC_OP(da(prev_current_time)));
                senis_inc4(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(p_uptime_ticks_ind)));
                senis_assign(b_ctx, _SNC_OP(ia(p_uptime_ticks_ind)), _SNC_OP(da(&((uint32_t*)p_rtc_time)[0])));
                senis_inc4(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(p_uptime_ticks_ind)));
                senis_assign(b_ctx, _SNC_OP(ia(p_uptime_ticks_ind)), _SNC_OP(da(&((uint32_t*)p_rtc_time)[1])));

                // Check if current time has changed; if yes update the SNC uptime ticks again
                senis_assign(b_ctx, _SNC_OP(da(new_time_val)), _SNC_OP(da(p_current_time)));
                senis_rdcgr(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(prev_current_time)),
                        _SNC_OP_DIRECT_ADDRESS(da(new_time_val)));
                senis_cobr_gr(b_ctx, _SNC_OP(l(pt_update_uptime_ticks_values)));
                senis_rdcgr(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(new_time_val)),
                        _SNC_OP_DIRECT_ADDRESS(da(prev_current_time)));
                senis_cobr_gr(b_ctx, _SNC_OP(l(pt_update_uptime_ticks_values)));
        }

        senis_inc4(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(p_uptime_ticks_ind)));

        senis_assign(b_ctx, _SNC_OP(ia(&SNC_ARG(uptime_ticks_ind))), _SNC_OP(da(p_uptime_ticks_ind)));

        _SNC_TMP_RMV(new_time_val);
        _SNC_TMP_RMV(prev_current_time);
        _SNC_TMP_RMV(p_uptime_ticks_ind);
}
#endif /* defined(OS_FREERTOS) && dg_configUSE_HW_TIMER */

#endif /* dg_configUSE_HW_SENSOR_NODE */


/**
 * \}
 * \}
 */
