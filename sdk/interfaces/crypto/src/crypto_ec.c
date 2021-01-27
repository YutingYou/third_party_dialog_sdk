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
 * @file crypto_ec.c
 *
 * @brief ecc primitives jumptables implementation.
 *
 * Copyright (C) 2018 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#include <sys_trng.h>
#include <ad_crypto.h>
#include <crypto_ec.h>
#include <crypto_ecc_provider_functions.h>

#if dg_ALLOW_DYNAMIC_LIB_PROVIDER
#define CONST_OR_DYNAMIC_LIB_PROVIDER
#else
#define CONST_OR_DYNAMIC_LIB_PROVIDER   const
#endif

CRYPTO_ECC_PROVIDER_RET (* CONST_OR_DYNAMIC_LIB_PROVIDER ecc_curve_init_jump_table[])(
        crypto_ec_params_t*) = {CURVE_SECP160R1_DEFAULT_INIT,
                CURVE_SECP192R1_DEFAULT_INIT,
                CURVE_SECP224R1_DEFAULT_INIT,
                CURVE_SECP256R1_DEFAULT_INIT,
                CURVE_SECP256K1_DEFAULT_INIT,
                CURVE_25519_DEFAULT_INIT
};
CRYPTO_ECC_PROVIDER_RET (* CONST_OR_DYNAMIC_LIB_PROVIDER ecc_compute_private_jump_table[])(
        crypto_ec_params_t*, uint8_t*) = {CURVE_SECP160R1_DEFAULT_COMPUTE_PRIVATE,
                CURVE_SECP192R1_DEFAULT_COMPUTE_PRIVATE,
                CURVE_SECP224R1_DEFAULT_COMPUTE_PRIVATE,
                CURVE_SECP256R1_DEFAULT_COMPUTE_PRIVATE,
                CURVE_SECP256K1_DEFAULT_COMPUTE_PRIVATE,
                CURVE_25519_DEFAULT_COMPUTE_PRIVATE
};

CRYPTO_ECC_PROVIDER_RET (* CONST_OR_DYNAMIC_LIB_PROVIDER ecc_compute_public_jump_table[])(
        crypto_ec_params_t*, uint8_t*, uint8_t*) = {CURVE_SECP160R1_DEFAULT_COMPUTE_PUBLIC,
                CURVE_SECP192R1_DEFAULT_COMPUTE_PUBLIC,
                CURVE_SECP224R1_DEFAULT_COMPUTE_PUBLIC,
                CURVE_SECP256R1_DEFAULT_COMPUTE_PUBLIC,
                CURVE_SECP256K1_DEFAULT_COMPUTE_PUBLIC,
                CURVE_25519_DEFAULT_COMPUTE_PUBLIC
};

CRYPTO_ECC_PROVIDER_RET (* CONST_OR_DYNAMIC_LIB_PROVIDER ecc_compute_shared_jump_table[])(
        crypto_ec_params_t*, uint8_t*, uint8_t*,
        uint8_t*) = {CURVE_SECP160R1_DEFAULT_COMPUTE_SHARED,
                CURVE_SECP192R1_DEFAULT_COMPUTE_SHARED,
                CURVE_SECP224R1_DEFAULT_COMPUTE_SHARED,
                CURVE_SECP256R1_DEFAULT_COMPUTE_SHARED,
                CURVE_SECP256K1_DEFAULT_COMPUTE_SHARED,
                CURVE_25519_DEFAULT_COMPUTE_SHARED
};

/**
 * \brief ecc crypto library mutex.
 */
typedef enum {
        CRYPTO_ECC_AVAIL = 0,     /**< ecc operations are free to use. */
        CRYPTO_ECC_NOT_AVAIL = 1, /**< ecc operations are not free to use */
        CRYPTO_ECC_LAST_VALUE /**< ecc last value. */
} CRYPTO_ECC_MUTEX_t;

__RETAINED_RW static CRYPTO_ECC_MUTEX_t crypto_ecc_mutex = CRYPTO_ECC_AVAIL;

/**
 * \brief Acguire ecc operations mutex
 */
