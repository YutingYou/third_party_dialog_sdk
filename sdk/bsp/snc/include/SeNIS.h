/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SNC_SENIS
 *
 * \brief SNC-Sensor Node Instruction Set (SeNIS) -based framework
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file SeNIS.h
 *
 * @brief SNC-Sensor Node Instruction Set (SeNIS) -based framework for implementing uCodes
 *        header file
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SENIS_H_
#define SENIS_H_


#if dg_configUSE_HW_SENSOR_NODE

#include <stdint.h>

/*
 * ENUMERATION, DATA TYPE AND STRUCTURE DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief Macros that define the addressing mode inside a SeNIS-based construct
 *        (i.e. SENIS_[construct], where "construct" can be "if", "while", "dowhile" etc.)
 *
 * There are three addressing modes:
 * da(addr)     is used for defining direct addressing for a given address "addr" passed as
 *              argument to a SeNIS construct, meaning that the value of the address being passed
 *              is an address to either System RAM or a Register
 * ia(ind_addr) is used for defining indirect addressing for a given address "ind_addr" passed as
 *              argument to a SeNIS construct, meaning that the value of the address being passed
 *              is a pointer to a memory space in which the requested address resides
 *              (Note: indirect addressing for a Register address is NOT ALLOWED)
 * l(label_idx) is used for defining direct addressing for a given address implied by a label name
 *              (see SENIS_label)
 *
 */
#define da(addr)        (D, (uint32_t*)(addr))
#define ia(ind_addr)    (I, (uint32_t*)(ind_addr))
#define l(label_idx)    (D, (uint32_t*)(&_SNC_UCODE(__senis_custom_labels[label_idx])))

/**
 * \brief SeNIS condition operator type
 *
 */
typedef enum {
        LT,             /**< Less-Than condition operator type                  */
        LTEQ,           /**< Less-Than-EQual condition operator type            */
        GT,             /**< Greater-Than condition operator type               */
        GTEQ,           /**< Greater-Than-EQual condition operator type         */
        EQ,             /**< EQual condition operator type                      */
        NEQ,            /**< Not-EQual condition operator type                  */
        BIT,            /**< set-BIT-position value condition operator type     */
        NBIT,           /**< Non-set-BIT-position value condition operator type */
} SENIS_COND_TYPE;

#include "snc.h"

/*
 * FUNCTION DECLARATIONS
 *****************************************************************************************
 */

//==================== SeNIS label definition functions ========================

/**
 * \brief Function used in SNC context to create a list of SeNIS labels
 *
 * \param [in] ... labels       comma separated list of label names
 *
 * Example usage:
 * \code{.c}
 * SNC_UCODE_BLOCK_DEF(myUcodeLabelExample)
 * {
 *         SENIS_labels(label1, label2, label3);
 *         ...
 *         SENIS_label(label1);
 *         ...
 *         SENIS_label(label2);
 *         ...
 *         SENIS_label(label3);
 *         ...
 * }
 * \endcode
 *
 * \sa SENIS_label
 *
 */
#define SENIS_labels(...)                                                                       \
        _SENIS_labels(__VA_ARGS__)

/**
 * \brief Function used in SNC context to place a SeNIS label in uCode implementation
 *        (label must be previously defined inside a SENIS_labels statement)
 *
 * \param [in] label            the name of the label to be placed in the uCode
 *
 * Example usage:
 * \code{.c}
 * uint32_t var;
 *
 * SNC_UCODE_BLOCK_DEF(myUcodeLabelExample)
 * {
 *         SENIS_labels(label1, label2);
 *         ...
 *         SENIS_wadad(da(&SNC->SNC_STATUS_REG), da(&var));
 *
 *         SENIS_label(label1);
 *
 *         SENIS_cobr_eq(l(label2));
 *         SENIS_wadva(da(&var), 5);
 *
 *         SENIS_label(label2);
 *         ...
 * }
 * \endcode
 *
 * \sa SENIS_labels
 *
 */
#define SENIS_label(label)                                                                      \
        _SENIS_label(label)

//==================== SeNIS main functions ====================================

/**
 * \brief Macro used in SNC context for calling a "NOP", i.e. a SeNIS ASM("NOP") equivalent
 *
 */
#define SENIS_nop                                                                               \
        _SENIS_nop

/**
 * \brief Function used in SNC context to store the contents of a Register/RAM address
 *        to another one
 *
 * In case RAM addresses are given, either direct or indirect addressing can be defined.
 * In direct addressing mode (i.e. da()) the contents of the given RAM addresses are changed/copied,
 * while in indirect addressing mode (i.e. ia()) the contents of the RAM addresses indicated by
 * the contents of the given RAM addresses are changed/copied.
 * In case Register addresses are given, the addressing mode can be only direct (i.e. da()).
 *
 * \param [in] dst_addr         (uint32_t*: use da() or ia())
 *                              destination Register/RAM address
 * \param [in] src_addr         (uint32_t*: use da() or ia())
 *                              source Register/RAM address
 *
 */
#define SENIS_wadad(dst_addr, src_addr)                                                         \
        _SENIS_wadad(dst_addr, src_addr)

