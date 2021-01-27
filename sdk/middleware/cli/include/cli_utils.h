/**
 * \addtogroup UTILITIES
 * \{
 * \addtogroup CLI_UTILS
 *
 * \brief Command Line Interface Utilities
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file cli_utils.h
 *
 * @brief Declarations for CLI service utilities
 *
 * Copyright (C) 2015-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef CLI_UTILS_H
#define CLI_UTILS_H

#if dg_configUSE_CLI

#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * \brief Verify if given argument is a number or not
 *
 * \param [in] arg      Argument to check
 * \param [out] v       Argument parsed to a number
 *
 * \return true if argument was properly parsed to a number, false otherwise
 */
__STATIC_INLINE bool verify_num(const char *arg, long *v)
{
        char *check_ptr;
        errno = 0;

        *v = strtol(arg, &check_ptr, 0);

        if (errno == ERANGE) {
            return false;
        }

        return (*arg != '\0' && *check_ptr == '\0');
}

/*
 * \brief Verify if given argument is a non-negative number or not
 *
 * \param [in] arg      Argument to check
 * \param [out] v       Argument parsed to a number
 *
 * \return true if argument was properly parsed to a non-negative number, false otherwise
 */
__STATIC_INLINE bool verify_non_neg_num(const char *arg, unsigned long long *v)
{
        char *check_ptr;
        errno = 0;

        /*
         * Check if the argument doesn't include '-' character at the first position, that informs
         * that this is a negative number
         */
        if (arg[0] == '-') {
                return false;
        }

        *v = strtoull(arg, &check_ptr, 0);

        if (errno == ERANGE) {
            return false;
        }

        return (*arg != '\0' && *check_ptr == '\0');
}

/*
 * \brief Parse argument to uint64_t
 *
 * \param [in] arg      Argument to parse
 * \param [out] val     Argument parsed to a number
 *
 * \return true if parsed correctly, false otherwise
 */
__STATIC_INLINE bool parse_u64(const char *arg, uint64_t *val)
{
        unsigned long long buf;

        if (!verify_non_neg_num(arg, &buf)) {
                return false;
        }

        *val = (uint64_t)buf;
        return true;
}

/*
 * \brief Parse argument to uint32_t
 *
 * \param [in] arg      Argument to parse
 * \param [out] val     Argument parsed to a number
 *
 * \return true if parsed correctly, false otherwise
 */
__STATIC_INLINE bool parse_u32(const char *arg, uint32_t *val)
{
        unsigned long long buf;

        if (!verify_non_neg_num(arg, &buf)) {
                return false;
        }

        if (buf > ULONG_MAX) {
                return false;
        }

        *val = (uint32_t)buf;
        return true;
}

/*
 * \brief Parse argument to uint16_t
 *
 * \param [in] arg      Argument to parse
 * \param [out] val     Argument parsed to a number
 *
 * \return true if parsed correctly, false otherwise
 */
__STATIC_INLINE bool parse_u16(const char *arg, uint16_t *val)
{
        unsigned long long buf;

        if (!verify_non_neg_num(arg, &buf)) {
                return false;
        }

        if (buf > USHRT_MAX) {
                return false;
        }

        *val = (uint16_t)buf;
        return true;
}

/*
 * \brief Parse argument to uint8_t
 *
 * \param [in] arg      Argument to parse
 * \param [out] val     Argument parsed to a number
 *
 * \return true if parsed correctly, false otherwise
 */
__STATIC_INLINE bool parse_u8(const char *arg, uint8_t *val)
{
        unsigned long long buf;

        if (!verify_non_neg_num(arg, &buf)) {
                return false;
        }

        if (buf > UCHAR_MAX) {
                return false;
        }

        *val = (uint8_t)buf;
        return true;
}

/*
 * \brief Parse argument to int16_t
 *
 * \param [in] arg      Argument to parse
 * \param [out] val     Argument parsed to a number
 *
 * \return true if parsed correctly, false otherwise
 */
__STATIC_INLINE bool parse_16(const char *arg, int16_t *val)
{
        long buf = 0;

        if (!verify_num(arg, &buf)) {
                return false;
        }

        if (buf > SHRT_MAX) {
                return false;
        }

        *val = (int16_t)buf;
        return true;
}

/*
 * \brief Parse argument to bool
 *
 * \param [in] arg      Argument to parse
 * \param [out] val     Argument parsed to a boolean value
 *
 * \return true if parsed correctly and number has proper value, false otherwise
 */
__STATIC_INLINE bool parse_bool(const char *arg, bool *val)
{
        unsigned long long buf;

        if (!verify_non_neg_num(arg, &buf)) {
                return false;
        }

        /* Valid values are only 0 (false) or 1 (true) */
        if (buf > 1) {
                return false;
        }

        *val = (bool)buf;
        return true;
}

/*
 * \brief Parse argument to size_t
 *
 * \param [in] arg      Argument to parse
 * \param [out] val     Argument parsed to a number
 *
 * \return true if parsed correctly, false otherwise
 */
__STATIC_INLINE bool parse_size_t(const char *arg, size_t *val)
{
        unsigned long long buf;

        if (!verify_non_neg_num(arg, &buf)) {
                return false;
        }

        if (sizeof(size_t) == 4 && buf > ULONG_MAX) {
                return false;
        }

        *val = (size_t)buf;
        return true;
}

#ifdef __cplusplus
}
#endif

#endif /* dg_configUSE_CLI */

#endif /* CLI_UTILS_H */

/**
 * \}
 * \}
 */
