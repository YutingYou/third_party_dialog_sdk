/**
 * \addtogroup PLA_BSP_SYSTEM
 * \{
 * \addtogroup BSP_SUOTA
 * \{
 */

/**
 *****************************************************************************************
 *
 * @file suota_security_ext.h
 *
 * @brief SUOTA security extension definitions.
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *****************************************************************************************
 */

#ifndef SUOTA_SECURITY_EXT_H_
#define SUOTA_SECURITY_EXT_H_

#include <stdint.h>

/** Security section type */
#define SECURITY_HDR_TYPE_SECURITY_SECTION              0x22AA
/** Signature section type */
#define SECURITY_HDR_TYPE_SIGNATURE_SECTION             0x33AA
/** Device administration section type */
#define SECURITY_HDR_TYPE_DEVICE_ADMIN_SECTION          0x44AA
/** Key revocation record type */
#define SECURITY_HDR_TYPE_KEY_REVOCATION_RECORD         0x55AA
/** Firmware version number type */
#define SECURITY_HDR_TYPE_FW_VERSION_NUMBER             0x66AA
/** Rollback prevention segment type (new minimum firmware version) */
#define SECURITY_HDR_TYPE_ROLLBACK_PREVENTION_SEGMENT   0x77AA

/**
 * Security section content - mainly configuration of the signature verification algorithm
 */
typedef struct {
        /** Asymmetric public key ID (key index or OTP address) */
        uint32_t public_key_id;
        /** Signature generation mode: ECDSA/EdDSA*/
        uint8_t mode;
        /** Elliptic curve */
        uint8_t curve;
        /** Hash method */
        uint8_t hash;
} __attribute__((packed)) suota_security_header_t;

/**
 * Security section content for DA1469x devices
 */
typedef struct {
        /** Public key index (used for signature verification) */
        uint8_t ecc_key_idx;
        /** Symmetric key index (used for executable decryption) */
        uint8_t sym_key_idx;
        /** Nonce (used for executable decryption) */
        uint8_t nonce[8];
} __attribute__((packed)) suota_security_header_da1469x_t;

/**
 * FW version number
 */
typedef struct {
        /** The most significant part of the FW version */
        uint16_t major;
        /** The second significant part of the FW version */
        uint16_t minor;
} __attribute__((packed)) security_hdr_fw_version_t;

/**
 * Digital signature generation/verification algorithm
 */
typedef enum {
        /** Elliptic Curve Digital Signature Algorithm */
        SECURITY_HDR_MODE_ECDSA         = 0x01,
        /** Edwards-curve Digital Signature Algorithm */
        SECURITY_HDR_MODE_EDDSA,
} __attribute__((packed)) security_hdr_mode_t;

/**
 * Elliptic curve
 *
 * \note Edwards 25519 curve is used in EdDSA only.
 *
 */
typedef enum {
        /** 192-bits NIST curve */
        SECURITY_HDR_ECC_CURVE_SECP192R1        = 0x01,
        /** 224-bits NIST curve */
        SECURITY_HDR_ECC_CURVE_SECP224R1,
        /** 256-bits NIST curve */
        SECURITY_HDR_ECC_CURVE_SECP256R1,
        /** Edwards 25519 curve */
        SECURITY_HDR_ECC_CURVE_EDWARDS25519,
} __attribute__((packed)) security_hdr_ecc_curve_t;

/**
 * Hash method
 */
typedef enum {
        /** SHA 224 */
        SECURITY_HDR_HASH_SHA_224       = 0x01,
        /** SHA 256 */
        SECURITY_HDR_HASH_SHA_256,
        /** SHA 384 */
        SECURITY_HDR_HASH_SHA_384,
        /** SHA 512 */
        SECURITY_HDR_HASH_SHA_512,
} __attribute__((packed)) security_hdr_hash_t;

/**
 * Key type
 */
typedef enum {
        /** Asymmetric, public key used in signature verification (index only) */
        SECURITY_HDR_KEY_TYPE_SIGNATURE   = 0xA1,
        /** Symmetric key used in executable decryption (index only) */
        SECURITY_HDR_KEY_TYPE_DECRYPTION  = 0xA2,
        /** Symmetric key used in user data encryption (index only) */
        SECURITY_HDR_KEY_TYPE_USER_DATA   = 0xA3,
        /** Asymmetric, public key used in signature verification (index or address) */
        SECURITY_HDR_KEY_TYPE_PUBLIC      = 0xA4,
        /** Symmetric key used in user data encryption (index or address) */
        SECURITY_HDR_KEY_TYPE_SYMMETRIC   = 0xA5,
} __attribute__((packed)) security_hdr_key_type_t;

#endif /* SUOTA_SECURITY_EXT_H_ */

/**
 * \}
 * \}
 */