/**
 * \brief Function used in SNC context to store a value to a destination Register/RAM address
 *
 * For the given RAM address either direct or indirect addressing can be defined.
 * In direct addressing mode (i.e. da()) the contents of the given RAM address are changed,
 * while in indirect addressing mode (i.e. ia()) the contents of the RAM address indicated by
 * the contents of the given RAM address are changed.
 * In case a Register address is given, the addressing mode can be only direct (i.e. da()).
 *
 * \param [in] dst_addr         (uint32_t*: use da() or ia())
 *                              destination Register/RAM address
 * \param [in] value            (uint32_t: build-time-only value)
 *                              value to be stored
 *
 */
#define SENIS_wadva(dst_addr, value)                                                            \
        _SENIS_wadva(dst_addr, value)

/**
 * \brief Function used in SNC context to XOR the contents of a Register/RAM address with
 *        a bit mask value and store the result back to the particular address
 *
 * For the given Register/RAM address, the addressing mode can be only direct (i.e. da()).
 *
 * \param [in] dst_addr         (uint32_t*: use da())
 *                              destination Register/RAM address
 *                              (only direct addressing is supported)
 * \param [in] bit_mask_value   (uint32_t: build-time-only value)
 *                              bit mask value that the contents of the
 *                              dst_addr are to be XORed with
 *
 */
#define SENIS_tobre(dst_addr, bit_mask_value)                                                   \
        _SENIS_tobre(dst_addr, bit_mask_value)

/**
 * \brief Function used in SNC context to compare the contents of a Register/RAM address against
 *        a value that has '1' at the given bit position
 *
 * If the contents of that address have '1' at the given bit position, then EQ_FLAG
 * in SNC_STATUS_REG will be set.
 * For the given Register/RAM address, the addressing mode can be only direct (i.e. da()).
 *
 * \param [in] addr             (uint32_t*: use da())
 *                              Register/RAM address whose contents are compared
 *                              (only direct addressing is supported)
 * \param [in] bit_pos_value    (uint32_t: build-time-only value)
 *                              bit position to check (0-31)
 *
 */
#define SENIS_rdcbi(addr, bit_pos_value)                                                        \
        _SENIS_rdcbi(addr, bit_pos_value)

/**
 * \brief Function used in SNC context to compare the contents of a Register/RAM address
 *        against another one
 *
 * If the contents of the first address have greater value than the contents of the second one,
 * then GR_FLAG in SNC_STATUS_REG will be set.
 * For the given Register/RAM addresses, the addressing mode can be only direct (i.e. da()).
 *
 * \param [in] addr             (uint32_t*: use da())
 *                              Register/RAM address whose contents are compared
 *                              (only direct addressing is supported)
 * \param [in] gt_addr          (uint32_t*: use da())
 *                              Register/RAM address whose contents are compared to the first address
 *                              (only direct addressing is supported)
 *
 */
#define SENIS_rdcgr(addr, gt_addr)                                                              \
        _SENIS_rdcgr(addr, gt_addr)

/**
 * \brief Function used in SNC context to branch/jump to a specific RAM address based on
 *        EQ_FLAG status in SNC_STATUS_REG
 *
 * It checks the value of EQ_FLAG. If it is set, then a branch/jump is performed either to
 * the given RAM address when direct addressing is used (i.e. da()) or the RAM address
 * indicated by the contents of the given RAM address when indirect addressing is used (i.e. ia()).
 *
 * \param [in] addr             (uint32_t*: use da() or ia())
 *                              RAM address where the SNC execution flow will branch/jump to
 *                              using either direct or indirect addressing
 *
 */
#define SENIS_cobr_eq(addr)                                                                     \
        _SENIS_cobr_eq(addr)

/**
 * \brief Function used in SNC context to branch/jump to a specific RAM address based on
 *        GR_FLAG status in SNC_STATUS_REG
 *
 * It checks the value of GR_FLAG. If it is set, then a branch/jump is performed either to
 * to the given RAM address when direct addressing is used (i.e. da()) or the RAM address
 * indicated by the contents of the given RAM address when indirect addressing is used (i.e. ia()).
 *
 * \param [in] addr             (uint32_t*: use da() or ia())
 *                              RAM address where the SNC execution flow will branch/jump to
 *                              using either direct or indirect addressing
 *
 */
#define SENIS_cobr_gr(addr)                                                                     \
        _SENIS_cobr_gr(addr)

/**
 * \brief Function used in SNC context to branch/jump to a specific RAM address for a
 *        specific number of times
 *
 * It checks the value of the internal counter maintained in SNC HW for the loop-flavor of "COBR"
 * instruction. If it is 0, then it is set to the given value (i.e. cnt_value). If finally the
 * value of the internal counter is greater than 0, then it is decremented by 1 and a branch/jump
 * is performed to the given RAM address.
 * Assuming internal counter is resembled by a static integer variable called "internal_cnt" and
 * the RAM address to branch to (i.e. addr) is referenced by the label "LABEL_addr", then
 * the equivalent implementation in C language is shown below:
 *
 * \code{.c}
 * if (!internal_cnt) {
 *         internal_cnt = cnt_value;
 * }
 * if (internal_cnt > 0) {
 *         internal_cnt--;
 *         goto LABEL_addr;
 * }
 * \endcode
 *
 * For the given RAM address, the addressing mode can be only direct (i.e. da()).
 * Use SENIS_cobr_loop_reset() function when the internal counter is to be reset.
 *
 * \param [in] addr             (uint32_t*: use da())
 *                              RAM address where the SNC execution flow will branch/jump to
 * \param [in] cnt_value        (uint8_t: build-time-only value)
 *                              number of times the branch/jump to the given RAM address
 *                              will occur (0-127)
 *
 * * Example usage:
 * \code{.c}
 * SNC_UCODE_BLOCK_DEF(myUcodeCobrLoopExample)
 * {
 *         SENIS_labels(label1, label2);
 *         ...
 *         // Performing SENIS_nop 10+1 times
 *         SENIS_label(label1);
 *         SENIS_nop;
 *         SENIS_cobr_loop(l(label1), 10);
 *         ...
 * }
 * \endcode
 *
 * \sa SENIS_cobr_loop_reset
 *
 */
