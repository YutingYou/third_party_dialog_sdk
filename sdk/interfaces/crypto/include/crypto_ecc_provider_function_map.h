/**
 * \addtogroup SECURITY_TOOLBOX
 * \{
 * \addtogroup CURVES
 *
 * \brief Elliptic curves provider function mapping.
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file crypto_ecc_provider_function_map.h
 *
 * @brief  ECC curvetype - libprovider mapping macro definitions.
 *
 *   Mapping functions to be called for each elliptic curve provided
 *   by each ecc library provider.
 *
 * Copyright (C) 2018 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */


#ifndef SDK_CRYPTO_INCLUDE_CRYPTO_ECC_PROVIDER_FUNCTION_MAP_H_
#define SDK_CRYPTO_INCLUDE_CRYPTO_ECC_PROVIDER_FUNCTION_MAP_H_

/*
 * This set of definitions maps curve init and prototype functions depending on ecc params
 */

/* SECP160R1 */
#if dg_USE_CURVE_SECP160R1
    #if dg_USE_UECC_LIB
        #define CURVE_SECP160R1_UECC_LIB_INIT             curve_init_secp160r1
        #define CURVE_SECP160R1_UECC_LIB_COMPUTE_PUBLIC   compute_public_key_uecc
        #define CURVE_SECP160R1_UECC_LIB_COMPUTE_PRIVATE  compute_private_key_uecc
        #define CURVE_SECP160R1_UECC_LIB_COMPUTE_SHARED   compute_shared_secret_uecc
    #else
        #define CURVE_SECP160R1_UECC_LIB_INIT             ecc_lib_provider_error_handler
        #define CURVE_SECP160R1_UECC_LIB_COMPUTE_PUBLIC   ecc_lib_provider_error_handler
        #define CURVE_SECP160R1_UECC_LIB_COMPUTE_PRIVATE  ecc_lib_provider_error_handler
        #define CURVE_SECP160R1_UECC_LIB_COMPUTE_SHARED   ecc_lib_provider_error_handler
    #endif

#else
    #define CURVE_SECP160R1_UECC_LIB_INIT             ecc_lib_provider_error_handler
    #define CURVE_SECP160R1_UECC_LIB_COMPUTE_PUBLIC   ecc_lib_provider_error_handler
    #define CURVE_SECP160R1_UECC_LIB_COMPUTE_PRIVATE  ecc_lib_provider_error_handler
    #define CURVE_SECP160R1_UECC_LIB_COMPUTE_SHARED   ecc_lib_provider_error_handler
#endif

/* SECP192R1 */
#if dg_USE_CURVE_SECP192R1
    #if dg_USE_HW_ECC
        #define CURVE_SECP192R1_HW_ECC_LIB_INIT             curve_init_secp192r1
        #define CURVE_SECP192R1_HW_ECC_LIB_COMPUTE_PUBLIC   compute_public_key_hw
        #define CURVE_SECP192R1_HW_ECC_LIB_COMPUTE_PRIVATE  compute_private_key_hw
        #define CURVE_SECP192R1_HW_ECC_LIB_COMPUTE_SHARED   compute_shared_secret_hw
    #else
        #define CURVE_SECP192R1_HW_ECC_LIB_INIT             ecc_lib_provider_error_handler
        #define CURVE_SECP192R1_HW_ECC_LIB_COMPUTE_PUBLIC   ecc_lib_provider_error_handler
        #define CURVE_SECP192R1_HW_ECC_LIB_COMPUTE_PRIVATE  ecc_lib_provider_error_handler
        #define CURVE_SECP192R1_HW_ECC_LIB_COMPUTE_SHARED   ecc_lib_provider_error_handler
    #endif

    #if dg_USE_UECC_LIB
        #define CURVE_SECP192R1_UECC_LIB_INIT             curve_init_secp192r1
        #define CURVE_SECP192R1_UECC_LIB_COMPUTE_PUBLIC   compute_public_key_uecc
        #define CURVE_SECP192R1_UECC_LIB_COMPUTE_PRIVATE  compute_private_key_uecc
        #define CURVE_SECP192R1_UECC_LIB_COMPUTE_SHARED   compute_shared_secret_uecc
    #else
        #define CURVE_SECP192R1_UECC_LIB_INIT             ecc_lib_provider_error_handler
        #define CURVE_SECP192R1_UECC_LIB_COMPUTE_PUBLIC   ecc_lib_provider_error_handler
        #define CURVE_SECP192R1_UECC_LIB_COMPUTE_PRIVATE  ecc_lib_provider_error_handler
        #define CURVE_SECP192R1_UECC_LIB_COMPUTE_SHARED   ecc_lib_provider_error_handler
    #endif

