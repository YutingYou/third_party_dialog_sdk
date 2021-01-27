/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SNC_SENIS
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file SeNIS.c
 *
 * @brief SNC-Sensor Node Instruction Set (SeNIS) -based framework for implementing uCodes
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#if dg_configUSE_HW_SENSOR_NODE

#include <string.h>
#include <stdarg.h>

#include "snc_defs.h"

#include "snc_hw_sys.h"

/*
 * DATA TYPE AND STRUCTURE DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief SNC context
 *
 */
_SNC_RETAINED snc_context_t snc_context = { 0 };

#define SNC_MAX_CONST_NUMBER            4
/**
 * \brief Variables used for accelerating specific operation cases, e.g. SENIS_rdcgr_z, SENIS_goto.
 *        They can be used for similar purposes when implementing uCodes,
 *        but they must NOT be changed.
 *
 */
_SNC_RETAINED uint32_t snc_const[SNC_MAX_CONST_NUMBER + 1] = {
        0, 1, 2, 3, 4
};

/**
 * \brief Array used for switching/shifting over all possible options for RDCBI bit-position
 *        argument, thus allowing for RDCBI configuration at SNC-execution time.
 *
 */
_SNC_RETAINED uint32_t senis_RDCBI_bit_pos_array[] = {
        _SENIS_RDCBI_BIT_POS( 0), _SENIS_RDCBI_BIT_POS( 1), _SENIS_RDCBI_BIT_POS( 2), _SENIS_RDCBI_BIT_POS( 3),
        _SENIS_RDCBI_BIT_POS( 4), _SENIS_RDCBI_BIT_POS( 5), _SENIS_RDCBI_BIT_POS( 6), _SENIS_RDCBI_BIT_POS( 7),
        _SENIS_RDCBI_BIT_POS( 8), _SENIS_RDCBI_BIT_POS( 9), _SENIS_RDCBI_BIT_POS(10), _SENIS_RDCBI_BIT_POS(11),
        _SENIS_RDCBI_BIT_POS(12), _SENIS_RDCBI_BIT_POS(13), _SENIS_RDCBI_BIT_POS(14), _SENIS_RDCBI_BIT_POS(15),
        _SENIS_RDCBI_BIT_POS(16), _SENIS_RDCBI_BIT_POS(17), _SENIS_RDCBI_BIT_POS(18), _SENIS_RDCBI_BIT_POS(19),
        _SENIS_RDCBI_BIT_POS(20), _SENIS_RDCBI_BIT_POS(21), _SENIS_RDCBI_BIT_POS(22), _SENIS_RDCBI_BIT_POS(23),
        _SENIS_RDCBI_BIT_POS(24), _SENIS_RDCBI_BIT_POS(25), _SENIS_RDCBI_BIT_POS(26), _SENIS_RDCBI_BIT_POS(27),
        _SENIS_RDCBI_BIT_POS(28), _SENIS_RDCBI_BIT_POS(29), _SENIS_RDCBI_BIT_POS(30), _SENIS_RDCBI_BIT_POS(31),
};

/**
 * \brief Array used for switching/shifting over all possible options for TOBRE bit-mask
 *        argument, thus allowing for TOBRE configuration at SNC-execution time.
 *
 */
_SNC_RETAINED uint32_t senis_TOBRE_bit_mask_array[] = {
        (1 << 0), (1 << 1), (1 << 2), (1 << 3), (1 << 4), (1 << 5), (1 << 6), (1 << 7),
        (1 << 8), (1 << 9), (1 <<10), (1 <<11), (1 <<12), (1 <<13), (1 <<14), (1 <<15),
        (1 <<16), (1 <<17), (1 <<18), (1 <<19), (1 <<20), (1 <<21), (1 <<22), (1 <<23),
        (1 <<24), (1 <<25), (1 <<26), (1 <<27), (1 <<28), (1 <<29), (1 <<30), (1 <<31),
};

/*
 * FUNCTION DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief Creates a uCode
 *
 * This function creates a uCode, which can be either an SNC-main-uCode used by SNC adapter,
 * or a Function-uCode used as part of an SNC driver implementation, or a uCode-Block handling
 * a PDC event sent to SNC. All arguments of this function are set implicitly, through the macro
 * being used for defining the uCode, i.e. SNC_MAIN_DEF(), SNC_FUNC_DEF() and SNC_UCODE_BLOCK_DEF(),
 * respectively.
 *
 * \param [in] perif_type       the uCode type
 * \param [in] context          the uCode context
 * \param [in] args_size        the size of its arguments (Valid only for the case of a Function-uCode)
 * \param [in] ucode_build      the builder function of the uCode
 * \param [in] ucode_id         the uCode id (Valid only for the case of a uCode-Block)
 *
 * \sa SNC_UCODE_BLOCK_DEF
 * \sa SNC_FUNC_DEF
 * \sa SNC_MAIN_DEF
 *
 */
void snc_ucode_create(SNC_UCODE_TYPE ucode_type, void* context, uint32_t args_size,
        ucode_build_func_t ucode_build, uint32_t ucode_id)
{
        uint32_t** p_ucode = NULL;
        uint32_t** p_lp = NULL;

#if dg_configUSE_SNC_DEBUGGER
        uint32_t* p_size = NULL;
#if dg_configUSE_HW_SENSOR_NODE_EMU
        snc_emu_sbs_attrs_t** pp_sbs_attrs;
        snc_emu_sbs_attrs_t* p_sbs_attrs;
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */
#endif /* dg_configUSE_SNC_DEBUGGER */

        ASSERT_WARNING(ucode_build);
        ASSERT_WARNING(context);

        switch (ucode_type) {
        case SNC_UCODE_TYPE_UCODE_BLOCK:
                p_ucode = &((snc_ucode_context_t*)context)->ucode;
                p_lp = &((snc_ucode_context_t*)context)->lp;
#if dg_configUSE_SNC_DEBUGGER
                p_size = &((snc_ucode_context_t*)context)->size;
#if dg_configUSE_HW_SENSOR_NODE_EMU
                pp_sbs_attrs = (snc_emu_sbs_attrs_t**)(&((snc_ucode_context_t*)context)->dbg_attrs);
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */
#endif /* dg_configUSE_SNC_DEBUGGER */
                ((snc_ucode_context_t*)context)->ucode_id = ucode_id;
                break;
        case SNC_UCODE_TYPE_FUNC:
                p_ucode = &((snc_func_context_t*)context)->ucode;
                p_lp = &((snc_func_context_t*)context)->lp;
#if dg_configUSE_SNC_DEBUGGER
                p_size = &((snc_func_context_t*)context)->size;
#if dg_configUSE_HW_SENSOR_NODE_EMU
                pp_sbs_attrs = (snc_emu_sbs_attrs_t**)(&((snc_func_context_t*)context)->dbg_attrs);
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */
#endif /* dg_configUSE_SNC_DEBUGGER */
                ((snc_func_context_t*)context)->numOfConns++;
                break;
        case SNC_UCODE_TYPE_MAIN:
                p_ucode = &((snc_main_context_t*)context)->ucode;
                p_lp = NULL;
#if dg_configUSE_SNC_DEBUGGER
                p_size = &((snc_main_context_t*)context)->size;
#if dg_configUSE_HW_SENSOR_NODE_EMU
                pp_sbs_attrs = (snc_emu_sbs_attrs_t**)(&((snc_main_context_t*)context)->dbg_attrs);
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */
#endif /* dg_configUSE_SNC_DEBUGGER */
                break;
        default:
                ASSERT_WARNING(0);
        }

        if (*p_ucode != NULL) {
                return;
        }

        uint32_t dummy_storage[2];

        /* First phase:
         * Calculate the required memory space size to be used for determining the labels defined
         * in the uCode.
         */
        b_ctx_t b_ctx = {
                .ucode_type = ucode_type,
                .ucode_this_ctx = context,

                .creating = true,
                .header_len = args_size / sizeof(uint32_t),
                .body_len = 0,
                .footer_len = 0,
                .l_upd = 0,
                .l_index = 0,
                .labels = dummy_storage,
                .labels_len = 0,
                .upd = 0,
                .index = 0,
                .fs_index = 0,
                .ft_index = 0,
                .len = 0,
                .ucode = dummy_storage,

#if dg_configUSE_SNC_DEBUGGER
                .debug_en = 1,
                .bkpt_sbs_control_func = NULL,
#if dg_configUSE_HW_SENSOR_NODE_EMU
                .bkpt_emu_sbs_funcs = (bkpt_func_t*)dummy_storage,
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */
#endif /* dg_configUSE_SNC_DEBUGGER */
        };

        ucode_build(&b_ctx, NULL);

        /* Second phase:
         * Allocate the required memory space for determining the uCode labels,
         * store the memory addresses they refer to while traversing the uCode, and
         * calculate the required memory space size to which the uCode will be stored.
         */
        b_ctx.labels_len = b_ctx.l_index;
        if (b_ctx.labels_len > 0) {
                b_ctx.labels = OS_MALLOC(b_ctx.labels_len * sizeof(uint32_t));
                OS_ASSERT(b_ctx.labels);
        }

        b_ctx.l_upd = -1;
        b_ctx.l_index = 0;
        b_ctx.index = 0;
        b_ctx.fs_index = 0;
        b_ctx.ft_index = 0;
        b_ctx.len = b_ctx.ft_index;

        ucode_build(&b_ctx, NULL);

        /* Third phase:
         * Allocate the required memory space and create/build and store the uCode.
         */
        if (ucode_type != SNC_UCODE_TYPE_MAIN) {
                b_ctx.lp = p_lp;
                /* SENIS_return to be appended at the end of a non SNC-main-uCode */
                b_ctx.index += 3;
        }

        b_ctx.body_len = b_ctx.index;
        b_ctx.footer_len = b_ctx.fs_index + b_ctx.len;
        b_ctx.ucode = OS_MALLOC(
                (b_ctx.header_len + b_ctx.body_len + b_ctx.footer_len) * sizeof(uint32_t));
        OS_ASSERT(b_ctx.ucode);

        /* The .ucode member in the uCode context data structure points to uCode body */
        b_ctx.ucode += b_ctx.header_len;

#if dg_configUSE_SNC_DEBUGGER
        ASSERT_WARNING(p_size != NULL);
        *p_size = b_ctx.body_len;
#if dg_configUSE_HW_SENSOR_NODE_EMU
        p_sbs_attrs = OS_MALLOC(sizeof(snc_emu_sbs_attrs_t) + b_ctx.body_len * sizeof(uint32_t));
        OS_ASSERT(p_sbs_attrs);
        p_sbs_attrs->ucode_start_address = b_ctx.ucode;
        p_sbs_attrs->ucode_size = b_ctx.body_len;
        memset(p_sbs_attrs->sbs_funcs, 0, (b_ctx.body_len) * sizeof(uint32_t));
        p_sbs_attrs->next = NULL;

        *pp_sbs_attrs = p_sbs_attrs;

        if (snc_context.snc_emu_dbg_list) {
                snc_emu_sbs_attrs_t* p_cur_sbs_range =
                        (snc_emu_sbs_attrs_t*)snc_context.snc_emu_dbg_list;

                while (p_cur_sbs_range->next) {
                        p_cur_sbs_range = p_cur_sbs_range->next;
                }

                p_cur_sbs_range->next = p_sbs_attrs;
        } else {
                snc_context.snc_emu_dbg_list = (snc_emu_dbg_attrs_t)p_sbs_attrs;
        }
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */
#endif /* dg_configUSE_SNC_DEBUGGER */

        b_ctx.l_index = 0;
        b_ctx.upd = -1;
        b_ctx.index = 0;
        b_ctx.ft_index = b_ctx.body_len + b_ctx.fs_index;
        b_ctx.len = b_ctx.ft_index;
        b_ctx.fs_index = b_ctx.body_len;
        b_ctx.senis_break_addr = NULL;
        b_ctx.senis_continue_addr = NULL;

#if dg_configUSE_SNC_DEBUGGER
#if dg_configUSE_HW_SENSOR_NODE_EMU
        b_ctx.bkpt_emu_sbs_funcs = p_sbs_attrs->sbs_funcs;
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */
#endif /* dg_configUSE_SNC_DEBUGGER */

        ucode_build(&b_ctx, (void*)(b_ctx.ucode - b_ctx.header_len));

        /* If it is not an SNC-main-uCode, then append SENIS_return */
        if (ucode_type != SNC_UCODE_TYPE_MAIN) {
                senis_return(&b_ctx);
        }

        ASSERT_WARNING(p_ucode != NULL);
        *p_ucode = b_ctx.ucode;

        /* Free allocated space used for labels */
        if (b_ctx.labels_len > 0) {
                OS_FREE(b_ctx.labels);
        }
}

/**
 * \brief Deletes a uCode
 *
 * This function deletes a uCode. All arguments of this function are set based on the macro
 * being used for defining the uCode type, i.e. SNC_MAIN_DEF(), SNC_FUNC_DEF() and
 * SNC_UCODE_BLOCK_DEF().
 *
 * \param [in] perif_type       the uCode type
 * \param [in] context          the uCode context
 * \param [in] args_size        the size of its arguments (Valid only for the case of a function-uCode)
 * \param [in] ucode_build      the builder function of the uCode
 *
 * \sa SNC_UCODE_BLOCK_DEF
 * \sa SNC_FUNC_DEF
 * \sa SNC_MAIN_DEF
 *
 */