#define SENIS_cobr_loop(addr, cnt_value)                                                        \
        _SENIS_cobr_loop(addr, cnt_value)

/**
 * \brief Function used in SNC context to increment the contents of a RAM address by one
 *
 * For the given RAM address, the addressing mode can be only direct (i.e. da()).
 *
 * \param [in] dst_addr         (uint32_t*: use da())
 *                              RAM address whose contents are incremented
 *
 */
#define SENIS_inc1(dst_addr)                                                                    \
        _SENIS_inc1(dst_addr)

/**
 * \brief Function used in SNC context to increment the contents of a RAM address by four
 *
 * For the given RAM address, the addressing mode can be only direct (i.e. da()).
 *
 * \param [in] dst_addr         (uint32_t*: use da())
 *                              RAM address whose contents are incremented
 *
 */
#define SENIS_inc4(dst_addr)                                                                    \
        _SENIS_inc4(dst_addr)

/**
 * \brief Function used in SNC context to perform a delay in low power clock ticks (max. 255)
 *
 * \param [in] delay_value      (uint8_t: build-time-only value)
 *                              delay in low power clock ticks, where a tick is an internal of an
 *                              8-bit timer running on the low power clock.
 *                              Maximum value 255 ticks (0xFF).
 *
 */
#define SENIS_del(delay_value)                                                                  \
        _SENIS_del(delay_value)

/**
 * \brief Macro used in SNC context to designate the end of SNC execution
 *
 * It notifies the Power Domains Controller to power down the Sensor Node Controller and
 * possibly set the system in sleep mode.
 *
 */
#define SENIS_slp                                                                               \
        _SENIS_slp

//==================== SeNIS main functions extensions =========================

/**
 * \brief Function used in SNC context to compare the contents of a Register/RAM address
 *        against 0
 *
 * If the contents of the given Register/RAM address have a value greater than 0,
 * then GR_FLAG in SNC_STATUS_REG of SNC will be set.
 * For the given Register/RAM address, the addressing mode can be only direct (i.e. da()).
 *
 * \param [in] addr             (uint32_t*: use da())
 *                              Register/RAM address whose contents are compared to 0
 *
 */
#define SENIS_rdcgr_z(addr)                                                                     \
        _SENIS_rdcgr_z(addr)

/**
 * \brief Function used in SNC context to reset the internal counter maintained in SNC HW for the
 *        loop-flavor of "COBR" instruction (i.e. SENIS_cobr_loop())
 *
 * Typically this function is used when SNC execution flow branches/jumps earlier (i.e. before the
 * internal counter reaches 0) out of the loop defined by SENIS_cobr_loop() function.
 * In that case, if the internal counter is not reset, the next time a SENIS_cobr_loop()
 * instruction is to be called, the internal counter continues from the previous non-zero value.
 *
 * \sa SENIS_cobr_loop
 *
 */
#define SENIS_cobr_loop_reset()                                                                 \
        _SENIS_cobr_loop_reset()

//==================== SeNIS extension for delay related functions =============

/**
 * \brief Function used in SNC context to perform a delay in low power clock ticks (max. 36460)
 *
 * This function is actually an expansion of SENIS_del() in terms of greater maximum
 * limit in the input delay parameter value.
 *
 * \param [in] delay_ticks      (uint32_t: build-time-only value)
 *                              delay in low power clock ticks
 *
 * \sa SENIS_del
 *
 */
#define SENIS_del_lp_clk(delay_ticks)                                                           \
        _SENIS_del_lp_clk(delay_ticks)

/**
 * \brief Function used in SNC context to perform a delay in milliseconds (max. 1020)
 *
 * It must be used only when XTAL32K low power clock is enabled.
 *
 * \param [in] delay_ms         (uint32_t: build-time-only value)
 *                              delay in milliseconds
 *
 */
#define SENIS_del_ms(delay_ms)                                                                  \
        _SENIS_del_ms(delay_ms)

//==================== SeNIS extension for C-like clauses ======================

/**
 * \brief Function used in SNC context to store to a Register/RAM address either a value or the
 *        contents of another Register/RAM address
 *
 * This is a SeNIS equivalent to C language simple assignment ("=") statement.
 * For a given RAM address, the addressing mode can be either direct (i.e. da()) or
 * indirect (i.e. ia()).
 * For a given Register address, the addressing mode can be only direct (i.e. da()).
 *
 * \param [in] op1      (uint32_t*: use da() or ia())
 *                      destination Register/RAM address where data will be assigned
 * \param [in] op2      (uint32_t: use da() or ia() or build-time-only value)
 *                      source Register/RAM address or value
 *
 * In case op2 is a Register/RAM address, op1 will be assigned with the data residing in that address
 * Example - x = y; :
 * \code{.c}
 * {
 *         ...
 *         SENIS_assign(da(&x), da(&y));
 *         ...
 * }
 * \endcode
 *
 * In case op2 is a value, op1 will be just assigned with the given value
 * Example - x = 0xCAFE; :
 * \code{.c}
 * {
 *         ...
 *         SENIS_assign(da(&x), 0xCAFE);
 *         ...
 * }
 * \endcode
 *
 */
