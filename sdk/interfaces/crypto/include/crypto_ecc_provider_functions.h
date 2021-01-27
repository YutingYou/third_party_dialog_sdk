/**
 * \addtogroup SECURITY_TOOLBOX
 * \{
 * \addtogroup CURVES
 *
 * \brief Elliptic curves primitives and basic datatypes.
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file crypto_ecc_provider_functions.h
 *
 * @brief Elliptic curves primitives and basic datatypes. Depending on ecc library and
 *        curve settings, the primitives provided from this header will be mapped to the
 *        respective functions.
 *
 * Copyright (C) 2018 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#ifndef SDK_CRYPTO_INCLUDE_CRYPTO_ECC_PROVIDER_FUNCTIONS_H_
#define SDK_CRYPTO_INCLUDE_CRYPTO_ECC_PROVIDER_FUNCTIONS_H_

#include <sys_trng.h>
#include <ad_crypto.h>
#include <hw_ecc.h>
#include "sodium.h"
#include "uECC.h"
#include <crypto_ecc_provider_functions.h>
#include <crypto_ec.h>

/* Compute private key external functions.*/
CRYPTO_ECC_PROVIDER_RET compute_private_key_uecc(crypto_ec_params_t *curve, uint8_t *d);
CRYPTO_ECC_PROVIDER_RET compute_private_key_25519(crypto_ec_params_t *curve, uint8_t *d);
CRYPTO_ECC_PROVIDER_RET compute_private_key_hw(crypto_ec_params_t *curve, uint8_t *d);

/* Compute public key external functions.*/
CRYPTO_ECC_PROVIDER_RET compute_public_key_25519_hw(crypto_ec_params_t *curve, uint8_t *d, uint8_t *Q);
CRYPTO_ECC_PROVIDER_RET compute_public_key_25519_sodium(crypto_ec_params_t *curve, uint8_t *d, uint8_t *Q);
CRYPTO_ECC_PROVIDER_RET compute_public_key_hw(crypto_ec_params_t *curve, uint8_t *d, uint8_t *Q);
CRYPTO_ECC_PROVIDER_RET compute_public_key_uecc(crypto_ec_params_t *curve, uint8_t *d, uint8_t *Q);

/* Compute shared key external functions.*/
CRYPTO_ECC_PROVIDER_RET compute_shared_secret_hw(crypto_ec_params_t *curve, uint8_t *d, uint8_t *Qp, uint8_t *s);
CRYPTO_ECC_PROVIDER_RET compute_shared_secret_uecc(crypto_ec_params_t *curve, uint8_t *d, uint8_t *Qp, uint8_t *s);
CRYPTO_ECC_PROVIDER_RET compute_shared_secret_25519_sodium(crypto_ec_params_t *curve, uint8_t *d, uint8_t *Qp, uint8_t *s);
CRYPTO_ECC_PROVIDER_RET compute_shared_secret_25519_hw(crypto_ec_params_t *curve, uint8_t *d, uint8_t *Qp, uint8_t *s);

/* Curve init functions .*/
CRYPTO_ECC_PROVIDER_RET curve_init_secp160r1(crypto_ec_params_t *curve);
CRYPTO_ECC_PROVIDER_RET curve_init_secp192r1(crypto_ec_params_t *curve);
CRYPTO_ECC_PROVIDER_RET curve_init_secp224r1(crypto_ec_params_t *curve);
CRYPTO_ECC_PROVIDER_RET curve_init_secp256r1(crypto_ec_params_t *curve);
CRYPTO_ECC_PROVIDER_RET curve_init_secp256k1(crypto_ec_params_t *curve);
CRYPTO_ECC_PROVIDER_RET curve_init_25519(crypto_ec_params_t *curve);

/* This is the error handling function of ecc library providing mechanism.
 * In case a call to a curve or a library that is not provided depending
 * on settings in crypto_ecc_provider_params.h this function will be called.
 */
CRYPTO_ECC_PROVIDER_RET ecc_lib_provider_error_handler();

#endif /* SDK_CRYPTO_INCLUDE_CRYPTO_ECC_PROVIDER_FUNCTIONS_H_ */
/**
 * \}
 * \}
 */