static CRYPTO_ECC_PROVIDER_RET crypto_ecc_mutex_acquire(void)
{
        CRYPTO_ECC_PROVIDER_RET return_status;
        GLOBAL_INT_DISABLE();
        return_status = CRYPTO_ECC_PROVIDER_RET_MUTEX_LOCKED;
        if (crypto_ecc_mutex == CRYPTO_ECC_AVAIL) {
                crypto_ecc_mutex = CRYPTO_ECC_NOT_AVAIL;
                return_status = CRYPTO_ECC_PROVIDER_RET_OK;
        }
        GLOBAL_INT_RESTORE();
        return return_status;
}

/**
 * \brief Release ecc operations mutex
 */
static CRYPTO_ECC_PROVIDER_RET crypto_ecc_mutex_release(void)
{
        crypto_ecc_mutex = CRYPTO_ECC_AVAIL;
        return CRYPTO_ECC_PROVIDER_RET_OK;
}

#if dg_ALLOW_DYNAMIC_LIB_PROVIDER
CRYPTO_ECC_PROVIDER_RET crypto_ecc_set_curve_lib_provider(CRYPTO_ECC_CURVE curve_type, ECC_CRYPTO_LIB_PROVIDER lib_provider_input)
{

        //check that a valid library provider has been requested.
        if (lib_provider_input >= CRYPTO_ECC_PROVIDER_LAST_VALUE) {
                return CRYPTO_ECC_PROVIDER_RET_INVALID_LIB;
        }
        //check that the requested curve is provided by the requested library.
        if (!(lib_provider_truthtable[curve_type][lib_provider_input])) {
                return CRYPTO_ECC_PROVIDER_RET_NOT_PROVIDED;
        }
        if (crypto_ecc_mutex_acquire() != CRYPTO_ECC_PROVIDER_RET_OK) {
                return CRYPTO_ECC_PROVIDER_RET_MUTEX_LOCKED;
        }
        /* Update ecc provider jumptables.*/
        switch (curve_type) {
        case ECC_CRYPTO_SECP160R1:
                if (lib_provider_input == UECC_LIB) {
                        ecc_curve_init_jump_table[curve_type] =
                        CURVE_SECP160R1_UECC_LIB_INIT;
                        ecc_compute_private_jump_table[curve_type] =
                        CURVE_SECP160R1_UECC_LIB_COMPUTE_PRIVATE;
                        ecc_compute_public_jump_table[curve_type] =
                        CURVE_SECP160R1_UECC_LIB_COMPUTE_PUBLIC;
                        ecc_compute_shared_jump_table[curve_type] =
                        CURVE_SECP160R1_UECC_LIB_COMPUTE_SHARED;
                }
                break;
        case ECC_CRYPTO_SECP192R1:
                if (lib_provider_input == HW_ECC) {
                        ecc_curve_init_jump_table[curve_type] =
                        CURVE_SECP192R1_HW_ECC_LIB_INIT;
                        ecc_compute_private_jump_table[curve_type] =
                        CURVE_SECP192R1_HW_ECC_LIB_COMPUTE_PRIVATE;
                        ecc_compute_public_jump_table[curve_type] =
                        CURVE_SECP192R1_HW_ECC_LIB_COMPUTE_PUBLIC;
                        ecc_compute_shared_jump_table[curve_type] =
                        CURVE_SECP192R1_HW_ECC_LIB_COMPUTE_SHARED;
                }
                else if (lib_provider_input == UECC_LIB) {
                        ecc_curve_init_jump_table[curve_type] =
                        CURVE_SECP192R1_UECC_LIB_INIT;
                        ecc_compute_private_jump_table[curve_type] =
                        CURVE_SECP192R1_UECC_LIB_COMPUTE_PRIVATE;
                        ecc_compute_public_jump_table[curve_type] =
                        CURVE_SECP192R1_UECC_LIB_COMPUTE_PUBLIC;
                        ecc_compute_shared_jump_table[curve_type] =
                        CURVE_SECP192R1_UECC_LIB_COMPUTE_SHARED;
                }
                break;
        case ECC_CRYPTO_SECP224R1:
                if (lib_provider_input == HW_ECC) {
                        ecc_curve_init_jump_table[curve_type] =
                        CURVE_SECP224R1_HW_ECC_LIB_INIT;
                        ecc_compute_private_jump_table[curve_type] =
                        CURVE_SECP224R1_HW_ECC_LIB_COMPUTE_PRIVATE;
                        ecc_compute_public_jump_table[curve_type] =
                        CURVE_SECP224R1_HW_ECC_LIB_COMPUTE_PUBLIC;
                        ecc_compute_shared_jump_table[curve_type] =
                        CURVE_SECP224R1_HW_ECC_LIB_COMPUTE_SHARED;
                }
                else if (lib_provider_input == UECC_LIB) {
                        ecc_curve_init_jump_table[curve_type] =
                        CURVE_SECP224R1_UECC_LIB_INIT;
                        ecc_compute_private_jump_table[curve_type] =
                        CURVE_SECP224R1_UECC_LIB_COMPUTE_PRIVATE;
                        ecc_compute_public_jump_table[curve_type] =
                        CURVE_SECP224R1_UECC_LIB_COMPUTE_PUBLIC;
                        ecc_compute_shared_jump_table[curve_type] =
                        CURVE_SECP224R1_UECC_LIB_COMPUTE_SHARED;
                }
                break;
        case ECC_CRYPTO_SECP256R1:
                if (lib_provider_input == HW_ECC) {
                        ecc_curve_init_jump_table[curve_type] =
                        CURVE_SECP256R1_HW_ECC_LIB_INIT;
                        ecc_compute_private_jump_table[curve_type] =
                        CURVE_SECP256R1_HW_ECC_LIB_COMPUTE_PRIVATE;
                        ecc_compute_public_jump_table[curve_type] =
                        CURVE_SECP256R1_HW_ECC_LIB_COMPUTE_PUBLIC;
                        ecc_compute_shared_jump_table[curve_type] =
                        CURVE_SECP256R1_HW_ECC_LIB_COMPUTE_SHARED;
                }
                else if (lib_provider_input == UECC_LIB) {
                        ecc_curve_init_jump_table[curve_type] =
                        CURVE_SECP256R1_UECC_LIB_INIT;
                        ecc_compute_private_jump_table[curve_type] =
                        CURVE_SECP256R1_UECC_LIB_COMPUTE_PRIVATE;
                        ecc_compute_public_jump_table[curve_type] =
                        CURVE_SECP256R1_UECC_LIB_COMPUTE_PUBLIC;
                        ecc_compute_shared_jump_table[curve_type] =
                        CURVE_SECP256R1_UECC_LIB_COMPUTE_SHARED;
                }
                break;
        case ECC_CRYPTO_SECP256K1:
                if (lib_provider_input == UECC_LIB) {
                        ecc_curve_init_jump_table[curve_type] =
                        CURVE_SECP256K1_UECC_LIB_INIT;
                        ecc_compute_private_jump_table[curve_type] =
                        CURVE_SECP256K1_UECC_LIB_COMPUTE_PRIVATE;
                        ecc_compute_public_jump_table[curve_type] =
                        CURVE_SECP256K1_UECC_LIB_COMPUTE_PUBLIC;
                        ecc_compute_shared_jump_table[curve_type] =
                        CURVE_SECP256K1_UECC_LIB_COMPUTE_SHARED;
                }
                break;
        case ECC_CRYPTO_CURVE25519:
                if (lib_provider_input == HW_ECC) {
                        ecc_curve_init_jump_table[curve_type] = CURVE_25519_HW_ECC_LIB_INIT;
                        ecc_compute_private_jump_table[curve_type] =
                        CURVE_25519_HW_ECC_LIB_COMPUTE_PRIVATE;
                        ecc_compute_public_jump_table[curve_type] =
                        CURVE_25519_HW_ECC_LIB_COMPUTE_PUBLIC;
                        ecc_compute_shared_jump_table[curve_type] =
                        CURVE_25519_HW_ECC_LIB_COMPUTE_SHARED;
                }
                else if (lib_provider_input == SODIUM_LIB) {
                        ecc_curve_init_jump_table[curve_type] =
                        CURVE_25519_SODIUM_LIB_INIT;
                        ecc_compute_private_jump_table[curve_type] =
                        CURVE_25519_SODIUM_LIB_COMPUTE_PRIVATE;
                        ecc_compute_public_jump_table[curve_type] =
                        CURVE_25519_SODIUM_LIB_COMPUTE_PUBLIC;
                        ecc_compute_shared_jump_table[curve_type] =
                        CURVE_25519_SODIUM_LIB_COMPUTE_SHARED;
                }
                break;
        default:
                crypto_ecc_mutex_release();
                return CRYPTO_ECC_PROVIDER_RET_ERROR;
        }
        crypto_ecc_mutex_release();

        /* Update lib provider index array. */
        curve_lib_provider_index[curve_type] = lib_provider_input;
        crypto_ecc_mutex_release();
        return CRYPTO_ECC_PROVIDER_RET_OK;
}
#endif

