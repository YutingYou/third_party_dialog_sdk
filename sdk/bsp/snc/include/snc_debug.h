/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SNC_DEBUG
 *
 * \brief Debugging in SNC context
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file snc_debug.h
 *
 * @brief SNC-Debugging Sensor Node Controller (SNC) API
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SNC_DEBUG_H_
#define SNC_DEBUG_H_


#if dg_configUSE_HW_SENSOR_NODE

#if dg_configUSE_SNC_DEBUGGER

#include "snc_defs.h"

/*
 * MACRO DEPENDENCIES
 *****************************************************************************************
 */

#include "snc_debug_macros.h"

/*
 * FUNCTION DECLARATIONS
 *****************************************************************************************
 */

/**
 * \brief Macro used in SNC context to define and control an SNC breakpoint group
 *
 * When using this macro, an SNC breakpoint group is defined.
 *
 * If a breakpoint is set to the particular macro, then all SNC breakpoints and step-by step
 * debugging regions defined for that group (e.g. foo_group), using SNC_BKPT(foo_group) and
 * SNC_STEP_BY_STEP_[BEGIN/END](foo_group), respectively, are enabled, resulting in both SNC and
 * SYSCPU (CM33) halting when the SNC execution flow reaching SNC breakpoints.
 *
 * Specifically, use:
 *  - "step-return":    to show the current SeNIS construct command being executed in a uCode, and
 *  - "resume":         to move to the next SeNIS construct command
 *
 * \param [in] bkpt_group       the name of the SNC breakpoint group defined
 *
 * \sa SNC_BKPT
 * \sa SNC_STEP_BY_STEP
 * \sa SNC_STEP_BY_STEP_BEGIN
 * \sa SNC_STEP_BY_STEP_END
 *
 */
#define SNC_BKPT_GROUP(bkpt_group)                                                              \
        _SNC_BKPT_GROUP(bkpt_group)

/**
 * \brief Macro used in SNC context to define and control the default SNC breakpoint group
 *
 * When using this macro, the default SNC breakpoint group is defined.
 *
 * If a breakpoint is set to the particular macro, then all SNC breakpoints and step-by step
 * debugging regions defined for the default group (i.e. snc_bkpt_group_dflt), using
 * SNC_BKPT() and SNC_STEP_BY_STEP_[BEGIN/END]() macros with empty bkpt_group attribute,
 * respectively, are enabled, resulting in both SNC and SYSCPU (CM33) halting when the
 * SNC execution flow reaching SNC breakpoints.
 *
 * Specifically, use:
 *  - "step-return":    to show the current SeNIS construct command being executed in a uCode, and
 *  - "resume":         to move to the next SeNIS construct command
 *
 * \sa SNC_BKPT
 * \sa SNC_STEP_BY_STEP
 * \sa SNC_STEP_BY_STEP_BEGIN
 * \sa SNC_STEP_BY_STEP_END
 *
 */
#define SNC_BKPT_GROUP_DFLT()                                                                   \
        _SNC_BKPT_GROUP_DFLT()

#if dg_configUSE_HW_SENSOR_NODE_EMU
/**
 * \brief Macro used in SNC Emulator context to define and control SNC Emulator breakpoint group
 *        enabling step-by-step debugging over SeNIS construct commands
 *
 * When using this macro, an SNC Emulator breakpoint group is defined.
 *
 * If a breakpoint is set to the particular macro, then all SNC Emulator breakpoints defined for
 * that group (i.e. snc_bkpt_group_emu) are enabled, resulting in both SNC Emulator and
 * SYSCPU (CM33) halting for every SeNIS construct command being executed.
 *
 * Unless an SNC breakpoint group is defined for a SeNIS construct command,
 * an SNC Emulator step-by-step breakpoint is defined.
 *
 * Specifically, use:
 *  - "step-return":    to show the current SeNIS construct command being executed in a uCode, and
 *  - "resume":         to move to the next SeNIS construct command
 *
 */
#define SNC_BKPT_GROUP_EMU()                                                                    \
        _SNC_BKPT_GROUP_EMU()
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */

