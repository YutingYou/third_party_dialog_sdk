/**
 * \addtogroup BSP
 * \{
 * \addtogroup INTERFACES
 * \{
 * \addtogroup SECURITY_TOOLBOX
 * \{
 * \addtogroup CURVES
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file crypto_ecc_provider_functions.c
 *
 * @brief ecc functions implementing primitives for all provided curves and libraries.
 *
 * Copyright (C) 2018-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <string.h>
#include "crypto_ecc_provider_functions.h"
#include "sys_trng.h"

#if dg_USE_HW_ECC

#define CRYPTO_ECDH_ENABLE_CALCR2       (1 << ECC_ECC_COMMAND_REG_ECC_CalcR2_Pos)
#define CRYPTO_ECDH_ENABLE_SIGNB        (1 << ECC_ECC_COMMAND_REG_ECC_SignB_Pos)

CRYPTO_ECC_PROVIDER_RET compute_public_key_25519_hw(crypto_ec_params_t *curve, uint8_t *d, uint8_t *Q)
{
        uint32_t cmd;
        unsigned int ecc_status = 0;
        volatile uint8_t *base_addr;
        volatile uint8_t *din;

        base_addr = ad_crypto_get_ecc_base_addr();
        din = (uint8_t *)hw_ecc_get_location_address(4, base_addr);
        cmd = curve->cmd | CRYPTO_ECDH_ENABLE_CALCR2 | HW_ECC_CMD_OP_C25519_PNT_MULT;

        hw_ecc_write256(0, curve->q, base_addr);
        hw_ecc_write256(2, curve->Gx, base_addr);
        hw_ecc_write256(3, curve->a, base_addr);
        hw_ecc_write256(4, (const uint8_t *)d, base_addr);
        din[0] &= 248;
        din[31] &= 127;
        din[31] |= 64;
        hw_ecc_cfg_ops(2, 4, 6);
        hw_ecc_write_command_register_raw(cmd);
        hw_ecc_start();

        ad_crypto_wait_ecc_event(OS_EVENT_FOREVER, &ecc_status);

        if (ecc_status != 0) {
                return CRYPTO_ECC_PROVIDER_RET_ERROR;
        }

        hw_ecc_read256(6, Q, base_addr);

        return CRYPTO_ECC_PROVIDER_RET_OK;
}

CRYPTO_ECC_PROVIDER_RET compute_private_key_hw(crypto_ec_params_t *curve, uint8_t *d)
{
        uint32_t cmd = curve->cmd | CRYPTO_ECDH_ENABLE_SIGNB | HW_ECC_CMD_OP_CHECK_PXY;
        volatile uint8_t *base_addr = ad_crypto_get_ecc_base_addr();
        unsigned int ecc_status = 0;

        /* Curve operand size cannot be larger than 32 bytes (256-bits) */
        ASSERT_ERROR(curve->o_sz <= 32);

        memset(d, 0, 32 - curve->o_sz); /* make sure it is properly zero-padded */
        hw_ecc_write256_r(1, curve->n, base_addr);
        hw_ecc_write_command_register_raw(cmd);
        hw_ecc_cfg_ops(6, 0, 0);
        /* Loop until we get a number smaller than the cyclic subgroup order n */
        do {
                sys_trng_get_bytes(&d[32 - curve->o_sz], curve->o_sz);
                d[31] |= 1; /* Avoid the extremely unlikely case of d = 0 */
                hw_ecc_write256_r(6, d, base_addr);
                hw_ecc_start();

                ad_crypto_wait_ecc_event(OS_EVENT_FOREVER, &ecc_status);
        } while (ecc_status & HW_ECC_STATUS_COUPLE_NOT_VALID);

        return CRYPTO_ECC_PROVIDER_RET_OK;
}

CRYPTO_ECC_PROVIDER_RET compute_shared_secret_25519_hw(crypto_ec_params_t *curve, uint8_t *d, uint8_t *Qp, uint8_t *s)
{
        uint32_t cmd;
        unsigned int ecc_status = 0;
        volatile uint8_t *base_addr = ad_crypto_get_ecc_base_addr();

        cmd = curve->cmd | HW_ECC_CMD_OP_C25519_PNT_MULT;

        hw_ecc_write256(2, Qp, base_addr);
//        if (full_setup_needed) {
//                hw_ecc_write256(0, curve->q, base_addr);
//                hw_ecc_write256(3, curve->a, base_addr);
//                hw_ecc_write256(4, d, base_addr);
//                cmd |= CRYPTO_ECDH_ENABLE_CALCR2;
//        }

        hw_ecc_cfg_ops(2, 4, 8);
        hw_ecc_write_command_register_raw(cmd);
        hw_ecc_start();

        ad_crypto_wait_ecc_event(OS_EVENT_FOREVER, &ecc_status);

        if (ecc_status != 0) {
                return CRYPTO_ECC_PROVIDER_RET_ERROR;
        }

        hw_ecc_read256(8, s, base_addr);
        return CRYPTO_ECC_PROVIDER_RET_OK;

}

