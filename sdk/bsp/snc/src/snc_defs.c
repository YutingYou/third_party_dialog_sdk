/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SNC_DEFS
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file snc_defs.c
 *
 * @brief SNC-Implementation of Sensor Node Controller definitions
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#if dg_configUSE_HW_SENSOR_NODE

#include "snc_defs.h"

/*
 * DATA TYPE DEFINITIONS
 *****************************************************************************************
 */

/*
 * DATA STRUCTURE DEFINITIONS
 *****************************************************************************************
 */

/*
 * FUNCTION DEFINITIONS
 *****************************************************************************************
 */

static void snc_min_max(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
        SENIS_OPER_TYPE op2_type, uint32_t* op2,
        SENIS_OPER_TYPE min_max_addr_type, uint32_t* min_max_addr,
        bool is_max)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(op1_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(op2_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(min_max_addr_type < SENIS_OPER_TYPE_VALUE);

        if (op1_type == SENIS_OPER_TYPE_VALUE && op2_type == SENIS_OPER_TYPE_VALUE) {
                uint32_t op1_val = (uint32_t)op1;
                uint32_t op2_val = (uint32_t)op2;

                if (is_max) {
                        senis_assign(b_ctx, min_max_addr_type, min_max_addr,
                                _SNC_OP((op1_val > op2_val) ? op1_val : op2_val));
                } else {
                        senis_assign(b_ctx, min_max_addr_type, min_max_addr,
                                _SNC_OP((op1_val < op2_val) ? op1_val : op2_val));
                }

                return;
        }

        uint32_t* clause_op1 = senis_add_clause_op(b_ctx, op1_type, op1);
        uint32_t* clause_op2 = senis_add_clause_op(b_ctx, op2_type, op2);

        _SNC_TMP_ADD(uint32_t, temp_min_max, sizeof(uint32_t));

        SENIS_labels(min_max_clause_keep_cur_min_max);

        senis_assign(b_ctx, _SNC_OP(da(temp_min_max)), op2_type, op2);

        if (is_max) {
                senis_rdcgr(b_ctx, clause_op2, clause_op1);
        } else {
                senis_rdcgr(b_ctx, clause_op1, clause_op2);
        }

        senis_cobr_gr(b_ctx, _SNC_OP(l(min_max_clause_keep_cur_min_max)));

        senis_assign(b_ctx, _SNC_OP(da(temp_min_max)), op1_type, op1);

        SENIS_label(min_max_clause_keep_cur_min_max);

        senis_assign(b_ctx, min_max_addr_type, min_max_addr, _SNC_OP(da(temp_min_max)));

        senis_rmv_clause_op(b_ctx, op2_type, clause_op2);
        senis_rmv_clause_op(b_ctx, op1_type, clause_op1);

        _SNC_TMP_RMV(temp_min_max);
}

void snc_min(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
        SENIS_OPER_TYPE op2_type, uint32_t* op2,
        SENIS_OPER_TYPE min_addr_type, uint32_t* min_addr)
{
        snc_min_max(b_ctx, op1_type, op1, op2_type, op2, min_addr_type, min_addr, false);
}

void snc_max(b_ctx_t* b_ctx, SENIS_OPER_TYPE op1_type, uint32_t* op1,
        SENIS_OPER_TYPE op2_type, uint32_t* op2,
        SENIS_OPER_TYPE max_addr_type, uint32_t* max_addr)
{
        snc_min_max(b_ctx, op1_type, op1, op2_type, op2, max_addr_type, max_addr, true);
}

SNC_FUNC_DECL(snc_get_field_ucode, uint32_t field_msk, uint32_t var_val,
        uint32_t* field_val_addr, uint32_t* get_field_src_TOBRE_value_addr);

void snc_get_field(b_ctx_t* b_ctx, uint32_t field_msk, uint32_t field_pos,
        SENIS_OPER_TYPE val_addr_type, uint32_t* val_addr,
        SENIS_OPER_TYPE field_val_addr_type, uint32_t* field_val_addr)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(val_addr_type <= SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(field_val_addr_type < SENIS_OPER_TYPE_VALUE);

        if ((field_msk >> field_pos) == 0 || field_pos > 31) {
                senis_assign(b_ctx, field_val_addr_type, field_val_addr, _SNC_OP(0));
                return;
        }

        if (_SNC_ADDR_IS_REG(field_val_addr)) {
                _SNC_TMP_ADD(uint32_t, p_tmp_reg_val, sizeof(uint32_t));

                SENIS_assign(da(p_tmp_reg_val), da(field_val_addr));

                senis_call(b_ctx, SNC_UCODE_CTX(snc_get_field_ucode), 2 * 4,
                                  _SNC_OP(field_msk),
                                  val_addr_type, val_addr, SENIS_OPER_TYPE_VALUE, p_tmp_reg_val,
                                  _SNC_OP(&senis_TOBRE_bit_mask_array[field_pos]));

                SENIS_assign(da(field_val_addr), da(p_tmp_reg_val));

                _SNC_TMP_RMV(p_tmp_reg_val);
        } else {
                senis_call(b_ctx, SNC_UCODE_CTX(snc_get_field_ucode), 2 * 4,
                                  _SNC_OP(field_msk),
                                  val_addr_type, val_addr, field_val_addr_type + 1, field_val_addr,
                                  _SNC_OP(&senis_TOBRE_bit_mask_array[field_pos]));
        }
}

SNC_FUNC_DEF(snc_get_field_ucode, uint32_t field_msk, uint32_t var_val,
        uint32_t* field_val_addr, uint32_t* get_field_src_TOBRE_value_addr)
{
        SENIS_labels(
                get_field_blk_begin, get_field_blk_cond,
                get_field_ph_tobre_field_msk_bit,
                get_field_ph_tobre_var_bit,
                get_field_ph_tobre_field_val_bit,
                get_field_pt_keep_0_in_the_current_bit
        );

        _SNC_TMP_ADD(uint32_t, temp_field_val, sizeof(uint32_t));
        _SNC_TMP_ADD(uint32_t*, get_field_dst_TOBRE_value_addr, sizeof(uint32_t*));
        _SNC_TMP_ADD(uint32_t, temp_val, sizeof(uint32_t));

        SENIS_assign(da(temp_field_val), 0);
        SENIS_assign(da(get_field_dst_TOBRE_value_addr), &senis_TOBRE_bit_mask_array[0]);

        SENIS_goto(l(get_field_blk_cond));

        SENIS_label(get_field_blk_begin);

        SENIS_assign(da(temp_val), da(&SNC_ARG(field_msk)));
        SENIS_assign(l(get_field_ph_tobre_field_msk_bit) + 1,
                ia(&SNC_ARG(get_field_src_TOBRE_value_addr)));
        SENIS_label(get_field_ph_tobre_field_msk_bit);
        senis_tobre(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(temp_val)), SNC_PLACE_HOLDER);
        SENIS_rdcgr(da(temp_val), da(&SNC_ARG(field_msk)));
        SENIS_cobr_gr(l(get_field_pt_keep_0_in_the_current_bit));

        SENIS_assign(da(&SNC_ARG(field_msk)), da(temp_val));

        SENIS_assign(da(temp_val), da(&SNC_ARG(var_val)));
        SENIS_assign(l(get_field_ph_tobre_var_bit) + 1, ia(&SNC_ARG(get_field_src_TOBRE_value_addr)));
        SENIS_label(get_field_ph_tobre_var_bit);
        senis_tobre(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(temp_val)), SNC_PLACE_HOLDER);
        SENIS_rdcgr(da(temp_val), da(&SNC_ARG(var_val)));
        SENIS_cobr_gr(l(get_field_pt_keep_0_in_the_current_bit));

        SENIS_assign(l(get_field_ph_tobre_field_val_bit) + 1, ia(get_field_dst_TOBRE_value_addr));
        SENIS_label(get_field_ph_tobre_field_val_bit);
        senis_tobre(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(temp_field_val)), SNC_PLACE_HOLDER);

        SENIS_label(get_field_pt_keep_0_in_the_current_bit);

        SENIS_inc4(da(&SNC_ARG(get_field_src_TOBRE_value_addr)));
        SENIS_inc4(da(get_field_dst_TOBRE_value_addr));

        SENIS_label(get_field_blk_cond);

        SENIS_rdcgr_z(da(&SNC_ARG(field_msk)));
        SENIS_cobr_gr(l(get_field_blk_begin));

        SENIS_assign(ia(&SNC_ARG(field_val_addr)), da(temp_field_val));

        _SNC_TMP_RMV(temp_val);
        _SNC_TMP_RMV(get_field_dst_TOBRE_value_addr);
        _SNC_TMP_RMV(temp_field_val);
}