/**
 * \brief Macro used in SNC context to define an SNC breakpoint in uCode implementation
 *
 * When using this macro, the SNC context variable snc_bkpt will be filled with the local
 * function pointer defined for the particular breakpoint in uCode implementation which enables
 * the breakpoint control and then the Sensor Node handler will be triggered.
 *
 * If a breakpoint is set to the position where the corresponding SNC breakpoint group
 * is defined using SNC_BKPT_GROUP() macro, then all SNC breakpoints defined for that group
 * are enabled, resulting in both SNC and SYSCPU (CM33) halting when the SNC execution flow
 * reaching those SNC breakpoints.
 *
 * If no group is defined, then default group is considered (i.e. snc_bkpt_group_dflt), which is
 * defined using SNC_BKPT_GROUP_DFLT() macro.
 *
 * SNC (Emulator or not) execution is blocked until the SNC context variable snc_bkpt is cleared
 * inside the Sensor Node Handler.
 *
 * \param [in] bkpt_group       the name of the SNC breakpoint group the particular breakpoint
 *                              belongs to
 *
 * Example usage:
 * \code{.c}
 * SNC_BKPT_GROUP(bkpt_group1);                 // Definition of bkpt_group1 breakpoint group
 * SNC_BKPT_GROUP(bkpt_group2);                 // Definition of bkpt_group2 breakpoint group
 *
 * uint32_t foo_var1 = 0;                       // Definition of the variable foo_var1
 * uint32_t foo_var2 = 0;                       // Definition of the variable foo_var2
 * uint32_t foo_var3 = 0;                       // Definition of the variable foo_var3
 *
 * SNC_UCODE_BLOCK_DEF(foo_ucode)               // Definition of foo_ucode uCode-Block
 * {
 *         ...
 *         SNC_BKPT(bkpt_group1);               // SNC breakpoint of bkpt_group1 group
 *         SENIS_assign(da(&foo_var1), 0);      // Access foo_var1 in SNC context
 *         ...
 *         SNC_BKPT(bkpt_group1);               // SNC breakpoint of bkpt_group1 group
 *         SENIS_assign(da(&foo_var2), 0);      // Access foo_var2 in SNC context
 *         SNC_BKPT(bkpt_group2);               // SNC breakpoint of bkpt_group2 group
 *         ...
 *         SNC_BKPT();                          // SNC breakpoint of snc_bkpt_group_dflt group
 *         SENIS_assign(da(&foo_var3), 0);      // Access foo_var3 in SNC context
 *         SNC_BKPT();                          // SNC breakpoint of snc_bkpt_group_dflt group
 *         ...
 * }
 * \endcode
 *
 * \sa SNC_BKPT_GROUP
 * \sa SNC_BKPT_GROUP_DFLT
 * \sa snc_context_t
 *
 */
#define SNC_BKPT(bkpt_group)                                                                    \
        _SNC_BKPT(bkpt_group)

/**
 * \brief Macro used in SNC context to define the starting position and control an
 *        SNC step-by-step debugging region over SeNIS construct commands
 *
 * When using this macro, the starting position of an SNC step-by-step debugging region over
 * the included SeNIS construct commands in a uCode is defined. Step-by-step debugging is
 * controlled through the corresponding SNC breakpoint group definition, i.e. SNC_BKPT_GROUP().
 *
 * If a breakpoint is set to the position where the corresponding SNC breakpoint group
 * is defined using SNC_BKPT_GROUP() macro, then step-by-step debugging is enabled over the
 * included SeNIS construct commands where SNC_STEP_BY_STEP() macro is used, resulting in both SNC
 * and SYSCPU (CM33) halting for every SeNIS construct command being executed.
 *
 * If no group is defined, then default group is considered (i.e. snc_bkpt_group_dflt), which is
 * defined using SNC_BKPT_GROUP_DFLT() macro.
 *
 * The end of that region is defined using the SNC_STSP_BY_STEP_END() macro.
 *
 * \param [in] bkpt_group       the name of the SNC breakpoint group the particular step-by-step
 *                              debugging region belongs to
 *
 * Example usage:
 * \code{.c}
 * SNC_BKPT_GROUP(bkpt_group1);                 // Definition of bkpt_group1 breakpoint group
 * SNC_BKPT_GROUP(bkpt_group2);                 // Definition of bkpt_group2 breakpoint group
 *
 * uint32_t foo_array[10] = {0};                // Definition of the array foo_array
 *
 * SNC_UCODE_BLOCK_DEF(foo_ucode)               // Definition of foo_ucode uCode-Block
 * {
 *         ...
 *         SNC_STEP_BY_STEP_BEGIN(bkpt_group1); // Starting position of SNC step-by-step
 *                                              // debugging region of bkpt_group1 group
 *         SENIS_assign(da(&foo_array[0]), 0);  // Access foo_array[0] in SNC context
 *         SENIS_inc1(da(&foo_array[0]));       // Access foo_array[0] in SNC context
 *         SENIS_assign(da(&foo_array[1]), 0);  // Access foo_array[1] in SNC context
 *         SNC_STEP_BY_STEP_END(bkpt_group1);   // Position where SNC step-by-step debugging
 *                                              // region of bkpt_group1 group stops
 *         ...
 *         SNC_STEP_BY_STEP_BEGIN(bkpt_group2); // Starting position of SNC step-by-step
 *                                              // debugging region of bkpt_group2 group
 *         ...
 *         SNC_STEP_BY_STEP_END(bkpt_group2);   // Position where SNC step-by-step debugging
 *                                              // region of bkpt_group2 group stops
 *         ...
 *         SNC_STEP_BY_STEP_BEGIN();            // Starting position of SNC step-by-step
 *                                              // debugging region of snc_bkpt_group_dflt group
 *         ...
 *         SNC_STEP_BY_STEP_END();              // Position where SNC step-by-step debugging
 *                                              // region of snc_bkpt_group_dflt group stops
 *         ...
 * }
 * \endcode
 *
 * \sa SNC_BKPT_GROUP
 * \sa SNC_BKPT_GROUP_DFLT
 * \sa SNC_STEP_BY_STEP
 * \sa SNC_STEP_BY_STEP_END
 *
 */