void snc_ucode_delete(SNC_UCODE_TYPE ucode_type, void* context, uint32_t args_size,
        ucode_build_func_t ucode_build)
{
        struct deleted_ucode_ctx_t {
                uint32_t* ucode;        //  Pointer to the uCode starting address
        } *deleted_ucode_ctx;

        uint32_t numOfConns = 0;

#if dg_configUSE_SNC_DEBUGGER
        uint32_t* p_size = NULL;
#if dg_configUSE_HW_SENSOR_NODE_EMU
        snc_emu_sbs_attrs_t** pp_sbs_attrs;
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */
#endif /* dg_configUSE_SNC_DEBUGGER */

        uint32_t dummy_storage;

        b_ctx_t b_ctx = {
                .ucode_type = ucode_type,
                .ucode_this_ctx = context,

                .creating = false,
                .header_len = args_size / sizeof(uint32_t),
                .labels = &dummy_storage,
                .ucode = &dummy_storage,

#if dg_configUSE_SNC_DEBUGGER
                .debug_en = 1,
                .bkpt_sbs_control_func = NULL,
#if dg_configUSE_HW_SENSOR_NODE_EMU
                .bkpt_emu_sbs_funcs = (bkpt_func_t*)&dummy_storage,
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */
#endif /* dg_configUSE_SNC_DEBUGGER */
        };

        ASSERT_WARNING(context);
        ASSERT_WARNING(ucode_build);

        deleted_ucode_ctx = (struct deleted_ucode_ctx_t*)context;

        ASSERT_WARNING(deleted_ucode_ctx->ucode);

        ucode_build(&b_ctx, NULL);

        switch (ucode_type) {
        case SNC_UCODE_TYPE_UCODE_BLOCK:
        {
                numOfConns = 1;

#if dg_configUSE_SNC_DEBUGGER
                p_size = &((snc_ucode_context_t*)context)->size;
#if dg_configUSE_HW_SENSOR_NODE_EMU
                pp_sbs_attrs = (snc_emu_sbs_attrs_t**)(&((snc_ucode_context_t*)context)->dbg_attrs);
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */
#endif /* dg_configUSE_SNC_DEBUGGER */

                break;
        }
        case SNC_UCODE_TYPE_FUNC:
        {
                uint32_t* p_numOfConns = &((snc_func_context_t*)context)->numOfConns;
                ASSERT_WARNING(*p_numOfConns);

                numOfConns = *p_numOfConns;

                (*p_numOfConns)--;

#if dg_configUSE_SNC_DEBUGGER
                p_size = &((snc_func_context_t*)context)->size;
#if dg_configUSE_HW_SENSOR_NODE_EMU
                pp_sbs_attrs = (snc_emu_sbs_attrs_t**)(&((snc_func_context_t*)context)->dbg_attrs);
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */
#endif /* dg_configUSE_SNC_DEBUGGER */

                break;
        }
        case SNC_UCODE_TYPE_MAIN:
        {
                numOfConns = 1;

#if dg_configUSE_SNC_DEBUGGER
                p_size = &((snc_main_context_t*)context)->size;
#if dg_configUSE_HW_SENSOR_NODE_EMU
                pp_sbs_attrs = (snc_emu_sbs_attrs_t**)(&((snc_main_context_t*)context)->dbg_attrs);
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */
#endif /* dg_configUSE_SNC_DEBUGGER */

                break;
        }
        default:
                ASSERT_WARNING(0);
        }

        if (numOfConns == 1 && deleted_ucode_ctx->ucode != NULL) {
                OS_FREE(deleted_ucode_ctx->ucode - args_size / sizeof(uint32_t));
                deleted_ucode_ctx->ucode = NULL;

#if dg_configUSE_SNC_DEBUGGER
                ASSERT_WARNING(p_size != NULL);
                *p_size = 0;
#if dg_configUSE_HW_SENSOR_NODE_EMU
                {
                        snc_emu_sbs_attrs_t* p_sbs_attrs = *pp_sbs_attrs;

                        if (snc_context.snc_emu_dbg_list == (snc_emu_dbg_attrs_t)p_sbs_attrs) {
                                snc_context.snc_emu_dbg_list = (snc_emu_dbg_attrs_t)p_sbs_attrs->next;
                        } else {
                                snc_emu_sbs_attrs_t* p_cur_sbs_attrs =
                                        (snc_emu_sbs_attrs_t*)snc_context.snc_emu_dbg_list;

                                while (p_cur_sbs_attrs->next != p_sbs_attrs) {
                                        p_cur_sbs_attrs = p_cur_sbs_attrs->next;
                                }

                                p_cur_sbs_attrs->next = p_sbs_attrs->next;
                        }

                        OS_FREE(p_sbs_attrs);
                }
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */
#endif /* dg_configUSE_SNC_DEBUGGER */
        }
}

uint32_t snc_get_SNC_to_CM33_trigger(void)
{
        return snc_context.snc_to_CM33_trigger;
}

void snc_clear_SNC_to_CM33_trigger(snc_ucode_context_t* ucode_ctx)
{
        ASSERT_WARNING(ucode_ctx);

        snc_enter_SNC_critical_section();

        snc_context.snc_to_CM33_trigger &= ~(1 << (ucode_ctx->ucode_id));

        snc_leave_SNC_critical_section();
}

void snc_notify_SNC_ucode(snc_ucode_context_t* ucode_ctx)
{
        snc_acquire_SNC_ucode_rsrc(ucode_ctx);

        ucode_ctx->CM33_to_SNC_triggered = 1;

        snc_release_SNC_ucode_rsrc(ucode_ctx);
}

void snc_acquire_SNC_ucode_rsrc(snc_ucode_context_t* ucode_ctx)
{
        ASSERT_WARNING(ucode_ctx);

        volatile uint32_t* p_flags = &ucode_ctx->flags;

        do {
                ucode_ctx->cmd |= (1 << SNC_UCODE_BLOCK_CMD_CM33_CTRL);

                if (!(*p_flags & (1 << SNC_UCODE_BLOCK_FLAG_SNC_CTRL))) {
                        break;
                } else {
                        ucode_ctx->cmd &= ~(1 << SNC_UCODE_BLOCK_CMD_CM33_CTRL);
                }

                while ((*p_flags & (1 << SNC_UCODE_BLOCK_FLAG_SNC_CTRL)));
        } while (1);
}

void snc_release_SNC_ucode_rsrc(snc_ucode_context_t* ucode_ctx)
{
        ASSERT_WARNING(ucode_ctx);

        ucode_ctx->cmd &= ~(1 << SNC_UCODE_BLOCK_CMD_CM33_CTRL);
}

void snc_enable_SNC_ucode(snc_ucode_context_t* ucode_ctx)
{
        ASSERT_WARNING(ucode_ctx);

        ucode_ctx->cmd &= ~(1 << SNC_UCODE_BLOCK_CMD_DISABLE);
}

void snc_disable_SNC_ucode(snc_ucode_context_t* ucode_ctx)
{
        ASSERT_WARNING(ucode_ctx);

        volatile uint32_t* p_flags = &ucode_ctx->flags;

        ucode_ctx->cmd |= (1 << SNC_UCODE_BLOCK_CMD_DISABLE);

        while ((*p_flags & (1 << SNC_UCODE_BLOCK_FLAG_RUNNING)));
}

void snc_enter_SNC_critical_section(void)
{
        sys_sw_bsr_acquire(SW_BSR_MASTER_SYSCPU, BSR_PERIPH_ID_SNC);
}

void snc_leave_SNC_critical_section(void)
{
        sys_sw_bsr_release(SW_BSR_MASTER_SYSCPU, BSR_PERIPH_ID_SNC);
}

void snc_mutex_SNC_create(snc_cm33_mutex_t* mutex)
{
        ASSERT_WARNING(mutex);

        mutex->cm33_semph = 0;
        mutex->snc_semph = 0;

#ifndef OS_BAREMETAL
        OS_MUTEX_CREATE(mutex->os_semph);
        OS_ASSERT(mutex->os_semph);
#endif /* OS_BAREMETAL */
}

void snc_mutex_SNC_delete(snc_cm33_mutex_t* mutex)
{
        ASSERT_WARNING(mutex);

#ifndef OS_BAREMETAL
        OS_MUTEX_DELETE(mutex->os_semph);
#endif /* OS_BAREMETAL */
}

void snc_mutex_SNC_lock(snc_cm33_mutex_t* mutex)
{
        ASSERT_WARNING(mutex);

        volatile uint32_t* p_snc_semph = &mutex->snc_semph;

#ifndef OS_BAREMETAL
        OS_MUTEX_GET(mutex->os_semph, OS_MUTEX_FOREVER);
#endif /* OS_BAREMETAL */

        do {
                mutex->cm33_semph = 1;

                if (*p_snc_semph == 0) {
                        break;
                } else {
                        mutex->cm33_semph = 0;
                }

                while (*p_snc_semph != 0);
        } while (1);
}

void snc_mutex_SNC_unlock(snc_cm33_mutex_t* mutex)
{
        ASSERT_WARNING(mutex);

        mutex->cm33_semph = 0;

#ifndef OS_BAREMETAL
        OS_MUTEX_PUT(mutex->os_semph);
#endif /* OS_BAREMETAL */
}

//==================== Calling an SNC-function -related macros =================

void senis_call(b_ctx_t* b_ctx, snc_func_context_t* ucode_ctx, uint32_t num_of_args, ...)
{
        va_list args;

        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(ucode_ctx);

        if (b_ctx->upd > 0) {
                if (b_ctx->creating) {
                        ucode_ctx->ucode_create();
                } else {
                        ucode_ctx->ucode_delete();
                }
        }

        va_start(args, num_of_args);

        for (uint32_t i = num_of_args / 2; i > 0; i--) {
                senis_assign(b_ctx, _SNC_OP(da(ucode_ctx->ucode - i)),
                        va_arg(args, uint32_t), va_arg(args, uint32_t*));
        }

        va_end(args);

        senis_wadva(b_ctx, _SNC_OP(da(&SNC->SNC_STATUS_REG)), SENIS_FLAG_EQ);
        senis_wadad(b_ctx, _SNC_OP(da(&ucode_ctx->lp)), _SNC_OP(da(&SNC->SNC_PC_REG)));
        senis_cobr_eq(b_ctx, _SNC_OP(ia(&ucode_ctx->ucode)));
}

//==================== Returning from an SNC-function -related macros ==========

void senis_return(b_ctx_t* b_ctx)
{
        ASSERT_WARNING(b_ctx);

        senis_wadva(b_ctx, _SNC_OP(da(&SNC->SNC_STATUS_REG)), SENIS_FLAG_GR | SENIS_FLAG_NEQ);
        senis_cobr_gr(b_ctx, _SNC_OP(ia(b_ctx->lp)));
}

//==================== Label definition function ===============================

// no implementation

//==================== SeNIS main functions ====================================

void senis_nop(b_ctx_t* b_ctx)
{
        ASSERT_WARNING(b_ctx);

        _SNC_UCODE(_SNC_UCODE_UPD) = (uint32_t)((SENIS_OP_NOP << 28));
}

void senis_wadad(b_ctx_t* b_ctx, SENIS_OPER_TYPE dst_addr_type, uint32_t* dst_addr,
                                 SENIS_OPER_TYPE src_addr_type, uint32_t* src_addr)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(dst_addr_type < SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(src_addr_type < SENIS_OPER_TYPE_VALUE);

        _SNC_UCODE(_SNC_UCODE_UPD) =
                (uint32_t)((SENIS_OP_WADAD << 28) | (_SNC_ADDR_SET_MODE(dst_addr)) | (_SNC_ADDR_SET_VALUE(dst_addr)) |
                                                    ((dst_addr_type == SENIS_OPER_TYPE_ADDRESS_DA)?SENIS_FLAG_DA1:0) |
                                                    ((src_addr_type == SENIS_OPER_TYPE_ADDRESS_IA)?SENIS_FLAG_InA2:0));
        _SNC_UCODE(_SNC_UCODE_UPD) =
                (uint32_t)(                         (_SNC_ADDR_SET_MODE(src_addr)) | (_SNC_ADDR_SET_VALUE(src_addr)));
}

void senis_wadva(b_ctx_t* b_ctx, SENIS_OPER_TYPE dst_addr_type, uint32_t* dst_addr,
                                 uint32_t value)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(dst_addr_type < SENIS_OPER_TYPE_VALUE);

        _SNC_UCODE(_SNC_UCODE_UPD) =
                (uint32_t)((SENIS_OP_WADVA << 28) | (_SNC_ADDR_SET_MODE(dst_addr)) | (_SNC_ADDR_SET_VALUE(dst_addr)) |
                                                    ((dst_addr_type == SENIS_OPER_TYPE_ADDRESS_DA)?SENIS_FLAG_DA1:0));
        _SNC_UCODE(_SNC_UCODE_UPD) =
                (uint32_t)(                         value);
}

void senis_tobre(b_ctx_t* b_ctx, uint32_t* dst_addr, uint32_t bit_mask_value)
{
        ASSERT_WARNING(b_ctx);

        _SNC_UCODE(_SNC_UCODE_UPD)=
                (uint32_t)((SENIS_OP_TOBRE << 28) | (_SNC_ADDR_SET_MODE(dst_addr)) | (_SNC_ADDR_SET_VALUE(dst_addr)));
        _SNC_UCODE(_SNC_UCODE_UPD) =
                (uint32_t)(                         bit_mask_value);
}

void senis_rdcbi(b_ctx_t* b_ctx, uint32_t* addr, uint32_t bit_pos_value)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(bit_pos_value < 32);

        _SNC_UCODE(_SNC_UCODE_UPD)=
                (uint32_t)((SENIS_OP_RDCBI << 28) | (_SNC_ADDR_SET_MODE(addr)) | (_SNC_ADDR_SET_VALUE(addr)) |
                                                    _SENIS_RDCBI_BIT_POS(bit_pos_value));
}

void senis_rdcgr(b_ctx_t* b_ctx, uint32_t* addr, uint32_t* gt_addr)
{
        ASSERT_WARNING(b_ctx);

        _SNC_UCODE(_SNC_UCODE_UPD) =
                (uint32_t)((SENIS_OP_RDCGR << 28) | (_SNC_ADDR_SET_MODE(addr)) | (_SNC_ADDR_SET_VALUE(addr)));
        _SNC_UCODE(_SNC_UCODE_UPD) =
                (uint32_t)(                         (_SNC_ADDR_SET_MODE(gt_addr)) | (_SNC_ADDR_SET_VALUE(gt_addr)));
}

void senis_cobr_eq(b_ctx_t* b_ctx, SENIS_OPER_TYPE addr_type, uint32_t* addr)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING((addr_type < SENIS_OPER_TYPE_VALUE) &&
                (!b_ctx->upd || !_SNC_ADDR_IS_REG(addr)));

        _SNC_UCODE(_SNC_UCODE_UPD) =
                (uint32_t)((SENIS_OP_COBR << 28) | (_SNC_ADDR_SET_VALUE(addr)) |
                                                   ((addr_type == SENIS_OPER_TYPE_ADDRESS_DA) ?
                                                           ((0x0A) << 20) : ((0x1A) << 20)));
}

void senis_cobr_gr(b_ctx_t* b_ctx, SENIS_OPER_TYPE addr_type, uint32_t* addr)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING((addr_type < SENIS_OPER_TYPE_VALUE) &&
                (!b_ctx->upd || !_SNC_ADDR_IS_REG(addr)));

        _SNC_UCODE(_SNC_UCODE_UPD) =
                (uint32_t)((SENIS_OP_COBR << 28) | (_SNC_ADDR_SET_VALUE(addr)) |
                                                   ((addr_type == SENIS_OPER_TYPE_ADDRESS_DA) ? ((0x05) << 20) : ((0x15) << 20)));
}

void senis_cobr_loop(b_ctx_t* b_ctx, uint32_t* addr, uint8_t cnt_value)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING((!b_ctx->upd || !_SNC_ADDR_IS_REG(addr)));

        _SNC_UCODE(_SNC_UCODE_UPD) =
                (uint32_t)((SENIS_OP_COBR << 28) | (_SNC_ADDR_SET_VALUE(addr)) | ((0x80 | ((cnt_value) & 0x7F)) << 20));
}

void senis_inc1(b_ctx_t* b_ctx, uint32_t* dst_addr)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING((!b_ctx->upd || !_SNC_ADDR_IS_REG(dst_addr)));

        _SNC_UCODE(_SNC_UCODE_UPD) =
                (uint32_t)((SENIS_OP_INC << 28) | (_SNC_ADDR_SET_VALUE(dst_addr)));
}

void senis_inc4(b_ctx_t* b_ctx, uint32_t* dst_addr)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING((!b_ctx->upd || !_SNC_ADDR_IS_REG(dst_addr)));

        _SNC_UCODE(_SNC_UCODE_UPD) =
                (uint32_t)((SENIS_OP_INC << 28) | (_SNC_ADDR_SET_VALUE(dst_addr)) | SENIS_FLAG_INC4);
}

void senis_del(b_ctx_t* b_ctx, uint8_t delay_value)
{
        ASSERT_WARNING(b_ctx);

        _SNC_UCODE(_SNC_UCODE_UPD) =
                (uint32_t)((SENIS_OP_DEL << 28) | (delay_value));
}

void senis_slp(b_ctx_t* b_ctx)
{
        ASSERT_WARNING(b_ctx);

        _SNC_UCODE(_SNC_UCODE_UPD) =
                (uint32_t)((SENIS_OP_SLP << 28));
}

//==================== SeNIS main functions extensions =========================

void senis_rdcgr_z(b_ctx_t* b_ctx, uint32_t* addr)
{
        ASSERT_WARNING(b_ctx);

        senis_rdcgr(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(addr)), _SNC_OP_DIRECT_ADDRESS(da(&snc_const[0])));
}