#else
    #define CURVE_SECP192R1_HW_ECC_LIB_INIT             ecc_lib_provider_error_handler
    #define CURVE_SECP192R1_HW_ECC_LIB_COMPUTE_PUBLIC   ecc_lib_provider_error_handler
    #define CURVE_SECP192R1_HW_ECC_LIB_COMPUTE_PRIVATE  ecc_lib_provider_error_handler
    #define CURVE_SECP192R1_HW_ECC_LIB_COMPUTE_SHARED   ecc_lib_provider_error_handler
    #define CURVE_SECP192R1_UECC_LIB_INIT             ecc_lib_provider_error_handler
    #define CURVE_SECP192R1_UECC_LIB_COMPUTE_PUBLIC   ecc_lib_provider_error_handler
    #define CURVE_SECP192R1_UECC_LIB_COMPUTE_PRIVATE  ecc_lib_provider_error_handler
    #define CURVE_SECP192R1_UECC_LIB_COMPUTE_SHARED   ecc_lib_provider_error_handler
#endif

/* SECP224R1 */
#if dg_USE_CURVE_SECP224R1
    #if dg_USE_HW_ECC
        #define CURVE_SECP224R1_HW_ECC_LIB_INIT             curve_init_secp224r1
        #define CURVE_SECP224R1_HW_ECC_LIB_COMPUTE_PUBLIC   compute_public_key_hw
        #define CURVE_SECP224R1_HW_ECC_LIB_COMPUTE_PRIVATE  compute_private_key_hw
        #define CURVE_SECP224R1_HW_ECC_LIB_COMPUTE_SHARED   compute_shared_secret_hw
    #else
        #define CURVE_SECP224R1_HW_ECC_LIB_INIT             ecc_lib_provider_error_handler
        #define CURVE_SECP224R1_HW_ECC_LIB_COMPUTE_PUBLIC   ecc_lib_provider_error_handler
        #define CURVE_SECP224R1_HW_ECC_LIB_COMPUTE_PRIVATE  ecc_lib_provider_error_handler
        #define CURVE_SECP224R1_HW_ECC_LIB_COMPUTE_SHARED   ecc_lib_provider_error_handler
    #endif

    #if dg_USE_UECC_LIB
        #define CURVE_SECP224R1_UECC_LIB_INIT             curve_init_secp224r1
        #define CURVE_SECP224R1_UECC_LIB_COMPUTE_PUBLIC   compute_public_key_uecc
        #define CURVE_SECP224R1_UECC_LIB_COMPUTE_PRIVATE  compute_private_key_uecc
        #define CURVE_SECP224R1_UECC_LIB_COMPUTE_SHARED   compute_shared_secret_uecc
    #else
        #define CURVE_SECP224R1_UECC_LIB_INIT             ecc_lib_provider_error_handler
        #define CURVE_SECP224R1_UECC_LIB_COMPUTE_PUBLIC   ecc_lib_provider_error_handler
        #define CURVE_SECP224R1_UECC_LIB_COMPUTE_PRIVATE  ecc_lib_provider_error_handler
        #define CURVE_SECP224R1_UECC_LIB_COMPUTE_SHARED   ecc_lib_provider_error_handler
    #endif

