/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SNC_DEBUG
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file snc_debug.c
 *
 * @brief Implementation of SNC-Debugging Sensor Node Controller (SNC) API
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#if dg_configUSE_HW_SENSOR_NODE

#if dg_configUSE_SNC_DEBUGGER

#include "snc_defs.h"

#include "snc_debug.h"

/*
 * FUNCTION DEFINITIONS
 *****************************************************************************************
 */

void snc_hw_sys_bkpt(b_ctx_t* b_ctx, bkpt_func_t bkpt_func, bkpt_control_func_t bkpt_control_func)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(bkpt_func);

        if (!b_ctx->debug_en) {
                return;
        }

        if (bkpt_control_func) {
                SENIS_labels(wait_while_snc_stopped);

                _SNC_TMP_ADD(uint32_t, tmp_snc_status_reg, sizeof(uint32_t));

                // Set the snc_bkpt context variable equal to the address of the function defined
                // for the particular breakpoint in uCode implementation
                senis_assign(b_ctx, _SNC_OP(da(&snc_context.snc_bkpt)), _SNC_OP(bkpt_func));
                // Set the snc_bkpt_control context variable equal to the address of the function defined
                // for controlling the breakpoint set to its prototype and "halting" SNC
                senis_assign(b_ctx, _SNC_OP(da(&snc_context.snc_bkpt_control)),
                        _SNC_OP(bkpt_control_func));

                // Send a notification
                senis_assign(b_ctx, _SNC_OP(da(&SNC->SNC_CTRL_REG)),
                        _SNC_OP((REG_MSK(SNC, SNC_CTRL_REG, SNC_IRQ_CONFIG)) |
                                (REG_MSK(SNC, SNC_CTRL_REG, SNC_IRQ_EN)) |
                                (REG_MSK(SNC, SNC_CTRL_REG, SNC_SW_CTRL))));

                senis_assign(b_ctx, _SNC_OP(da(tmp_snc_status_reg)),
                        _SNC_OP(da(&SNC->SNC_STATUS_REG)));

                // Wait until the SNC_SW_CTRL bit is cleared
                SENIS_label(wait_while_snc_stopped);
                //
                senis_rdcbi(b_ctx, (uint32_t*)&SNC->SNC_CTRL_REG,
                        REG_POS(SNC, SNC_CTRL_REG, SNC_SW_CTRL));
                senis_cobr_eq(b_ctx, _SNC_OP(l(wait_while_snc_stopped)));

                senis_assign(b_ctx, _SNC_OP(da(&SNC->SNC_STATUS_REG)), _SNC_OP(da(tmp_snc_status_reg)));

                _SNC_TMP_RMV(tmp_snc_status_reg);
        }
#if dg_configUSE_HW_SENSOR_NODE_EMU
        else {
                ASSERT_WARNING(b_ctx->bkpt_emu_sbs_funcs);
                if (b_ctx->bkpt_emu_sbs_funcs[b_ctx->upd & b_ctx->index] == NULL) {
                        b_ctx->bkpt_emu_sbs_funcs[b_ctx->upd & b_ctx->index] = bkpt_func;
                }
        }
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */
}

void snc_process_bkpt(void)
{
        if (snc_context.snc_bkpt) {
                uint32_t snc_ctrl_reg_local = 0;

                // Halt CM33 execution in order to catch the SNC breakpoint
                snc_context.snc_bkpt(snc_context.snc_bkpt_control);

                // Clear the breakpoint context variable
                snc_context.snc_bkpt = NULL;

                // Un - halt the Sensor Node Controller
#if dg_configUSE_HW_SENSOR_NODE_EMU
                HW_SNC_REG_SETF(SNC_CTRL_REG, SNC_SW_CTRL, 0);
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */
                REG_SET_FIELD(SNC, SNC_CTRL_REG, SNC_EN, snc_ctrl_reg_local, 1);
                REG_SET_FIELD(SNC, SNC_CTRL_REG, SNC_SW_CTRL, snc_ctrl_reg_local, 0);
                SNC->SNC_CTRL_REG = snc_ctrl_reg_local;
        }
}

#if dg_configUSE_HW_SENSOR_NODE_EMU
void snc_process_bkpt_emu(uint32_t pc)
{
        snc_emu_sbs_attrs_t* cur_sbs_attrs_p = (snc_emu_sbs_attrs_t*)snc_context.snc_emu_dbg_list;
        uint32_t* pc_addr = (uint32_t*)pc;

        while (cur_sbs_attrs_p) {
                uint32_t* cur_ucode_first_addr = cur_sbs_attrs_p->ucode_start_address;
                uint32_t* cur_ucode_last_addr =
                        &cur_sbs_attrs_p->ucode_start_address[cur_sbs_attrs_p->ucode_size];

                if (pc_addr < cur_ucode_last_addr && pc_addr >= cur_ucode_first_addr) {
                        bkpt_func_t* cur_sbs_funcs = cur_sbs_attrs_p->sbs_funcs;

                        if (cur_sbs_funcs) {
                                bkpt_func_t cur_sbs_func =
                                        (cur_sbs_funcs[(uint32_t)(pc_addr - cur_ucode_first_addr)]);

                                if (cur_sbs_func) {
                                        cur_sbs_func(snc_bkpt_control_emu_group_func);
                                }
                        }

                        break;
                }

                cur_sbs_attrs_p = cur_sbs_attrs_p->next;
        }
}
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */

#endif /* dg_configUSE_SNC_DEBUGGER */

#endif /* dg_configUSE_HW_SENSOR_NODE */


/**
 * \}
 * \}
 */