SNC_FUNC_DECL(snc_clr_field_ucode, uint32_t field_msk, uint32_t* var_addr,
        uint32_t* clr_field_msk_TOBRE_value_addr);

void snc_clr_field(b_ctx_t* b_ctx, uint32_t field_msk, uint32_t field_pos,
        SENIS_OPER_TYPE addr_type, uint32_t* addr)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(addr_type < SENIS_OPER_TYPE_VALUE);

        if ((field_msk >> field_pos) == 0 || field_pos > 31) {
                return;
        }

        if (_SNC_ADDR_IS_REG(addr)) {
                _SNC_TMP_ADD(uint32_t, p_tmp_reg_val, sizeof(uint32_t));

                SENIS_assign(da(p_tmp_reg_val), da(addr));

                senis_call(b_ctx, SNC_UCODE_CTX(snc_clr_field_ucode), 2 * 3,
                                  _SNC_OP(field_msk),
                                  SENIS_OPER_TYPE_VALUE, p_tmp_reg_val,
                                  _SNC_OP(&senis_TOBRE_bit_mask_array[field_pos]));

                SENIS_assign(da(addr), da(p_tmp_reg_val));

                _SNC_TMP_RMV(p_tmp_reg_val);
        } else {
                senis_call(b_ctx, SNC_UCODE_CTX(snc_clr_field_ucode), 2 * 3,
                                  _SNC_OP(field_msk),
                                  addr_type + 1, addr,
                                  _SNC_OP(&senis_TOBRE_bit_mask_array[field_pos]));
        }
}

