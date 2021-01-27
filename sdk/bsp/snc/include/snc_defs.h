/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SNC_DEFS
 *
 * \brief Sensor Node Controller (SNC) definitions
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file snc_defs.h
 *
 * @brief SNC-Header file with Sensor Node Controller definitions
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SNC_DEFS_H_
#define SNC_DEFS_H_


#if dg_configUSE_HW_SENSOR_NODE

#include "sdk_defs.h"
#include "SeNIS.h"

/**
 * \brief Initialized SNC data retained memory attribute
 */
#define _SNC_RETAINED __attribute__((section(".snc_region")))

/*
 * MACRO DEPENDENCIES
 *****************************************************************************************
 */

#include "snc_utils.h"
#include "snc_defs_macros.h"

/*
 * FUNCTION DECLARATIONS
 *****************************************************************************************
 */

/**
 * \brief Macro used in SNC context to return the minimum of two values
 *
 * For a given RAM address, the addressing mode can be either direct (i.e. da()) or
 * indirect (i.e. ia()).
 * For a given Register address, the addressing mode is always direct (i.e. da()).
 *
 * \param [in] op1      (uint32_t: use da() or ia() or build-time-only value)
 *                      first Register/RAM address (contents) or value to be checked for
 *                      the minimum value
 * \param [in] op2      (uint32_t: use da() or ia() or build-time-only value)
 *                      second Register/RAM address (contents) or value to be checked for
 *                      the minimum value
 * \param [out] min     (uint32_t: use da() or ia())
 *                      Register/RAM address to store the minimum value
 *
 */
#define SNC_MIN(op1, op2, min)                                                                  \
        _SNC_MIN(op1, op2, min)

/**
 * \brief Macro used in SNC context to return the maximum of two values
 *
 * For a given RAM address, the addressing mode can be either direct (i.e. da()) or
 * indirect (i.e. ia()).
 * For a given Register address, the addressing mode is always direct (i.e. da()).
 *
 * \param [in] op1      (uint32_t: use da() or ia() or build-time-only value)
 *                      first Register/RAM address (contents) or value to be checked for
 *                      the maximum value
 * \param [in] op2      (uint32_t: use da() or ia() or build-time-only value)
 *                      second Register/RAM address (contents) or value to be checked for
 *                      the maximum value
 * \param [out] max     (uint32_t: use da() or ia())
 *                      Register/RAM address to store the maximum value
 *
 */
#define SNC_MAX(op1, op2, max)                                                                  \
        _SNC_MAX(op1, op2, max)

/**
 * \brief Macro used in SNC context to read a field value.
 *
 * It returns a field value in a Register/RAM address.
 *
 * For a given RAM address, the addressing mode can be either direct (i.e. da()) or
 * indirect (i.e. ia()).
 * For a given Register address, the addressing mode is always direct (i.e. da()).
 *
 * \param [in] field_msk        (uint32_t: build-time-only value)
 *                              the field mask
 * \param [in] field_pos        (uint32_t: build-time-only value)
 *                              the field position
 * \param [in] val_addr         (uint32_t: use da() or ia() or build-time-only value)
 *                              the value or Register/RAM address from which the field value is
 *                              acquired
 * \param [out] field_val       (uint32_t*: use da() or ia())
 *                              Register/RAM address to store the field value
 *
 * e.g.
 * \code{.c}
 * uint32_t tmp;
 * uint32_t counter;
 *
 * SNC_UCODE_BLOCK_DEF(myUcodeVarGetFieldExample)
 * {
 *         ...
 *         SENIS_assign(da(&tmp), da(&CRG_TOP->TRIM_CTRL_REG));
 *         SNC_GET_FIELD(REG_MSK(CRG_TOP, TRIM_CTRL_REG, XTAL_COUNT_N),
 *                 REG_POS(CRG_TOP, TRIM_CTRL_REG, XTAL_COUNT_N),
 *                 da(&tmp), da(&counter));
 *         ...
 * }
 * \endcode
 *
 */
#define SNC_GET_FIELD(field_msk, field_pos, val_addr, field_val)                                \
        _SNC_GET_FIELD(field_msk, field_pos, val_addr, field_val)