#define SENIS_assign(op1, op2)                                                                  \
        _SENIS_assign(op1, op2)

/**
 * \brief Function used in SNC context to perform a XOR on the contents of a Register/RAM address
 *        with the contents of another Register/RAM address or value
 *
 * This is a SeNIS equivalent to C language bitwise-XOR-assignment ("^=") statement.
 * For a given RAM address, the addressing mode can be either direct (i.e. da()) or
 * indirect (i.e. ia()).
 * For a given Register address, the addressing mode can be only direct (i.e. da()).
 *
 * \param [in] op1      (uint32_t*: use da() or ia())
 *                      destination Register/RAM address where data will be XORed
 * \param [in] op2      (uint32_t: use da() or ia() or build-time-only value)
 *                      Register/RAM address (contents) or value to be XORed to op1
 *
 * In case op2 is a Register/RAM address defined for op2, op1 contents will be XORed with the
 * contents residing in op2 address (either using direct or indirect addressing mode).
 * Example - x ^= y; :
 * \code{.c}
 * {
 *         ...
 *         SENIS_xor(da(&x), da(&y));
 *         ...
 * }
 * \endcode
 *
 * In case op2 is a value, op1 contents will be XORed with the given op2 value
 * Example - x ^= 0xCAFE; :
 * \code{.c}
 * {
 *         ...
 *         SENIS_xor(da(&x), 0xCAFE);
 *         ...
 * }
 * \endcode
 *
 */
#define SENIS_xor(op1, op2)                                                                     \
        _SENIS_xor(op1, op2)

/**
 * \brief In SNC context, this is a SeNIS equivalent to C language "if" clause
 *
 * The SeNIS language implementation of an "if" clause.
 *
 * General Syntax:
 *
 * SENIS_if ({op1_t}({op1}) [, {cond}, {op2_t}({op2})]) {
 *         // Code to execute if SENIS_if conditional expression evaluates to true
 * }
 *
 * where:
 * op1_t: is either ia, da or nothing if op1 is a value
 * op1  : is an address if ia or da prefixes are used, else a value
 * op2_t: is either ia, da or nothing if op2 is a value
 * op2  : is an address if ia or da prefixes are used, else a value
 * cond : GTEQ(>=), GT(>), LTEQ(<=), LT(<), EQ(==), NEQ(!=),
 *        BIT  (set-bit condition     => true if in the given bit position (in op2) of the
 *                                       contents of op1 the value is "1",
 *                                       being equivalent to:
 *                                       "if (op1 & (1 << op2)) {...}")
 *        NBIT (non-set-bit condition => true if in the given bit position (in op2) of the
 *                                       contents of op1 the value is "0",
 *                                       being equivalent to:
 *                                       "if (!(op1 & (1 << op2))) {...}")
 *
 * In case BIT or NBIT condition is used, op2 must be an uint32_t build-time-only value.
 *
 * Example - variable cond variable:
 * if (a == b) a++;
 * \code{.c}
 * uint32_t a, b;
 *
 * {
 *         ...
 *         SENIS_if (da(&a), EQ, da(&b)) {
 *                 SENIS_inc1(da(&a));
 *         }
 *         ...
 * }
 * \endcode
 *
 * Example - variable cond value:
 * if (a > 0xAA) a = 0xAA;
 * \code{.c}
 * uint32_t a;
 *
 * {
 *         ...
 *         SENIS_if (da(&a), GT, 0xA) {
 *                 SENIS_assign(da(&a), 0xAA);
 *         }
 *         ...
 * }
 * \endcode
 *
 * Example - variable:
 * if (a) a = 0xAA;
 * \code{.c}
 * uint32_t a;
 *
 * {
 *         ...
 *         SENIS_if (da(&a)) {
 *                 SENIS_assign(da(&a), 0xAA);
 *         }
 *         ...
 * }
 * \endcode
 *
 * Example - always true:
 * if (1) a = 0xAA;
 * \code{.c}
 * uint32_t a;
 *
 * {
 *         ...
 *         SENIS_if (1) {
 *                 SENIS_assign(da(&a), 0xAA);
 *         }
 *         ...
 * }
 * \endcode
 *
 */
#define SENIS_if(...)                                                                           \
        _SENIS_if(__VA_ARGS__)

/**
 * \brief In SNC context, this is a SeNIS equivalent to C language "else" clause
 *
 * The SeNIS language implementation of an "else" clause.
 * It must be used inside a SENIS_if clause block.
 *
 * General Syntax:
 *
 * SENIS_if (conditional expression) {
 *         // Code to execute inside "if" clause block if conditional expression evaluates to true
 * SENIS_else {
 *         // Code to execute inside "else" clause block if conditional expression evaluates to false
 * }}
 *
 * Example:
 * if (a == b) {a++; a++;}
 * else {b++; b++; b++;}
 * \code{.c}
 * uint32_t a, b;
 *
 * {
 *         ...
 *         SENIS_if (da(&a), EQ, da(&b)) {
 *                 SENIS_inc1(da(&a));
 *                 SENIS_inc1(da(&a));
 *         SENIS_else {
 *                 SENIS_inc1(da(&b));
 *                 SENIS_inc1(da(&b));
 *                 SENIS_inc1(da(&b));
 *         }}
 *         ...
 * }
 * \endcode
 *
 */
