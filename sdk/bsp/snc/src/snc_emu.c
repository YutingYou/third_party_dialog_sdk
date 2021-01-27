/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SENSOR_NODE_EMU
 * \{
 */

/**
 *****************************************************************************************
 *
 * @file snc_emu.c
 *
 * @brief Implementation of Sensor Node Controller Emulator Low Level Driver
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#if dg_configUSE_HW_SENSOR_NODE & dg_configUSE_HW_SENSOR_NODE_EMU

#include "snc_defs.h"

#include "snc_emu.h"

/*
 * ENUMERATION DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief Sensor Node IRQ configuration types
 *
 */
typedef enum {
        HW_SNC_IRQ_CONFIG_NONE = 0,
        HW_SNC_IRQ_CONFIG_CM33,
        HW_SNC_IRQ_CONFIG_PDC,
        HW_SNC_IRQ_CONFIG_CM33_AND_PDC,
} HW_SNC_IRQ_CONFIG;

/*
 * DATA STRUCTURE DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief Sensor Node Emulator local interrupt callback
 *
 */
__RETAINED static hw_snc_interrupt_cb_t snc_intr_cb;

/**
 * \brief Sensor Node Emulator local delay function (in low power clock ticks)
 *
 */
__RETAINED static hw_snc_emu_delay_cb_t delay_cb;

/**
 * \brief Sensor Node Emulator local execution indication function
 *
 */
__RETAINED static hw_snc_emu_exec_ind_cb_t emu_exec_ind_cb;

/**
 * \brief Sensor Node Emulator Context Structure
 *
 */
__RETAINED_RW static struct hw_snc_emu_context_t {
        uint32_t SNC_CTRL_REG;
        uint32_t SNC_STATUS_REG;
        uint32_t SNC_LP_TIMER_REG;
        uint32_t SNC_PC_REG;
        uint32_t SNC_R1_REG;
        uint32_t SNC_R2_REG;
        uint32_t SNC_TMP1_REG;
        uint32_t SNC_TMP2_REG;
} hw_snc_emu_context = {
        0x0,
        0x20,
        0x0,
        0x20000000
};

/**
 * \brief Local variable holding the SNC Emulator base address
 *
 */
__RETAINED_RW static uint32_t base_reg = 0x20000000;

/**
 * \brief SNC Emulator execution context status
 *
 */
__RETAINED_RW static struct hw_snc_emu_status_t {
        uint32_t* pc;
        uint32_t branch_loop_cnt;
} hw_snc_emu_status = {
        (uint32_t*)0x20000000,
        0,
};

/*
 * FUNCTION DEFINITIONS
 *****************************************************************************************
 */

static void execute_snc_emu(void);

//===================== Register Read/Write functions ==========================

void hw_snc_SNC_CTRL_REG_setf_SNC_EN(uint32_t val)
{
        REG_SET_FIELD(SNC, SNC_CTRL_REG, SNC_EN, hw_snc_emu_context.SNC_CTRL_REG, val);

        if (REG_GET_FIELD(SNC, SNC_CTRL_REG, SNC_SW_CTRL, hw_snc_emu_context.SNC_CTRL_REG)) {
                if (val == 1) {
                        if (REG_GET_FIELD(SNC, SNC_STATUS_REG, SNC_IS_STOPPED,
                                hw_snc_emu_context.SNC_STATUS_REG) ||
                                REG_GET_FIELD(SNC, SNC_STATUS_REG, SNC_DONE_STATUS, hw_snc_emu_context.SNC_STATUS_REG)) {
                                if (REG_GET_FIELD(SNC, SNC_STATUS_REG, SNC_DONE_STATUS,
                                        hw_snc_emu_context.SNC_STATUS_REG)) {
                                        hw_snc_emu_status.pc = (uint32_t*)hw_snc_get_base_address();
                                }
                                REG_SET_FIELD(SNC, SNC_STATUS_REG, SNC_PC_LOADED,
                                        hw_snc_emu_context.SNC_STATUS_REG, 0);
                                REG_SET_FIELD(SNC, SNC_STATUS_REG, SNC_IS_STOPPED,
                                        hw_snc_emu_context.SNC_STATUS_REG, 0);
                                REG_SET_FIELD(SNC, SNC_STATUS_REG, SNC_DONE_STATUS,
                                        hw_snc_emu_context.SNC_STATUS_REG, 0);
                                hw_snc_emu_context.SNC_PC_REG = (uint32_t)hw_snc_emu_status.pc;
                                hw_snc_emu_status.branch_loop_cnt = 0;

                                execute_snc_emu();
                        }
                } else {
                        hw_snc_emu_status.pc = (uint32_t*)hw_snc_emu_context.SNC_PC_REG;
                        REG_SET_FIELD(SNC, SNC_STATUS_REG, SNC_IS_STOPPED,
                                hw_snc_emu_context.SNC_STATUS_REG, 1);
                }
        }
}

void hw_snc_SNC_CTRL_REG_setf_SNC_SW_CTRL(uint32_t val)
{
        if (REG_GET_FIELD(SNC, SNC_CTRL_REG, SNC_SW_CTRL, hw_snc_emu_context.SNC_CTRL_REG) != val &&
                val == 1) {
                HW_SNC_REG_SETF(SNC_CTRL_REG, SNC_EN, HW_SNC_REG_GETF(SNC_CTRL_REG, SNC_EN));
        }
        REG_SET_FIELD(SNC, SNC_CTRL_REG, SNC_SW_CTRL, hw_snc_emu_context.SNC_CTRL_REG, val);
}

void hw_snc_SNC_CTRL_REG_setf_BUS_ERROR_DETECT_EN(uint32_t val)
{
        REG_SET_FIELD(SNC, SNC_CTRL_REG, BUS_ERROR_DETECT_EN, hw_snc_emu_context.SNC_CTRL_REG, val);
}

void hw_snc_SNC_CTRL_REG_setf_SNC_RESET(uint32_t val)
{
        if (val) {
                hw_snc_emu_status.pc = (uint32_t*)hw_snc_get_base_address();

                if (!REG_GET_FIELD(SNC, SNC_STATUS_REG, SNC_IS_STOPPED,
                        hw_snc_emu_context.SNC_STATUS_REG)) {
                        REG_SET_FIELD(SNC, SNC_STATUS_REG, SNC_PC_LOADED,
                                hw_snc_emu_context.SNC_STATUS_REG, 0);
                        REG_SET_FIELD(SNC, SNC_STATUS_REG, SNC_DONE_STATUS,
                                hw_snc_emu_context.SNC_STATUS_REG, 0);
                        hw_snc_emu_context.SNC_PC_REG = (uint32_t)hw_snc_emu_status.pc;
                        hw_snc_emu_status.branch_loop_cnt = 0;

                        execute_snc_emu();
                }
        }
}

void hw_snc_SNC_CTRL_REG_setf_SNC_BRANCH_LOOP_INIT(uint32_t val)
{
        if (val) {
                hw_snc_emu_status.branch_loop_cnt = 0;
        }
}

void hw_snc_SNC_CTRL_REG_setf_SNC_IRQ_EN(uint32_t val)
{
        REG_SET_FIELD(SNC, SNC_CTRL_REG, SNC_IRQ_EN, hw_snc_emu_context.SNC_CTRL_REG, val);
        REG_SETF(SNC, SNC_CTRL_REG, SNC_IRQ_EN, val);
}

void hw_snc_SNC_CTRL_REG_setf_SNC_IRQ_CONFIG(uint32_t val)
{
        REG_SET_FIELD(SNC, SNC_CTRL_REG, SNC_IRQ_CONFIG, hw_snc_emu_context.SNC_CTRL_REG, val);
        REG_SETF(SNC, SNC_CTRL_REG, SNC_IRQ_CONFIG, val);
}

void hw_snc_SNC_CTRL_REG_setf_SNC_IRQ_ACK(uint32_t val)
{
        if (val) {
                REG_SET_FIELD(SNC, SNC_CTRL_REG, SNC_IRQ_EN, hw_snc_emu_context.SNC_CTRL_REG, 0);
        }
        REG_SETF(SNC, SNC_CTRL_REG, SNC_IRQ_ACK, val);
}

void hw_snc_SNC_STATUS_REG_setf_EQ_FLAG(uint32_t val)
{
        REG_SET_FIELD(SNC, SNC_STATUS_REG, EQ_FLAG, hw_snc_emu_context.SNC_STATUS_REG, val);
}

void hw_snc_SNC_STATUS_REG_setf_GR_FLAG(uint32_t val)
{
        REG_SET_FIELD(SNC, SNC_STATUS_REG, GR_FLAG, hw_snc_emu_context.SNC_STATUS_REG, val);
}

void hw_snc_SNC_STATUS_REG_setf_SNC_DONE_STATUS(uint32_t val)
{
        // do nothing
}

void hw_snc_SNC_STATUS_REG_setf_BUS_ERROR_STATUS(uint32_t val)
{
        // do nothing
}

void hw_snc_SNC_STATUS_REG_setf_HARD_FAULT_STATUS(uint32_t val)
{
        // do nothing
}

void hw_snc_SNC_STATUS_REG_setf_SNC_IS_STOPPED(uint32_t val)
{
        // do nothing
}

void hw_snc_SNC_STATUS_REG_setf_SNC_PC_LOADED(uint32_t val)
{
        // do nothing
}

void hw_snc_SNC_LP_TIMER_REG_setf_LP_TIMER(uint32_t val)
{
        // do nothing
}

void hw_snc_SNC_PC_REG_setf_PC_REG(uint32_t val)
{
        if (REG_GET_FIELD(SNC, SNC_STATUS_REG, SNC_IS_STOPPED, hw_snc_emu_context.SNC_STATUS_REG)) {
                uint32_t pc_local = (uint32_t)hw_snc_emu_status.pc;

                REG_SET_FIELD(SNC, SNC_PC_REG, PC_REG, pc_local, val);
                hw_snc_emu_status.pc = (uint32_t*)pc_local;
                hw_snc_emu_context.SNC_PC_REG = (uint32_t)hw_snc_emu_status.pc;

                REG_SET_FIELD(SNC, SNC_STATUS_REG, SNC_PC_LOADED, hw_snc_emu_context.SNC_STATUS_REG,
                        1);
        }
}

void hw_snc_SNC_R1_REG_setf_R1_REG(uint32_t val)
{
        // do nothing
}

void hw_snc_SNC_R2_REG_setf_R2_REG(uint32_t val)
{
        // do nothing
}

void hw_snc_SNC_TMP1_REG_setf_TMP1_REG(uint32_t val)
{
        // do nothing
}

void hw_snc_SNC_TMP2_REG_setf_TMP2_REG(uint32_t val)
{
        // do nothing
}

uint32_t hw_snc_SNC_CTRL_REG_getf_SNC_EN(void)
{
        return REG_GET_FIELD(SNC, SNC_CTRL_REG, SNC_EN, hw_snc_emu_context.SNC_CTRL_REG);
}

uint32_t hw_snc_SNC_CTRL_REG_getf_SNC_SW_CTRL(void)
{
        return REG_GET_FIELD(SNC, SNC_CTRL_REG, SNC_SW_CTRL, hw_snc_emu_context.SNC_CTRL_REG);
}

uint32_t hw_snc_SNC_CTRL_REG_getf_BUS_ERROR_DETECT_EN(void)
{
        return REG_GET_FIELD(SNC, SNC_CTRL_REG, BUS_ERROR_DETECT_EN,
                hw_snc_emu_context.SNC_CTRL_REG);
}

uint32_t hw_snc_SNC_CTRL_REG_getf_SNC_RESET(void)
{
        return REG_GET_FIELD(SNC, SNC_CTRL_REG, SNC_RESET, hw_snc_emu_context.SNC_CTRL_REG);
}

uint32_t hw_snc_SNC_CTRL_REG_getf_SNC_BRANCH_LOOP_INIT(void)
{
        return REG_GET_FIELD(SNC, SNC_CTRL_REG, SNC_BRANCH_LOOP_INIT,
                hw_snc_emu_context.SNC_CTRL_REG);
}

uint32_t hw_snc_SNC_CTRL_REG_getf_SNC_IRQ_EN(void)
{
        return REG_GET_FIELD(SNC, SNC_CTRL_REG, SNC_IRQ_EN, hw_snc_emu_context.SNC_CTRL_REG);
}

uint32_t hw_snc_SNC_CTRL_REG_getf_SNC_IRQ_CONFIG(void)
{
        return REG_GET_FIELD(SNC, SNC_CTRL_REG, SNC_IRQ_CONFIG, hw_snc_emu_context.SNC_CTRL_REG);
}

uint32_t hw_snc_SNC_CTRL_REG_getf_SNC_IRQ_ACK(void)
{
        return REG_GET_FIELD(SNC, SNC_CTRL_REG, SNC_IRQ_ACK, hw_snc_emu_context.SNC_CTRL_REG);
}

uint32_t hw_snc_SNC_STATUS_REG_getf_EQ_FLAG(void)
{
        return REG_GET_FIELD(SNC, SNC_STATUS_REG, EQ_FLAG, hw_snc_emu_context.SNC_STATUS_REG);
}

uint32_t hw_snc_SNC_STATUS_REG_getf_GR_FLAG(void)
{
        return REG_GET_FIELD(SNC, SNC_STATUS_REG, GR_FLAG, hw_snc_emu_context.SNC_STATUS_REG);
}

uint32_t hw_snc_SNC_STATUS_REG_getf_SNC_DONE_STATUS(void)
{
        return REG_GET_FIELD(SNC, SNC_STATUS_REG, SNC_DONE_STATUS,
                hw_snc_emu_context.SNC_STATUS_REG);
}

uint32_t hw_snc_SNC_STATUS_REG_getf_BUS_ERROR_STATUS(void)
{
        return REG_GET_FIELD(SNC, SNC_STATUS_REG, BUS_ERROR_STATUS,
                hw_snc_emu_context.SNC_STATUS_REG);
}

uint32_t hw_snc_SNC_STATUS_REG_getf_HARD_FAULT_STATUS(void)
{
        return REG_GET_FIELD(SNC, SNC_STATUS_REG, HARD_FAULT_STATUS,
                hw_snc_emu_context.SNC_STATUS_REG);
}

uint32_t hw_snc_SNC_STATUS_REG_getf_SNC_IS_STOPPED(void)
{
        return REG_GET_FIELD(SNC, SNC_STATUS_REG, SNC_IS_STOPPED, hw_snc_emu_context.SNC_STATUS_REG);
}

uint32_t hw_snc_SNC_STATUS_REG_getf_SNC_PC_LOADED(void)
{
        return REG_GET_FIELD(SNC, SNC_STATUS_REG, SNC_PC_LOADED, hw_snc_emu_context.SNC_STATUS_REG);
}

uint32_t hw_snc_SNC_LP_TIMER_REG_getf_LP_TIMER(void)
{
        return REG_GET_FIELD(SNC, SNC_LP_TIMER_REG, LP_TIMER, hw_snc_emu_context.SNC_LP_TIMER_REG);
}

uint32_t hw_snc_SNC_PC_REG_getf_PC_REG(void)
{
        return REG_GET_FIELD(SNC, SNC_PC_REG, PC_REG, (uint32_t )hw_snc_emu_status.pc);
}

uint32_t hw_snc_SNC_R1_REG_getf_R1_REG(void)
{
        return REG_GET_FIELD(SNC, SNC_R1_REG, R1_REG, hw_snc_emu_context.SNC_R1_REG);
}

uint32_t hw_snc_SNC_R2_REG_getf_R2_REG(void)
{
        return REG_GET_FIELD(SNC, SNC_R2_REG, R2_REG, hw_snc_emu_context.SNC_R2_REG);
}

uint32_t hw_snc_SNC_TMP1_REG_getf_TMP1_REG(void)
{
        return REG_GET_FIELD(SNC, SNC_TMP1_REG, TMP1_REG, hw_snc_emu_context.SNC_TMP1_REG);
}

uint32_t hw_snc_SNC_TMP2_REG_getf_TMP2_REG(void)
{
        return REG_GET_FIELD(SNC, SNC_TMP2_REG, TMP2_REG, hw_snc_emu_context.SNC_TMP2_REG);
}

//==================== Initialization function =================================

void hw_snc_init(const hw_snc_config_t *cfg)
{
        // initialize SNC_CTRL_REG register and manually disable SNC
        uint32_t snc_ctrl_reg_local = 0;
        REG_SET_FIELD(SNC, SNC_CTRL_REG, SNC_SW_CTRL, snc_ctrl_reg_local, 1);
        REG_SET_FIELD(SNC, SNC_CTRL_REG, SNC_EN, snc_ctrl_reg_local, 0);
        hw_snc_emu_context.SNC_CTRL_REG = snc_ctrl_reg_local;
        SNC->SNC_CTRL_REG = snc_ctrl_reg_local;

        hw_snc_emu_context.SNC_STATUS_REG = 0;
        REG_SET_FIELD(SNC, SNC_STATUS_REG, SNC_IS_STOPPED, hw_snc_emu_context.SNC_STATUS_REG, 1);

        while (!hw_snc_is_stopped());

        // set the SNC base address
        if (cfg->base_address > 0x0) {
                hw_snc_set_base_address(cfg->base_address);
        }

        // set the SNC clock division factor
        hw_snc_set_clock_div(cfg->clk_div);

        // if SW-Control is enabled
        if (cfg->sw_ctrl_en) {
                // set the bus error detection
                if (cfg->bus_error_detect_en) {
                        hw_snc_bus_error_detection_enable();
                } else {
                        hw_snc_bus_error_detection_disable();
                }

                // set PC value to base address
                hw_snc_set_pc(hw_snc_get_base_address());

                // enable/disable PDC event related to SNC
                if (cfg->snc_irq_to_pdc_en) {
                        hw_snc_pdc_event_enable();
                } else {
                        hw_snc_pdc_event_disable();
                }

                // enable/disable SNC interrupt to CM33
                if (cfg->snc_irq_to_cm33_en) {
                        hw_snc_interrupt_enable();
                } else {
                        hw_snc_interrupt_disable();
                }
        }
        // if PDC control is enabled
        else {
                hw_snc_pdc_ctrl_enable();
        }
}

//==================== Control functions =======================================

void hw_snc_manual_enable(void)
{
        REG_SET_FIELD(SNC, SNC_CTRL_REG, SNC_SW_CTRL, hw_snc_emu_context.SNC_CTRL_REG, 1);
        HW_SNC_REG_SETF(SNC_CTRL_REG, SNC_EN, 1);
}

void hw_snc_manual_disable(void)
{
        REG_SET_FIELD(SNC, SNC_CTRL_REG, SNC_SW_CTRL, hw_snc_emu_context.SNC_CTRL_REG, 1);
        HW_SNC_REG_SETF(SNC_CTRL_REG, SNC_EN, 0);
        while (!REG_GET_FIELD(SNC, SNC_STATUS_REG, SNC_IS_STOPPED,
                hw_snc_emu_context.SNC_STATUS_REG));
}

void hw_snc_pdc_ctrl_enable(void)
{
        HW_SNC_REG_SETF(SNC_CTRL_REG, SNC_EN, 0);
        REG_SET_FIELD(SNC, SNC_CTRL_REG, SNC_SW_CTRL, hw_snc_emu_context.SNC_CTRL_REG, 0);
}

//==================== Configuration functions =================================

SNC_MAIN_DEF(snc_main_emu)
{
        SENIS_labels(wait_for_snc_main, snc_slp);

        senis_wadva(b_ctx, _SNC_OP(da(&SNC->SNC_CTRL_REG)), 0);
        senis_wadva(b_ctx, _SNC_OP(da(&SNC->SNC_STATUS_REG)), 0);
        senis_wadva(b_ctx, _SNC_OP(da(&SNC->SNC_LP_TIMER_REG)), 0);

        senis_wadva(b_ctx, _SNC_OP(da(&hw_snc_emu_context.SNC_CTRL_REG)), 0);
        senis_wadva(b_ctx, _SNC_OP(da(&hw_snc_emu_context.SNC_STATUS_REG)), 0);
        senis_wadva(b_ctx, _SNC_OP(da(&hw_snc_emu_context.SNC_LP_TIMER_REG)), 0);

        senis_wadad(b_ctx, _SNC_OP(da(&hw_snc_emu_status.pc)), _SNC_OP(da(&base_reg)));
        senis_wadad(b_ctx, _SNC_OP(da(&hw_snc_emu_context.SNC_PC_REG)),
                _SNC_OP(da(&hw_snc_emu_status.pc)));
        senis_wadva(b_ctx, _SNC_OP(da(&hw_snc_emu_status.branch_loop_cnt)), 0);
        senis_wadva(b_ctx, _SNC_OP(da(&snc_context.snc_emu)), 1);

        senis_wadva(b_ctx, _SNC_OP(da(&SNC->SNC_CTRL_REG)),
                (REG_MSK(SNC, SNC_CTRL_REG, SNC_IRQ_CONFIG)) |
                (REG_MSK(SNC, SNC_CTRL_REG, SNC_IRQ_EN)));

        SENIS_label(wait_for_snc_main);

        senis_rdcbi(b_ctx, &hw_snc_emu_context.SNC_STATUS_REG,
                REG_POS(SNC, SNC_STATUS_REG, SNC_DONE_STATUS));
        senis_cobr_eq(b_ctx, _SNC_OP(l(snc_slp)));
        senis_goto(b_ctx, _SNC_OP(l(wait_for_snc_main)));

        SENIS_label(snc_slp);

        senis_slp(b_ctx);
}

void hw_snc_set_base_address(uint32_t base_address)
{
        // Register SNC main with SNC and create it
        snc_main_context_t* snc_main_emu_ucode_ctx = snc_main_emu_get_context();
        snc_main_emu_ucode_ctx->ucode_create();

        ASSERT_WARNING(IS_SYSRAM_ADDRESS(black_orca_phy_addr(base_address)));
        base_reg = (uint32_t)base_address;

        REG_SETF(MEMCTRL, SNC_BASE_REG, SNC_BASE_ADDRESS,
                ((uint32_t)snc_main_emu_ucode_ctx->ucode) >> 2);
}

void hw_snc_interrupt_enable(void)
{
        if (REG_GETF(SNC, SNC_CTRL_REG, SNC_IRQ_CONFIG) == 0) {
                REG_SETF(SNC, SNC_CTRL_REG, SNC_IRQ_ACK, 1);
                HW_SNC_REG_SETF(SNC_CTRL_REG, SNC_IRQ_ACK, 1);
        }
        SNC->SNC_CTRL_REG |= (1 << REG_POS(SNC, SNC_CTRL_REG, SNC_IRQ_CONFIG));
        hw_snc_emu_context.SNC_CTRL_REG |= (1 << REG_POS(SNC, SNC_CTRL_REG, SNC_IRQ_CONFIG));
}

void hw_snc_interrupt_disable(void)
{
        SNC->SNC_CTRL_REG &= ~(1 << REG_POS(SNC, SNC_CTRL_REG, SNC_IRQ_CONFIG));
        hw_snc_emu_context.SNC_CTRL_REG &= ~(1 << REG_POS(SNC, SNC_CTRL_REG, SNC_IRQ_CONFIG));
        if (REG_GETF(SNC, SNC_CTRL_REG, SNC_IRQ_CONFIG) == 0) {
                REG_SETF(SNC, SNC_CTRL_REG, SNC_IRQ_ACK, 1);
                HW_SNC_REG_SETF(SNC_CTRL_REG, SNC_IRQ_ACK, 1);
        }
}

void hw_snc_register_int(hw_snc_interrupt_cb_t handler)
{
        GLOBAL_INT_DISABLE();
        snc_intr_cb = handler;
        NVIC_ClearPendingIRQ(SNC_IRQn);
        REG_SETF(SNC, SNC_CTRL_REG, SNC_IRQ_ACK, 1);
        REG_SET_FIELD(SNC, SNC_CTRL_REG, SNC_IRQ_EN, hw_snc_emu_context.SNC_CTRL_REG, 0);
        GLOBAL_INT_RESTORE();
        NVIC_EnableIRQ(SNC_IRQn);
}

void hw_snc_unregister_int(void)
{
        NVIC_DisableIRQ(SNC_IRQn);
        NVIC_ClearPendingIRQ(SNC_IRQn);
        snc_intr_cb = NULL;
}

void hw_snc_pdc_event_enable(void)
{
        if (REG_GETF(SNC, SNC_CTRL_REG, SNC_IRQ_CONFIG) == 0) {
                REG_SETF(SNC, SNC_CTRL_REG, SNC_IRQ_ACK, 1);
                HW_SNC_REG_SETF(SNC_CTRL_REG, SNC_IRQ_ACK, 1);
        }
        SNC->SNC_CTRL_REG |= (1 << (REG_POS(SNC, SNC_CTRL_REG, SNC_IRQ_CONFIG) + 1));
        hw_snc_emu_context.SNC_CTRL_REG |= (1 << (REG_POS(SNC, SNC_CTRL_REG, SNC_IRQ_CONFIG) + 1));
}

void hw_snc_pdc_event_disable(void)
{
        SNC->SNC_CTRL_REG &= ~(1 << (REG_POS(SNC, SNC_CTRL_REG, SNC_IRQ_CONFIG) + 1));
        hw_snc_emu_context.SNC_CTRL_REG &= ~(1 << (REG_POS(SNC, SNC_CTRL_REG, SNC_IRQ_CONFIG) + 1));
        if (REG_GETF(SNC, SNC_CTRL_REG, SNC_IRQ_CONFIG) == 0) {
                REG_SETF(SNC, SNC_CTRL_REG, SNC_IRQ_ACK, 1);
                HW_SNC_REG_SETF(SNC_CTRL_REG, SNC_IRQ_ACK, 1);
        }
}

void hw_snc_set_emu_delay_func(hw_snc_emu_delay_cb_t cb)
{
        delay_cb = cb;
}

void hw_snc_set_emu_exec_indication(hw_snc_emu_exec_ind_cb_t cb)
{
        emu_exec_ind_cb = cb;
}

void hw_snc_emu_run(void)
{
        execute_snc_emu();
}

//==================== State Acquisition functions =============================

uint32_t hw_snc_get_base_address(void)
{
        return base_reg;
}

//==================== IRQ Handler =============================================

void Sensor_Node_Handler(void)
{
        REG_SETF(SNC, SNC_CTRL_REG, SNC_IRQ_ACK, 1);
        HW_SNC_REG_SETF(SNC_CTRL_REG, SNC_IRQ_ACK, 1);

        if (snc_intr_cb) {
                snc_intr_cb();
        }
}

//==================== Static functions ========================================

#define SENIS_OPCODE(command)           (((command) & 0xf0000000) >> 28)
#define SENIS_WADAD_OP1_M(command)      (((command) & 0x0007ffff) + SENIS_MEM_BASE_ADDRESS)
#define SENIS_WADAD_OP1_R(command)      (((command) & 0x0007ffff) + SENIS_REGS_BASE_ADDRESS)
#define SENIS_WADAD_OP2_M(command)      (((command) & 0x0007ffff) + SENIS_MEM_BASE_ADDRESS)
#define SENIS_WADAD_OP2_R(command)      (((command) & 0x0007ffff) + SENIS_REGS_BASE_ADDRESS)

/**
 * \brief Sensor Node Emulator read register function. This function is used by the SNC Emulator
 * in order to emulate an SNC register read
 *
 * \param[in] reg       The register to read from
 *
 * \return uint32_t     The register value
 */
static uint32_t read_reg(uint32_t* reg)
{
        if (reg < &SNC->SNC_CTRL_REG || reg > &SNC->SNC_TMP2_REG) {
                return *reg;
        }

        if (reg == &SNC->SNC_PC_REG) {
                return (uint32_t)hw_snc_emu_status.pc;
        }

        return *(uint32_t*)(((uint32_t)reg - (uint32_t)&SNC->SNC_CTRL_REG) +
                (uint32_t)&hw_snc_emu_context.SNC_CTRL_REG);
}

/**
 * \brief Sensor Node Emulator write register function. This function is used by the SNC Emulator
 * in order to emulate an SNC register write
 *
 * \param[in] reg       The register to write to
 * \param[in] value     The desired register value
 *
 */
static void write_reg(uint32_t* reg, uint32_t value)
{
        if (reg < &SNC->SNC_CTRL_REG || reg > &SNC->SNC_TMP2_REG) {
                *reg = value;
        } else {
                if (reg == &SNC->SNC_CTRL_REG) {
                        HW_SNC_REG_SETF(SNC_CTRL_REG, SNC_BRANCH_LOOP_INIT,
                                REG_GET_FIELD(SNC, SNC_CTRL_REG, SNC_BRANCH_LOOP_INIT, value));
                        HW_SNC_REG_SETF(SNC_CTRL_REG, BUS_ERROR_DETECT_EN,
                                REG_GET_FIELD(SNC, SNC_CTRL_REG, BUS_ERROR_DETECT_EN, value));
                        if (REG_GET_FIELD(SNC, SNC_CTRL_REG, SNC_SW_CTRL, value)) {
                                if (REG_GET_FIELD(SNC, SNC_CTRL_REG, SNC_EN, value)) {
                                        hw_snc_manual_enable();
                                } else {
                                        hw_snc_emu_context.SNC_PC_REG =
                                                (uint32_t)hw_snc_emu_status.pc;
                                        hw_snc_manual_disable();
                                }
                        } else {
                                hw_snc_pdc_ctrl_enable();
                        }
                        HW_SNC_REG_SETF(SNC_CTRL_REG, SNC_IRQ_ACK,
                                REG_GET_FIELD(SNC, SNC_CTRL_REG, SNC_IRQ_ACK, value));
                        HW_SNC_REG_SETF(SNC_CTRL_REG, SNC_IRQ_CONFIG,
                                REG_GET_FIELD(SNC, SNC_CTRL_REG, SNC_IRQ_CONFIG, value));
                        HW_SNC_REG_SETF(SNC_CTRL_REG, SNC_IRQ_EN,
                                REG_GET_FIELD(SNC, SNC_CTRL_REG, SNC_IRQ_EN, value));

                        HW_SNC_REG_SETF(SNC_CTRL_REG, SNC_RESET,
                                REG_GET_FIELD(SNC, SNC_CTRL_REG, SNC_RESET, value));
                } else if (reg == &SNC->SNC_STATUS_REG) {
                        HW_SNC_REG_SETF(SNC_STATUS_REG, EQ_FLAG,
                                REG_GET_FIELD(SNC, SNC_STATUS_REG, EQ_FLAG, value));
                        HW_SNC_REG_SETF(SNC_STATUS_REG, GR_FLAG,
                                REG_GET_FIELD(SNC, SNC_STATUS_REG, GR_FLAG, value));
                }
        }
}

/**
 * \brief The core SNC emulator function.
 * This function emulates each of the SNC commands and updates the
 * virtual SNC registers accordingly
 *
 */
static void execute_snc_emu(void)
{
        volatile uint32_t cmd_1st_part = 0;
        volatile uint32_t cmd_2nd_part = 0;

        while (1) {
                // Call SNC emulator execution indication function when a new SeNIS command is to
                // be executed
                if (emu_exec_ind_cb) {
                        emu_exec_ind_cb((uint32_t)hw_snc_emu_status.pc);
                }

                cmd_1st_part = *hw_snc_emu_status.pc;
                hw_snc_emu_status.pc++;

                switch (SENIS_OPCODE(cmd_1st_part)) {
                case SENIS_OP_NOP:
                        break;

                case SENIS_OP_WADAD:
                {
                        uint32_t read_val;

                        cmd_2nd_part = *hw_snc_emu_status.pc;
                        hw_snc_emu_status.pc++;

                        if (cmd_2nd_part & SENIS_FLAG_REG) {
                                read_val = read_reg((uint32_t*)(SENIS_WADAD_OP2_R(cmd_2nd_part)));
                        } else if (cmd_1st_part & SENIS_FLAG_InA2) {
                                uint32_t** cmd_2nd_op_addr =
                                        ((uint32_t**)(SENIS_WADAD_OP2_M(cmd_2nd_part)));

                                /* This assertion is here to indicate that indirect addressing
                                 * has been defined for an address in the second operand of the
                                 * command with a register address value pointing to that address
                                 * (e.g. using ia(reg_addr)), which is not permitted.
                                 */
                                ASSERT_WARNING(_SNC_ADDR_IS_MEM(cmd_2nd_op_addr));

                                /* This assertion is here to indicate that indirect addressing
                                 * has been defined for a register address in the second operand of
                                 * the command with a memory address value pointing to that address
                                 * (e.g. using ia(mem_addr) and *mem_addr is equal to reg_addr),
                                 * which is not permitted.
                                 */
                                ASSERT_WARNING(_SNC_ADDR_IS_MEM(*cmd_2nd_op_addr));

                                read_val = **cmd_2nd_op_addr;
                        } else {
                                read_val = *(uint32_t*)(SENIS_WADAD_OP2_M(cmd_2nd_part));
                        }

                        if (cmd_1st_part & SENIS_FLAG_REG) {
                                write_reg((uint32_t*)(SENIS_WADAD_OP1_R(cmd_1st_part)), read_val);
                        } else if (cmd_1st_part & SENIS_FLAG_DA1) {
                                *(uint32_t*)(SENIS_WADAD_OP1_M(cmd_1st_part)) = read_val;
                        } else {
                                **((uint32_t**)(SENIS_WADAD_OP1_M(cmd_1st_part))) = read_val;
                        }

                        break;
                }
                case SENIS_OP_WADVA:
                {
                        uint32_t value;

                        value = *hw_snc_emu_status.pc;
                        hw_snc_emu_status.pc++;

                        if (cmd_1st_part & SENIS_FLAG_REG) {
                                write_reg((uint32_t*)(SENIS_WADAD_OP1_R(cmd_1st_part)), value);
                        } else if (cmd_1st_part & SENIS_FLAG_DA1) {
                                *(uint32_t*)(SENIS_WADAD_OP1_M(cmd_1st_part)) = value;
                        } else {
                                **((uint32_t**)(SENIS_WADAD_OP1_M(cmd_1st_part))) = value;
                        }

                        break;
                }
                case SENIS_OP_TOBRE:
                {
                        uint32_t bit_mask;

                        bit_mask = *hw_snc_emu_status.pc;
                        hw_snc_emu_status.pc++;

                        if (cmd_1st_part & SENIS_FLAG_REG) {
                                uint32_t read_val;
                                read_val = read_reg((uint32_t*)(SENIS_WADAD_OP1_R(cmd_1st_part)));
                                write_reg((uint32_t*)(SENIS_WADAD_OP1_R(cmd_1st_part)),
                                        read_val ^ bit_mask);
                        } else {
                                uint32_t* dst_addr = (uint32_t*)(SENIS_WADAD_OP1_M(cmd_1st_part));
                                *dst_addr ^= bit_mask;
                        }

                        break;
                }
                case SENIS_OP_RDCBI:
                {
                        uint32_t read_val;
                        uint32_t bit_pos;

                        bit_pos = ((cmd_1st_part >> 23) & 0x1f);

                        if (cmd_1st_part & SENIS_FLAG_REG) {
                                read_val = read_reg((uint32_t*)(SENIS_WADAD_OP1_R(cmd_1st_part)));
                        } else {
                                read_val = *(uint32_t*)(SENIS_WADAD_OP1_M(cmd_1st_part));
                        }

                        HW_SNC_REG_SETF(SNC_STATUS_REG, EQ_FLAG, (read_val & (1 << bit_pos)) > 0);

                        break;
                }
                case SENIS_OP_RDCGR:
                {
                        uint32_t read_val;
                        uint32_t gt_read_val;

                        cmd_2nd_part = *hw_snc_emu_status.pc;
                        hw_snc_emu_status.pc++;

                        if (cmd_1st_part & SENIS_FLAG_REG) {
                                read_val = read_reg((uint32_t*)(SENIS_WADAD_OP1_R(cmd_1st_part)));
                        } else {
                                read_val = *(uint32_t*)(SENIS_WADAD_OP1_M(cmd_1st_part));
                        }

                        if (cmd_2nd_part & SENIS_FLAG_REG) {
                                gt_read_val = read_reg(
                                        (uint32_t*)(SENIS_WADAD_OP2_R(cmd_2nd_part)));
                        } else {
                                gt_read_val = *(uint32_t*)(SENIS_WADAD_OP2_M(cmd_2nd_part));
                        }

                        HW_SNC_REG_SETF(SNC_STATUS_REG, GR_FLAG, read_val > gt_read_val);

                        break;
                }
                case SENIS_OP_COBR:
                {
                        uint32_t cobr_attr;

                        cobr_attr = ((cmd_1st_part >> 20) & 0xff);

                        if (cobr_attr & 0x80) {
                                if (cobr_attr != 0x80) {
                                        if (hw_snc_emu_status.branch_loop_cnt == 0x80) { // counter reached 0
                                                hw_snc_emu_status.branch_loop_cnt = 0; // reset counter
                                        } else if (hw_snc_emu_status.branch_loop_cnt & 0x80) { // counting down
                                                hw_snc_emu_status.branch_loop_cnt--;
                                                hw_snc_emu_status.pc = (uint32_t*)SENIS_WADAD_OP1_M(
                                                        cmd_1st_part);
                                        } else {                                      // set counter
                                                hw_snc_emu_status.branch_loop_cnt = cobr_attr - 1;
                                                hw_snc_emu_status.pc = (uint32_t*)SENIS_WADAD_OP1_M(
                                                        cmd_1st_part);
                                        }
                                }
                        } else {
                                uint32_t cobr_flavor_code = cobr_attr & 0x0f;

                                if ((cobr_flavor_code == 0x0A
                                        && HW_SNC_REG_GETF(SNC_STATUS_REG, EQ_FLAG))
                                        ||
                                        (cobr_flavor_code == 0x05
                                                && HW_SNC_REG_GETF(SNC_STATUS_REG, GR_FLAG))) {
                                        if (cobr_attr & 0x10) {
                                                hw_snc_emu_status.pc =
                                                        *((uint32_t**)(SENIS_WADAD_OP1_M(
                                                                cmd_1st_part)));
                                        } else {
                                                hw_snc_emu_status.pc = (uint32_t*)SENIS_WADAD_OP1_M(
                                                        cmd_1st_part);
                                        }
                                }
                        }

                        break;
                }
                case SENIS_OP_INC:
                {
                        uint32_t* addr;

                        addr = (uint32_t*)(SENIS_WADAD_OP1_M(cmd_1st_part));

                        if (cmd_1st_part & SENIS_FLAG_INC4) {
                                *addr = *addr + 4;
                        } else {
                                *addr = *addr + 1;
                        }

                        break;
                }
                case SENIS_OP_DEL:
                {
                        hw_snc_emu_context.SNC_LP_TIMER_REG = (cmd_1st_part & 0xff);

                        if (delay_cb && hw_snc_emu_context.SNC_LP_TIMER_REG) {

                                delay_cb(hw_snc_emu_context.SNC_LP_TIMER_REG);

                                hw_snc_emu_context.SNC_LP_TIMER_REG = 0;
                        }

                        break;
                }
                case SENIS_OP_SLP:
                {
                        REG_SET_FIELD(SNC, SNC_STATUS_REG, SNC_DONE_STATUS,
                                hw_snc_emu_context.SNC_STATUS_REG, 1);
                        REG_SET_FIELD(SNC, SNC_LP_TIMER_REG, LP_TIMER,
                                hw_snc_emu_context.SNC_LP_TIMER_REG, 0);

                        return;
                }
                default:
                        REG_SET_FIELD(SNC, SNC_STATUS_REG, HARD_FAULT_STATUS,
                                hw_snc_emu_context.SNC_STATUS_REG, 1);
                } // switch (SENIS_OPCODE(cmd_1st_part))
        } // while (1)
}

#endif /* dg_configUSE_HW_SENSOR_NODE & dg_configUSE_HW_SENSOR_NODE_EMU */


/**
 * \}
 * \}
 */