#else
    #define CURVE_SECP224R1_HW_ECC_LIB_INIT             ecc_lib_provider_error_handler
    #define CURVE_SECP224R1_HW_ECC_LIB_COMPUTE_PUBLIC   ecc_lib_provider_error_handler
    #define CURVE_SECP224R1_HW_ECC_LIB_COMPUTE_PRIVATE  ecc_lib_provider_error_handler
    #define CURVE_SECP224R1_HW_ECC_LIB_COMPUTE_SHARED   ecc_lib_provider_error_handler
    #define CURVE_SECP224R1_UECC_LIB_INIT             ecc_lib_provider_error_handler
    #define CURVE_SECP224R1_UECC_LIB_COMPUTE_PUBLIC   ecc_lib_provider_error_handler
    #define CURVE_SECP224R1_UECC_LIB_COMPUTE_PRIVATE  ecc_lib_provider_error_handler
    #define CURVE_SECP224R1_UECC_LIB_COMPUTE_SHARED   ecc_lib_provider_error_handler
#endif

/* SECP256R1 */
#if dg_USE_CURVE_SECP256R1
    #if dg_USE_HW_ECC
        #define CURVE_SECP256R1_HW_ECC_LIB_INIT             curve_init_secp256r1
        #define CURVE_SECP256R1_HW_ECC_LIB_COMPUTE_PUBLIC   compute_public_key_hw
        #define CURVE_SECP256R1_HW_ECC_LIB_COMPUTE_PRIVATE  compute_private_key_hw
        #define CURVE_SECP256R1_HW_ECC_LIB_COMPUTE_SHARED   compute_shared_secret_hw
    #else
        #define CURVE_SECP256R1_HW_ECC_LIB_INIT             ecc_lib_provider_error_handler
        #define CURVE_SECP256R1_HW_ECC_LIB_COMPUTE_PUBLIC   ecc_lib_provider_error_handler
        #define CURVE_SECP256R1_HW_ECC_LIB_COMPUTE_PRIVATE  ecc_lib_provider_error_handler
        #define CURVE_SECP256R1_HW_ECC_LIB_COMPUTE_SHARED   ecc_lib_provider_error_handler
    #endif

    #if dg_USE_UECC_LIB
        #define CURVE_SECP256R1_UECC_LIB_INIT             curve_init_secp256r1
        #define CURVE_SECP256R1_UECC_LIB_COMPUTE_PUBLIC   compute_public_key_uecc
        #define CURVE_SECP256R1_UECC_LIB_COMPUTE_PRIVATE  compute_private_key_uecc
        #define CURVE_SECP256R1_UECC_LIB_COMPUTE_SHARED   compute_shared_secret_uecc
    #else
        #define CURVE_SECP256R1_UECC_LIB_INIT             ecc_lib_provider_error_handler
        #define CURVE_SECP256R1_UECC_LIB_COMPUTE_PUBLIC   ecc_lib_provider_error_handler
        #define CURVE_SECP256R1_UECC_LIB_COMPUTE_PRIVATE  ecc_lib_provider_error_handler
        #define CURVE_SECP256R1_UECC_LIB_COMPUTE_SHARED   ecc_lib_provider_error_handler
    #endif

#else
    #define CURVE_SECP256R1_HW_ECC_LIB_INIT             ecc_lib_provider_error_handler
    #define CURVE_SECP256R1_HW_ECC_LIB_COMPUTE_PUBLIC   ecc_lib_provider_error_handler
    #define CURVE_SECP256R1_HW_ECC_LIB_COMPUTE_PRIVATE  ecc_lib_provider_error_handler
    #define CURVE_SECP256R1_HW_ECC_LIB_COMPUTE_SHARED   ecc_lib_provider_error_handler
    #define CURVE_SECP256R1_UECC_LIB_INIT             ecc_lib_provider_error_handler
    #define CURVE_SECP256R1_UECC_LIB_COMPUTE_PUBLIC   ecc_lib_provider_error_handler
    #define CURVE_SECP256R1_UECC_LIB_COMPUTE_PRIVATE  ecc_lib_provider_error_handler
    #define CURVE_SECP256R1_UECC_LIB_COMPUTE_SHARED   ecc_lib_provider_error_handler
#endif