CRYPTO_ECC_PROVIDER_RET compute_public_key_hw(crypto_ec_params_t *curve, uint8_t *d, uint8_t *Q) {

        uint32_t cmd;
        unsigned int ecc_status = 0;
        volatile uint8_t *base_addr = ad_crypto_get_ecc_base_addr();

        cmd = curve->cmd | CRYPTO_ECDH_ENABLE_CALCR2 | HW_ECC_CMD_OP_POINT_MLT;

        hw_ecc_write256_r(0, curve->q, base_addr);
        hw_ecc_write256_r(2, curve->Gx, base_addr);
        hw_ecc_write256_r(3, curve->Gy, base_addr);
        hw_ecc_write256_r(4, curve->a, base_addr);
        hw_ecc_write256_r(5, curve->b, base_addr);
//      if (full_setup_needed) {
        hw_ecc_write256_r(6, d, base_addr);
//      }
        hw_ecc_cfg_ops(2, 6, 8);
        hw_ecc_write_command_register_raw(cmd);
        hw_ecc_start();

        ad_crypto_wait_ecc_event(OS_EVENT_FOREVER, &ecc_status);

        if (ecc_status != 0) {
                return CRYPTO_ECC_PROVIDER_RET_ERROR;
        }

        hw_ecc_read256_r(8, Q, base_addr);
        hw_ecc_read256_r(9, Q + 32, base_addr);

        return CRYPTO_ECC_PROVIDER_RET_OK;

}

CRYPTO_ECC_PROVIDER_RET compute_shared_secret_hw(crypto_ec_params_t *curve, uint8_t *d, uint8_t *Qp, uint8_t *s){

        uint32_t cmd;
        unsigned int ecc_status = 0;
        volatile uint8_t *base_addr = ad_crypto_get_ecc_base_addr();

        cmd = curve->cmd | HW_ECC_CMD_OP_POINT_MLT;
        hw_ecc_write256_r(10, Qp, base_addr);
        hw_ecc_write256_r(11, (Qp + 32), base_addr);
//        if (full_setup_needed) {
                hw_ecc_write256_r(0, curve->q, base_addr);
                hw_ecc_write256_r(4, curve->a, base_addr);
                hw_ecc_write256_r(5, curve->b, base_addr);
                hw_ecc_write256_r(6, d, base_addr);
                cmd |= CRYPTO_ECDH_ENABLE_CALCR2;
//        }
        hw_ecc_cfg_ops(10, 6, 12);
        hw_ecc_write_command_register_raw(cmd);
        hw_ecc_start();

        ad_crypto_wait_ecc_event(OS_EVENT_FOREVER, &ecc_status);

        if (ecc_status != 0) {
                return CRYPTO_ECC_PROVIDER_RET_ERROR;
        }

        hw_ecc_read256_r(12, s, base_addr);

        return CRYPTO_ECC_PROVIDER_RET_OK;

}
#endif

#if dg_USE_SODIUM_LIB || dg_USE_HW_ECC
CRYPTO_ECC_PROVIDER_RET compute_private_key_25519(crypto_ec_params_t *curve, uint8_t *d)
{
        sys_trng_get_bytes(d, 32);
        return CRYPTO_ECC_PROVIDER_RET_OK;
}
#endif

#if dg_USE_SODIUM_LIB
CRYPTO_ECC_PROVIDER_RET compute_public_key_25519_sodium(crypto_ec_params_t *curve, uint8_t *d, uint8_t *Q)
{
        crypto_scalarmult_base(Q, d);
        return CRYPTO_ECC_PROVIDER_RET_OK;
}

CRYPTO_ECC_PROVIDER_RET compute_shared_secret_25519_sodium(crypto_ec_params_t *curve, uint8_t *d, uint8_t *Qp, uint8_t *s)
{
        int dummy __UNUSED;
        dummy = crypto_scalarmult(s, d, Qp);
        return CRYPTO_ECC_PROVIDER_RET_OK;
}
#endif

#if dg_USE_UECC_LIB
CRYPTO_ECC_PROVIDER_RET compute_private_key_uecc(crypto_ec_params_t *curve, uint8_t *d)
{
        /* uecc is not right aligned so we have to make sure everytime a call is made that
         * proper zero padding is applied and the correct index is provided.
         */
        memset( d, 0, 32);
        sys_trng_get_bytes(&d[32-curve->o_sz], curve->o_sz);
        return CRYPTO_ECC_PROVIDER_RET_OK;
}