void senis_cobr_loop_reset(b_ctx_t* b_ctx)
{
        ASSERT_WARNING(b_ctx);

        SENIS_labels(pt_wait_for_snc);

        SENIS_label(pt_wait_for_snc);

        senis_rdcbi(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(&SNC->SNC_CTRL_REG)),
                REG_POS(SNC, SNC_CTRL_REG, SNC_IRQ_EN));
        senis_cobr_eq(b_ctx, _SNC_OP(l(pt_wait_for_snc)));

        senis_tobre(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(&SNC->SNC_CTRL_REG)),
                REG_MSK(SNC, SNC_CTRL_REG, SNC_BRANCH_LOOP_INIT));
}

void senis_del_lp_clk(b_ctx_t* b_ctx, uint32_t delay_ticks)
{
        ASSERT_WARNING(b_ctx);

        if (delay_ticks == 0) {
                return;
        }
        if (delay_ticks <= 4) {
                senis_del(b_ctx, 1);
        } else if (delay_ticks <= 255 + 4) {
                delay_ticks -= 4;
                senis_del(b_ctx, delay_ticks);
        } else if (delay_ticks <= 255 + 4 + 4) {
                senis_del(b_ctx, 255);
        } else if (delay_ticks < 255 + 8 + 255) {
                uint32_t applied_rem_delay_ticks;

                delay_ticks -= 8;
                applied_rem_delay_ticks = delay_ticks - (delay_ticks / 0xFF) * 0xFF;

                senis_del(b_ctx, 255);
                senis_del(b_ctx, applied_rem_delay_ticks);
        } else if (delay_ticks <= 255 + 8 + 255 + 4) {
                senis_del(b_ctx, 255);
                senis_del(b_ctx, 255);
        } else {
                uint32_t applied_num_delay_255ticks;
                uint32_t applied_rem_delay_ticks;

                SENIS_labels(begin);

                delay_ticks -= 12;

                applied_num_delay_255ticks = delay_ticks / 0xFF;

                delay_ticks -= ((applied_num_delay_255ticks - 2) * 4);

                applied_num_delay_255ticks = delay_ticks / 0xFF;
                applied_rem_delay_ticks = delay_ticks - applied_num_delay_255ticks * 0xFF;

                _SNC_TMP_ADD(uint32_t, temp_delay_cnt, sizeof(uint32_t));

                senis_wadva(b_ctx, _SNC_OP(da(temp_delay_cnt)), applied_num_delay_255ticks - 1);
                senis_tobre(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(temp_delay_cnt)), 0xFFFFFFFF);

                SENIS_label(begin);

                senis_del(b_ctx, 0xFF);
                senis_inc1(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(temp_delay_cnt)));
                senis_rdcgr_z(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(temp_delay_cnt)));
                senis_cobr_gr(b_ctx, _SNC_OP(l(begin)));
                if (applied_rem_delay_ticks) {
                        senis_del(b_ctx, applied_rem_delay_ticks);
                }

                _SNC_TMP_RMV(temp_delay_cnt);
        }
}

void senis_del_ms(b_ctx_t* b_ctx, uint32_t delay_ms)
{
        ASSERT_WARNING(b_ctx);
#if (dg_configUSE_LP_CLK == LP_CLK_32768 || dg_configUSE_LP_CLK == LP_CLK_32000)
        ASSERT_WARNING(delay_ms <= ((uint64_t)0xFFFFFFFF * 1000) / dg_configXTAL32K_FREQ);
        senis_del_lp_clk(b_ctx, (delay_ms * dg_configXTAL32K_FREQ) / 1000);
#elif (dg_configUSE_LP_CLK == LP_CLK_RCX && defined(OS_FREERTOS))
        ASSERT_WARNING(delay_ms <= ((uint64_t)0xFFFFFFFF * 1000) / rcx_clock_hz);
        senis_del_lp_clk(b_ctx, (delay_ms * rcx_clock_hz) / 1000);
#else
        ASSERT_ERROR(0);
#endif
}

//==================== SeNIS extension for c-like clauses =====================

void senis_assign(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_addr_type, uint32_t* op1_addr,
                                  SENIS_OPER_TYPE op2_type, uint32_t* op2)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(op1_addr_type < SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(op2_type <= SENIS_OPER_TYPE_VALUE);

        if (op2_type == SENIS_OPER_TYPE_VALUE) {
                senis_wadva(b_ctx, op1_addr_type, op1_addr, (uint32_t)op2);
        } else {
                senis_wadad(b_ctx, op1_addr_type, op1_addr, op2_type, op2);
        }
}

void senis_xor(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_addr_type, uint32_t* op1_addr,
                               SENIS_OPER_TYPE op2_type, uint32_t* op2)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(op1_addr_type < SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(op2_type <= SENIS_OPER_TYPE_VALUE);

        _SNC_TMP_ADD(uint32_t*, temp_op1_addr, sizeof(uint32_t*));

        if (op2_type == SENIS_OPER_TYPE_VALUE) {
                if (op1_addr_type == SENIS_OPER_TYPE_ADDRESS_IA) {
                        senis_assign(b_ctx, _SNC_OP(da(temp_op1_addr)), _SNC_OP(ia(op1_addr)));
                        senis_tobre(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(temp_op1_addr)), (uint32_t)op2);
                        senis_assign(b_ctx, _SNC_OP(ia(op1_addr)), _SNC_OP(da(temp_op1_addr)));
                } else {
                        senis_tobre(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(op1_addr)), (uint32_t)op2);
                }
        } else {
                SENIS_labels(ph_tobre_pos);

                if (op1_addr_type == SENIS_OPER_TYPE_ADDRESS_IA) {
                        senis_assign(b_ctx, _SNC_OP(da(temp_op1_addr)), _SNC_OP(ia(op1_addr)));
                }

                senis_wadad(b_ctx, _SNC_OP(l(ph_tobre_pos) + 1), op2_type, op2);

                SENIS_label(ph_tobre_pos);

                if (op1_addr_type == SENIS_OPER_TYPE_ADDRESS_IA) {
                        senis_tobre(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(temp_op1_addr)), SNC_PLACE_HOLDER);
                        senis_assign(b_ctx, _SNC_OP(ia(op1_addr)), _SNC_OP(da(temp_op1_addr)));
                } else {
                        senis_tobre(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(op1_addr)), SNC_PLACE_HOLDER);
                }
        }

        _SNC_TMP_RMV(temp_op1_addr);
}

uint32_t* senis_add_clause_op(b_ctx_t* b_ctx, SENIS_OPER_TYPE op_type, uint32_t* op)
{
        uint32_t* clause_op = NULL;

        _SNC_TMP_ADD(uint32_t, temp_op_i2d, sizeof(uint32_t));

        switch (op_type) {
        case SENIS_OPER_TYPE_ADDRESS_DA:
        {
                clause_op = op;
                break;
        }
        case SENIS_OPER_TYPE_ADDRESS_IA:
        {
                senis_wadad(b_ctx, _SNC_OP(da(temp_op_i2d)), _SNC_OP(ia(op)));
                clause_op = temp_op_i2d;
                break;
        }
        case SENIS_OPER_TYPE_VALUE:
        {
                uint32_t op_val = (uint32_t)op;

                if (op_val <= SNC_MAX_CONST_NUMBER) {
                        clause_op = &snc_const[op_val];
                } else {
                        _SNC_STATIC(uint32_t, const_op_val, sizeof(uint32_t), op_val);
                        clause_op = const_op_val;
                }
                break;
        }
        default:
                ASSERT_WARNING(0);
        }

        return clause_op;
}

void senis_rmv_clause_op(b_ctx_t* b_ctx, SENIS_OPER_TYPE clause_op_type, uint32_t* clause_op)
{
        if (clause_op_type == SENIS_OPER_TYPE_ADDRESS_IA) {
                _SNC_TMP_RMV(clause_op);
        }
}

static void senis_ifelse_blk_sel_else(b_ctx_t* b_ctx, senis_ifelse_blk_t ifelse_blk,
        senis_blk_t* else_blk_p)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(ifelse_blk);
        ASSERT_WARNING(else_blk_p);

        b_ctx_t cur_b_ctx;
        uint32_t dummy_storage[2];

        *else_blk_p = NULL;

        memcpy(&cur_b_ctx, b_ctx, sizeof(b_ctx_t));
        b_ctx->l_upd = b_ctx->upd = 0;
        b_ctx->labels = b_ctx->ucode = dummy_storage;
#if dg_configUSE_SNC_DEBUGGER
#if dg_configUSE_HW_SENSOR_NODE_EMU
        b_ctx->bkpt_emu_sbs_funcs = (bkpt_func_t*)dummy_storage;
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */
#endif /* dg_configUSE_SNC_DEBUGGER */
        ifelse_blk(b_ctx, else_blk_p);
        memcpy(b_ctx, &cur_b_ctx, sizeof(b_ctx_t));
}

void senis_if_lt(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                 SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                 senis_ifelse_blk_t ifelse_blk)
{
        senis_if_gt(b_ctx, op2_type, op2, op1_type, op1, ifelse_blk);
}