SNC_FUNC_DEF(snc_clr_field_ucode, uint32_t field_msk, uint32_t* var_addr,
        uint32_t* clr_field_msk_TOBRE_value_addr)
{
        SENIS_labels(
                clr_field_blk_begin, clr_field_blk_cond,
                clr_field_ph_tobre_field_msk_bit,
                clr_field_ph_tobre_var_bit,
                clr_field_pt_keep_value_in_the_current_bit
        );

        _SNC_TMP_ADD(uint32_t, temp_var_val, sizeof(uint32_t));
        _SNC_TMP_ADD(uint32_t, temp_val, sizeof(uint32_t));

        SENIS_assign(da(temp_var_val), ia(&SNC_ARG(var_addr)));

        SENIS_goto(l(clr_field_blk_cond));

        SENIS_label(clr_field_blk_begin);

        SENIS_assign(da(temp_val), da(&SNC_ARG(field_msk)));
        SENIS_assign(l(clr_field_ph_tobre_field_msk_bit) + 1,
                ia(&SNC_ARG(clr_field_msk_TOBRE_value_addr)));
        SENIS_label(clr_field_ph_tobre_field_msk_bit);
        senis_tobre(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(temp_val)), SNC_PLACE_HOLDER);
        SENIS_rdcgr(da(temp_val), da(&SNC_ARG(field_msk)));
        SENIS_cobr_gr(l(clr_field_pt_keep_value_in_the_current_bit));

        SENIS_assign(da(&SNC_ARG(field_msk)), da(temp_val));

        SENIS_assign(da(temp_val), da(temp_var_val));
        SENIS_assign(l(clr_field_ph_tobre_var_bit) + 1, ia(&SNC_ARG(clr_field_msk_TOBRE_value_addr)));
        SENIS_label(clr_field_ph_tobre_var_bit);
        senis_tobre(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(temp_val)), SNC_PLACE_HOLDER);
        SENIS_rdcgr(da(temp_val), da(temp_var_val));
        SENIS_cobr_gr(l(clr_field_pt_keep_value_in_the_current_bit));

        SENIS_assign(da(temp_var_val), da(temp_val));

        SENIS_label(clr_field_pt_keep_value_in_the_current_bit);

        SENIS_inc4(da(&SNC_ARG(clr_field_msk_TOBRE_value_addr)));

        SENIS_label(clr_field_blk_cond);

        SENIS_rdcgr_z(da(&SNC_ARG(field_msk)));
        SENIS_cobr_gr(l(clr_field_blk_begin));

        SENIS_assign(ia(&SNC_ARG(var_addr)), da(temp_var_val));

        _SNC_TMP_RMV(temp_val);
        _SNC_TMP_RMV(temp_var_val);
}