/* SECP256K1 */
#if dg_USE_CURVE_SECP256K1
    #if dg_USE_UECC_LIB
        #define CURVE_SECP256K1_UECC_LIB_INIT             curve_init_secp256k1
        #define CURVE_SECP256K1_UECC_LIB_COMPUTE_PUBLIC   compute_public_key_uecc
        #define CURVE_SECP256K1_UECC_LIB_COMPUTE_PRIVATE  compute_private_key_uecc
        #define CURVE_SECP256K1_UECC_LIB_COMPUTE_SHARED   compute_shared_secret_uecc
    #else
        #define CURVE_SECP256K1_UECC_LIB_INIT             ecc_lib_provider_error_handler
        #define CURVE_SECP256K1_UECC_LIB_COMPUTE_PUBLIC   ecc_lib_provider_error_handler
        #define CURVE_SECP256K1_UECC_LIB_COMPUTE_PRIVATE  ecc_lib_provider_error_handler
        #define CURVE_SECP256K1_UECC_LIB_COMPUTE_SHARED   ecc_lib_provider_error_handler
    #endif

#else
    #define CURVE_SECP256K1_HW_ECC_LIB_INIT             ecc_lib_provider_error_handler
    #define CURVE_SECP256K1_HW_ECC_LIB_COMPUTE_PUBLIC   ecc_lib_provider_error_handler
    #define CURVE_SECP256K1_HW_ECC_LIB_COMPUTE_PRIVATE  ecc_lib_provider_error_handler
    #define CURVE_SECP256K1_HW_ECC_LIB_COMPUTE_SHARED   ecc_lib_provider_error_handler
    #define CURVE_SECP256K1_UECC_LIB_INIT             ecc_lib_provider_error_handler
    #define CURVE_SECP256K1_UECC_LIB_COMPUTE_PUBLIC   ecc_lib_provider_error_handler
    #define CURVE_SECP256K1_UECC_LIB_COMPUTE_PRIVATE  ecc_lib_provider_error_handler
    #define CURVE_SECP256K1_UECC_LIB_COMPUTE_SHARED   ecc_lib_provider_error_handler
#endif

/* CURVE25519 */
#if dg_USE_CURVE_25519
    #if dg_USE_HW_ECC
        #define CURVE_25519_HW_ECC_LIB_INIT             curve_init_25519
        #define CURVE_25519_HW_ECC_LIB_COMPUTE_PUBLIC   compute_public_key_25519_hw
        #define CURVE_25519_HW_ECC_LIB_COMPUTE_PRIVATE  compute_private_key_25519
        #define CURVE_25519_HW_ECC_LIB_COMPUTE_SHARED   compute_shared_secret_25519_hw
    #else
        #define CURVE_25519_HW_ECC_LIB_INIT             ecc_lib_provider_error_handler
        #define CURVE_25519_HW_ECC_LIB_COMPUTE_PUBLIC   ecc_lib_provider_error_handler
        #define CURVE_25519_HW_ECC_LIB_COMPUTE_PRIVATE  ecc_lib_provider_error_handler
        #define CURVE_25519_HW_ECC_LIB_COMPUTE_SHARED   ecc_lib_provider_error_handler
    #endif

    #if dg_USE_SODIUM_LIB
        #define CURVE_25519_SODIUM_LIB_INIT             curve_init_25519
        #define CURVE_25519_SODIUM_LIB_COMPUTE_PUBLIC   compute_public_key_25519_sodium
        #define CURVE_25519_SODIUM_LIB_COMPUTE_PRIVATE  compute_private_key_25519
        #define CURVE_25519_SODIUM_LIB_COMPUTE_SHARED   compute_shared_secret_25519_sodium
    #else
        #define CURVE_25519_SODIUM_LIB_INIT             ecc_lib_provider_error_handler
        #define CURVE_25519_SODIUM_LIB_COMPUTE_PUBLIC   ecc_lib_provider_error_handler
        #define CURVE_25519_SODIUM_LIB_COMPUTE_PRIVATE  ecc_lib_provider_error_handler
        #define CURVE_25519_SODIUM_LIB_COMPUTE_SHARED   ecc_lib_provider_error_handler
    #endif

