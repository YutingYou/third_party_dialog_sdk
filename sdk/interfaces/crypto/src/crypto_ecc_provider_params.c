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
 * @file crypto_ecc_provider_params.c
 *
 * @brief ecc parameter arrays initialization.
 *
 * Copyright (C) 2018 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include "crypto_ecc_provider_params.h"
#include <stdint.h>


ECC_CRYPTO_LIB_PROVIDER curve_lib_provider_index[ECC_CRYPTO_LAST_VALUE] =  { CURVE_SECP160R1_DEFAULT_LIB_PROVIDER,
        CURVE_SECP192R1_DEFAULT_LIB_PROVIDER,
        CURVE_SECP224R1_DEFAULT_LIB_PROVIDER,
        CURVE_SECP256R1_DEFAULT_LIB_PROVIDER,
        CURVE_SECP256K1_DEFAULT_LIB_PROVIDER,
        CURVE_25519_DEFAULT_LIB_PROVIDER};

/* This is used in order to check if a library provider is available and if it is
 * implementing a certain curve, it will be marked as true in the lib provider truthtable.
 */
#if dg_USE_HW_ECC
#define HW_ECC_TRUTHTABLE_FLAG true
#else
#define HW_ECC_TRUTHTABLE_FLAG false
#endif
#if dg_USE_UECC_LIB
#define UECC_TRUTHTABLE_FLAG true
#else
#define UECC_TRUTHTABLE_FLAG false
#endif
#if dg_USE_SODIUM_LIB
#define SODIUM_TRUTHTABLE_FLAG true
#else
#define SODIUM_TRUTHTABLE_FLAG false
#endif

#if dg_USE_CURVE_SECP160R1
#define dg_USE_CURVE_SECP160R1_FLAG true
#else
#define dg_USE_CURVE_SECP160R1_FLAG false
#endif
#if dg_USE_CURVE_SECP192R1
#define dg_USE_CURVE_SECP192R1_FLAG true
#else
#define dg_USE_CURVE_SECP192R1_FLAG false
#endif
#if dg_USE_CURVE_SECP224R1
#define dg_USE_CURVE_SECP224R1_FLAG true
#else
#define dg_USE_CURVE_SECP224R1_FLAG false
#endif
#if dg_USE_CURVE_SECP256R1
#define dg_USE_CURVE_SECP256R1_FLAG true
#else
#define dg_USE_CURVE_SECP256R1_FLAG false
#endif
#if dg_USE_CURVE_SECP256K1
#define dg_USE_CURVE_SECP256K1_FLAG true
#else
#define dg_USE_CURVE_SECP256K1_FLAG false
#endif
#if dg_USE_CURVE_25519
#define dg_USE_CURVE_25519_FLAG true
#else
#define dg_USE_CURVE_25519_FLAG false
#endif

/*
 * Truthtable that provides info which library provides which curve, for sanitizing input when
 * setting libprovider during curve initialization.
 * Row order must be the same as CRYPTO_ECC_CURVE
 * Column order must be the same as ECC_CRYPTO_LIB_PROVIDER
 *                                    |hw_ecc | uecc | sodium |
 *                         secp160r1: |   F   |   T  |    F   |
 *                         secp192r1: |   T   |   T  |    F   |
 *                         secp224r1: |   T   |   T  |    F   |
 *                         secp256r1: |   T   |   T  |    F   |
 *                         secp256k1: |   F   |   T  |    F   |
 *                        curve25519: |   T   |   F  |    T   |
 */
const bool lib_provider_truthtable[ECC_CRYPTO_LAST_VALUE][CRYPTO_ECC_PROVIDER_LAST_VALUE] =
{      //|                      hw_ecc                        |                      uecc                          |                     sodium                          |
       { false,                                                 UECC_TRUTHTABLE_FLAG && dg_USE_CURVE_SECP160R1_FLAG,  false                                              },    //secp160r1: |   F   |   T  |    F   |
       { HW_ECC_TRUTHTABLE_FLAG && dg_USE_CURVE_SECP192R1_FLAG, UECC_TRUTHTABLE_FLAG && dg_USE_CURVE_SECP192R1_FLAG,  false                                              },    //secp192r1: |   T   |   T  |    F   |
       { HW_ECC_TRUTHTABLE_FLAG && dg_USE_CURVE_SECP224R1_FLAG, UECC_TRUTHTABLE_FLAG && dg_USE_CURVE_SECP224R1_FLAG,  false                                              },    //secp224r1: |   T   |   T  |    F   |
       { HW_ECC_TRUTHTABLE_FLAG && dg_USE_CURVE_SECP256R1_FLAG, UECC_TRUTHTABLE_FLAG && dg_USE_CURVE_SECP256R1_FLAG,  false                                              },    //secp256r1: |   T   |   T  |    F   |
       { false,                                                 UECC_TRUTHTABLE_FLAG && dg_USE_CURVE_SECP256K1_FLAG,  false                                              },    //secp256k1: |   F   |   T  |    F   |
       { HW_ECC_TRUTHTABLE_FLAG && dg_USE_CURVE_25519_FLAG,     false,                                                SODIUM_TRUTHTABLE_FLAG && dg_USE_CURVE_25519_FLAG  }    //curve25519: |   T   |   F  |    T   |

};
/**
 * \}
 * \}
 * \}
 * \}
 */