void senis_if_lteq(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                   SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                   senis_ifelse_blk_t ifelse_blk)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(op1_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(op2_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(ifelse_blk);

        if (op1_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op1_val = (uint32_t)op1;

                if (op1_val == 0) {
                        senis_if_true(b_ctx, ifelse_blk);
                        return;
                } else if (op2_type == SENIS_OPER_TYPE_VALUE) {
                        if (op1_val <= (uint32_t)op2) {
                                senis_if_true(b_ctx, ifelse_blk);
                        } else {
                                senis_if_false(b_ctx, ifelse_blk);
                        }
                        return;
                }
        }

        if (op2_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op2_val = (uint32_t)op2;

                if (op2_val == 0) {
                        senis_if_gt_z(b_ctx, op1_type, op1, ifelse_blk);
                        return;
                }
        }

        uint32_t* clause_op1 = senis_add_clause_op(b_ctx, op1_type, op1);
        uint32_t* clause_op2 = senis_add_clause_op(b_ctx, op2_type, op2);

        senis_blk_t else_blk;

        senis_ifelse_blk_sel_else(b_ctx, ifelse_blk, &else_blk);

        SENIS_labels(if_clause_end, ifelse_clause_end);

        senis_rdcgr(b_ctx, clause_op1, clause_op2);
        senis_cobr_gr(b_ctx, _SNC_OP(l(if_clause_end)));

        senis_rmv_clause_op(b_ctx, op2_type, clause_op2);
        senis_rmv_clause_op(b_ctx, op1_type, clause_op1);

        ifelse_blk(b_ctx, NULL);

        if (else_blk) {
                senis_goto(b_ctx, _SNC_OP(l(ifelse_clause_end)));

                SENIS_label(if_clause_end);

                else_blk(b_ctx);

                SENIS_label(ifelse_clause_end);
        } else {
                SENIS_label(if_clause_end);
        }
}

void senis_if_gt(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                 SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                 senis_ifelse_blk_t ifelse_blk)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(op1_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(op2_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(ifelse_blk);

        if (op1_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op1_val = (uint32_t)op1;

                if (op1_val == 0) {
                        senis_if_false(b_ctx, ifelse_blk);
                        return;
                } else if (op2_type == SENIS_OPER_TYPE_VALUE) {
                        if (op1_val > (uint32_t)op2) {
                                senis_if_true(b_ctx, ifelse_blk);
                        } else {
                                senis_if_false(b_ctx, ifelse_blk);
                        }
                        return;
                }
        }

        if (op2_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op2_val = (uint32_t)op2;

                if (op2_val == 0) {
                        senis_if_gt_z(b_ctx, op1_type, op1, ifelse_blk);
                        return;
                }
        }

        uint32_t* clause_op1 = senis_add_clause_op(b_ctx, op1_type, op1);
        uint32_t* clause_op2 = senis_add_clause_op(b_ctx, op2_type, op2);

        senis_blk_t else_blk;

        senis_ifelse_blk_sel_else(b_ctx, ifelse_blk, &else_blk);

        SENIS_labels(else_clause_end, ifelse_clause_end);

        senis_rdcgr(b_ctx, clause_op1, clause_op2);
        senis_cobr_gr(b_ctx, _SNC_OP(l(else_clause_end)));

        senis_rmv_clause_op(b_ctx, op2_type, clause_op2);
        senis_rmv_clause_op(b_ctx, op1_type, clause_op1);

        if (else_blk) {
                else_blk(b_ctx);
        }

        senis_goto(b_ctx, _SNC_OP(l(ifelse_clause_end)));

        SENIS_label(else_clause_end);

        ifelse_blk(b_ctx, NULL);

        SENIS_label(ifelse_clause_end);
}

void senis_if_gteq(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                   SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                   senis_ifelse_blk_t ifelse_blk)
{
        senis_if_lteq(b_ctx, op2_type, op2, op1_type, op1, ifelse_blk);
}

void senis_if_eq(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                 SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                 senis_ifelse_blk_t ifelse_blk)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(op1_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(op2_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(ifelse_blk);

        if (op1_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op1_val = (uint32_t)op1;

                if (op1_val == 0) {
                        senis_if_eq_z(b_ctx, op2_type, op2, ifelse_blk);
                        return;
                } else if (op2_type == SENIS_OPER_TYPE_VALUE) {
                        if (op1_val == (uint32_t)op2) {
                                senis_if_true(b_ctx, ifelse_blk);
                        } else {
                                senis_if_false(b_ctx, ifelse_blk);
                        }
                        return;
                }
        }

        if (op2_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op2_val = (uint32_t)op2;

                if (op2_val == 0) {
                        senis_if_eq_z(b_ctx, op1_type, op1, ifelse_blk);
                        return;
                }
        }

        uint32_t* clause_op1 = senis_add_clause_op(b_ctx, op1_type, op1);
        uint32_t* clause_op2 = senis_add_clause_op(b_ctx, op2_type, op2);

        senis_blk_t else_blk;

        senis_ifelse_blk_sel_else(b_ctx, ifelse_blk, &else_blk);

        SENIS_labels(if_clause_end, ifelse_clause_end);

        senis_rdcgr(b_ctx, clause_op1, clause_op2);
        senis_cobr_gr(b_ctx, _SNC_OP(l(if_clause_end)));
        senis_rdcgr(b_ctx, clause_op2, clause_op1);
        senis_cobr_gr(b_ctx, _SNC_OP(l(if_clause_end)));

        senis_rmv_clause_op(b_ctx, op2_type, clause_op2);
        senis_rmv_clause_op(b_ctx, op1_type, clause_op1);

        ifelse_blk(b_ctx, NULL);

        if (else_blk) {
                senis_goto(b_ctx, _SNC_OP(l(ifelse_clause_end)));

                SENIS_label(if_clause_end);

                else_blk(b_ctx);

                SENIS_label(ifelse_clause_end);
        } else {
                SENIS_label(if_clause_end);
        }
}

void senis_if_neq(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                  SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                  senis_ifelse_blk_t ifelse_blk)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(op1_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(op2_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(ifelse_blk);

        if (op1_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op1_val = (uint32_t)op1;

                if (op1_val == 0) {
                        senis_if_gt_z(b_ctx, op2_type, op2, ifelse_blk);
                        return;
                } else if (op2_type == SENIS_OPER_TYPE_VALUE) {
                        if (op1_val != (uint32_t)op2) {
                                senis_if_true(b_ctx, ifelse_blk);
                        } else {
                                senis_if_false(b_ctx, ifelse_blk);
                        }
                        return;
                }
        }

        if (op2_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op2_val = (uint32_t)op2;

                if (op2_val == 0) {
                        senis_if_gt_z(b_ctx, op1_type, op1, ifelse_blk);
                        return;
                }
        }

        uint32_t* clause_op1 = senis_add_clause_op(b_ctx, op1_type, op1);
        uint32_t* clause_op2 = senis_add_clause_op(b_ctx, op2_type, op2);

        senis_blk_t else_blk;

        senis_ifelse_blk_sel_else(b_ctx, ifelse_blk, &else_blk);

        SENIS_labels(else_clause_end, ifelse_clause_end);

        senis_rdcgr(b_ctx, clause_op1, clause_op2);
        senis_cobr_gr(b_ctx, _SNC_OP(l(else_clause_end)));
        senis_rdcgr(b_ctx, clause_op2, clause_op1);
        senis_cobr_gr(b_ctx, _SNC_OP(l(else_clause_end)));

        senis_rmv_clause_op(b_ctx, op2_type, clause_op2);
        senis_rmv_clause_op(b_ctx, op1_type, clause_op1);

        if (else_blk) {
                else_blk(b_ctx);
        }

        senis_goto(b_ctx, _SNC_OP(l(ifelse_clause_end)));

        SENIS_label(else_clause_end);

        ifelse_blk(b_ctx, NULL);

        SENIS_label(ifelse_clause_end);
}

void senis_if_bit(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1, uint32_t op2,
                                  senis_ifelse_blk_t ifelse_blk)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(op1_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(op2 < 32);
        ASSERT_WARNING(ifelse_blk);

        if (op1_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op1_val = (uint32_t)op1;

                if (op1_val & (1 << op2)) {
                        senis_if_true(b_ctx, ifelse_blk);
                } else {
                        senis_if_false(b_ctx, ifelse_blk);
                }
                return;
        }

        uint32_t* clause_op1 = senis_add_clause_op(b_ctx, op1_type, op1);

        senis_blk_t else_blk;

        senis_ifelse_blk_sel_else(b_ctx, ifelse_blk, &else_blk);

        SENIS_labels(else_clause_end, ifelse_clause_end);

        senis_rdcbi(b_ctx, clause_op1, op2);
        senis_cobr_eq(b_ctx, _SNC_OP(l(else_clause_end)));

        senis_rmv_clause_op(b_ctx, op1_type, clause_op1);

        if (else_blk) {
                else_blk(b_ctx);
        }

        senis_goto(b_ctx, _SNC_OP(l(ifelse_clause_end)));

        SENIS_label(else_clause_end);

        ifelse_blk(b_ctx, NULL);

        SENIS_label(ifelse_clause_end);
}

void senis_if_nbit(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1, uint32_t op2,
                                   senis_ifelse_blk_t ifelse_blk)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(op1_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(op2 < 32);
        ASSERT_WARNING(ifelse_blk);

        if (op1_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op1_val = (uint32_t)op1;

                if (op1_val & (1 << op2)) {
                        senis_if_false(b_ctx, ifelse_blk);
                } else {
                        senis_if_true(b_ctx, ifelse_blk);
                }
                return;
        }

        uint32_t* clause_op1 = senis_add_clause_op(b_ctx, op1_type, op1);

        senis_blk_t else_blk;

        senis_ifelse_blk_sel_else(b_ctx, ifelse_blk, &else_blk);

        SENIS_labels(if_clause_end, ifelse_clause_end);

        senis_rdcbi(b_ctx, clause_op1, op2);

        senis_rmv_clause_op(b_ctx, op1_type, clause_op1);

        if (else_blk) {
                senis_cobr_eq(b_ctx, _SNC_OP(l(if_clause_end)));

                ifelse_blk(b_ctx, NULL);

                senis_goto(b_ctx, _SNC_OP(l(ifelse_clause_end)));

                SENIS_label(if_clause_end);

                else_blk(b_ctx);

        } else {
                senis_cobr_eq(b_ctx, _SNC_OP(l(ifelse_clause_end)));

                ifelse_blk(b_ctx, NULL);
        }

        SENIS_label(ifelse_clause_end);
}

void senis_if_gt_z(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                   senis_ifelse_blk_t ifelse_blk)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(op1_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(ifelse_blk);

        if (op1_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op1_val = (uint32_t)op1;

                if (op1_val > 0) {
                        senis_if_true(b_ctx, ifelse_blk);
                } else {
                        senis_if_false(b_ctx, ifelse_blk);
                }
                return;
        }

        uint32_t* clause_op1 = senis_add_clause_op(b_ctx, op1_type, op1);

        senis_blk_t else_blk;

        senis_ifelse_blk_sel_else(b_ctx, ifelse_blk, &else_blk);

        SENIS_labels(if_clause_end, ifelse_clause_end);

        senis_rdcgr(b_ctx, &snc_const[1], clause_op1);
        senis_cobr_gr(b_ctx, _SNC_OP(l(if_clause_end)));

        senis_rmv_clause_op(b_ctx, op1_type, clause_op1);

        ifelse_blk(b_ctx, NULL);

        if (else_blk) {
                senis_goto(b_ctx, _SNC_OP(l(ifelse_clause_end)));

                SENIS_label(if_clause_end);

                else_blk(b_ctx);

                SENIS_label(ifelse_clause_end);
        } else {
                SENIS_label(if_clause_end);
        }
}

void senis_if_eq_z(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                   senis_ifelse_blk_t ifelse_blk)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(op1_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(ifelse_blk);

        if (op1_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op1_val = (uint32_t)op1;

                if (op1_val == 0) {
                        senis_if_true(b_ctx, ifelse_blk);
                } else {
                        senis_if_false(b_ctx, ifelse_blk);
                }
                return;
        }

        uint32_t* clause_op1 = senis_add_clause_op(b_ctx, op1_type, op1);

        senis_blk_t else_blk;

        senis_ifelse_blk_sel_else(b_ctx, ifelse_blk, &else_blk);

        SENIS_labels(else_clause_end, ifelse_clause_end);

        senis_rdcgr(b_ctx, &snc_const[1], clause_op1);
        senis_cobr_gr(b_ctx, _SNC_OP(l(else_clause_end)));

        senis_rmv_clause_op(b_ctx, op1_type, clause_op1);

        if (else_blk) {
                else_blk(b_ctx);
        }

        senis_goto(b_ctx, _SNC_OP(l(ifelse_clause_end)));

        SENIS_label(else_clause_end);

        ifelse_blk(b_ctx, NULL);

        SENIS_label(ifelse_clause_end);
}

void senis_if_true(b_ctx_t* b_ctx, senis_ifelse_blk_t ifelse_blk)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(ifelse_blk);

        ifelse_blk(b_ctx, NULL);
}

void senis_if_false(b_ctx_t* b_ctx, senis_ifelse_blk_t ifelse_blk)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(ifelse_blk);

        senis_blk_t else_blk;

        senis_ifelse_blk_sel_else(b_ctx, ifelse_blk, &else_blk);

        if (else_blk) {
                else_blk(b_ctx);
        }
}

void senis_while_lt(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                    SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                    senis_while_blk_t while_blk)
{
        senis_while_gt(b_ctx, op2_type, op2, op1_type, op1, while_blk);
}

void senis_while_lteq(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                      SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                      senis_while_blk_t while_blk)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(op1_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(op2_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(while_blk);

        if (op1_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op1_val = (uint32_t)op1;

                if (op1_val == 0) {
                        senis_while_true(b_ctx, while_blk);
                        return;
                } else if (op2_type == SENIS_OPER_TYPE_VALUE) {
                        if (op1_val <= (uint32_t)op2) {
                                senis_while_true(b_ctx, while_blk);
                        }
                        return;
                }
        }

        if (op2_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op2_val = (uint32_t)op2;

                if (op2_val == 0) {
                        senis_while_eq_z(b_ctx, op1_type, op1, while_blk);
                        return;
                }
        }

        uint32_t* clause_op1 = senis_add_clause_op(b_ctx, op1_type, op1);
        uint32_t* clause_op2 = senis_add_clause_op(b_ctx, op2_type, op2);

        SENIS_labels(while_clause_begin, while_clause_end);

        uint32_t* prev_senis_continue_addr = b_ctx->senis_continue_addr;
        uint32_t* prev_senis_break_addr = b_ctx->senis_break_addr;

        b_ctx->senis_continue_addr = _SNC_OP_DIRECT_ADDRESS(l(while_clause_begin));
        b_ctx->senis_break_addr = _SNC_OP_DIRECT_ADDRESS(l(while_clause_end));

        SENIS_label(while_clause_begin);

        if (op1_type == SENIS_OPER_TYPE_ADDRESS_IA) {
                senis_wadad(b_ctx, _SNC_OP(da(clause_op1)), _SNC_OP(ia(op1)));
        }
        if (op2_type == SENIS_OPER_TYPE_ADDRESS_IA) {
                senis_wadad(b_ctx, _SNC_OP(da(clause_op2)), _SNC_OP(ia(op2)));
        }
        senis_rdcgr(b_ctx, clause_op1, clause_op2);
        senis_cobr_gr(b_ctx, _SNC_OP(l(while_clause_end)));

        senis_rmv_clause_op(b_ctx, op2_type, clause_op2);
        senis_rmv_clause_op(b_ctx, op1_type, clause_op1);

        while_blk(b_ctx);

        senis_goto(b_ctx, _SNC_OP(l(while_clause_begin)));

        SENIS_label(while_clause_end);

        b_ctx->senis_continue_addr = prev_senis_continue_addr;
        b_ctx->senis_break_addr = prev_senis_break_addr;
}

void senis_while_gt(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                    SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                    senis_while_blk_t while_blk)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(op1_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(op2_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(while_blk);

        if (op1_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op1_val = (uint32_t)op1;

                if (op1_val == 0) {
                        return;
                } else if (op2_type == SENIS_OPER_TYPE_VALUE) {
                        if (op1_val > (uint32_t)op2) {
                                senis_while_true(b_ctx, while_blk);
                        }
                        return;
                }
        }

        if (op2_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op2_val = (uint32_t)op2;

                if (op2_val == 0) {
                        senis_while_gt_z(b_ctx, op1_type, op1, while_blk);
                        return;
                }
        }

        uint32_t* clause_op1 = senis_add_clause_op(b_ctx, op1_type, op1);
        uint32_t* clause_op2 = senis_add_clause_op(b_ctx, op2_type, op2);

        SENIS_labels(while_blk_begin, while_blk_end, while_clause_end);

        uint32_t* prev_senis_continue_addr = b_ctx->senis_continue_addr;
        uint32_t* prev_senis_break_addr = b_ctx->senis_break_addr;

        b_ctx->senis_continue_addr = _SNC_OP_DIRECT_ADDRESS(l(while_blk_end));
        b_ctx->senis_break_addr = _SNC_OP_DIRECT_ADDRESS(l(while_clause_end));

        senis_goto(b_ctx, _SNC_OP(l(while_blk_end)));

        SENIS_label(while_blk_begin);

        while_blk(b_ctx);

        SENIS_label(while_blk_end);

        if (op1_type == SENIS_OPER_TYPE_ADDRESS_IA) {
                senis_wadad(b_ctx, _SNC_OP(da(clause_op1)), _SNC_OP(ia(op1)));
        }
        if (op2_type == SENIS_OPER_TYPE_ADDRESS_IA) {
                senis_wadad(b_ctx, _SNC_OP(da(clause_op2)), _SNC_OP(ia(op2)));
        }
        senis_rdcgr(b_ctx, clause_op1, clause_op2);
        senis_cobr_gr(b_ctx, _SNC_OP(l(while_blk_begin)));

        SENIS_label(while_clause_end);

        senis_rmv_clause_op(b_ctx, op2_type, clause_op2);
        senis_rmv_clause_op(b_ctx, op1_type, clause_op1);

        b_ctx->senis_continue_addr = prev_senis_continue_addr;
        b_ctx->senis_break_addr = prev_senis_break_addr;
}

void senis_while_gteq(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                      SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                      senis_while_blk_t while_blk)
{
        senis_while_lteq(b_ctx, op2_type, op2, op1_type, op1, while_blk);
}

void senis_while_eq(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                    SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                    senis_while_blk_t while_blk)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(op1_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(op2_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(while_blk);

        if (op1_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op1_val = (uint32_t)op1;

                if (op1_val == 0) {
                        senis_while_eq_z(b_ctx, op2_type, op2, while_blk);
                        return;
                } else if (op2_type == SENIS_OPER_TYPE_VALUE) {
                        if (op1_val == (uint32_t)op2) {
                                senis_while_true(b_ctx, while_blk);
                        }
                        return;
                }
        }

        if (op2_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op2_val = (uint32_t)op2;

                if (op2_val == 0) {
                        senis_while_eq_z(b_ctx, op1_type, op1, while_blk);
                        return;
                }
        }

        uint32_t* clause_op1 = senis_add_clause_op(b_ctx, op1_type, op1);
        uint32_t* clause_op2 = senis_add_clause_op(b_ctx, op2_type, op2);

        SENIS_labels(while_clause_begin, while_clause_end);

        uint32_t* prev_senis_continue_addr = b_ctx->senis_continue_addr;
        uint32_t* prev_senis_break_addr = b_ctx->senis_break_addr;

        b_ctx->senis_continue_addr = _SNC_OP_DIRECT_ADDRESS(l(while_clause_begin));
        b_ctx->senis_break_addr = _SNC_OP_DIRECT_ADDRESS(l(while_clause_end));

        SENIS_label(while_clause_begin);

        if (op1_type == SENIS_OPER_TYPE_ADDRESS_IA) {
                senis_wadad(b_ctx, _SNC_OP(da(clause_op1)), _SNC_OP(ia(op1)));
        }
        if (op2_type == SENIS_OPER_TYPE_ADDRESS_IA) {
                senis_wadad(b_ctx, _SNC_OP(da(clause_op2)), _SNC_OP(ia(op2)));
        }
        senis_rdcgr(b_ctx, clause_op1, clause_op2);
        senis_cobr_gr(b_ctx, _SNC_OP(l(while_clause_end)));
        senis_rdcgr(b_ctx, clause_op2, clause_op1);
        senis_cobr_gr(b_ctx, _SNC_OP(l(while_clause_end)));

        while_blk(b_ctx);

        senis_goto(b_ctx, _SNC_OP(l(while_clause_begin)));

        SENIS_label(while_clause_end);

        senis_rmv_clause_op(b_ctx, op2_type, clause_op2);
        senis_rmv_clause_op(b_ctx, op1_type, clause_op1);

        b_ctx->senis_continue_addr = prev_senis_continue_addr;
        b_ctx->senis_break_addr = prev_senis_break_addr;
}

void senis_while_neq(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                     SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                     senis_while_blk_t while_blk)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(op1_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(op2_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(while_blk);

        if (op1_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op1_val = (uint32_t)op1;

                if (op1_val == 0) {
                        senis_while_gt_z(b_ctx, op2_type, op2, while_blk);
                        return;
                } else if (op2_type == SENIS_OPER_TYPE_VALUE) {
                        if (op1_val != (uint32_t)op2) {
                                senis_while_true(b_ctx, while_blk);
                        }
                        return;
                }
        }

        if (op2_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op2_val = (uint32_t)op2;

                if (op2_val == 0) {
                        senis_while_gt_z(b_ctx, op1_type, op1, while_blk);
                        return;
                }
        }

        uint32_t* clause_op1 = senis_add_clause_op(b_ctx, op1_type, op1);
        uint32_t* clause_op2 = senis_add_clause_op(b_ctx, op2_type, op2);

        SENIS_labels(while_blk_begin, while_blk_end, while_clause_end);

        uint32_t* prev_senis_continue_addr = b_ctx->senis_continue_addr;
        uint32_t* prev_senis_break_addr = b_ctx->senis_break_addr;

        b_ctx->senis_continue_addr = _SNC_OP_DIRECT_ADDRESS(l(while_blk_end));
        b_ctx->senis_break_addr = _SNC_OP_DIRECT_ADDRESS(l(while_clause_end));

        senis_goto(b_ctx, _SNC_OP(l(while_blk_end)));

        SENIS_label(while_blk_begin);

        while_blk(b_ctx);

        SENIS_label(while_blk_end);

        if (op1_type == SENIS_OPER_TYPE_ADDRESS_IA) {
                senis_wadad(b_ctx, _SNC_OP(da(clause_op1)), _SNC_OP(ia(op1)));
        }
        if (op2_type == SENIS_OPER_TYPE_ADDRESS_IA) {
                senis_wadad(b_ctx, _SNC_OP(da(clause_op2)), _SNC_OP(ia(op2)));
        }
        senis_rdcgr(b_ctx, clause_op1, clause_op2);
        senis_cobr_gr(b_ctx, _SNC_OP(l(while_blk_begin)));
        senis_rdcgr(b_ctx, clause_op2, clause_op1);
        senis_cobr_gr(b_ctx, _SNC_OP(l(while_blk_begin)));

        SENIS_label(while_clause_end);

        senis_rmv_clause_op(b_ctx, op2_type, clause_op2);
        senis_rmv_clause_op(b_ctx, op1_type, clause_op1);

        b_ctx->senis_continue_addr = prev_senis_continue_addr;
        b_ctx->senis_break_addr = prev_senis_break_addr;
}

void senis_while_bit(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1, uint32_t op2,
                                     senis_while_blk_t while_blk)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(op1_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(op2 < 32);
        ASSERT_WARNING(while_blk);

        if (op1_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op1_val = (uint32_t)op1;

                if (op1_val & (1 << op2)) {
                        senis_while_true(b_ctx, while_blk);
                }
                return;
        }

        uint32_t* clause_op1 = senis_add_clause_op(b_ctx, op1_type, op1);

        SENIS_labels(while_blk_begin, while_blk_end, while_clause_end);

        uint32_t* prev_senis_continue_addr = b_ctx->senis_continue_addr;
        uint32_t* prev_senis_break_addr = b_ctx->senis_break_addr;

        b_ctx->senis_continue_addr = _SNC_OP_DIRECT_ADDRESS(l(while_blk_end));
        b_ctx->senis_break_addr = _SNC_OP_DIRECT_ADDRESS(l(while_clause_end));

        senis_goto(b_ctx, _SNC_OP(l(while_blk_end)));

        SENIS_label(while_blk_begin);

        while_blk(b_ctx);

        SENIS_label(while_blk_end);

        if (op1_type == SENIS_OPER_TYPE_ADDRESS_IA) {
                senis_wadad(b_ctx, _SNC_OP(da(clause_op1)), _SNC_OP(ia(op1)));
        }
        senis_rdcbi(b_ctx, clause_op1, op2);
        senis_cobr_eq(b_ctx, _SNC_OP(l(while_blk_begin)));

        SENIS_label(while_clause_end);

        senis_rmv_clause_op(b_ctx, op1_type, clause_op1);

        b_ctx->senis_continue_addr = prev_senis_continue_addr;
        b_ctx->senis_break_addr = prev_senis_break_addr;
}

void senis_while_nbit(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1, uint32_t op2,
                                      senis_while_blk_t while_blk)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(op1_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(op2 < 32);
        ASSERT_WARNING(while_blk);

        if (op1_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op1_val = (uint32_t)op1;

                if (!(op1_val & (1 << op2))) {
                        senis_while_true(b_ctx, while_blk);
                }
                return;
        }

        uint32_t* clause_op1 = senis_add_clause_op(b_ctx, op1_type, op1);

        SENIS_labels(while_blk_begin, while_blk_end, while_clause_end);

        uint32_t* prev_senis_continue_addr = b_ctx->senis_continue_addr;
        uint32_t* prev_senis_break_addr = b_ctx->senis_break_addr;

        b_ctx->senis_continue_addr = _SNC_OP_DIRECT_ADDRESS(l(while_blk_end));
        b_ctx->senis_break_addr = _SNC_OP_DIRECT_ADDRESS(l(while_clause_end));

        senis_goto(b_ctx, _SNC_OP(l(while_blk_end)));

        SENIS_label(while_blk_begin);

        while_blk(b_ctx);

        SENIS_label(while_blk_end);

        if (op1_type == SENIS_OPER_TYPE_ADDRESS_IA) {
                senis_wadad(b_ctx, _SNC_OP(da(clause_op1)), _SNC_OP(ia(op1)));
        }
        senis_rdcbi(b_ctx, clause_op1, op2);
        senis_tobre(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(&SNC->SNC_STATUS_REG)), SENIS_FLAG_EQ);
        senis_cobr_eq(b_ctx, _SNC_OP(l(while_blk_begin)));

        SENIS_label(while_clause_end);

        senis_rmv_clause_op(b_ctx, op1_type, clause_op1);

        b_ctx->senis_continue_addr = prev_senis_continue_addr;
        b_ctx->senis_break_addr = prev_senis_break_addr;
}

void senis_while_gt_z(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                      senis_while_blk_t while_blk)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(op1_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(while_blk);

        if (op1_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op1_val = (uint32_t)op1;

                if (op1_val > 0) {
                        senis_while_true(b_ctx, while_blk);
                }
                return;
        }

        uint32_t* clause_op1 = senis_add_clause_op(b_ctx, op1_type, op1);

        SENIS_labels(while_blk_begin, while_blk_end, while_clause_end);

        uint32_t* prev_senis_continue_addr = b_ctx->senis_continue_addr;
        uint32_t* prev_senis_break_addr = b_ctx->senis_break_addr;

        b_ctx->senis_continue_addr = _SNC_OP_DIRECT_ADDRESS(l(while_blk_end));
        b_ctx->senis_break_addr = _SNC_OP_DIRECT_ADDRESS(l(while_clause_end));

        senis_goto(b_ctx, _SNC_OP(l(while_blk_end)));

        SENIS_label(while_blk_begin);

        while_blk(b_ctx);

        SENIS_label(while_blk_end);

        if (op1_type == SENIS_OPER_TYPE_ADDRESS_IA) {
                senis_wadad(b_ctx, _SNC_OP(da(clause_op1)), _SNC_OP(ia(op1)));
        }
        senis_rdcgr_z(b_ctx, clause_op1);
        senis_cobr_gr(b_ctx, _SNC_OP(l(while_blk_begin)));

        SENIS_label(while_clause_end);

        senis_rmv_clause_op(b_ctx, op1_type, clause_op1);

        b_ctx->senis_continue_addr = prev_senis_continue_addr;
        b_ctx->senis_break_addr = prev_senis_break_addr;
}

void senis_while_eq_z(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                      senis_while_blk_t while_blk)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(op1_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(while_blk);

        if (op1_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op1_val = (uint32_t)op1;

                if (op1_val == 0) {
                        senis_while_true(b_ctx, while_blk);
                }
                return;
        }

        uint32_t* clause_op1 = senis_add_clause_op(b_ctx, op1_type, op1);

        SENIS_labels(while_blk_begin, while_blk_end, while_clause_end);

        uint32_t* prev_senis_continue_addr = b_ctx->senis_continue_addr;
        uint32_t* prev_senis_break_addr = b_ctx->senis_break_addr;

        b_ctx->senis_continue_addr = _SNC_OP_DIRECT_ADDRESS(l(while_blk_end));
        b_ctx->senis_break_addr = _SNC_OP_DIRECT_ADDRESS(l(while_clause_end));

        senis_goto(b_ctx, _SNC_OP(l(while_blk_end)));

        SENIS_label(while_blk_begin);

        while_blk(b_ctx);

        SENIS_label(while_blk_end);

        if (op1_type == SENIS_OPER_TYPE_ADDRESS_IA) {
                senis_wadad(b_ctx, _SNC_OP(da(clause_op1)), _SNC_OP(ia(op1)));
        }
        senis_rdcgr(b_ctx, &snc_const[1], clause_op1);
        senis_cobr_gr(b_ctx, _SNC_OP(l(while_blk_begin)));

        SENIS_label(while_clause_end);

        senis_rmv_clause_op(b_ctx, op1_type, clause_op1);

        b_ctx->senis_continue_addr = prev_senis_continue_addr;
        b_ctx->senis_break_addr = prev_senis_break_addr;
}

void senis_while_true(b_ctx_t* b_ctx, senis_while_blk_t while_blk)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(while_blk);

        SENIS_labels(while_clause_begin, while_clause_end);

        uint32_t* prev_senis_continue_addr = b_ctx->senis_continue_addr;
        uint32_t* prev_senis_break_addr = b_ctx->senis_break_addr;

        b_ctx->senis_continue_addr = _SNC_OP_DIRECT_ADDRESS(l(while_clause_begin));
        b_ctx->senis_break_addr = _SNC_OP_DIRECT_ADDRESS(l(while_clause_end));

        SENIS_label(while_clause_begin);

        while_blk(b_ctx);

        senis_goto(b_ctx, _SNC_OP(l(while_clause_begin)));

        SENIS_label(while_clause_end);

        b_ctx->senis_continue_addr = prev_senis_continue_addr;
        b_ctx->senis_break_addr = prev_senis_break_addr;
}

void senis_dowhile_lt(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                      SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                      senis_dowhile_blk_t dowhile_blk)
{
        senis_dowhile_gt(b_ctx, op2_type, op2, op1_type, op1, dowhile_blk);
}

void senis_dowhile_lteq(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                        SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                        senis_dowhile_blk_t dowhile_blk)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(op1_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(op2_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(dowhile_blk);

        if (op1_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op1_val = (uint32_t)op1;

                if (op1_val == 0) {
                        senis_dowhile_true(b_ctx, dowhile_blk);
                        return;
                } else if (op2_type == SENIS_OPER_TYPE_VALUE) {
                        if (op1_val <= (uint32_t)op2) {
                                senis_dowhile_true(b_ctx, dowhile_blk);
                        } else {
                                senis_dowhile_false(b_ctx, dowhile_blk);
                        }
                        return;
                }
        }

        if (op2_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op2_val = (uint32_t)op2;

                if (op2_val == 0) {
                        senis_dowhile_eq_z(b_ctx, op1_type, op1, dowhile_blk);
                        return;
                }
        }

        uint32_t* clause_op1 = senis_add_clause_op(b_ctx, op1_type, op1);
        uint32_t* clause_op2 = senis_add_clause_op(b_ctx, op2_type, op2);

        SENIS_labels(dowhile_blk_begin, dowhile_blk_end, dowhile_clause_end);

        uint32_t* prev_senis_continue_addr = b_ctx->senis_continue_addr;
        uint32_t* prev_senis_break_addr = b_ctx->senis_break_addr;

        b_ctx->senis_continue_addr = _SNC_OP_DIRECT_ADDRESS(l(dowhile_blk_end));
        b_ctx->senis_break_addr = _SNC_OP_DIRECT_ADDRESS(l(dowhile_clause_end));

        SENIS_label(dowhile_blk_begin);

        dowhile_blk(b_ctx);

        SENIS_label(dowhile_blk_end);

        if (op1_type == SENIS_OPER_TYPE_ADDRESS_IA) {
                senis_wadad(b_ctx, _SNC_OP(da(clause_op1)), _SNC_OP(ia(op1)));
        }
        if (op2_type == SENIS_OPER_TYPE_ADDRESS_IA) {
                senis_wadad(b_ctx, _SNC_OP(da(clause_op2)), _SNC_OP(ia(op2)));
        }
        senis_rdcgr(b_ctx, clause_op1, clause_op2);
        senis_cobr_gr(b_ctx, _SNC_OP(l(dowhile_clause_end)));
        senis_goto(b_ctx, _SNC_OP(l(dowhile_blk_begin)));

        SENIS_label(dowhile_clause_end);

        senis_rmv_clause_op(b_ctx, op2_type, clause_op2);
        senis_rmv_clause_op(b_ctx, op1_type, clause_op1);

        b_ctx->senis_continue_addr = prev_senis_continue_addr;
        b_ctx->senis_break_addr = prev_senis_break_addr;
}

void senis_dowhile_gt(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                      SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                      senis_dowhile_blk_t dowhile_blk)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(op1_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(op2_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(dowhile_blk);

        if (op1_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op1_val = (uint32_t)op1;

                if (op1_val == 0) {
                        senis_dowhile_false(b_ctx, dowhile_blk);
                        return;
                } else if (op2_type == SENIS_OPER_TYPE_VALUE) {
                        if (op1_val > (uint32_t)op2) {
                                senis_dowhile_true(b_ctx, dowhile_blk);
                        } else {
                                senis_dowhile_false(b_ctx, dowhile_blk);
                        }
                        return;
                }
        }

        if (op2_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op2_val = (uint32_t)op2;

                if (op2_val == 0) {
                        senis_dowhile_gt_z(b_ctx, op1_type, op1, dowhile_blk);
                        return;
                }
        }

        uint32_t* clause_op1 = senis_add_clause_op(b_ctx, op1_type, op1);
        uint32_t* clause_op2 = senis_add_clause_op(b_ctx, op2_type, op2);

        SENIS_labels(dowhile_blk_begin, dowhile_blk_end, dowhile_clause_end);

        uint32_t* prev_senis_continue_addr = b_ctx->senis_continue_addr;
        uint32_t* prev_senis_break_addr = b_ctx->senis_break_addr;

        b_ctx->senis_continue_addr = _SNC_OP_DIRECT_ADDRESS(l(dowhile_blk_end));
        b_ctx->senis_break_addr = _SNC_OP_DIRECT_ADDRESS(l(dowhile_clause_end));

        SENIS_label(dowhile_blk_begin);

        dowhile_blk(b_ctx);

        SENIS_label(dowhile_blk_end);

        if (op1_type == SENIS_OPER_TYPE_ADDRESS_IA) {
                senis_wadad(b_ctx, _SNC_OP(da(clause_op1)), _SNC_OP(ia(op1)));
        }
        if (op2_type == SENIS_OPER_TYPE_ADDRESS_IA) {
                senis_wadad(b_ctx, _SNC_OP(da(clause_op2)), _SNC_OP(ia(op2)));
        }
        senis_rdcgr(b_ctx, clause_op1, clause_op2);
        senis_cobr_gr(b_ctx, _SNC_OP(l(dowhile_blk_begin)));

        SENIS_label(dowhile_clause_end);

        senis_rmv_clause_op(b_ctx, op2_type, clause_op2);
        senis_rmv_clause_op(b_ctx, op1_type, clause_op1);

        b_ctx->senis_continue_addr = prev_senis_continue_addr;
        b_ctx->senis_break_addr = prev_senis_break_addr;
}

void senis_dowhile_gteq(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                        SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                        senis_dowhile_blk_t dowhile_blk)
{
        senis_dowhile_lteq(b_ctx, op2_type, op2, op1_type, op1, dowhile_blk);
}

void senis_dowhile_eq(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                      SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                      senis_dowhile_blk_t dowhile_blk)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(op1_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(op2_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(dowhile_blk);

        if (op1_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op1_val = (uint32_t)op1;

                if (op1_val == 0) {
                        senis_dowhile_eq_z(b_ctx, op2_type, op2, dowhile_blk);
                        return;
                } else if (op2_type == SENIS_OPER_TYPE_VALUE) {
                        if (op1_val == (uint32_t)op2) {
                                senis_dowhile_true(b_ctx, dowhile_blk);
                        } else {
                                senis_dowhile_false(b_ctx, dowhile_blk);
                        }
                        return;
                }
        }

        if (op2_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op2_val = (uint32_t)op2;

                if (op2_val == 0) {
                        senis_dowhile_eq_z(b_ctx, op1_type, op1, dowhile_blk);
                        return;
                }
        }

        uint32_t* clause_op1 = senis_add_clause_op(b_ctx, op1_type, op1);
        uint32_t* clause_op2 = senis_add_clause_op(b_ctx, op2_type, op2);

        SENIS_labels(dowhile_blk_begin, dowhile_blk_end, dowhile_clause_end);

        uint32_t* prev_senis_continue_addr = b_ctx->senis_continue_addr;
        uint32_t* prev_senis_break_addr = b_ctx->senis_break_addr;

        b_ctx->senis_continue_addr = _SNC_OP_DIRECT_ADDRESS(l(dowhile_blk_end));
        b_ctx->senis_break_addr = _SNC_OP_DIRECT_ADDRESS(l(dowhile_clause_end));

        SENIS_label(dowhile_blk_begin);

        dowhile_blk(b_ctx);

        SENIS_label(dowhile_blk_end);

        if (op1_type == SENIS_OPER_TYPE_ADDRESS_IA) {
                senis_wadad(b_ctx, _SNC_OP(da(clause_op1)), _SNC_OP(ia(op1)));
        }
        if (op2_type == SENIS_OPER_TYPE_ADDRESS_IA) {
                senis_wadad(b_ctx, _SNC_OP(da(clause_op2)), _SNC_OP(ia(op2)));
        }
        senis_rdcgr(b_ctx, clause_op1, clause_op2);
        senis_cobr_gr(b_ctx, _SNC_OP(l(dowhile_clause_end)));
        senis_rdcgr(b_ctx, clause_op2, clause_op1);
        senis_cobr_gr(b_ctx, _SNC_OP(l(dowhile_clause_end)));
        senis_goto(b_ctx, _SNC_OP(l(dowhile_blk_begin)));

        SENIS_label(dowhile_clause_end);

        senis_rmv_clause_op(b_ctx, op2_type, clause_op2);
        senis_rmv_clause_op(b_ctx, op1_type, clause_op1);

        b_ctx->senis_continue_addr = prev_senis_continue_addr;
        b_ctx->senis_break_addr = prev_senis_break_addr;
}

void senis_dowhile_neq(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                       SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                       senis_dowhile_blk_t dowhile_blk)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(op1_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(op2_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(dowhile_blk);

        if (op1_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op1_val = (uint32_t)op1;

                if (op1_val == 0) {
                        senis_dowhile_gt_z(b_ctx, op2_type, op2, dowhile_blk);
                        return;
                } else if (op2_type == SENIS_OPER_TYPE_VALUE) {
                        if (op1_val != (uint32_t)op2) {
                                senis_dowhile_true(b_ctx, dowhile_blk);
                        } else {
                                senis_dowhile_false(b_ctx, dowhile_blk);
                        }
                        return;
                }
        }

        if (op2_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op2_val = (uint32_t)op2;

                if (op2_val == 0) {
                        senis_dowhile_gt_z(b_ctx, op1_type, op1, dowhile_blk);
                        return;
                }
        }

        uint32_t* clause_op1 = senis_add_clause_op(b_ctx, op1_type, op1);
        uint32_t* clause_op2 = senis_add_clause_op(b_ctx, op2_type, op2);

        SENIS_labels(dowhile_blk_begin, dowhile_blk_end, dowhile_clause_end);

        uint32_t* prev_senis_continue_addr = b_ctx->senis_continue_addr;
        uint32_t* prev_senis_break_addr = b_ctx->senis_break_addr;

        b_ctx->senis_continue_addr = _SNC_OP_DIRECT_ADDRESS(l(dowhile_blk_end));
        b_ctx->senis_break_addr = _SNC_OP_DIRECT_ADDRESS(l(dowhile_clause_end));

        SENIS_label(dowhile_blk_begin);

        dowhile_blk(b_ctx);

        SENIS_label(dowhile_blk_end);

        if (op1_type == SENIS_OPER_TYPE_ADDRESS_IA) {
                senis_wadad(b_ctx, _SNC_OP(da(clause_op1)), _SNC_OP(ia(op1)));
        }
        if (op2_type == SENIS_OPER_TYPE_ADDRESS_IA) {
                senis_wadad(b_ctx, _SNC_OP(da(clause_op2)), _SNC_OP(ia(op2)));
        }
        senis_rdcgr(b_ctx, clause_op1, clause_op2);
        senis_cobr_gr(b_ctx, _SNC_OP(l(dowhile_blk_begin)));
        senis_rdcgr(b_ctx, clause_op2, clause_op1);
        senis_cobr_gr(b_ctx, _SNC_OP(l(dowhile_blk_begin)));

        SENIS_label(dowhile_clause_end);

        senis_rmv_clause_op(b_ctx, op2_type, clause_op2);
        senis_rmv_clause_op(b_ctx, op1_type, clause_op1);

        b_ctx->senis_continue_addr = prev_senis_continue_addr;
        b_ctx->senis_break_addr = prev_senis_break_addr;
}

void senis_dowhile_bit(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1, uint32_t op2,
                                       senis_dowhile_blk_t dowhile_blk)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(op1_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(op2 < 32);
        ASSERT_WARNING(dowhile_blk);

        if (op1_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op1_val = (uint32_t)op1;

                if (op1_val & (1 << op2)) {
                        senis_dowhile_true(b_ctx, dowhile_blk);
                } else {
                        senis_dowhile_false(b_ctx, dowhile_blk);
                }
                return;
        }

        uint32_t* clause_op1 = senis_add_clause_op(b_ctx, op1_type, op1);

        SENIS_labels(dowhile_blk_begin, dowhile_blk_end, dowhile_clause_end);

        uint32_t* prev_senis_continue_addr = b_ctx->senis_continue_addr;
        uint32_t* prev_senis_break_addr = b_ctx->senis_break_addr;

        b_ctx->senis_continue_addr = _SNC_OP_DIRECT_ADDRESS(l(dowhile_blk_end));
        b_ctx->senis_break_addr = _SNC_OP_DIRECT_ADDRESS(l(dowhile_clause_end));

        SENIS_label(dowhile_blk_begin);

        dowhile_blk(b_ctx);

        SENIS_label(dowhile_blk_end);

        if (op1_type == SENIS_OPER_TYPE_ADDRESS_IA) {
                senis_wadad(b_ctx, _SNC_OP(da(clause_op1)), _SNC_OP(ia(op1)));
        }
        senis_rdcbi(b_ctx, clause_op1, op2);
        senis_cobr_eq(b_ctx, _SNC_OP(l(dowhile_blk_begin)));

        SENIS_label(dowhile_clause_end);

        senis_rmv_clause_op(b_ctx, op1_type, clause_op1);

        b_ctx->senis_continue_addr = prev_senis_continue_addr;
        b_ctx->senis_break_addr = prev_senis_break_addr;
}

void senis_dowhile_nbit(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1, uint32_t op2,
                                        senis_dowhile_blk_t dowhile_blk)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(op1_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(op2 < 32);
        ASSERT_WARNING(dowhile_blk);

        if (op1_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op1_val = (uint32_t)op1;

                if (op1_val & (1 << op2)) {
                        senis_dowhile_false(b_ctx, dowhile_blk);
                } else {
                        senis_dowhile_true(b_ctx, dowhile_blk);
                }
                return;
        }

        uint32_t* clause_op1 = senis_add_clause_op(b_ctx, op1_type, op1);

        SENIS_labels(dowhile_blk_begin, dowhile_blk_end, dowhile_clause_end);

        uint32_t* prev_senis_continue_addr = b_ctx->senis_continue_addr;
        uint32_t* prev_senis_break_addr = b_ctx->senis_break_addr;

        b_ctx->senis_continue_addr = _SNC_OP_DIRECT_ADDRESS(l(dowhile_blk_end));
        b_ctx->senis_break_addr = _SNC_OP_DIRECT_ADDRESS(l(dowhile_clause_end));

        SENIS_label(dowhile_blk_begin);

        dowhile_blk(b_ctx);

        SENIS_label(dowhile_blk_end);

        if (op1_type == SENIS_OPER_TYPE_ADDRESS_IA) {
                senis_wadad(b_ctx, _SNC_OP(da(clause_op1)), _SNC_OP(ia(op1)));
        }
        senis_rdcbi(b_ctx, clause_op1, op2);
        senis_tobre(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(&SNC->SNC_STATUS_REG)), SENIS_FLAG_EQ);
        senis_cobr_eq(b_ctx, _SNC_OP(l(dowhile_blk_begin)));

        SENIS_label(dowhile_clause_end);

        senis_rmv_clause_op(b_ctx, op1_type, clause_op1);

        b_ctx->senis_continue_addr = prev_senis_continue_addr;
        b_ctx->senis_break_addr = prev_senis_break_addr;
}

void senis_dowhile_gt_z(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                        senis_dowhile_blk_t dowhile_blk)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(op1_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(dowhile_blk);

        if (op1_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op1_val = (uint32_t)op1;

                if (op1_val > 0) {
                        senis_dowhile_true(b_ctx, dowhile_blk);
                } else {
                        senis_dowhile_false(b_ctx, dowhile_blk);
                }
                return;
        }

        uint32_t* clause_op1 = senis_add_clause_op(b_ctx, op1_type, op1);

        SENIS_labels(dowhile_blk_begin, dowhile_blk_end, dowhile_clause_end);

        uint32_t* prev_senis_continue_addr = b_ctx->senis_continue_addr;
        uint32_t* prev_senis_break_addr = b_ctx->senis_break_addr;

        b_ctx->senis_continue_addr = _SNC_OP_DIRECT_ADDRESS(l(dowhile_blk_end));
        b_ctx->senis_break_addr = _SNC_OP_DIRECT_ADDRESS(l(dowhile_clause_end));

        SENIS_label(dowhile_blk_begin);

        dowhile_blk(b_ctx);

        SENIS_label(dowhile_blk_end);

        if (op1_type == SENIS_OPER_TYPE_ADDRESS_IA) {
                senis_wadad(b_ctx, _SNC_OP(da(clause_op1)), _SNC_OP(ia(op1)));
        }
        senis_rdcgr_z(b_ctx, clause_op1);
        senis_cobr_gr(b_ctx, _SNC_OP(l(dowhile_blk_begin)));

        SENIS_label(dowhile_clause_end);

        senis_rmv_clause_op(b_ctx, op1_type, clause_op1);

        b_ctx->senis_continue_addr = prev_senis_continue_addr;
        b_ctx->senis_break_addr = prev_senis_break_addr;
}

void senis_dowhile_eq_z(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                                        senis_dowhile_blk_t dowhile_blk)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(op1_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(dowhile_blk);

        if (op1_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op1_val = (uint32_t)op1;

                if (op1_val == 0) {
                        senis_dowhile_true(b_ctx, dowhile_blk);
                } else {
                        senis_dowhile_false(b_ctx, dowhile_blk);
                }
                return;
        }

        uint32_t* clause_op1 = senis_add_clause_op(b_ctx, op1_type, op1);

        SENIS_labels(dowhile_blk_begin, dowhile_blk_end, dowhile_clause_end);

        uint32_t* prev_senis_continue_addr = b_ctx->senis_continue_addr;
        uint32_t* prev_senis_break_addr = b_ctx->senis_break_addr;

        b_ctx->senis_continue_addr = _SNC_OP_DIRECT_ADDRESS(l(dowhile_blk_end));
        b_ctx->senis_break_addr = _SNC_OP_DIRECT_ADDRESS(l(dowhile_clause_end));

        SENIS_label(dowhile_blk_begin);

        dowhile_blk(b_ctx);

        SENIS_label(dowhile_blk_end);

        if (op1_type == SENIS_OPER_TYPE_ADDRESS_IA) {
                senis_wadad(b_ctx, _SNC_OP(da(clause_op1)), _SNC_OP(ia(op1)));
        }
        senis_rdcgr(b_ctx, &snc_const[1], clause_op1);
        senis_cobr_gr(b_ctx, _SNC_OP(l(dowhile_blk_begin)));

        SENIS_label(dowhile_clause_end);

        senis_rmv_clause_op(b_ctx, op1_type, clause_op1);

        b_ctx->senis_continue_addr = prev_senis_continue_addr;
        b_ctx->senis_break_addr = prev_senis_break_addr;
}

void senis_dowhile_true(b_ctx_t* b_ctx, senis_dowhile_blk_t dowhile_blk)
{
        senis_while_true(b_ctx, dowhile_blk);
}

void senis_dowhile_false(b_ctx_t* b_ctx, senis_dowhile_blk_t dowhile_blk)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(dowhile_blk);

        SENIS_labels(dowhile_clause_end);

        uint32_t* prev_senis_continue_addr = b_ctx->senis_continue_addr;
        uint32_t* prev_senis_break_addr = b_ctx->senis_break_addr;

        b_ctx->senis_continue_addr = _SNC_OP_DIRECT_ADDRESS(l(dowhile_clause_end));
        b_ctx->senis_break_addr = _SNC_OP_DIRECT_ADDRESS(l(dowhile_clause_end));

        dowhile_blk(b_ctx);

        SENIS_label(dowhile_clause_end);

        b_ctx->senis_continue_addr = prev_senis_continue_addr;
        b_ctx->senis_break_addr = prev_senis_break_addr;
}

void senis_goto(b_ctx_t* b_ctx, SENIS_OPER_TYPE addr_type, uint32_t* addr)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING((addr_type < SENIS_OPER_TYPE_VALUE) &&
                (!b_ctx->upd || !_SNC_ADDR_IS_REG(addr)));

        senis_rdcbi(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(&snc_const[1])), 0);
        senis_cobr_eq(b_ctx, addr_type, addr);
}

SNC_FUNC_DECL(senis_add_sub_ucode, uint32_t op1_value, uint32_t op2_value, uint32_t* sum_addr,
        uint32_t is_add);

static void senis_add_sub(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_addr_type, uint32_t* op1_addr,
                                          SENIS_OPER_TYPE op2_type, uint32_t* op2, bool is_add)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(op1_addr_type < SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(op2_type <= SENIS_OPER_TYPE_VALUE);

        if (op2_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op2_val = (uint32_t)op2;

                if (op2_val == 0) {
                        return;
                }

                if (op1_addr_type == SENIS_OPER_TYPE_ADDRESS_DA) {
                        if (is_add && !_SNC_ADDR_IS_REG(op1_addr)) {
                                if (op2_val == 1) {
                                        senis_inc1(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(op1_addr)));
                                        return;
                                } else if (op2_val == 4) {
                                        senis_inc4(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(op1_addr)));
                                        return;
                                }
                        }
                }

                // Up to that op2 value, op1 is incremented using a sequence of SENIS_inc1/4 instructions
                if (op2_val <= (12 - ((1 - is_add) * 2))) {
                        uint32_t tmp_val;

                        _SNC_TMP_ADD(uint32_t, p_tmp_op1_val, sizeof(uint32_t));

                        senis_wadad(b_ctx, _SNC_OP(da(p_tmp_op1_val)), op1_addr_type, op1_addr);

                        if (!is_add) {
                                senis_tobre(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(p_tmp_op1_val)),
                                                   0xFFFFFFFF);
                        }

                        for (tmp_val = op2_val; tmp_val > 3; tmp_val -= 4) {
                                senis_inc4(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(p_tmp_op1_val)));
                        }

                        for (; tmp_val > 0; tmp_val--) {
                                senis_inc1(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(p_tmp_op1_val)));
                        }

                        if (!is_add) {
                                senis_tobre(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(p_tmp_op1_val)),
                                                   0xFFFFFFFF);
                        }

                        senis_wadad(b_ctx, op1_addr_type, op1_addr, _SNC_OP(da(p_tmp_op1_val)));

                        _SNC_TMP_RMV(p_tmp_op1_val);

                        return;
                }
        }

        if (_SNC_ADDR_IS_REG(op1_addr)) {
                _SNC_TMP_ADD(uint32_t, p_tmp_reg_val, sizeof(uint32_t));

                senis_wadad(b_ctx, _SNC_OP(da(p_tmp_reg_val)), op1_addr_type, op1_addr);

                senis_call(b_ctx, SNC_UCODE_CTX(senis_add_sub_ucode), 2 * 4,
                                  SENIS_OPER_TYPE_ADDRESS_DA, p_tmp_reg_val, op2_type, op2,
                                  SENIS_OPER_TYPE_VALUE, p_tmp_reg_val,
                                  SENIS_OPER_TYPE_VALUE, is_add);

                senis_wadad(b_ctx, op1_addr_type, op1_addr, _SNC_OP(da(p_tmp_reg_val)));

                _SNC_TMP_RMV(p_tmp_reg_val);
        } else {
                senis_call(b_ctx, SNC_UCODE_CTX(senis_add_sub_ucode), 2 * 4,
                                  op1_addr_type, op1_addr, op2_type, op2,
                                  op1_addr_type + 1, op1_addr,
                                  SENIS_OPER_TYPE_VALUE, is_add);
        }
}