CRYPTO_ECC_PROVIDER_RET crypto_ecc_curve_init(crypto_ec_params_t *curve,
        CRYPTO_ECC_CURVE curve_type)
{

        ECC_CRYPTO_LIB_PROVIDER curve_lib_provider = curve_lib_provider_index[curve_type];
        /* Check if the requested curve is not provided by the requested lib provider (or any).*/
        if (!(lib_provider_truthtable[curve_type][curve_lib_provider])) {
                return CRYPTO_ECC_PROVIDER_RET_NOT_PROVIDED;
        }
        if (crypto_ecc_mutex_acquire() == CRYPTO_ECC_PROVIDER_RET_OK) {
                /* Set curve type. */
                curve->type = curve_type;
                /* Init curve structure. */
                if (curve_type < ECC_CRYPTO_LAST_VALUE) {
                        ecc_curve_init_jump_table[curve_type](curve);
                }
                else {
                        crypto_ecc_mutex_release();
                        return CRYPTO_ECC_PROVIDER_RET_INVALID_CURVE;
                }
                crypto_ecc_mutex_release();
                return CRYPTO_ECC_PROVIDER_RET_OK;
        }
        else {
                return CRYPTO_ECC_PROVIDER_RET_MUTEX_LOCKED;
        }
}

CRYPTO_ECC_PROVIDER_RET crypto_ecc_compute_public_key(crypto_ec_params_t *curve, uint8_t *d,
        uint8_t *Q)
{
        if (crypto_ecc_mutex_acquire() == CRYPTO_ECC_PROVIDER_RET_OK) {
                if (curve->type < ECC_CRYPTO_LAST_VALUE) {
                        CRYPTO_ECC_PROVIDER_RET status = ecc_compute_public_jump_table[curve->type](
                                curve, d, Q);
                        crypto_ecc_mutex_release();
                        return status;
                }
                else {
                        crypto_ecc_mutex_release();
                        return CRYPTO_ECC_PROVIDER_RET_INVALID_CURVE;
                }
        }
        else {
                return CRYPTO_ECC_PROVIDER_RET_MUTEX_LOCKED;
        }
}

