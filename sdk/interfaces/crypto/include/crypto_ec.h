/**
 * \addtogroup SECURITY_TOOLBOX
 * \{
 * \addtogroup CURVES
 *
 * \brief Elliptic curves data.
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file crypto_ec.h
 *
 * @brief Elliptic curves data.
 *
 * Copyright (C) 2016-2018 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef CRYPTO_EC_H_
#define CRYPTO_EC_H_

#include <stdint.h>
#include "crypto_ecc_provider_params.h"
#include <hw_ecc_curves.h>
#include  "crypto_ecc_provider_function_map.h"
#if dg_USE_UECC_LIB
//This is needed because uECC curve datatype is part of crypto_uecc_curve structure.
#include "uECC.h"
#endif

/**
 * \brief Elliptic curve parameters.
 */
typedef struct {
#if dg_USE_HW_ECC
        const uint8_t *q;                     /**< Field size. */
        const uint8_t *n;                     /**< Subgroup order. */
        const uint8_t *Gx;                    /**< x coordinate of generator point. */
        const uint8_t *Gy;                    /**< y coordinate of generator point. */
        const uint8_t *a;                     /**< Parameter a of the curve. */
        const uint8_t *b;                     /**< Parameter b of the curve. */
        uint32_t cmd;                         /**< Command register for the curve. */
#endif
        uint8_t o_sz;                         /**< Operands size (bytes). */
        CRYPTO_ECC_CURVE type;                /**< Type of curve in use. */
#if dg_USE_UECC_LIB
        uECC_Curve crypto_uecc_curve;         /**< Curve object needed for uECC library operations. */
#endif
} crypto_ec_params_t;


#if dg_ALLOW_DYNAMIC_LIB_PROVIDER
/**
 * \brief Set which library provider will implement ecc functions for the specified curve.
 */
CRYPTO_ECC_PROVIDER_RET crypto_ecc_set_curve_lib_provider(CRYPTO_ECC_CURVE curve_type, ECC_CRYPTO_LIB_PROVIDER lib_provider_input);
#endif

/**
 * \brief Initialize ecc_curve.
 */
CRYPTO_ECC_PROVIDER_RET crypto_ecc_curve_init(crypto_ec_params_t *curve, CRYPTO_ECC_CURVE curve_type);

/**
 * \brief Compute public key ecc primitive.
 *
 * Calculates the public key given a private key. Depending on ecc library provider and curve settings,
 * the appropriate function call will be made from ecc_compute_public_jump_table.
 *
 * \param [in] curve    The curve parameter object for which the key will be generated
 * \param [in] d        Pointer to the private key.
 * \param [out] Q       Pointer to the public key to be generated.
 *
 * \returns ecc provider status.
 */
CRYPTO_ECC_PROVIDER_RET crypto_ecc_compute_public_key(crypto_ec_params_t *curve, uint8_t *d, uint8_t *Q);

/**
 * \brief Compute private key ecc primitive.
 *
 * Calculates the private key. Depending on ecc library provider and curve settings,
 * the appropriate function call will be made from ecc_compute_public_jump_table.
 *
 * \param [in] curve    The curve parameter object for which the key will be generated
 * \param [out] d       Pointer to the private key.
 *
 * \returns ecc provider status.
 */
CRYPTO_ECC_PROVIDER_RET crypto_ecc_compute_private_key(crypto_ec_params_t *curve, uint8_t *d);

/**
 * \brief Compute shared secret key ecc primitive.
 *
 * Calculates the public key given a private key. Depending on ecc library provider and curve settings,
 * the appropriate function call will be made from ecc_compute_public_jump_table.
 *
 * \param [in] curve    The curve parameter object for which the key will be generated
 * \param [in] d        Pointer to the private key.
 * \param [in] Qp       Pointer to the peers public key that will be used to create the shared key.
 * \param [out] s       Pointer to the generated shared key.
 *
 * \returns ecc provider status.
 */
CRYPTO_ECC_PROVIDER_RET crypto_ecc_compute_shared_secret(crypto_ec_params_t *curve, uint8_t *d, uint8_t *Qp, uint8_t *s);


#if dg_USE_HW_ECC
#define _CRYPTO_EC_CMD(_sb, _sa, _os, _f) \
        ((_sb << ECC_ECC_COMMAND_REG_ECC_SignB_Pos) | (_sa << ECC_ECC_COMMAND_REG_ECC_SignA_Pos) | \
         (_os << ECC_ECC_COMMAND_REG_ECC_SizeOfOperands_Pos) | (_f << ECC_ECC_COMMAND_REG_ECC_Field_Pos))

/**
 * \brief Parameter initialization for secp192r1 curve.
 *
 * This is NIST P-192, ANSI X9.62 prime192v1 curve.
 */
#define CRYPTO_EC_PARAMS_SECP192R1 \
        { hw_ecc_p192_q, hw_ecc_p192_n, hw_ecc_p192_Gx, hw_ecc_p192_Gy, hw_ecc_p192_a, hw_ecc_p192_b, \
          _CRYPTO_EC_CMD(HW_ECC_CMD_SIGNB_POS, HW_ECC_CMD_SIGNA_POS, HW_ECC_CMD_OP_SIZE_256B, HW_ECC_CMD_FIELD_FP), 24, ECC_CRYPTO_SECP192R1 }

/**
 * \brief Parameter initialization for secp224r1 curve.
 *
 * This is NIST P-224 curve.
 */
#define CRYPTO_EC_PARAMS_SECP224R1 \
        { hw_ecc_p224_q, hw_ecc_p224_n, hw_ecc_p224_Gx, hw_ecc_p224_Gy, hw_ecc_p224_a, hw_ecc_p224_b, \
          _CRYPTO_EC_CMD(HW_ECC_CMD_SIGNB_POS, HW_ECC_CMD_SIGNA_POS, HW_ECC_CMD_OP_SIZE_256B, HW_ECC_CMD_FIELD_FP), 28, ECC_CRYPTO_SECP224R1 }

/**
 * \brief Parameter initialization for secp256r1 curve.
 *
 * This is NIST P-256, ANSI X9.62 prime256v1 curve.
 */
#define CRYPTO_EC_PARAMS_SECP256R1 \
        { hw_ecc_p256_q, hw_ecc_p256_n, hw_ecc_p256_Gx, hw_ecc_p256_Gy, hw_ecc_p256_a, hw_ecc_p256_b, \
          _CRYPTO_EC_CMD(HW_ECC_CMD_SIGNB_POS, HW_ECC_CMD_SIGNA_POS, HW_ECC_CMD_OP_SIZE_256B, HW_ECC_CMD_FIELD_FP), 32, ECC_CRYPTO_SECP256R1 }

/**
 * \brief Parameter initialization for Curve25519.
 *
 * This is a Montgomery curve used for fast ECDH.
 */
#define CRYPTO_EC_PARAMS_CURVE25519 \
        { hw_ecc_curve25519_p, NULL, hw_ecc_curve25519_G, NULL, hw_ecc_curve25519_a24, NULL, \
          _CRYPTO_EC_CMD(HW_ECC_CMD_SIGNB_POS, HW_ECC_CMD_SIGNA_POS, HW_ECC_CMD_OP_SIZE_256B, HW_ECC_CMD_FIELD_FP), 32, ECC_CRYPTO_CURVE25519 }

#endif /*dg_USE_HW_ECC*/

#endif  /* CRYPTO_EC_H_ */

/**
 * \}
 * \}
 */