CRYPTO_ECC_PROVIDER_RET compute_public_key_uecc(crypto_ec_params_t *curve, uint8_t *d, uint8_t *Q){

        /*uecc is not right aligned so we have to make sure every time a call is made that
         *proper zero padding is applied and the correct index is provided.
         */
        memset( Q, 0, 64);
        //create temporary key array to store key in the format uecc accepts (leftaligned)
        uint8_t Qtemp[64];
        uECC_compute_public_key(&d[32-curve->o_sz], &Qtemp[0], curve->crypto_uecc_curve);
        //save public key in the correct format.
        memcpy(&Q[32-curve->o_sz], &Qtemp[0], curve->o_sz);
        memcpy(&Q[64-curve->o_sz], &Qtemp[curve->o_sz], curve->o_sz);

        return CRYPTO_ECC_PROVIDER_RET_OK;

}


CRYPTO_ECC_PROVIDER_RET compute_shared_secret_uecc(crypto_ec_params_t *curve, uint8_t *d, uint8_t *Qp, uint8_t *s){

        memset( s, 0, 32);
        //create temporary key array to store key in the format uecc accepts (leftaligned)
        uint8_t Qtemp[64];
        //save public key in the correct uecc format - different than
        memcpy( &Qtemp[0], &Qp[32-curve->o_sz], curve->o_sz);
        memcpy( &Qtemp[curve->o_sz], &Qp[64-curve->o_sz], curve->o_sz);
        uECC_shared_secret(&Qtemp[0], &d[32-curve->o_sz], &s[32-curve->o_sz], curve->crypto_uecc_curve);

        return CRYPTO_ECC_PROVIDER_RET_OK;
}



__RETAINED static uint8_t uecc_lib_initialized_flag;//UECC initialization flag should be zero initialized.

int uecc_hw_trng(uint8_t *dest, unsigned size)
{
        sys_trng_get_bytes(dest, size);
        return 1;
}

static uint8_t uecc_lib_init(void)
{
        if (uecc_lib_initialized_flag) {
                return 1;
        } else {
                uECC_set_rng(uecc_hw_trng);
                uecc_lib_initialized_flag = 1;
                return 1;
        }
}
#endif

CRYPTO_ECC_PROVIDER_RET curve_init_secp160r1(crypto_ec_params_t *curve)
{
#if dg_USE_HW_ECC
        curve->q = NULL;
        curve->n = NULL;
        curve->Gx = NULL;
        curve->Gy = NULL;
        curve->a = NULL;
        curve->b = NULL;
        curve->cmd = _CRYPTO_EC_CMD(HW_ECC_CMD_SIGNB_POS, HW_ECC_CMD_SIGNA_POS, HW_ECC_CMD_OP_SIZE_256B, HW_ECC_CMD_FIELD_FP);
#endif
        curve->o_sz = 20;
#if dg_USE_UECC_LIB
        if (curve_lib_provider_index[curve->type] == UECC_LIB) {
                uecc_lib_init();
                curve->crypto_uecc_curve = uECC_secp160r1();
        }
#endif
        return CRYPTO_ECC_PROVIDER_RET_OK;
}

CRYPTO_ECC_PROVIDER_RET curve_init_secp192r1(crypto_ec_params_t *curve)
{
#if dg_USE_HW_ECC
        curve->q = hw_ecc_p192_q;
        curve->n = hw_ecc_p192_n;
        curve->Gx = hw_ecc_p192_Gx;
        curve->Gy = hw_ecc_p192_Gy;
        curve->a = hw_ecc_p192_a;
        curve->b = hw_ecc_p192_b;
        curve->cmd = _CRYPTO_EC_CMD(HW_ECC_CMD_SIGNB_POS, HW_ECC_CMD_SIGNA_POS, HW_ECC_CMD_OP_SIZE_256B, HW_ECC_CMD_FIELD_FP);
#endif
        curve->o_sz = 24;
#if dg_USE_UECC_LIB
        if (curve_lib_provider_index[curve->type] == UECC_LIB) {
                uecc_lib_init();
                curve->crypto_uecc_curve = uECC_secp192r1();
        }
#endif
        return CRYPTO_ECC_PROVIDER_RET_OK;
}