#define SNC_STEP_BY_STEP_BEGIN(bkpt_group)                                                      \
        _SNC_STEP_BY_STEP_BEGIN(bkpt_group)

/**
 * \brief Macro used in SNC context to define the end of an SNC step-by-step debugging region
 *        over SeNIS construct commands
 *
 * When using this macro, the end of an SNC step-by-step debugging region over
 * the included SeNIS construct commands in a uCode is defined. After the position of that macro
 * step-by-step debugging is not anymore controlled through the corresponding SNC breakpoint group
 * definition, i.e. SNC_BKPT_GROUP().
 *
 * If no group is defined, then default group is considered (i.e. snc_bkpt_group_dflt), which is
 * defined using SNC_BKPT_GROUP_DFLT() macro.
 *
 * \param [in] bkpt_group       the name of the SNC breakpoint group the particular step-by-step
 *                              debugging region belongs to
 *
 * Example usage:
 * \code{.c}
 * SNC_BKPT_GROUP(bkpt_group1);                 // Definition of bkpt_group1 breakpoint group
 * SNC_BKPT_GROUP(bkpt_group2);                 // Definition of bkpt_group2 breakpoint group
 *
 * uint32_t foo_array[10] = {0};                // Definition of the array foo_array
 *
 * SNC_UCODE_BLOCK_DEF(foo_ucode)               // Definition of foo_ucode uCode-Block
 * {
 *         ...
 *         SNC_STEP_BY_STEP_BEGIN(bkpt_group1); // Starting position of SNC step-by-step
 *                                              // debugging region of bkpt_group1 group
 *         SENIS_assign(da(&foo_array[0]), 0);  // Access foo_array[0] in SNC context
 *         SENIS_inc1(da(&foo_array[0]));       // Access foo_array[0] in SNC context
 *         SENIS_assign(da(&foo_array[1]), 0);  // Access foo_array[1] in SNC context
 *         SNC_STEP_BY_STEP_END(bkpt_group1);   // Position where SNC step-by-step debugging
 *                                              // region of bkpt_group1 group stops
 *         ...
 *         SNC_STEP_BY_STEP_BEGIN(bkpt_group2); // Starting position of SNC step-by-step
 *                                              // debugging region of bkpt_group2 group
 *         ...
 *         SNC_STEP_BY_STEP_END(bkpt_group2);   // Position where SNC step-by-step debugging
 *                                              // region of bkpt_group2 group stops
 *         ...
 *         SNC_STEP_BY_STEP_BEGIN();            // Starting position of SNC step-by-step
 *                                              // debugging region of snc_bkpt_group_dflt group
 *         ...
 *         SNC_STEP_BY_STEP_END();              // Position where SNC step-by-step debugging
 *                                              // region of snc_bkpt_group_dflt group stops
 *         ...
 * }
 * \endcode
 *
 * \sa SNC_BKPT_GROUP
 * \sa SNC_BKPT_GROUP_DFLT
 * \sa SNC_STEP_BY_STEP
 * \sa SNC_STEP_BY_STEP_BEGIN
 *
 */
#define SNC_STEP_BY_STEP_END(bkpt_group)                                                        \
        _SNC_STEP_BY_STEP_END(bkpt_group)