#define SENIS_else                                                                              \
        _SENIS_else

/**
 * \brief In SNC context, this is SeNIS equivalent to C language "while" clause
 *
 * The SeNIS language implementation of a "while" clause.
 *
 * General Syntax:
 *
 * SENIS_while ({op1_t}({op1}) [, {cond}, {op2_t}({op2})]) {
 *         // Code to execute inside "while" clause
 * }
 *
 * where:
 * op1_t: is either ia, da or nothing if op1 is a value
 * op1  : is an address if ia or da prefixes are used, else a value
 * op2_t: is either ia, da or nothing if op2 is a value
 * op2  : is an address if ia or da prefixes are used, else a value
 * cond : GTEQ(>=), GT(>), LTEQ(<=), LT(<), EQ(==), NEQ(!=),
 *        BIT  (set-bit condition     => true if in the given bit position (in op2) of the
 *                                       contents of op1 the value is "1",
 *                                       being equivalent to:
 *                                       "while (op1 & (1 << op2)) {...}")
 *        NBIT (non-set-bit condition => true if in the given bit position (in op2) of the
 *                                       contents of op1 the value is "0",
 *                                       being equivalent to:
 *                                       "while (!(op1 & (1 << op2))) {...}")
 *
 * In case BIT or NBIT condition is used, op2 must be an uint32_t build-time-only value.
 *
 * Example:
 * while (a != 256) {a += 4; a += 4;}
 * \code{.c}
 * uint32_t a;
 *
 * {
 *         ...
 *         SENIS_while (da(&a), NEQ, 256) {
 *                 SENIS_inc4(da(&a));
 *                 SENIS_inc4(da(&a));
 *         }
 *         ...
 * }
 * \endcode
 *
 * Example:
 * while (a) {a += 4; a += 4;}
 * \code{.c}
 * uint32_t a;
 *
 * {
 *         ...
 *         SENIS_while (da(&a)) {
 *                 SENIS_inc4(da(&a));
 *                 SENIS_inc4(da(&a));
 *         }
 *         ...
 * }
 * \endcode
 *
 */
#define SENIS_while(...)                                                                        \
        _SENIS_while(__VA_ARGS__)

/**
 * \brief In SNC context, this is a SeNIS equivalent to C language "do-while" clause
 *
 * The SeNIS language implementation of a "do-while" clause.
 *
 * General Syntax:
 *
 * SENIS_dowhile ({op1_t}({op1}) [, {cond}, {op2_t}({op2})]) {
 *         // Code to execute inside "do-while" clause
 * }
 *
 * where:
 * op1_t: is either ia, da or nothing if op1 is a value
 * op1  : is an address if ia or da prefixes are used, else a value
 * op2_t: is either ia, da or nothing if op2 is a value
 * op2  : is an address if ia or da prefixes are used, else a value
 * cond : GTEQ(>=), GT(>), LTEQ(<=), LT(<), EQ(==), NEQ(!=),
 *        BIT  (set-bit condition     => true if in the given bit position (in op2) of the
 *                                       contents of op1 the value is "1",
 *                                       being equivalent to:
 *                                       "do {...} while (op1 & (1 << op2));)
 *        NBIT (non-set-bit condition => true if in the given bit position (in op2) of the
 *                                       contents of op1 the value is "0",
 *                                       being equivalent to:
 *                                       "do {...} while (!(op1 & (1 << op2)));)
 *
 * In case BIT or NBIT condition is used, op2 must be an uint32_t build-time-only value.
 *
 * Example:
 * do {a += 4; a += 4;} while (a != 256);
 * \code{.c}
 * uint32_t a;
 *
 * {
 *         ...
 *         SENIS_dowhile (da(&a), NEQ, 256) {
 *                 SENIS_inc4(da(&a));
 *                 SENIS_inc4(da(&a));
 *         }
 *         ...
 * }
 * \endcode
 *
 * Example:
 * do {a += 4; a += 4;} while (a);
 * \code{.c}
 * uint32_t a;
 *
 * {
 *         ...
 *         SENIS_dowhile (da(&a)) {
 *                 SENIS_inc4(da(&a));
 *                 SENIS_inc4(da(&a));
 *         }
 *         ...
 * }
 * \endcode
 *
 */
#define SENIS_dowhile(...)                                                                      \
        _SENIS_dowhile(__VA_ARGS__)

/**
 * \brief In SNC context, this is a SeNIS equivalent to C language "goto" statement
 *
 * It will jump/go-to the given uCode address or label (i.e. defined by SENIS_labels() and
 * SENIS_label() constructs).
 *
 * \param [in] addr             (uint32_t*: use da() or ia() for branching to an address and
 *                               l() for branching to a label)
 *                              uCode address or label to which SNC execution flow will "go-to",
 *                              using either direct or indirect addressing
 *
 * Example:
 * Label: {
 *         a += 4;
 *         a += 4;
 *         goto Label;
 * }
 * \code{.c}
 * uint32_t a;
 *
 * SNC_UCODE_BLOCK_DEF(myUcodeGotoExample)
 * {
 *         SENIS_labels(Label);
 *         ...
 *         SENIS_label(Label); {
 *                 SENIS_inc4(da(&a));
 *                 SENIS_inc4(da(&a));
 *                 SENIS_goto(l(Label));
 *         }
 *         ...
 * }
 * \endcode
 *
 */
