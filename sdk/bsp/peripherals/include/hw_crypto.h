/**
 * \addtogroup PLA_DRI_CRYPTO
 * \{
 * \addtogroup CRYPTO Crypto Engine Interrupts
 * \{
 * \brief Interrupt handling for the crypto engines (AES/HASH, ECC)
 */

/**
 ****************************************************************************************
 *
 * @file hw_crypto.h
 *
 * @brief Interrupt handling API for the AES/Hash and ECC Engines.
 *
 * Copyright (C) 2016-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifndef CRYPTO_H_
#define CRYPTO_H_


#if (dg_configUSE_HW_AES_HASH || dg_configUSE_HW_ECC)

#include "sdk_defs.h"

/**
 * \brief Crypto engine callback
  *
 * This function type is used for a callback called when the AES/HASH engine generates an interrupt.
 *
 * \param [in] status The status register of AES/HASH at the time of the interrupt.
  */
typedef void (*hw_crypto_cb)(unsigned int status);

/**
 * \brief Enable interrupt for AES/HASH crypto engine.
 *
 * \param [in] cb A callback to be called when interrupt occurs. It must always be provided.
 *
 * \note AES/HASH engine and ECC engine are common sources of CRYPTO system interrupt. This
 *       function only enables CRYPTO interrupt itself and registers a callback for AES/HASH
 *       related CRYPTO interrupts. In order to fully enable AES/HASH interrupts
 *       hw_aes_hash_enable_interrupt_source() must also be called
 */
void hw_crypto_enable_aes_hash_interrupt(hw_crypto_cb cb);


/**
 * \brief Disable interrupt for AES/HASH crypto engine.
 */
void hw_crypto_disable_aes_hash_interrupt(void);


/**
 * \brief Clear pending interrupt from AES/HASH engine.
 *
 * \note This function clears the pending CRYPTO interrupt only on the NVI Controller.
 *       Use hw_aes_hash_clear_interrupt_req() to clear the source of CRYPTO interrupt on
 *       the AES/HASH engine.
 */
__STATIC_INLINE void hw_crypto_clear_pending_interrupt(void)
{
        NVIC_ClearPendingIRQ(CRYPTO_IRQn);
}

#endif /* (dg_configUSE_HW_AES_HASH || dg_configUSE_HW_ECC) */
#endif /* CRYPTO_H_ */
/**
 * \}
 * \}
 */