/**
 * \brief Macro used in SNC context to clear a field value.
 *
 * It clears a field value in a Register/RAM address.
 *
 * For a given RAM address, the addressing mode can be either direct (i.e. da()) or
 * indirect (i.e. ia()).
 * For a given Register address, the addressing mode is always direct (i.e. da()).
 *
 * \param [in] field_msk        (uint32_t: build-time-only value)
 *                              the field mask
 * \param [in] field_pos        (uint32_t: build-time-only value)
 *                              the field position
 * \param [in] addr             (uint32_t*: use da() or ia())
 *                              the Register/RAM address at which the field is cleared
 *
 * e.g.
 * \code{.c}
 * uint32_t tmp;
 *
 * SNC_UCODE_BLOCK_DEF(myUcodeVarClrFieldExample)
 * {
 *         ...
 *         SENIS_assign(da(&tmp), da(&CRG_TOP->TRIM_CTRL_REG));
 *         SNC_CLR_FIELD(REG_MSK(CRG_TOP, TRIM_CTRL_REG, XTAL_COUNT_N),
 *                 REG_POS(CRG_TOP, TRIM_CTRL_REG, XTAL_COUNT_N), da(&tmp));
 *         SNC_CLR_FIELD(REG_MSK(CRG_TOP, TRIM_CTRL_REG, XTAL_TRIM_SELECT),
 *                 REG_POS(CRG_TOP, TRIM_CTRL_REG, XTAL_TRIM_SELECT), da(&tmp));
 *         SENIS_assign(da(&CRG_TOP->TRIM_CTRL_REG), da(&tmp));
 *         ...
 * }
 * \endcode
 *
 */
#define SNC_CLR_FIELD(field_msk, field_pos, addr)                                               \
        _SNC_CLR_FIELD(field_msk, field_pos, addr)

/**
 * \brief Macro used in SNC context to set a field value.
 *
 * It sets a field value in a Register/RAM address.
 *
 * For a given RAM address, the addressing mode can be either direct (i.e. da()) or
 * indirect (i.e. ia()).
 * For a given Register address, the addressing mode is always direct (i.e. da()).
 *
 * \param [in] field_msk        (uint32_t: build-time-only value)
 *                              the field mask
 * \param [in] field_pos        (uint32_t: build-time-only value)
 *                              the field position
 * \param [in] addr             (uint32_t*: use da() or ia())
 *                              the Register/RAM address at which the field is set
 * \param [in] field_val        (uint32_t: use da() or ia() or build-time-only value)
 *                              the value or Register/RAM address from which the field value is
 *                              acquired
 *
 * e.g.
 * \code{.c}
 * uint32_t tmp;
 *
 * SNC_UCODE_BLOCK_DEF(myUcodeVarSetFieldExample)
 * {
 *         ...
 *         SENIS_assign(da(&tmp), da(&CRG_TOP->TRIM_CTRL_REG));
 *         SNC_SET_FIELD(REG_MSK(CRG_TOP, TRIM_CTRL_REG, XTAL_COUNT_N),
 *                 REG_POS(CRG_TOP, TRIM_CTRL_REG, XTAL_COUNT_N), da(&tmp), 10);
 *         SNC_SET_FIELD(REG_MSK(CRG_TOP, TRIM_CTRL_REG, XTAL_TRIM_SELECT),
 *                 REG_POS(CRG_TOP, TRIM_CTRL_REG, XTAL_TRIM_SELECT), da(&tmp), 2);
 *         SENIS_assign(da(&CRG_TOP->TRIM_CTRL_REG), da(&tmp));
 *         ...
 * }
 * \endcode
 *
 */
#define SNC_SET_FIELD(field_msk, field_pos, addr, field_val)                                    \
        _SNC_SET_FIELD(field_msk, field_pos, addr, field_val)

/**
 * \brief Macro used in SNC context to clear a bit.
 *
 * It clears a bit in a Register/RAM address.
 *
 * For a given RAM address, the addressing mode can be either direct (i.e. da()) or
 * indirect (i.e. ia()).
 * For a given Register address, the addressing mode is always direct (i.e. da()).
 *
 * \param [in] addr             (uint32_t*: use da() or ia())
 *                              the Register/RAM address at which the bit is cleared
 * \param [in] bit_pos          (uint32_t: build-time-only value)
 *                              the bit position
 *
 */
#define SNC_CLR_BIT(addr, bit_pos)                                                              \
        _SNC_CLR_BIT(addr, bit_pos)