#else
    #define CURVE_25519_HW_ECC_LIB_INIT             ecc_lib_provider_error_handler
    #define CURVE_25519_HW_ECC_LIB_COMPUTE_PUBLIC   ecc_lib_provider_error_handler
    #define CURVE_25519_HW_ECC_LIB_COMPUTE_PRIVATE  ecc_lib_provider_error_handler
    #define CURVE_25519_HW_ECC_LIB_COMPUTE_SHARED   ecc_lib_provider_error_handler
    #define CURVE_25519_SODIUM_LIB_INIT             ecc_lib_provider_error_handler
    #define CURVE_25519_SODIUM_LIB_COMPUTE_PUBLIC   ecc_lib_provider_error_handler
    #define CURVE_25519_SODIUM_LIB_COMPUTE_PRIVATE  ecc_lib_provider_error_handler
    #define CURVE_25519_SODIUM_LIB_COMPUTE_SHARED   ecc_lib_provider_error_handler
#endif





/*
 * This set of definitions is used to formulate the default library providers for each
 * curve and properly map functions implementing ecc primitives.
 */
#define CURVE_SECP160R1_DEFAULT_INIT              CURVE_SECP160R1_UECC_LIB_INIT
#define CURVE_SECP160R1_DEFAULT_COMPUTE_PRIVATE   CURVE_SECP160R1_UECC_LIB_COMPUTE_PRIVATE
#define CURVE_SECP160R1_DEFAULT_COMPUTE_PUBLIC    CURVE_SECP160R1_UECC_LIB_COMPUTE_PUBLIC
#define CURVE_SECP160R1_DEFAULT_COMPUTE_SHARED    CURVE_SECP160R1_UECC_LIB_COMPUTE_SHARED

#if (CURVE_SECP192R1_DEFAULT_LIB_PROVIDER == HW_ECC_ID)
    #define CURVE_SECP192R1_DEFAULT_INIT              CURVE_SECP192R1_HW_ECC_LIB_INIT
    #define CURVE_SECP192R1_DEFAULT_COMPUTE_PRIVATE   CURVE_SECP192R1_HW_ECC_LIB_COMPUTE_PRIVATE
    #define CURVE_SECP192R1_DEFAULT_COMPUTE_PUBLIC    CURVE_SECP192R1_HW_ECC_LIB_COMPUTE_PUBLIC
    #define CURVE_SECP192R1_DEFAULT_COMPUTE_SHARED    CURVE_SECP192R1_HW_ECC_LIB_COMPUTE_SHARED
#elif (CURVE_SECP192R1_DEFAULT_LIB_PROVIDER == UECC_LIB_ID)
    #define CURVE_SECP192R1_DEFAULT_INIT              CURVE_SECP192R1_UECC_LIB_INIT
    #define CURVE_SECP192R1_DEFAULT_COMPUTE_PRIVATE   CURVE_SECP192R1_UECC_LIB_COMPUTE_PRIVATE
    #define CURVE_SECP192R1_DEFAULT_COMPUTE_PUBLIC    CURVE_SECP192R1_UECC_LIB_COMPUTE_PUBLIC
    #define CURVE_SECP192R1_DEFAULT_COMPUTE_SHARED    CURVE_SECP192R1_UECC_LIB_COMPUTE_SHARED
#else
    #error "Requested lib provider either not valid or not currently supported."
#endif


#if (CURVE_SECP224R1_DEFAULT_LIB_PROVIDER == HW_ECC_ID)
    #define CURVE_SECP224R1_DEFAULT_INIT              CURVE_SECP224R1_HW_ECC_LIB_INIT
    #define CURVE_SECP224R1_DEFAULT_COMPUTE_PRIVATE   CURVE_SECP224R1_HW_ECC_LIB_COMPUTE_PRIVATE
    #define CURVE_SECP224R1_DEFAULT_COMPUTE_PUBLIC    CURVE_SECP224R1_HW_ECC_LIB_COMPUTE_PUBLIC
    #define CURVE_SECP224R1_DEFAULT_COMPUTE_SHARED    CURVE_SECP224R1_HW_ECC_LIB_COMPUTE_SHARED
#elif (CURVE_SECP224R1_DEFAULT_LIB_PROVIDER == UECC_LIB_ID)
    #define CURVE_SECP224R1_DEFAULT_INIT              CURVE_SECP224R1_UECC_LIB_INIT
    #define CURVE_SECP224R1_DEFAULT_COMPUTE_PRIVATE   CURVE_SECP224R1_UECC_LIB_COMPUTE_PRIVATE
    #define CURVE_SECP224R1_DEFAULT_COMPUTE_PUBLIC    CURVE_SECP224R1_UECC_LIB_COMPUTE_PUBLIC
    #define CURVE_SECP224R1_DEFAULT_COMPUTE_SHARED    CURVE_SECP224R1_UECC_LIB_COMPUTE_SHARED