/**
 * \brief Macro used in SNC context to define an SNC step-by-step breakpoint in uCode implementation,
 *        enabling step-by-step debugging
 *
 * When using this macro inside an explicitly defined SNC step-by-step debugging region
 * (i.e. using SNC_STEP_BY_STEP_[BEGIN/END]() macros), the SNC context variable snc_bkpt will be
 * filled with the local function pointer defined for the particular breakpoint in uCode
 * implementation which enables the breakpoint control and then the Sensor Node handler will be
 * triggered.
 *
 * When using this macro in SNC Emulator context and outside an explicitly defined SNC step-by-step
 * debugging region, then SNC Emulator breakpoint group is implied and therefore SNC Emulator
 * breakpoint/step-by-step debugging mechanism is enabled for that breakpoint, being controlled
 * through SNC_BKPT_GROUP_EMU() macro.
 *
 * If a breakpoint is set to the position where the corresponding SNC breakpoint group
 * is defined using SNC_BKPT_GROUP() or SNC_BKPT_GROUP_DFLT() macros, then all SNC breakpoints
 * defined for that group are enabled, resulting in both SNC and SYSCPU (CM33) halting when
 * the SNC execution flow reaches those SNC step-by-step breakpoints.
 *
 * Inside an explicitly defined SNC step-by-step debugging region, SNC (Emulator or not) execution
 * is blocked until the SNC context variable snc_bkpt is cleared inside the Sensor Node Handler.
 *
 * \sa SNC_BKPT_GROUP
 * \sa SNC_BKPT_GROUP_DFLT
 * \sa SNC_BKPT_GROUP_EMU
 * \sa snc_context_t
 * \sa SNC_STEP_BY_STEP_BEGIN
 * \sa SNC_STEP_BY_STEP_END
 *
 */
#define SNC_STEP_BY_STEP()                                                                      \
        _SNC_STEP_BY_STEP()

/**
 * \brief Macro used in SNC context to set an SNC assertion in uCode implementation
 *
 * When using this macro, the SNC context variable snc_bkpt will be filled with the local
 * function pointer defined for the particular assertion in uCode implementation, implementing
 * an assertion and a breakpoint in its body, and then the Sensor Node handler will be
 * triggered. Both CM33 and SNC execution will be blocked forever.
 *
 * General Syntax:
 *
 * SNC_ASSERT({op1_t}({op1}) [, {cond}, {op2_t}({op2})]);
 *
 * where :
 * op1_t: is either ia, da or nothing if op1 is a value
 * op1  : is an address if ia or da prefixes are used, else a value
 * op2_t: is either ia, da or nothing if op2 is a value
 * op2  : is an address if ia or da prefixes are used, else a value
 * cond : GTEQ(>=), GT(>), LTEQ(<=), LT(<), EQ(==), NEQ(!=),
 *        BIT  (set-bit condition     => true if in the given bit position (in op2) of the
 *                                       contents of op1 the value is "1",
 *                                       being equivalent to:
 *                                       "ASSERT_WARNING(op1 & (1 << op2));")
 *        NBIT (non-set-bit condition => true if in the given bit position (in op2) of the
 *                                       contents of op1 the value is "0",
 *                                       being equivalent to:
 *                                       "ASSERT_WARNING(!(op1 & (1 << op2)));")
 *
 * In case BIT or NBIT condition is used, op2 must be an uint32_t build-time-only value.
 *
 * Example - variable cond variable:
 * ASSERT(a == b);
 * \code{.c}
 * uint32_t a, b;
 *
 * {
 *         ...
 *         SNC_ASSERT(da(&a), EQ, da(&b));
 *         ...
 * }
 * \endcode
 *
 * Example - variable cond value:
 * ASSERT(a > 0xAA);
 * \code{.c}
 * uint32_t a;
 *
 * {
 *         ...
 *         SNC_ASSERT(da(&a), GT, 0xA);
 *         ...
 * }
 * \endcode
 *
 * Example - variable:
 * ASSERT(a);
 * \code{.c}
 * uint32_t a;
 *
 * {
 *         ...
 *         SNC_ASSERT(da(&a));
 *         ...
 * }
 * \endcode
 *
 * Example - always false:
 * ASSERT(false);
 * \code{.c}
 * {
 *         ...
 *         SNC_ASSERT(false);
 *         ...
 * }
 * \endcode
 *
 * \note Active only while in development mode
 *
 */
#define SNC_ASSERT(...)                                                                         \
        _SNC_ASSERT(__VA_ARGS__)

#else

#define SNC_BKPT_GROUP(bkpt_group)
#define SNC_BKPT_GROUP_DFLT()
#define SNC_BKPT_GROUP_EMU()
#define SNC_BKPT(bkpt_group)
#define SNC_STEP_BY_STEP_BEGIN(bkpt_group)
#define SNC_STEP_BY_STEP_END(bkpt_group)
#define SNC_STEP_BY_STEP()
#define SNC_ASSERT(...)

#endif /* dg_configUSE_SNC_DEBUGGER */

#endif /* dg_configUSE_HW_SENSOR_NODE */


#endif /* SNC_DEBUG_H_ */

/**
 * \}
 * \}
 */