/**
 * \brief Macro used in SNC context to set a bit.
 *
 * It sets a bit in a Register/RAM address.
 *
 * For a given RAM address, the addressing mode can be either direct (i.e. da()) or
 * indirect (i.e. ia()).
 * For a given Register address, the addressing mode is always direct (i.e. da()).
 *
 * \param [in] addr             (uint32_t*: use da() or ia())
 *                              the Register/RAM address at which the bit is set
 * \param [in] bit_pos          (uint32_t: build-time-only value)
 *                              the bit position
 *
 */
#define SNC_SET_BIT(addr, bit_pos)                                                              \
        _SNC_SET_BIT(addr, bit_pos)

/**
 * \brief Macro used in SNC context to read a register field value.
 *
 * It returns a register field value (aimed to be used with local variables).
 *
 * For a given RAM address, the addressing mode can be either direct (i.e. da()) or
 * indirect (i.e. ia()).
 * For a given Register address, the addressing mode is always direct (i.e. da()).
 *
 * \param [in] base             the name of the register base
 * \param [in] reg              the register name
 * \param [in] field            the field name
 * \param [in] val_addr         (uint32_t: use da() or ia() or build-time-only value)
 *                              the value or Register/RAM address from which the field value is
 *                              acquired
 * \param [out] field_val       (uint32_t*: use da() or ia())
 *                              Register/RAM address to store the field value
 *
 * e.g.
 * \code{.c}
 * uint32_t tmp;
 * uint32_t counter;
 *
 * SNC_UCODE_BLOCK_DEF(myUcodeRegGetFieldExample)
 * {
 *         ...
 *         SENIS_assign(da(&tmp), da(&CRG_TOP->TRIM_CTRL_REG));
 *         SNC_REG_GET_FIELD(CRG_TOP, TRIM_CTRL_REG, XTAL_COUNT_N, da(&tmp), da(&counter));
 *         ...
 * }
 * \endcode
 *
 */