#else
    #error "Requested lib provider either not valid or not currently supported."
#endif


#if (CURVE_SECP256R1_DEFAULT_LIB_PROVIDER == HW_ECC_ID)
    #define CURVE_SECP256R1_DEFAULT_INIT              CURVE_SECP256R1_HW_ECC_LIB_INIT
    #define CURVE_SECP256R1_DEFAULT_COMPUTE_PRIVATE   CURVE_SECP256R1_HW_ECC_LIB_COMPUTE_PRIVATE
    #define CURVE_SECP256R1_DEFAULT_COMPUTE_PUBLIC    CURVE_SECP256R1_HW_ECC_LIB_COMPUTE_PUBLIC
    #define CURVE_SECP256R1_DEFAULT_COMPUTE_SHARED    CURVE_SECP256R1_HW_ECC_LIB_COMPUTE_SHARED
#elif (CURVE_SECP256R1_DEFAULT_LIB_PROVIDER == UECC_LIB_ID)
    #define CURVE_SECP256R1_DEFAULT_INIT              CURVE_SECP256R1_UECC_LIB_INIT
    #define CURVE_SECP256R1_DEFAULT_COMPUTE_PRIVATE   CURVE_SECP256R1_UECC_LIB_COMPUTE_PRIVATE
    #define CURVE_SECP256R1_DEFAULT_COMPUTE_PUBLIC    CURVE_SECP256R1_UECC_LIB_COMPUTE_PUBLIC
    #define CURVE_SECP256R1_DEFAULT_COMPUTE_SHARED    CURVE_SECP256R1_UECC_LIB_COMPUTE_SHARED
#else
    #error "Requested lib provider either not valid or not currently supported."
#endif


#define CURVE_SECP256K1_DEFAULT_INIT CURVE_SECP256K1_UECC_LIB_INIT
#define CURVE_SECP256K1_DEFAULT_COMPUTE_PRIVATE CURVE_SECP256K1_UECC_LIB_COMPUTE_PRIVATE
#define CURVE_SECP256K1_DEFAULT_COMPUTE_PUBLIC CURVE_SECP256K1_UECC_LIB_COMPUTE_PUBLIC
#define CURVE_SECP256K1_DEFAULT_COMPUTE_SHARED CURVE_SECP256K1_UECC_LIB_COMPUTE_SHARED


#if (CURVE_25519_DEFAULT_LIB_PROVIDER == HW_ECC_ID)
    #define CURVE_25519_DEFAULT_INIT              CURVE_25519_HW_ECC_LIB_INIT
    #define CURVE_25519_DEFAULT_COMPUTE_PRIVATE   CURVE_25519_HW_ECC_LIB_COMPUTE_PRIVATE
    #define CURVE_25519_DEFAULT_COMPUTE_PUBLIC    CURVE_25519_HW_ECC_LIB_COMPUTE_PUBLIC
    #define CURVE_25519_DEFAULT_COMPUTE_SHARED    CURVE_25519_HW_ECC_LIB_COMPUTE_SHARED
#elif (CURVE_25519_DEFAULT_LIB_PROVIDER == SODIUM_LIB_ID)
    #define CURVE_25519_DEFAULT_INIT              CURVE_25519_SODIUM_LIB_INIT
    #define CURVE_25519_DEFAULT_COMPUTE_PRIVATE   CURVE_25519_SODIUM_LIB_COMPUTE_PRIVATE
    #define CURVE_25519_DEFAULT_COMPUTE_PUBLIC    CURVE_25519_SODIUM_LIB_COMPUTE_PUBLIC
    #define CURVE_25519_DEFAULT_COMPUTE_SHARED    CURVE_25519_SODIUM_LIB_COMPUTE_SHARED
#else
    #error "Requested lib provider either not valid or not currently supported."
#endif


#endif /* SDK_CRYPTO_INCLUDE_CRYPTO_ECC_PROVIDER_FUNCTION_MAP_H_ */
/**
 * \}
 * \}
 */