#define SENIS_goto(addr)                                                                        \
        _SENIS_goto(addr)

/**
 * \brief Function used in SNC context to perform an addition to the contents of a given
 *        Register/RAM address
 *
 * This is a SeNIS equivalent to C language addition-assignment ("+=") statement.
 * For a given RAM address, the addressing mode can be either direct (i.e. da()) or
 * indirect (i.e. ia()).
 * For a given Register address, the addressing mode is always direct (i.e. da()).
 *
 * \param [in] op1              (uint32_t*: use da() or ia())
 *                              Register/RAM address the contents of which are to be incremented
 * \param [in] op2              (uint32_t: use da() or ia() or build-time-only value)
 *                              Register/RAM address contents or value indicating the value to
 *                              be added to the contents of op1
 *
 * In case of a Register/RAM address defined for op2, the value indicated in the contents residing
 * in op2 address (either using direct or indirect addressing mode) will be added to the
 * op1 contents.
 * Example - x += y; :
 * \code{.c}
 * {
 *         ...
 *         SENIS_add(da(&x), da(&y));
 *         ...
 * }
 * \endcode
 *
 * In case of a value, the given op2 value will be added to the op1 contents
 * Example - x += 8; :
 * \code{.c}
 * {
 *         ...
 *         SENIS_add(da(&x), 8);
 *         ...
 * }
 * \endcode
 *
 * \sa SENIS_inc1
 * \sa SENIS_inc4
 *
 */
#define SENIS_add(op1, op2)                                                                     \
        _SENIS_add(op1, op2)

/**
 * \brief Function used in SNC context to perform an subtraction from the contents of a given
 *        Register/RAM address
 *
 * This is a SeNIS equivalent to C language subtraction-assignment ("-=") statement.
 * For a given RAM address, the addressing mode can be either direct (i.e. da()) or
 * indirect (i.e. ia()).
 * For a given Register address, the addressing mode is always direct (i.e. da()).
 *
 * \param [in] op1              (uint32_t*: use da() or ia())
 *                              Register/RAM address the contents of which are to be decremented
 * \param [in] op2              (uint32_t: use da() or ia() or build-time-only value)
 *                              Register/RAM address contents or value indicating the value to
 *                              be subtracted from the contents of op1
 *
 * In case of a Register/RAM address defined for op2, the value indicated in the contents residing
 * in op2 address (either using direct or indirect addressing mode) will be subtracted from the
 * op1 contents.
 * Example - x -= y; :
 * \code{.c}
 * {
 *         ...
 *         SENIS_sub(da(&x), da(&y));
 *         ...
 * }
 * \endcode
 *
 * In case of a value, the given op2 value will be subtracted from the op1 contents
 * Example - x -= 8; :
 * \code{.c}
 * {
 *         ...
 *         SENIS_sub(da(&x), 8);
 *         ...
 * }
 * \endcode
 *
 */
#define SENIS_sub(op1, op2)                                                                     \
        _SENIS_sub(op1, op2)

/**
 * \brief Function used in SNC context to perform a Right-Shift over the contents of a given
 *        Register/RAM address
 *
 * This is a SeNIS equivalent to C language bitwise-right-shift-assignment (">>=") statement.
 * For a given RAM address, the addressing mode can be either direct (i.e. da()) or
 * indirect (i.e. ia()).
 * For a given Register address, the addressing mode is always direct (i.e. da()).
 *
 * \param [in] op1      (uint32_t*: use da() or ia())
 *                      Register/RAM address the contents of which are to be shifted
 * \param [in] op2      (uint32_t: use da() or ia() or build-time-only value)
 *                      Register/RAM address contents or value indicating the number of bits to shift
 *
 * In case of a Register/RAM address defined for op2, op1 contents will be shifted with the
 * contents residing in op2 address (either using direct or indirect addressing mode).
 * Example - x >>= y; :
 * \code{.c}
 * {
 *         ...
 *         SENIS_rshift(da(&x), da(&y));
 *         ...
 * }
 * \endcode
 *
 * In case of a value, op1 contents will be shifted with the given op2 value
 * Example - x >>= 8; :
 * \code{.c}
 * {
 *         ...
 *         SENIS_rshift(da(&x), 8);
 *         ...
 * }
 * \endcode
 *
 * \sa SENIS_rshift_masked
 *
 */
#define SENIS_rshift(op1, op2)                                                                  \
        _SENIS_rshift(op1, op2)

