/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SNC_DEFS
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file snc_defs_macros.h
 *
 * @brief SNC definitions macros
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SNC_DEFS_MACROS_H_
#define SNC_DEFS_MACROS_H_


#if dg_configUSE_HW_SENSOR_NODE

/*
 * FUNCTION DECLARATIONS
 *****************************************************************************************
 */

void snc_min(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                             SENIS_OPER_TYPE op2_type, uint32_t* op2,
                             SENIS_OPER_TYPE min_addr_type, uint32_t* min_addr);
#define _SNC_MIN(op1, op2, min)                                                                 \
        SNC_STEP_BY_STEP();                                                                     \
        snc_min(b_ctx, _SNC_OP(op1), _SNC_OP(op2), _SNC_OP_ADDRESS(min));

void snc_max(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
                             SENIS_OPER_TYPE op2_type, uint32_t* op2,
                             SENIS_OPER_TYPE max_addr_type, uint32_t* max_addr);
#define _SNC_MAX(op1, op2, max)                                                                 \
        SNC_STEP_BY_STEP();                                                                     \
        snc_max(b_ctx, _SNC_OP(op1), _SNC_OP(op2), _SNC_OP_ADDRESS(max));

void snc_get_field(b_ctx_t* b_ctx, uint32_t field_msk, uint32_t field_pos,
        SENIS_OPER_TYPE val_addr_type, uint32_t* val_addr,
        SENIS_OPER_TYPE field_val_addr_type, uint32_t* field_val_addr);
#define _SNC_GET_FIELD(field_msk, field_pos, val_addr, field_val)                               \
        SNC_STEP_BY_STEP();                                                                     \
        snc_get_field(b_ctx, field_msk, field_pos, _SNC_OP(val_addr), _SNC_OP_ADDRESS(field_val));

void snc_clr_field(b_ctx_t* b_ctx, uint32_t field_msk, uint32_t field_pos,
        SENIS_OPER_TYPE addr_type, uint32_t* addr);
#define _SNC_CLR_FIELD(field_msk, field_pos, addr)                                              \
        SNC_STEP_BY_STEP();                                                                     \
        snc_clr_field(b_ctx, field_msk, field_pos, _SNC_OP_ADDRESS(addr));

void snc_set_field(b_ctx_t* b_ctx, uint32_t field_msk, uint32_t field_pos,
        SENIS_OPER_TYPE addr_type, uint32_t* addr,
        SENIS_OPER_TYPE field_val_type, uint32_t* field_val);
#define _SNC_SET_FIELD(field_msk, field_pos, addr, field_val)                                   \
        SNC_STEP_BY_STEP();                                                                     \
        snc_set_field(b_ctx, field_msk, field_pos, _SNC_OP_ADDRESS(addr), _SNC_OP(field_val));

void snc_clr_bit(b_ctx_t* b_ctx, SENIS_OPER_TYPE addr_type, uint32_t* addr, uint32_t bit_pos);
#define _SNC_CLR_BIT(addr, bit_pos)                                                             \
        SNC_STEP_BY_STEP();                                                                     \
        snc_clr_bit(b_ctx, _SNC_OP_ADDRESS(addr), _SNC_OP_VALUE(uint32_t, bit_pos));

void snc_set_bit(b_ctx_t* b_ctx, SENIS_OPER_TYPE addr_type, uint32_t* addr, uint32_t bit_pos);
#define _SNC_SET_BIT(addr, bit_pos)                                                             \
        SNC_STEP_BY_STEP();                                                                     \
        snc_set_bit(b_ctx, _SNC_OP_ADDRESS(addr), _SNC_OP_VALUE(uint32_t, bit_pos));

#endif /* dg_configUSE_HW_SENSOR_NODE */


#endif /* SNC_DEFS_MACROS_H_ */

/**
 * \}
 * \}
 */
