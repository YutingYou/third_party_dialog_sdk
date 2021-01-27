/**
 * \addtogroup BSP
 * \{
 * \addtogroup INTERFACES
 * \{
 * \addtogroup SECURITY_TOOLBOX
 * \{
 * \addtogroup HMAC
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file crypto_ecdh.c
 *
 * @brief ECDH API implementation.
 *
 * Copyright (C) 2016-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <string.h>
#include "crypto_ecdh.h"
#include "sys_trng.h"
#include "ad_crypto.h"

#include "crypto_ec.h"

#if !dg_configCRYPTO_ADAPTER
#error dg_configCRYPTO_ADAPTER macro must be set to (1) in order to use ECDH.
#endif

#if (CRYPTO_ECDH_DO_NOT_USE_CURVE25519 == 1) && (CRYPTO_ECDH_USE_ONLY_CURVE25519 == 1)
#error Conflicting configuration macros for ECDH module.
#endif

#define CRYPTO_ECDH_ENABLE_CALCR2       (1 << ECC_ECC_COMMAND_REG_ECC_CalcR2_Pos)
#define CRYPTO_ECDH_ENABLE_SIGNB        (1 << ECC_ECC_COMMAND_REG_ECC_SignB_Pos)

__STATIC_INLINE void _crypto_ecdh_cleanup(void)
{
#if dg_USE_HW_ECC
        ad_crypto_disable_ecc_event();
        if (ad_crypto_release_ecc() != OS_OK) {
                /* This means that the resource was acquired by a different task or under ISR context.
                   The code should not reach here normally. */
                OS_ASSERT(0);
        }
#endif
}

CRYPTO_ECDH_RET crypto_ecdh_compute(crypto_ecdh_context_t *ctx, OS_TICK_TIME timeout)
{
        unsigned int flags = 0; /* This variable will hold operations performed within this call */
        if (!ctx) {
                return CRYPTO_ECDH_RET_ER;
        }
#if dg_USE_HW_ECC
        /* Enable engine clock and adapter event handling */
        if (ad_crypto_acquire_ecc(timeout) != OS_MUTEX_TAKEN) {
                return CRYPTO_ECDH_RET_TO;
        }

        ad_crypto_enable_ecc_event();
#endif
        /* Compute private key */
        if (!(ctx->flags & CRYPTO_ECDH_CTX_d)) {
                if (crypto_ecc_compute_private_key(&ctx->curve, ctx->d)!=CRYPTO_ECC_PROVIDER_RET_OK) {
                        _crypto_ecdh_cleanup();
                        return CRYPTO_ECDH_RET_ER;
                }

                flags |= CRYPTO_ECDH_CTX_d;
                ctx->flags |= CRYPTO_ECDH_CTX_d;
                ctx->flags &= ~CRYPTO_ECDH_CTX_Ql; /* If a new d is computed, previous Q is not valid */
        }

        /* Compute public key */
        if (!(ctx->flags & CRYPTO_ECDH_CTX_Ql)) {
                if (crypto_ecc_compute_public_key(&ctx->curve, ctx->d, &ctx->Ql[0][0])!=CRYPTO_ECC_PROVIDER_RET_OK) {
                        _crypto_ecdh_cleanup();
                        return CRYPTO_ECDH_RET_ER;
                }

                flags |= CRYPTO_ECDH_CTX_Ql;
                ctx->flags |= CRYPTO_ECDH_CTX_Ql;
        }

        /* Check for peer's public key availability. If not available there is nothing more to do so return. */
        if (!(ctx->flags & CRYPTO_ECDH_CTX_Qp)) {
                _crypto_ecdh_cleanup();
                return CRYPTO_ECDH_RET_MP;
        }

        /* Compute shared secret */
        if (!(ctx->flags & CRYPTO_ECDH_CTX_s)) {

                if (crypto_ecc_compute_shared_secret(&ctx->curve, ctx->d, &ctx->Qp[0][0], ctx->s)!=CRYPTO_ECC_PROVIDER_RET_OK) {
                        _crypto_ecdh_cleanup();
                        return CRYPTO_ECDH_RET_ER;
                }
                ctx->flags |= CRYPTO_ECDH_CTX_s;
        }
        _crypto_ecdh_cleanup();
        return CRYPTO_ECDH_RET_OK;
}

CRYPTO_ECDH_RET crypto_ecdh_init_context(crypto_ecdh_context_t *ctx, CRYPTO_ECC_CURVE curve_type)
{
        /*create curve based on requested curve type and curve provider library*/
        if (crypto_ecc_curve_init(&ctx->curve, curve_type) != CRYPTO_ECC_PROVIDER_RET_OK) {
                return CRYPTO_ECDH_RET_ER;
        }
        /*Initialize ecdh context*/
        memset(ctx->d, 0, 32);/**< Our private key. */
        memset(ctx->Ql, 0, 64);/**< Our public key (x coordinate in [0], y coordinate in [1]). */
        memset(ctx->Qp, 0, 64);/**< The peer's public key (x coordinate in [0], y coordinate in [1]). */
        memset(ctx->s, 0, 32);/**< The shared secret. */
        ctx->flags = 0;/**< ECDH context flags (::crypto_ecdh_context_flags). */
        return CRYPTO_ECDH_RET_OK;
}

/**
 * \}
 * \}
 * \}
 * \}
 */