SNC_FUNC_DECL(snc_set_field_ucode, uint32_t field_msk, uint32_t* var_addr,
        uint32_t field_val, uint32_t* set_field_var_TOBRE_value_addr);

void snc_set_field(b_ctx_t* b_ctx, uint32_t field_msk, uint32_t field_pos,
        SENIS_OPER_TYPE addr_type, uint32_t* addr,
        SENIS_OPER_TYPE field_val_type, uint32_t* field_val)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(addr_type < SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(field_val_type <= SENIS_OPER_TYPE_VALUE);

        if ((field_msk >> field_pos) == 0 || field_pos > 31) {
                return;
        }

        if (field_val_type == SENIS_OPER_TYPE_VALUE) {
                snc_clr_field(b_ctx, field_msk, field_pos, addr_type, addr);
                senis_xor(b_ctx, addr_type, addr,
                        _SNC_OP((((uint32_t)field_val) << field_pos) & field_msk));
        } else {
                if (_SNC_ADDR_IS_REG(addr)) {
                        _SNC_TMP_ADD(uint32_t, p_tmp_reg_val, sizeof(uint32_t));

                        SENIS_assign(da(p_tmp_reg_val), da(addr));

                        senis_call(b_ctx, SNC_UCODE_CTX(snc_set_field_ucode), 2 * 4,
                                          _SNC_OP(field_msk),
                                          SENIS_OPER_TYPE_VALUE, p_tmp_reg_val,
                                          field_val_type, field_val,
                                          _SNC_OP(&senis_TOBRE_bit_mask_array[field_pos]));

                        SENIS_assign(da(addr), da(p_tmp_reg_val));

                        _SNC_TMP_RMV(p_tmp_reg_val);
                } else {
                        senis_call(b_ctx, SNC_UCODE_CTX(snc_set_field_ucode), 2 * 4,
                                          _SNC_OP(field_msk),
                                          addr_type + 1, addr,
                                          field_val_type, field_val,
                                          _SNC_OP(&senis_TOBRE_bit_mask_array[field_pos]));
                }
        }
}

SNC_FUNC_DEF(snc_set_field_ucode, uint32_t field_msk, uint32_t* var_addr, uint32_t field_val,
        uint32_t* set_field_var_TOBRE_value_addr)
{
        SENIS_labels(
                clr_field_blk_begin, clr_field_blk_cond,
                clr_field_ph_tobre_field_msk_bit,
                clr_field_ph_tobre_var_bit,
                clr_field_ph_tobre_field_val_bit,
                clr_field_pt_keep_value_in_the_current_bit,
                clr_field_ph_set_var_bit,
                clr_field_pt_keep_0_in_the_current_bit
        );

        _SNC_TMP_ADD(uint32_t, temp_var_val, sizeof(uint32_t));
        _SNC_TMP_ADD(uint32_t*, set_field_val_TOBRE_value_addr, sizeof(uint32_t*));
        _SNC_TMP_ADD(uint32_t, temp_val, sizeof(uint32_t));

        SENIS_assign(da(temp_var_val), ia(&SNC_ARG(var_addr)));
        SENIS_assign(da(set_field_val_TOBRE_value_addr), &senis_TOBRE_bit_mask_array[0]);

        SENIS_goto(l(clr_field_blk_cond));

        SENIS_label(clr_field_blk_begin);

        SENIS_assign(da(temp_val), da(&SNC_ARG(field_msk)));
        SENIS_assign(l(clr_field_ph_tobre_field_msk_bit) + 1,
                ia(&SNC_ARG(set_field_var_TOBRE_value_addr)));
        SENIS_label(clr_field_ph_tobre_field_msk_bit);
        senis_tobre(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(temp_val)), SNC_PLACE_HOLDER);
        SENIS_rdcgr(da(temp_val), da(&SNC_ARG(field_msk)));
        SENIS_cobr_gr(l(clr_field_pt_keep_value_in_the_current_bit));

        SENIS_assign(da(&SNC_ARG(field_msk)), da(temp_val));

        SENIS_assign(da(temp_val), da(temp_var_val));
        SENIS_assign(l(clr_field_ph_tobre_var_bit) + 1, ia(&SNC_ARG(set_field_var_TOBRE_value_addr)));
        SENIS_label(clr_field_ph_tobre_var_bit);
        senis_tobre(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(temp_val)), SNC_PLACE_HOLDER);
        SENIS_rdcgr(da(temp_val), da(temp_var_val));
        SENIS_cobr_gr(l(clr_field_pt_keep_0_in_the_current_bit));

        SENIS_assign(da(temp_var_val), da(temp_val));

        SENIS_label(clr_field_pt_keep_0_in_the_current_bit);

        SENIS_assign(da(temp_val), da(&SNC_ARG(field_val)));
        SENIS_assign(l(clr_field_ph_tobre_field_val_bit) + 1, ia(set_field_val_TOBRE_value_addr));
        SENIS_label(clr_field_ph_tobre_field_val_bit);
        senis_tobre(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(temp_val)), SNC_PLACE_HOLDER);
        SENIS_rdcgr(da(temp_val), da(&SNC_ARG(field_val)));
        SENIS_cobr_gr(l(clr_field_pt_keep_value_in_the_current_bit));

        SENIS_assign(l(clr_field_ph_set_var_bit) + 1, ia(&SNC_ARG(set_field_var_TOBRE_value_addr)));
        SENIS_label(clr_field_ph_set_var_bit);
        senis_tobre(b_ctx, _SNC_OP_DIRECT_ADDRESS(da(temp_var_val)), SNC_PLACE_HOLDER);

        SENIS_label(clr_field_pt_keep_value_in_the_current_bit);

        SENIS_inc4(da(&SNC_ARG(set_field_var_TOBRE_value_addr)));
        SENIS_inc4(da(set_field_val_TOBRE_value_addr));

        SENIS_label(clr_field_blk_cond);

        SENIS_rdcgr_z(da(&SNC_ARG(field_msk)));
        SENIS_cobr_gr(l(clr_field_blk_begin));

        SENIS_assign(ia(&SNC_ARG(var_addr)), da(temp_var_val));

        _SNC_TMP_RMV(temp_val);
        _SNC_TMP_RMV(set_field_val_TOBRE_value_addr);
        _SNC_TMP_RMV(temp_var_val);
}