#define SNC_REG_GET_FIELD(base, reg, field, val_addr, field_val)                                \
        SNC_GET_FIELD(base ## _ ## reg ## _ ## field ## _Msk,                                   \
                base ## _ ## reg ## _ ## field ## _Pos, val_addr, field_val)

/**
 * \brief Macro used in SNC context to clear a register field value.
 *
 * It clears a register field value (aimed to be used with local variables).
 *
 * For a given RAM address, the addressing mode can be either direct (i.e. da()) or
 * indirect (i.e. ia()).
 * For a given Register address, the addressing mode is always direct (i.e. da()).
 *
 * \param [in] base             the name of the register base
 * \param [in] reg              the register name
 * \param [in] field            the field name
 * \param [in] addr             (uint32_t*: use da() or ia())
 *                              the Register/RAM address at which the field is cleared
 *
 * e.g.
 * \code{.c}
 * uint32_t tmp;
 *
 * SNC_UCODE_BLOCK_DEF(myUcodeRegClrFieldExample)
 * {
 *         ...
 *         SENIS_assign(da(&tmp), da(&CRG_TOP->TRIM_CTRL_REG));
 *         SNC_REG_CLR_FIELD(CRG_TOP, TRIM_CTRL_REG, XTAL_COUNT_N, da(&tmp));
 *         SNC_REG_CLR_FIELD(CRG_TOP, TRIM_CTRL_REG, XTAL_TRIM_SELECT, da(&tmp));
 *         SENIS_assign(da(&CRG_TOP->TRIM_CTRL_REG), da(&tmp));
 *         ...
 * }
 * \endcode
 *
 */
#define SNC_REG_CLR_FIELD(base, reg, field, addr)                                               \
        SNC_CLR_FIELD(base ## _ ## reg ## _ ## field ## _Msk,                                   \
                base ## _ ## reg ## _ ## field ## _Pos, addr)

/**
 * \brief Macro used in SNC context to set a register field value.
 *
 * It sets a register field value (aimed to be used with local variables).
 *
 * For a given RAM address, the addressing mode can be either direct (i.e. da()) or
 * indirect (i.e. ia()).
 * For a given Register address, the addressing mode is always direct (i.e. da()).
 *
 * \param [in] base             the name of the register base
 * \param [in] reg              the register name
 * \param [in] field            the field name
 * \param [in] addr             (uint32_t*: use da() or ia())
 *                              the Register/RAM address at which the field is set
 * \param [in] field_val        (uint32_t: use da() or ia() or build-time-only value)
 *                              the value or Register/RAM address from which the field value is
 *                              acquired
 *
 * e.g.
 * \code{.c}
 * uint32_t tmp;
 *
 * SNC_UCODE_BLOCK_DEF(myUcodeRegSetFieldExample)
 * {
 *         ...
 *         SENIS_assign(da(&tmp), da(&CRG_TOP->TRIM_CTRL_REG));
 *         SNC_REG_SET_FIELD(CRG_TOP, TRIM_CTRL_REG, XTAL_COUNT_N, da(&tmp), 10);
 *         SNC_REG_SET_FIELD(CRG_TOP, TRIM_CTRL_REG, XTAL_TRIM_SELECT, da(&tmp), 2);
 *         SENIS_assign(da(&CRG_TOP->TRIM_CTRL_REG), da(&tmp));
 *         ...
 * }
 * \endcode
 *
 */
#define SNC_REG_SET_FIELD(base, reg, field, addr, field_val)                                    \
        SNC_SET_FIELD(base ## _ ## reg ## _ ## field ## _Msk,                                   \
                base ## _ ## reg ## _ ## field ## _Pos, addr, field_val)

/**
 * \brief Macro used in SNC context to read the value of a register field.
 *
 * For a given RAM address, the addressing mode can be either direct (i.e. da()) or
 * indirect (i.e. ia()).
 * For a given Register address, the addressing mode is always direct (i.e. da()).
 *
 * \param [in] base             the name of the register base
 * \param [in] reg              the register name
 * \param [in] field            the field name
 * \param [out] field_val       (uint32_t*: use da() or ia())
 *                              Register/RAM address to store the field value
 *
 * e.g.
 * \code{.c}
 * uint32_t val;
 *
 * SNC_UCODE_BLOCK_DEF(myUcodeRegGetFExample)
 * {
 *         ...
 *         SNC_REG_GETF(CRG_TOP, TRIM_CTRL_REG, XTAL_COUNT_N, da(&val));
 *         ...
 * }
 * \endcode
 *
 */
#define SNC_REG_GETF(base, reg, field, field_val)                                               \
        SNC_GET_FIELD(base ## _ ## reg ## _ ## field ## _Msk,                                   \
                base ## _ ## reg ## _ ## field ## _Pos, da(&base->reg), field_val)

/**
 * \brief Macro used in SNC context to set the value of a register field.
 *
 * For a given RAM address, the addressing mode can be either direct (i.e. da()) or
 * indirect (i.e. ia()).
 * For a given Register address, the addressing mode is always direct (i.e. da()).
 *
 * \param [in] base             the name of the register base
 * \param [in] reg              the register name
 * \param [in] field            the field name
 * \param [in] field_val        (uint32_t: use da() or ia() or build-time-only value)
 *                              the value or Register/RAM address from which the field value is
 *                              acquired
 *
 * e.g.
 * \code{.c}
 * uint32_t new_value;
 *
 * SNC_UCODE_BLOCK_DEF(myUcodeRegSetFExample)
 * {
 *         ...
 *         SNC_REG_SETF(CRG_TOP, TRIM_CTRL_REG, XTAL_COUNT_N, da(&new_value));
 *         ...
 * }
 * \endcode
 *
 */
#define SNC_REG_SETF(base, reg, field, field_val)                                               \
        SNC_SET_FIELD(base ## _ ## reg ## _ ## field ## _Msk,                                   \
                base ## _ ## reg ## _ ## field ## _Pos, da(&base->reg), field_val)

/**
 * \brief Macro used in SNC context to clear a bit of a register.
 *
 * \param [in] base             the name of the register base
 * \param [in] reg              the register name
 * \param [in] field            the field name
 *
 */
#define SNC_REG_CLR_BIT(base, reg, field)                                                       \
        SNC_CLR_BIT(da(&base->reg), base ## _ ## reg ## _ ## field ## _Pos)

/**
 * \brief Macro used in SNC context to set a bit of a register.
 *
 * \param [in] base             the name of the register base
 * \param [in] reg              the register name
 * \param [in] field            the field name
 *
 */
#define SNC_REG_SET_BIT(base, reg, field)                                                       \
        SNC_SET_BIT(da(&base->reg), base ## _ ## reg ## _ ## field ## _Pos)

#endif /* dg_configUSE_HW_SENSOR_NODE */


#endif /* SNC_DEFS_H_ */

/**
 * \}
 * \}
 */