/**
 * \brief Function used in SNC context to perform a masked Right-Shift over the contents of a given
 *        Register/RAM address
 *
 * For a given RAM address, the addressing mode can be either direct (i.e. da()) or
 * indirect (i.e. ia()).
 * For a given Register address, the addressing mode is always direct (i.e. da()).
 *
 * \param [in] op1              (uint32_t*: use da() or ia())
 *                              Register/RAM address the contents of which are to be shifted
 * \param [in] op2              (uint32_t: use da() or ia() or build-time-only value)
 *                              Register/RAM address contents or value indicating the number of
 *                              bits to shift
 * \param [in] bit_msk_len      (uint32_t: use da() or ia() or build-time-only value)
 *                              length of the bit-mask to be applied to the Right-Shift result value
 *
 * In case of a Register/RAM address defined for op2, op1 contents will be shifted with the contents
 * residing in op2 address and then masked based on the given bit_msk_len value
 * (either using direct or indirect addressing mode).
 * Example - x = ((x >> y) & ((bit_msk_len) ? ((1 << bit_msk_len) - 1) : 0xffffffff)); :
 * \code{.c}
 * {
 *         ...
 *         SENIS_rshift_masked(da(&x), da(&y), da(&bit_msk_len));
 *         ...
 * }
 * \endcode
 *
 * In case of a value, op1 contents will be shifted with the given op2 value
 * Example - x = ((x >> 8) & ((5) ? ((1 << 5) - 1) : 0xffffffff));
 *        or x = ((x & >> 8) & 0x11111); :
 * \code{.c}
 * {
 *         ...
 *         SENIS_rshift_masked(da(&x), 8, 5);
 *         ...
 * }
 * \endcode
 *
 * \sa SENIS_rshift
 *
 */
#define SENIS_rshift_masked(op1, op2, bit_msk_len)                                              \
        _SENIS_rshift_masked(op1, op2, bit_msk_len)

/**
 * \brief Function used in SNC context to perform a Left-Shift over the contents of a given
 *        Register/RAM address
 *
 * This is a SeNIS equivalent to C language bitwise-left-shift-assignment ("<<=") statement.
 * For a given RAM address, the addressing mode can be either direct (i.e. da()) or
 * indirect (i.e. ia()).
 * For a given Register address, the addressing mode is always direct (i.e. da()).
 *
 * \param [in] op1              (uint32_t*: use da() or ia())
 *                              Register/RAM address the contents of which are to be shifted
 * \param [in] op2              (uint32_t: use da() or ia() or build-time-only value)
 *                              Register/RAM address contents or value indicating the number of
 *                              bits to shift
 *
 * In case of a Register/RAM address defined for op2, op1 contents will be shifted with the
 * contents residing in op2 address (either using direct or indirect addressing mode).
 * Example - x <<= y; :
 * \code{.c}
 * {
 *         ...
 *         SENIS_lshift(da(&x), da(&y));
 *         ...
 * }
 * \endcode
 *
 * In case of a value, op1 contents will be shifted with the given op2 value
 * Example - x <<= 8; :
 * \code{.c}
 * {
 *         ...
 *         SENIS_lshift(da(&x), 8);
 *         ...
 * }
 * \endcode
 *
 * \sa SENIS_lshift_masked
 *
 */
#define SENIS_lshift(op1, op2)                                                                  \
        _SENIS_lshift(op1, op2)

/**
 * \brief Function used in SNC context to perform a masked Left-Shift over the contents of a given
 *        Register/RAM address
 *
 * For a given RAM address, the addressing mode can be either direct (i.e. da()) or
 * indirect (i.e. ia()).
 * For a given Register address, the addressing mode is always direct (i.e. da()).
 *
 * \param [in] op1              (uint32_t*: use da() or ia())
 *                              Register/RAM address the contents of which are to be shifted
 * \param [in] op2              (uint32_t: use da() or ia() or build-time-only value)
 *                              Register/RAM address contents or value indicating the number of
 *                              bits to shift
 * \param [in] bit_msk_len      (uint32_t: use da() or ia() or build-time-only value)
 *                              length of the bit-mask to be applied to the shifted value (i.e. op1)
 *
 * In case of a Register/RAM address defined for op2, op1 contents will be masked based on the
 * given bit_msk_len value and shifted with the contents residing in op2 address (either using
 * direct or indirect addressing mode).
 * Example - x = ((x & ((bit_msk_len) ? ((1 << bit_msk_len) - 1) : 0xffffffff)) << y); :
 * \code{.c}
 * {
 *         ...
 *         SENIS_lshift_masked(da(&x), da(&y), da(&bit_msk_len));
 *         ...
 * }
 * \endcode
 *
 * In case of a value, op1 contents will be shifted with the given op2 value
 * Example - x = ((x & ((5) ? ((1 << 5) - 1) : 0xffffffff)) << 8);
 *        or x = ((x & 0b11111) << 8); :
 * \code{.c}
 * {
 *         ...
 *         SENIS_lshift_masked(da(&x), 8, 5);
 *         ...
 * }
 * \endcode
 *
 * \sa SENIS_lshift
 *
 */
#define SENIS_lshift_masked(op1, op2, bit_msk_len)                                              \
        _SENIS_lshift_masked(op1, op2, bit_msk_len)

/**
 * \brief In SNC context, this is a SeNIS equivalent to C language "break" construct
 *
 * It will break its innermost SENIS_while() {...} or SENIS_dowhile() {...} loop.
 *
 * Example:
 * while (1) {a += 4; a += 4; if (a > 8) break;}
 * \code{.c}
 * uint32_t a;
 *
 * {
 *         ...
 *         SENIS_while (1) {
 *                 SENIS_inc4(da(&a));
 *                 SENIS_inc4(da(&a));
 *                 SENIS_if (da(&a), GT, 8) {
 *                         SENIS_break;
 *                 }
 *         }
 *         ...
 * }
 * \endcode
 *
 */