void snc_clr_bit(b_ctx_t* b_ctx, SENIS_OPER_TYPE addr_type, uint32_t* addr, uint32_t bit_pos)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(addr_type < SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(bit_pos < 32);

        uint32_t* clause_addr = senis_add_clause_op(b_ctx, addr_type, addr);

        SENIS_labels(clr_bit_clause_keep_bit_value, clr_bit_clause_tobre_bit_value);

        senis_rdcbi(b_ctx, clause_addr, bit_pos);
        senis_cobr_eq(b_ctx, _SNC_OP(l(clr_bit_clause_tobre_bit_value)));
        senis_goto(b_ctx, _SNC_OP(l(clr_bit_clause_keep_bit_value)));

        SENIS_label(clr_bit_clause_tobre_bit_value);

        senis_tobre(b_ctx, clause_addr, (1 << bit_pos));

        if (addr_type == SENIS_OPER_TYPE_ADDRESS_IA) {
                senis_assign(b_ctx, _SNC_OP(ia(addr)), _SNC_OP(da(clause_addr)));
        }

        SENIS_label(clr_bit_clause_keep_bit_value);

        senis_rmv_clause_op(b_ctx, addr_type, clause_addr);
}

void snc_set_bit(b_ctx_t* b_ctx, SENIS_OPER_TYPE addr_type, uint32_t* addr, uint32_t bit_pos)
{
        ASSERT_WARNING(b_ctx);
        ASSERT_WARNING(addr_type < SENIS_OPER_TYPE_VALUE);
        ASSERT_WARNING(bit_pos < 32);

        uint32_t* clause_addr = senis_add_clause_op(b_ctx, addr_type, addr);

        SENIS_labels(set_bit_clause_keep_bit_value);

        senis_rdcbi(b_ctx, clause_addr, bit_pos);
        senis_cobr_eq(b_ctx, _SNC_OP(l(set_bit_clause_keep_bit_value)));

        senis_tobre(b_ctx, clause_addr, (1 << bit_pos));

        if (addr_type == SENIS_OPER_TYPE_ADDRESS_IA) {
                senis_assign(b_ctx, _SNC_OP(ia(addr)), _SNC_OP(da(clause_addr)));
        }

        SENIS_label(set_bit_clause_keep_bit_value);

        senis_rmv_clause_op(b_ctx, addr_type, clause_addr);
}

#endif /* dg_configUSE_HW_SENSOR_NODE */


/**
 * \}
 * \}
 */

