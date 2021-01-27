/**
 * \addtogroup SECURITY_TOOLBOX
 * \{
 * \addtogroup CURVES
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file crypto_ecc_provider_params.h
 *
 * @brief Elliptic curves parameters.
 *
 * Copyright (C) 2018 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SDK_CRYPTO_INCLUDE_CRYPTO_ECC_PROVIDER_PARAMS_H_
#define SDK_CRYPTO_INCLUDE_CRYPTO_ECC_PROVIDER_PARAMS_H_

#include <stdbool.h>

#define HW_ECC_ID 0
#define UECC_LIB_ID 1
#define SODIUM_LIB_ID 2

/**
 * \brief ECC library providers.
 */
typedef enum {
        HW_ECC = HW_ECC_ID,               /**< hw ecc coprocessor. */
        UECC_LIB = UECC_LIB_ID,           /**< uECC library. */
        SODIUM_LIB = SODIUM_LIB_ID,       /**< sodium library. */
        CRYPTO_ECC_PROVIDER_LAST_VALUE,
} ECC_CRYPTO_LIB_PROVIDER;

/**
 * \brief ECC supported curves type.
 */
typedef enum {
        ECC_CRYPTO_SECP160R1 =0,
        ECC_CRYPTO_SECP192R1 =1,
        ECC_CRYPTO_SECP224R1 =2,
        ECC_CRYPTO_SECP256R1 =3,
        ECC_CRYPTO_SECP256K1 =4,
        ECC_CRYPTO_CURVE25519 =5,
        ECC_CRYPTO_LAST_VALUE
} CRYPTO_ECC_CURVE;

/**
 * \brief ECC Provider API return codes.
 */
typedef enum {
        CRYPTO_ECC_PROVIDER_RET_OK = 0,           /**< No error. */
        CRYPTO_ECC_PROVIDER_RET_INIT_FAIL,        /**< Failed to initialize external library during curve initialization. */
        CRYPTO_ECC_PROVIDER_RET_NOT_PROVIDED,     /**< No provider library for the requested curve. */
        CRYPTO_ECC_PROVIDER_RET_INVALID_LIB,      /**< Invalid  library request. */
        CRYPTO_ECC_PROVIDER_RET_INVALID_CURVE,    /**< Invalid  curve request. */
        CRYPTO_ECC_PROVIDER_RET_MUTEX_LOCKED,     /**< Crypto library mutex is locked. */
        CRYPTO_ECC_PROVIDER_RET_ERROR             /**< Other error. */
} CRYPTO_ECC_PROVIDER_RET;

/**
 * \brief Dynamic ecc library selection flag.
 *
 * Set in order to allow ecc library providers to be changed during runtime.
 * This is practical only for testing purposes, ie cross checking library implementations
 * of the same algorithm from the same binary file.
 */
#define dg_ALLOW_DYNAMIC_LIB_PROVIDER   0

/*
 * Set in order to provide the selected curve.
 */
#define dg_USE_CURVE_SECP160R1          1
#define dg_USE_CURVE_SECP192R1          1
#define dg_USE_CURVE_SECP224R1          1
#define dg_USE_CURVE_SECP256R1          1
#define dg_USE_CURVE_SECP256K1          1
#define dg_USE_CURVE_25519              1

/*
 * Set in order to use the selected ecc library providers.
 */
#define dg_USE_HW_ECC                   0
#define dg_USE_UECC_LIB                 1
#define dg_USE_SODIUM_LIB               1

/* Check hw_ecc module availability.*/
#if ( ((!dg_USE_HW_ECC)&&(dg_configUSE_HW_ECC)) || ((dg_USE_HW_ECC)&&(!dg_configUSE_HW_ECC)) )
#error "In order to (exclude/include) hw_ecc, dg_configUSE_HW_ECC must also be set to (0/1) in config_*.h"
#endif
/*
 * Set default library provider for each curve.
 * If dynamic library selection is disabled these libraries will be used
 * for the ecc implementation for each curve.
 * If dynamic library selection is enabled these libraries will be the
 * initialized values of the lib provider index table and if a library
 * provider is not explicitly set before using a certain curve, this is
 * the library that will try to implement the ecc functionality.
 */
#define CURVE_SECP160R1_DEFAULT_LIB_PROVIDER          UECC_LIB_ID
#define CURVE_SECP192R1_DEFAULT_LIB_PROVIDER          UECC_LIB_ID
#define CURVE_SECP224R1_DEFAULT_LIB_PROVIDER          UECC_LIB_ID
#define CURVE_SECP256R1_DEFAULT_LIB_PROVIDER          UECC_LIB_ID
#define CURVE_SECP256K1_DEFAULT_LIB_PROVIDER          UECC_LIB_ID
#define CURVE_25519_DEFAULT_LIB_PROVIDER              SODIUM_LIB_ID

/**
 * \brief Index table informs which curve is implemented by which library provider.
 *
 * This is used for dynamically changing library provider for a specific curve.
 * Row index must be the same as CRYPTO_ECC_CURVE
 */
extern ECC_CRYPTO_LIB_PROVIDER curve_lib_provider_index[ECC_CRYPTO_LAST_VALUE];

/**
 * \brief Truth table informs if a curve is provided by the respective library.
 *
 * This is used for checking if a requested curved can be implemented by a
 * requested library.
 */
extern const bool lib_provider_truthtable[ECC_CRYPTO_LAST_VALUE][CRYPTO_ECC_PROVIDER_LAST_VALUE];


#endif /* SDK_CRYPTO_INCLUDE_CRYPTO_ECC_PROVIDER_PARAMS_H_ */
/**
 * \}
 * \}
 */