#define SENIS_break                                                                             \
        _SENIS_break

/**
 * \brief In SNC context, this is a SeNIS equivalent to C language "continue" construct
 *
 * It will continue its innermost SENIS_while() {...} or SENIS_dowhile() {...} loop.
 *
 * Example:
 * while (b <= 4) {a += 4; a += 4; if (a < 16) continue; b += 4;}
 * \code{.c}
 * uint32_t a, b;
 *
 * {
 *         ...
 *         SENIS_while (da(&b), LTEQ, 4) {
 *                 SENIS_inc4(da(&a));
 *                 SENIS_inc4(da(&a));
 *                 SENIS_if (da(&a), LT, 16) {
 *                         SENIS_continue;
 *                 }
 *                 SENIS_inc4(da(&b));
 *          }
 *          ...
 * }
 * \endcode
 *
 */
#define SENIS_continue                                                                          \
        _SENIS_continue

/**
 * \brief In SNC context, this is a SeNIS equivalent to C language memcpy() function
 *
 * It copies 32bit words from a given RAM address to another one.
 *
 * Two modes are supported based on the addressing mode of dst_addr and src_addr addresses
 * being passed.
 * Direct addressing is used for passing the address of a buffer (i.e. da(buf1)), while
 * indirect addressing is used for passing an indirect address to a buffer (i.e. ia(&buf2_ind))
 * which is internally increased, so that it can point to the next word to access.
 *
 * \param [in] dst_addr         (uint32_t*: use da() or ia())
 *                              destination RAM address of the copied data
 * \param [in] src_addr         (uint32_t*: use da() or ia())
 *                              source RAM address of the data to copy
 * \param [in] len              (uint32_t: use da() or ia() or build-time-only value)
 *                              data length in 32bit words to copy
 *
 * Example:
 * memcpy(buf1, buf2, 4 * sizeof(uint32_t));
 * \code{.c}
 * uint32_t buf1[4], buf2[4];
 * uint32_t* buf2_ind = &buf2[0];
 *
 * {
 *         ...
 *         SENIS_copy(da(buf1), ia(&buf2_ind), sizeof(buf1) / sizeof(uint32_t));
 *         // buf2_ind becomes equal to &buf2[4]
 *         ...
 * }
 * \endcode
 *
 */
#define SENIS_copy(dst_addr, src_addr, len)                                                     \
        _SENIS_copy(dst_addr, src_addr, len)

/**
 * \brief In SNC context, this is a SeNIS equivalent to C language memcmp() function
 *
 * It compares 32bit words starting from a given RAM address to 32bit words starting from
 * another RAM address.
 *
 * Two modes are supported based on the addressing mode of addr and cmp_addr addresses
 * being passed.
 * Direct addressing is used for passing the address of a buffer (i.e. da(buf1)), while
 * indirect addressing is used for passing an indirect address to a buffer (i.e. ia(&buf2_ind))
 * which is internally increased, so that it can point to the next word to access.
 *
 * \param [in]  addr            (uint32_t*: use da() or ia())
 *                              pointer to the 1st address
 * \param [in]  cmp_addr        (uint32_t*: use da() or ia())
 *                              pointer to the 2nd address
 * \param [in]  len             (uint32_t: use da() or ia() or build-time-only value)
 *                              the length in 32bit words to be compared
 * \param [out] rtn_addr        (uint32_t*: use da() or ia())
 *                              pointer to a variable where the returned result is to be stored
 *                              (0 if they match, 1 if not)
 *
 * Example:
 * rslt = memcmp(buf1, buf2, 4 * sizeof(uint32_t));
 * \code{.c}
 * uint32_t buf1[4] = {1}, buf2[4] = {1};
 * uint32_t* buf1_ind = &buf1[0];
 * uint32_t* buf2_ind = &buf2[0];
 * uint32_t rslt;
 *
 * {
 *         ...
 *         SENIS_compare(ia(&buf1_ind), ia(&buf2_ind), sizeof(buf1) / sizeof(uint32_t), da(&rslt));
 *         // buf1_ind becomes equal to &buf1[4], buf2_ind becomes equal to &buf2[4] and
 *         // rslt becomes equal to 0
 *         ...
 * }
 * \endcode
 *
 */
#define SENIS_compare(addr, cmp_addr, len, rtn_addr)                                            \
        _SENIS_compare(addr, cmp_addr, len, rtn_addr)

//==================== Returning-from-an-SNC-function ==========================

/**
 * \brief In SNC context, this is a SeNIS equivalent to C language "return" construct
 *
 * It is used when returning from a uCode function is needed.
 * Adding SENIS_return at the end of a uCode definition is optional.
 *
 * Example:
 * \code{.c}
 * uint32_t a;
 *
 * SNC_UCODE_BLOCK_DEF(myUcodeReturnExample)
 * {
 *         ...
 *         SENIS_if (da(&a)) {
 *                 SENIS_return;
 *         }
 *         ...
 *         SENIS_return; // optional
 * }
 * \endcode
 *
 */
#define SENIS_return                                                                            \
        _SENIS_return

#endif /* dg_configUSE_HW_SENSOR_NODE */


#endif /* SENIS_H_ */

/**
 * \}
 * \}
 */