void senis_add(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_addr_type, uint32_t* op1_addr,
                               SENIS_OPER_TYPE op2_type, uint32_t* op2)
{
        senis_add_sub(b_ctx, op1_addr_type, op1_addr, op2_type, op2, true);
}

void senis_sub(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_addr_type, uint32_t* op1_addr,
                               SENIS_OPER_TYPE op2_type, uint32_t* op2)
{
        senis_add_sub(b_ctx, op1_addr_type, op1_addr, op2_type, op2, false);
}

SNC_FUNC_DEF(senis_add_sub_ucode, uint32_t op1_value, uint32_t op2_value, uint32_t* sum_addr,
        uint32_t is_add)
{
        uint32_t* p_op1_value = &SNC_ARG(op1_value); // Assumed to be the greater operand value
        uint32_t* p_op2_value = &SNC_ARG(op2_value);
        uint32_t* p_is_add = &SNC_ARG(is_add);

        SENIS_labels(pt_op2_set_for_add, pt_op1_is_gt_op2, pt_minus_min_is_gt_plus_min, pt_add_sum_exit);

        _SNC_TMP_ADD(uint32_t, temp_op_val_addr, sizeof(uint32_t));

        SENIS_rdcbi(da(p_is_add), 0);
        SENIS_cobr_eq(l(pt_op2_set_for_add));

        SENIS_tobre(da(p_op1_value), 0xFFFFFFFF);

        SENIS_label(pt_op2_set_for_add);

        SENIS_rdcgr(da(p_op1_value), da(p_op2_value));
        SENIS_cobr_gr(l(pt_op1_is_gt_op2));

        // Swap values
        SENIS_wadad(da(temp_op_val_addr), da(p_op1_value));
        SENIS_wadad(da(p_op1_value), da(p_op2_value));
        SENIS_wadad(da(p_op2_value), da(temp_op_val_addr));

        SENIS_label(pt_op1_is_gt_op2);

        SENIS_wadad(da(temp_op_val_addr), da(p_op1_value));
        SENIS_tobre(da(temp_op_val_addr), 0xFFFFFFFF);

        SENIS_rdcgr(da(temp_op_val_addr), da(p_op2_value));
        SENIS_cobr_gr(l(pt_minus_min_is_gt_plus_min));

        // Swap values
        SENIS_wadad(da(p_op1_value), da(p_op2_value));
        SENIS_tobre(da(p_op1_value), 0xFFFFFFFF);
        SENIS_inc1(da(p_op1_value));
        SENIS_tobre(da(p_is_add), 0x1);
        SENIS_wadad(da(p_op2_value), da(temp_op_val_addr));

        SENIS_label(pt_minus_min_is_gt_plus_min);

        _SNC_TMP_RMV(temp_op_val_addr);

        { // Perform fast addition
                SENIS_labels(
                        pt_check_div_2_value, pt_check_div_4_value, pt_check_div_8_value,
                        pt_check_div_16_value, pt_check_div_32_value,
                        pt_exit
                );

                SENIS_tobre(da(p_op2_value), 0xFFFFFFFF);

                // Check if there is a (*p_op2_value % 2) value to increment
                SENIS_rdcbi(da(p_op2_value), 0);
                SENIS_cobr_eq(l(pt_check_div_2_value));

                SENIS_inc1(da(p_op1_value));
                SENIS_tobre(da(p_op2_value), 1);

                SENIS_label(pt_check_div_2_value);

                // Check if there is a (*p_op2_value / 2) value to increment
                SENIS_rdcbi(da(p_op2_value), 1);
                SENIS_cobr_eq(l(pt_check_div_4_value));

                SENIS_inc1(da(p_op1_value));
                SENIS_inc1(da(p_op1_value));
                SENIS_tobre(da(p_op2_value), 2);

                SENIS_label(pt_check_div_4_value);

                // Check if there is a (*p_op2_value / 4) value to increment
                SENIS_rdcbi(da(p_op2_value), 2);
                SENIS_cobr_eq(l(pt_check_div_8_value));

                SENIS_inc4(da(p_op1_value));
                SENIS_tobre(da(p_op2_value), 4);

                SENIS_label(pt_check_div_8_value);

                // Check if there is a (*p_op2_value / 8) value to increment
                SENIS_rdcbi(da(p_op2_value), 3);
                SENIS_cobr_eq(l(pt_check_div_16_value));

                SENIS_inc4(da(p_op1_value));
                SENIS_inc4(da(p_op1_value));
                SENIS_tobre(da(p_op2_value), 8);

                SENIS_label(pt_check_div_16_value);

                // Check if there is a (*p_op2_value / 16) value to increment
                SENIS_rdcbi(da(p_op2_value), 4);
                SENIS_cobr_eq(l(pt_exit));

                SENIS_inc4(da(p_op1_value));
                SENIS_inc4(da(p_op1_value));
                SENIS_inc4(da(p_op1_value));
                SENIS_inc4(da(p_op1_value));
                SENIS_tobre(da(p_op2_value), 16);

                SENIS_label(pt_exit);

                SENIS_tobre(da(p_op2_value), 0xFFFFFFFF);
        }
        { // Perform carry-based addition
                SENIS_labels(
                        pt_sum_begin,
                        pt_sum_cond_num_of_bits, pt_sum_cond,
                        pt_sum_bit_is_ready,
                        ph_tobre_bit_xored_op1op2, ph_tobre_bit_op2,
                        pt_sum_end
                );

                _SNC_TMP_ADD(uint32_t, carry_addr, sizeof(uint32_t));
                _SNC_TMP_ADD(uint32_t*, pp_tobre_bit_mask_array, sizeof(uint32_t*));
                _SNC_TMP_ADD(uint32_t, temp_value_addr, sizeof(uint32_t));

                SENIS_wadva(da(carry_addr), 0);
                SENIS_wadva(da(pp_tobre_bit_mask_array), &senis_TOBRE_bit_mask_array[4]);
                SENIS_wadad(l(ph_tobre_bit_op2) + 1, da(&senis_TOBRE_bit_mask_array[4]));
                SENIS_xor(da(p_op1_value), da(p_op2_value));

                SENIS_goto(l(pt_sum_cond));

                SENIS_label(pt_sum_cond_num_of_bits);

                SENIS_rdcbi(l(ph_tobre_bit_op2) + 1, 31);
                SENIS_cobr_eq(l(pt_sum_end));

                SENIS_label(pt_sum_begin);

                SENIS_inc4(da(pp_tobre_bit_mask_array));

                SENIS_wadad(l(ph_tobre_bit_xored_op1op2) + 1, ia(pp_tobre_bit_mask_array));
                SENIS_wadad(l(ph_tobre_bit_op2) + 1, l(ph_tobre_bit_xored_op1op2) + 1);

                SENIS_wadad(da(temp_value_addr), da(p_op1_value));
                SENIS_label(ph_tobre_bit_xored_op1op2);
                senis_tobre(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(p_op1_value)), SNC_PLACE_HOLDER);
                SENIS_rdcgr(da(temp_value_addr), da(p_op1_value));

                SENIS_rdcbi(da(carry_addr), 0);
                SENIS_cobr_eq(l(pt_sum_bit_is_ready));

                SENIS_wadad(da(p_op1_value), da(temp_value_addr));

                SENIS_label(pt_sum_bit_is_ready);

                SENIS_cobr_gr(l(pt_sum_cond));

                SENIS_wadva(da(carry_addr), 0);

                SENIS_wadad(da(temp_value_addr), da(p_op2_value));
                SENIS_label(ph_tobre_bit_op2);
                senis_tobre(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(temp_value_addr)), SNC_PLACE_HOLDER);

                SENIS_rdcgr(da(temp_value_addr), da(p_op2_value));
                SENIS_cobr_gr(l(pt_sum_cond));

                SENIS_wadva(da(carry_addr), 1);

                SENIS_label(pt_sum_cond);

                SENIS_rdcgr(da(p_op2_value), l(ph_tobre_bit_op2) + 1);
                SENIS_cobr_gr(l(pt_sum_begin));

                SENIS_rdcbi(da(carry_addr), 0);
                SENIS_cobr_eq(l(pt_sum_cond_num_of_bits));

                SENIS_label(pt_sum_end);

                _SNC_TMP_RMV(temp_value_addr);
                _SNC_TMP_RMV(pp_tobre_bit_mask_array);
                _SNC_TMP_RMV(carry_addr);
        }

        SENIS_rdcbi(da(p_is_add), 0);
        SENIS_cobr_eq(l(pt_add_sum_exit));

        SENIS_tobre(da(p_op1_value), 0xFFFFFFFF);

        SENIS_label(pt_add_sum_exit);

        SENIS_wadad(ia(&SNC_ARG(sum_addr)), da(p_op1_value));
}