CRYPTO_ECC_PROVIDER_RET crypto_ecc_compute_private_key(crypto_ec_params_t *curve, uint8_t *d)
{
        if (crypto_ecc_mutex_acquire() == CRYPTO_ECC_PROVIDER_RET_OK) {
                if (curve->type < ECC_CRYPTO_LAST_VALUE) {
                        CRYPTO_ECC_PROVIDER_RET status =
                                ecc_compute_private_jump_table[curve->type](curve, d);
                        crypto_ecc_mutex_release();
                        return status;
                }
                else {
                        crypto_ecc_mutex_release();
                        return CRYPTO_ECC_PROVIDER_RET_INVALID_CURVE;
                }
        }
        else {
                return CRYPTO_ECC_PROVIDER_RET_MUTEX_LOCKED;
        }
}

CRYPTO_ECC_PROVIDER_RET crypto_ecc_compute_shared_secret(crypto_ec_params_t *curve, uint8_t *d,
        uint8_t *Qp, uint8_t *s)
{
        if (crypto_ecc_mutex_acquire() == CRYPTO_ECC_PROVIDER_RET_OK) {
                if (curve->type < ECC_CRYPTO_LAST_VALUE) {
                        CRYPTO_ECC_PROVIDER_RET status = ecc_compute_shared_jump_table[curve->type](
                                curve, d, Qp, s);
                        crypto_ecc_mutex_release();
                        return status;
                }
                else {
                        crypto_ecc_mutex_release();
                        return CRYPTO_ECC_PROVIDER_RET_INVALID_CURVE;
                }
        }
        else {
                return CRYPTO_ECC_PROVIDER_RET_MUTEX_LOCKED;
        }
}

/**
 * \}
 * \}
 * \}
 * \}
 */