CRYPTO_ECC_PROVIDER_RET curve_init_secp224r1(crypto_ec_params_t *curve)
{
#if dg_USE_HW_ECC
        curve->q = hw_ecc_p224_q;
        curve->n = hw_ecc_p224_n;
        curve->Gx = hw_ecc_p224_Gx;
        curve->Gy = hw_ecc_p224_Gy;
        curve->a = hw_ecc_p224_a;
        curve->b = hw_ecc_p224_b;
        curve->cmd = _CRYPTO_EC_CMD(HW_ECC_CMD_SIGNB_POS, HW_ECC_CMD_SIGNA_POS, HW_ECC_CMD_OP_SIZE_256B, HW_ECC_CMD_FIELD_FP);
#endif
        curve->o_sz = 28;
#if dg_USE_UECC_LIB
        if (curve_lib_provider_index[curve->type] == UECC_LIB) {
                uecc_lib_init();
                curve->crypto_uecc_curve = uECC_secp224r1();
        }
#endif
        return CRYPTO_ECC_PROVIDER_RET_OK;
}

CRYPTO_ECC_PROVIDER_RET curve_init_secp256r1(crypto_ec_params_t *curve)
{
#if dg_USE_HW_ECC
        curve->q = hw_ecc_p256_q;
        curve->n = hw_ecc_p256_n;
        curve->Gx = hw_ecc_p256_Gx;
        curve->Gy = hw_ecc_p256_Gy;
        curve->a = hw_ecc_p256_a;
        curve->b = hw_ecc_p256_b;
        curve->cmd = _CRYPTO_EC_CMD(HW_ECC_CMD_SIGNB_POS, HW_ECC_CMD_SIGNA_POS, HW_ECC_CMD_OP_SIZE_256B, HW_ECC_CMD_FIELD_FP);
#endif
        curve->o_sz = 32;
#if dg_USE_UECC_LIB
        if (curve_lib_provider_index[curve->type] == UECC_LIB) {
                uecc_lib_init();
                curve->crypto_uecc_curve = uECC_secp256r1();
        }
#endif
        return CRYPTO_ECC_PROVIDER_RET_OK;
}

CRYPTO_ECC_PROVIDER_RET curve_init_secp256k1(crypto_ec_params_t *curve)
{
#if dg_USE_HW_ECC
        curve->q = NULL;
        curve->n = NULL;
        curve->Gx = NULL;
        curve->Gy = NULL;
        curve->a = NULL;
        curve->b = NULL;
        curve->cmd = _CRYPTO_EC_CMD(HW_ECC_CMD_SIGNB_POS, HW_ECC_CMD_SIGNA_POS, HW_ECC_CMD_OP_SIZE_256B, HW_ECC_CMD_FIELD_FP);
#endif
        curve->o_sz = 32;
#if dg_USE_UECC_LIB
        if (curve_lib_provider_index[curve->type] == UECC_LIB) {
                uecc_lib_init();
                curve->crypto_uecc_curve = uECC_secp256k1();
        }
#endif
        return CRYPTO_ECC_PROVIDER_RET_OK;
}

#if dg_USE_SODIUM_LIB
/*
 * Structure to point to sodium library in order to use hw_trng as rng source.
 */
struct randombytes_implementation sodium_hw_trng_implementation = {
    SODIUM_C99(.implementation_name =) NULL,
    SODIUM_C99(.random =) sys_trng_rand,
    SODIUM_C99(.stir =) sys_trng_stir,
    SODIUM_C99(.uniform =) NULL,
    SODIUM_C99(.buf =) sys_trng_get_bytes,
    SODIUM_C99(.close =) NULL
};
#endif

CRYPTO_ECC_PROVIDER_RET curve_init_25519(crypto_ec_params_t *curve)
{
#if dg_USE_SODIUM_LIB
        /*Initialize sodium library if is used*/
        if (curve_lib_provider_index[curve->type]==SODIUM_LIB) {
                /* Set hw_trng as sodium library rng source.*/
                randombytes_set_implementation(&sodium_hw_trng_implementation);
                if (sodium_init() < 0) {
                        /* panic! the library couldn't be initialized, it is not safe to use */
                        return CRYPTO_ECC_PROVIDER_RET_INIT_FAIL;
                }
        }
#endif
#if dg_USE_HW_ECC
        curve->q = hw_ecc_curve25519_p;
        curve->n = NULL;
        curve->Gx = hw_ecc_curve25519_G;
        curve->Gy = NULL;
        curve->a = hw_ecc_curve25519_a24;
        curve->b = NULL;
        curve->cmd = _CRYPTO_EC_CMD(HW_ECC_CMD_SIGNB_POS, HW_ECC_CMD_SIGNA_POS, HW_ECC_CMD_OP_SIZE_256B, HW_ECC_CMD_FIELD_FP);
#endif
        curve->o_sz = 32;
        return CRYPTO_ECC_PROVIDER_RET_OK;
}

CRYPTO_ECC_PROVIDER_RET ecc_lib_provider_error_handler()
{
        configASSERT(0);
        return CRYPTO_ECC_PROVIDER_RET_OK;
}

/**
 * \}
 * \}
 * \}
 * \}
 */