SNC_FUNC_DECL(senis_shift_ucode, uint32_t* addr, uint32_t shift_value, uint32_t shift_bit_len,
        uint32_t* shift_src_RDCBI_value_addr, uint32_t* shift_dst_TOBRE_value_addr);

static void senis_shift(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_addr_type, uint32_t* op1_addr,
                                        SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                        SENIS_OPER_TYPE bit_msk_len_type, uint32_t* bit_msk_len,
                                        SENIS_SHIFT_TYPE shift_type)
{
        uint32_t* shift_src_RDCBI_value_addr;
        uint32_t* shift_dst_TOBRE_value_addr;

        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(op1_addr_type < SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(op2_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(bit_msk_len_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(shift_type <= SENIS_SHIFT_TYPE_LEFT);

        if (op2_type == SENIS_OPER_TYPE_VALUE) {
                if ((uint32_t)op2 > 31) {
                        senis_assign(b_ctx, op1_addr_type, op1_addr, _SNC_OP(0));
                        return;
                }
        }

        if (bit_msk_len_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t bit_msk_len_val = (uint32_t)bit_msk_len;

                if (bit_msk_len_val == 0 || bit_msk_len_val == 32) {
                        if (op2_type == SENIS_OPER_TYPE_VALUE) {
                                if ((uint32_t)op2 == 0) {
                                        return;
                                }
                        }
                }
        }

        if (shift_type == SENIS_SHIFT_TYPE_RIGHT) {
                if (op2_type == SENIS_OPER_TYPE_VALUE) {
                        shift_src_RDCBI_value_addr = &senis_RDCBI_bit_pos_array[(uint32_t)op2];
                } else {
                        shift_src_RDCBI_value_addr = NULL;
                }
                shift_dst_TOBRE_value_addr = &senis_TOBRE_bit_mask_array[0];
        } else {
                shift_src_RDCBI_value_addr = &senis_RDCBI_bit_pos_array[0];
                if (op2_type == SENIS_OPER_TYPE_VALUE) {
                        shift_dst_TOBRE_value_addr = &senis_TOBRE_bit_mask_array[(uint32_t)op2];
                } else {
                        shift_dst_TOBRE_value_addr = NULL;
                }
        }

        if (_SNC_ADDR_IS_REG(op1_addr)) {
                _SNC_TMP_ADD(uint32_t, p_tmp_reg_val, sizeof(uint32_t));

                SENIS_assign(da(p_tmp_reg_val), da(op1_addr));

                senis_call(b_ctx, SNC_UCODE_CTX(senis_shift_ucode), 2 * 5,
                                  SENIS_OPER_TYPE_VALUE, p_tmp_reg_val, op2_type, op2,
                                  bit_msk_len_type, bit_msk_len,
                                  _SNC_OP(shift_src_RDCBI_value_addr),
                                  _SNC_OP(shift_dst_TOBRE_value_addr));

                SENIS_assign(da(op1_addr), da(p_tmp_reg_val));

                _SNC_TMP_RMV(p_tmp_reg_val);
        } else {
                senis_call(b_ctx, SNC_UCODE_CTX(senis_shift_ucode), 2 * 5,
                                  op1_addr_type + 1, op1_addr, op2_type, op2,
                                  bit_msk_len_type, bit_msk_len,
                                  _SNC_OP(shift_src_RDCBI_value_addr),
                                  _SNC_OP(shift_dst_TOBRE_value_addr));
        }
}

void senis_rshift(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_addr_type, uint32_t* op1_addr,
                                  SENIS_OPER_TYPE op2_type, uint32_t* op2)
{
        senis_shift(b_ctx, op1_addr_type, op1_addr, op2_type, op2, _SNC_OP(0),
                SENIS_SHIFT_TYPE_RIGHT);
}

void senis_rshift_masked(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_addr_type, uint32_t* op1_addr,
                                         SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                         SENIS_OPER_TYPE bit_msk_len_type, uint32_t* bit_msk_len)
{
        senis_shift(b_ctx, op1_addr_type, op1_addr, op2_type, op2, bit_msk_len_type, bit_msk_len,
                SENIS_SHIFT_TYPE_RIGHT);
}

void senis_lshift(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_addr_type, uint32_t* op1_addr,
                                  SENIS_OPER_TYPE op2_type, uint32_t* op2)
{
        senis_shift(b_ctx, op1_addr_type, op1_addr, op2_type, op2, _SNC_OP(0),
                SENIS_SHIFT_TYPE_LEFT);
}

void senis_lshift_masked(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_addr_type, uint32_t* op1_addr,
                                         SENIS_OPER_TYPE op2_type, uint32_t* op2,
                                         SENIS_OPER_TYPE bit_msk_len_type, uint32_t* bit_msk_len)
{
        senis_shift(b_ctx, op1_addr_type, op1_addr, op2_type, op2, bit_msk_len_type, bit_msk_len,
                SENIS_SHIFT_TYPE_LEFT);
}

SNC_FUNC_DEF(senis_shift_ucode, uint32_t* addr, uint32_t shift_value, uint32_t shift_bit_len,
        uint32_t* shift_src_RDCBI_value_addr, uint32_t* shift_dst_TOBRE_value_addr)
{
        SENIS_labels(
                shift_pt_check_is_left_shift,
                shift_pt_check_shift_value,
                shift_blk_begin_find_first_idx,
                shift_blk_cond_find_first_idx,
                shift_pt_operation,
                shift_pt_shift_values_are_set,
                shift_blk_begin,
                shift_ph_check_src_bit,
                shift_pt_set_1_in_the_current_bit, shift_pt_keep_0_in_the_current_bit,
                shift_blk_cond,
                shift_blk_end,
                shift_pt_exit
        );

        _SNC_STATIC(uint32_t, snc_thirty_one, sizeof(uint32_t), 31);

        SNC_ASSERT(da(&SNC_ARG(shift_bit_len)), LTEQ, 32);

        _SNC_TMP_ADD(uint32_t, temp_shifted_value, sizeof(uint32_t));
        _SNC_TMP_ADD(uint32_t, temp_shift_value, sizeof(uint32_t));

        _SNC_TMP_ADD(uint32_t**, temp_shift_array_value_ind_addr, sizeof(uint32_t**));
        _SNC_TMP_ADD(uint32_t*, temp_shift_array_value_addr, sizeof(uint32_t*));

        SENIS_assign(da(temp_shifted_value), 0);

        SENIS_rdcgr_z(da(&SNC_ARG(shift_src_RDCBI_value_addr)));
        SENIS_cobr_gr(l(shift_pt_check_is_left_shift));

        SENIS_assign(da(temp_shift_array_value_ind_addr), &SNC_ARG(shift_src_RDCBI_value_addr));
        SENIS_assign(da(&SNC_ARG(shift_src_RDCBI_value_addr)), &senis_RDCBI_bit_pos_array[0]);

        SENIS_goto(l(shift_pt_check_shift_value));

        SENIS_label(shift_pt_check_is_left_shift);

        SENIS_rdcgr_z(da(&SNC_ARG(shift_dst_TOBRE_value_addr)));
        SENIS_cobr_gr(l(shift_pt_operation));

        SENIS_assign(da(temp_shift_array_value_ind_addr), &SNC_ARG(shift_dst_TOBRE_value_addr));
        SENIS_assign(da(&SNC_ARG(shift_dst_TOBRE_value_addr)), &senis_TOBRE_bit_mask_array[0]);

        SENIS_label(shift_pt_check_shift_value);

        SENIS_rdcgr(da(&snc_const[1]), da(&SNC_ARG(shift_value)));
        SENIS_cobr_gr(l(shift_pt_operation));

        SENIS_rdcgr(da(&SNC_ARG(shift_value)), da(snc_thirty_one));
        SENIS_cobr_gr(l(shift_blk_end));

        SENIS_assign(da(temp_shift_value), 0);
        SENIS_assign(da(temp_shift_array_value_addr), ia(temp_shift_array_value_ind_addr));

        SENIS_goto(l(shift_blk_cond_find_first_idx));

        SENIS_label(shift_blk_begin_find_first_idx);

        SENIS_inc1(da(temp_shift_value));
        SENIS_inc4(da(temp_shift_array_value_addr));

        SENIS_label(shift_blk_cond_find_first_idx);

        SENIS_rdcgr(da(&SNC_ARG(shift_value)), da(temp_shift_value));
        SENIS_cobr_gr(l(shift_blk_begin_find_first_idx));

        SENIS_assign(ia(temp_shift_array_value_ind_addr), da(temp_shift_array_value_addr));

        _SNC_TMP_RMV(temp_shift_array_value_addr);
        _SNC_TMP_RMV(temp_shift_array_value_ind_addr);

        SENIS_label(shift_pt_operation);

        SENIS_assign(da(temp_shift_value), da(&SNC_ARG(shift_value)));
        SENIS_tobre(da(temp_shift_value), 0xffffffff);
        SENIS_tobre(da(temp_shift_value), 0xffffffe0);
        SENIS_inc1(da(temp_shift_value));

        SENIS_rdcgr(da(&SNC_ARG(shift_bit_len)), da(temp_shift_value));
        SENIS_cobr_gr(l(shift_pt_shift_values_are_set));

        SENIS_rdcgr(da(&snc_const[1]), da(&SNC_ARG(shift_bit_len)));
        SENIS_cobr_gr(l(shift_pt_shift_values_are_set));

        SENIS_assign(da(temp_shift_value), da(&SNC_ARG(shift_bit_len)));

        SENIS_label(shift_pt_shift_values_are_set);

        _SNC_TMP_ADD(uint32_t, temp_addr_value, sizeof(uint32_t));

        SENIS_tobre(da(temp_shift_value), 0xffffffff);

        SENIS_assign(da(temp_addr_value), ia(&SNC_ARG(addr)));

        SENIS_goto(l(shift_blk_cond));

        SENIS_label(shift_blk_begin);

        SENIS_assign(l(shift_ph_check_src_bit), ia(&SNC_ARG(shift_src_RDCBI_value_addr)));
        SENIS_tobre(l(shift_ph_check_src_bit), _SENIS_B_RDCBI_D(temp_addr_value, 0));

        SENIS_label(shift_ph_check_src_bit);
        senis_rdcbi(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(SNC_PLACE_HOLDER)), SNC_PLACE_HOLDER);
        SENIS_cobr_eq(l(shift_pt_set_1_in_the_current_bit));
        SENIS_goto(l(shift_pt_keep_0_in_the_current_bit));

        SENIS_label(shift_pt_set_1_in_the_current_bit);

        SENIS_xor(da(temp_shifted_value), ia(&SNC_ARG(shift_dst_TOBRE_value_addr)));

        SENIS_label(shift_pt_keep_0_in_the_current_bit);

        SENIS_inc4(da(&SNC_ARG(shift_src_RDCBI_value_addr)));
        SENIS_inc4(da(&SNC_ARG(shift_dst_TOBRE_value_addr)));

        SENIS_label(shift_blk_cond);

        SENIS_inc1(da(temp_shift_value));
        SENIS_rdcgr_z(da(temp_shift_value));
        SENIS_cobr_gr(l(shift_blk_begin));

        _SNC_TMP_RMV(temp_addr_value);

        SENIS_label(shift_blk_end);

        SENIS_assign(ia(&SNC_ARG(addr)), da(temp_shifted_value));

        SENIS_label(shift_pt_exit);

        _SNC_TMP_RMV(temp_shift_value);
        _SNC_TMP_RMV(temp_shifted_value);
}

void senis_break(b_ctx_t* b_ctx)
{
        ASSERT_WARNING(b_ctx);

        senis_goto(b_ctx, _SNC_OP(da(b_ctx->senis_break_addr)));
}

void senis_continue(b_ctx_t* b_ctx)
{
        ASSERT_WARNING(b_ctx);

        senis_goto(b_ctx, _SNC_OP(da(b_ctx->senis_continue_addr)));
}

void senis_copy(b_ctx_t* b_ctx, SENIS_OPER_TYPE dst_addr_type, uint32_t* dst_addr,
                                SENIS_OPER_TYPE src_addr_type, uint32_t* src_addr,
                                SENIS_OPER_TYPE len_type, uint32_t* len)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING((dst_addr_type < SENIS_OPER_TYPE_VALUE) &&
                (!b_ctx->upd || !_SNC_ADDR_IS_REG(dst_addr)));
        ASSERT_WARNING((src_addr_type < SENIS_OPER_TYPE_VALUE) &&
                (!b_ctx->upd || !_SNC_ADDR_IS_REG(src_addr)));
        ASSERT_WARNING(len_type <= SENIS_OPER_TYPE_VALUE);

        uint32_t** applied_dst_ind_addr;
        uint32_t** applied_src_ind_addr;

        bool optim_len_value_ver_used;

        SENIS_labels(copy_while_blk_begin, copy_while_blk_cond);

        _SNC_TMP_ADD(uint32_t*, temp_dst_addr, sizeof(uint32_t*));
        _SNC_TMP_ADD(uint32_t*, temp_src_addr, sizeof(uint32_t*));
        _SNC_TMP_ADD(uint32_t, temp_len, sizeof(uint32_t));

        if (dst_addr_type == SENIS_OPER_TYPE_ADDRESS_DA) {
                senis_wadva(b_ctx, _SNC_OP(da(&temp_dst_addr[0])), (uint32_t)dst_addr);
                applied_dst_ind_addr = &temp_dst_addr[0];
        } else {
                applied_dst_ind_addr = (uint32_t**)dst_addr;
        }

        if (src_addr_type == SENIS_OPER_TYPE_ADDRESS_DA) {
                senis_wadva(b_ctx, _SNC_OP(da(&temp_src_addr[0])), (uint32_t)src_addr);
                applied_src_ind_addr = &temp_src_addr[0];
        } else {
                applied_src_ind_addr = (uint32_t**)src_addr;
        }

        if (len_type == SENIS_OPER_TYPE_VALUE && ((uint32_t)len) <= (((uint32_t)1) << 31)) {
                optim_len_value_ver_used = true;
        } else {
                optim_len_value_ver_used = false;
        }

        if (optim_len_value_ver_used) {
                senis_wadva(b_ctx, _SNC_OP(da(&temp_len[0])), (((uint32_t )0) - ((uint32_t )len)));
        } else {
                senis_assign(b_ctx, _SNC_OP(da(&temp_len[0])), len_type, len);
                senis_tobre(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(&temp_len[0])), 0xFFFFFFFF);
                senis_inc1(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(&temp_len[0])));
        }

        senis_goto(b_ctx, _SNC_OP(l(copy_while_blk_cond)));

        SENIS_label(copy_while_blk_begin);

        senis_wadad(b_ctx, _SNC_OP(ia(applied_dst_ind_addr)), _SNC_OP(ia(applied_src_ind_addr)));
        senis_inc4(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(applied_dst_ind_addr)));
        senis_inc4(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(applied_src_ind_addr)));
        senis_inc1(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(&temp_len[0])));

        SENIS_label(copy_while_blk_cond);

        if (optim_len_value_ver_used) {
                senis_rdcbi(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(&temp_len[0])), 31);
                senis_cobr_eq(b_ctx, _SNC_OP(l(copy_while_blk_begin)));
        } else {
                senis_rdcgr_z(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(&temp_len[0])));
                senis_cobr_gr(b_ctx, _SNC_OP(l(copy_while_blk_begin)));
        }

        _SNC_TMP_RMV(temp_len);
        _SNC_TMP_RMV(temp_src_addr);
        _SNC_TMP_RMV(temp_dst_addr);
}

SNC_FUNC_DECL(senis_compare_ucode, uint32_t** ind_addr, uint32_t** cmp_ind_addr, uint32_t len,
        uint32_t* rtn_addr);

void senis_compare(b_ctx_t* b_ctx, SENIS_OPER_TYPE addr_type, uint32_t* addr,
                                   SENIS_OPER_TYPE cmp_addr_type, uint32_t* cmp_addr,
                                   SENIS_OPER_TYPE len_type, uint32_t* len,
                                   SENIS_OPER_TYPE rtn_addr_type, uint32_t* rtn_addr)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING((addr_type < SENIS_OPER_TYPE_VALUE) &&
                (!b_ctx->upd || !_SNC_ADDR_IS_REG(addr)));
        ASSERT_WARNING((cmp_addr_type < SENIS_OPER_TYPE_VALUE) &&
                (!b_ctx->upd || !_SNC_ADDR_IS_REG(cmp_addr)));
        ASSERT_WARNING(len_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(rtn_addr_type < SENIS_OPER_TYPE_VALUE);

        uint32_t** applied_ind_addr;
        uint32_t** applied_cmp_ind_addr;

        _SNC_TMP_ADD(uint32_t*, temp_addr, sizeof(uint32_t*));
        _SNC_TMP_ADD(uint32_t*, temp_cmp_addr, sizeof(uint32_t*));
        _SNC_TMP_ADD(uint32_t, temp_rtn_addr, sizeof(uint32_t));

        if (addr_type == SENIS_OPER_TYPE_ADDRESS_DA) {
                senis_assign(b_ctx, _SNC_OP(da(&temp_addr[0])), _SNC_OP(addr));
                applied_ind_addr = &temp_addr[0];
        } else {
                applied_ind_addr = (uint32_t**)addr;
        }

        if (cmp_addr_type == SENIS_OPER_TYPE_ADDRESS_DA) {
                senis_assign(b_ctx, _SNC_OP(da(&temp_cmp_addr[0])), _SNC_OP(cmp_addr));
                applied_cmp_ind_addr = &temp_cmp_addr[0];
        } else {
                applied_cmp_ind_addr = (uint32_t**)cmp_addr;
        }

        if (rtn_addr == NULL || _SNC_ADDR_IS_REG(rtn_addr)) {
                rtn_addr = &temp_rtn_addr[0];
                rtn_addr_type = SENIS_OPER_TYPE_ADDRESS_DA;
        }

        senis_call(b_ctx, SNC_UCODE_CTX(senis_compare_ucode), 2 * 4,
                _SNC_OP(applied_ind_addr), _SNC_OP(applied_cmp_ind_addr),
                len_type, len, rtn_addr_type + 1, rtn_addr);

        if (_SNC_ADDR_IS_REG(rtn_addr)) {
                SENIS_assign(da(rtn_addr), da(temp_rtn_addr));
        }

        _SNC_TMP_RMV(temp_rtn_addr);
        _SNC_TMP_RMV(temp_cmp_addr);
        _SNC_TMP_RMV(temp_addr);
}

SNC_FUNC_DEF(senis_compare_ucode, uint32_t** ind_addr, uint32_t** cmp_ind_addr, uint32_t len,
        uint32_t* rtn_addr)
{
        SENIS_labels(while_cmp_start, check_cmp_len, memcmp_return_false, memcmp_return_true);

        _SNC_TMP_ADD(uint32_t*, temp_addr, sizeof(uint32_t*));
        _SNC_TMP_ADD(uint32_t*, temp_cmp_addr, sizeof(uint32_t*));

        SENIS_assign(da(&temp_addr[0]), ia(&SNC_ARG(ind_addr)));
        SENIS_assign(da(&temp_cmp_addr[0]), ia(&SNC_ARG(cmp_ind_addr)));

        SENIS_tobre(da(&SNC_ARG(len)), 0xFFFFFFFF);
        SENIS_inc1(da(&SNC_ARG(len)));

        SENIS_goto(l(check_cmp_len));

        SENIS_label(while_cmp_start);

        SENIS_rdcgr(da(&temp_addr[0]), da(&temp_cmp_addr[0]));
        SENIS_cobr_gr(l(memcmp_return_false));
        SENIS_inc4(da(&temp_addr[0]));
        SENIS_inc4(da(&temp_cmp_addr[0]));
        SENIS_inc1(da(&SNC_ARG(len)));

        SENIS_label(check_cmp_len);

        SENIS_rdcbi(da(&SNC_ARG(len)), 31);
        SENIS_cobr_eq(l(while_cmp_start));
        SENIS_goto(l(memcmp_return_true));

        SENIS_label(memcmp_return_false);

        SENIS_assign(ia(&SNC_ARG(ind_addr)), da(&temp_addr[0]));
        SENIS_assign(ia(&SNC_ARG(cmp_ind_addr)), da(&temp_cmp_addr[0]));
        SENIS_wadva(ia(&SNC_ARG(rtn_addr)), 1);
        SENIS_return;

        SENIS_label(memcmp_return_true);

        SENIS_assign(ia(&SNC_ARG(ind_addr)), da(&temp_addr[0]));
        SENIS_assign(ia(&SNC_ARG(cmp_ind_addr)), da(&temp_cmp_addr[0]));
        SENIS_wadva(ia(&SNC_ARG(rtn_addr)), 0);
        SENIS_return;

        _SNC_TMP_RMV(temp_cmp_addr);
        _SNC_TMP_RMV(temp_addr);
}

#endif /* dg_configUSE_HW_SENSOR_NODE */


/**
 * \}
 * \}
 */
