/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SNC_DEBUG
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file snc_debug_macros.h
 *
 * @brief SNC definitions of SNC debugging API macros
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SNC_DEBUG_MACROS_H_
#define SNC_DEBUG_MACROS_H_


#if dg_configUSE_HW_SENSOR_NODE

#if dg_configUSE_SNC_DEBUGGER

/*
 * FUNCTION DECLARATIONS
 *****************************************************************************************
 */

void snc_hw_sys_bkpt(b_ctx_t* b_ctx, bkpt_func_t bkpt_func, bkpt_control_func_t bkpt_control_func);

#define _SNC_SELECT_BKPT_GROUP(bkpt_group, applied_bkpt_group, ...) applied_bkpt_group
#define _SNC_ISDFLT_BKPT_GROUP(...)                                                             \
        _SNC_SELECT_BKPT_GROUP(, ##__VA_ARGS__, snc_bkpt_group_dflt)

#define _SNC_BKPT_GROUP(bkpt_group)                                                             \
        __STATIC_FORCEINLINE                                                                    \
        int snc_bkpt_control_##bkpt_group##_func(void) { volatile int dummy = 0; (void)dummy; return 0; } \
        typedef int bkpt_group

#define _SNC_BKPT_GROUP_DFLT()                                                                  \
        _SNC_BKPT_GROUP(snc_bkpt_group_dflt)

#if dg_configUSE_HW_SENSOR_NODE_EMU
#define _SNC_BKPT_GROUP_EMU()                                                                   \
        __STATIC_INLINE int snc_bkpt_control_emu_group_func(void) { return 0; }                 \
        typedef int snc_bkpt_group_emu
#endif /* dg_configUSE_HW_SENSOR_NODE_EMU */

#define ___SNC_BKPT(bkpt_group)                                                                 \
        {                                                                                       \
                __attribute__((noinline)) bkpt_group bkpt_func()                                \
                {                                                                               \
                        return snc_bkpt_control_##bkpt_group##_func();                          \
                }                                                                               \
                snc_hw_sys_bkpt(b_ctx, bkpt_func, bkpt_func);                                   \
        }
#define __SNC_BKPT(...) ___SNC_BKPT(__VA_ARGS__)
#define _SNC_BKPT(bkpt_group)                                                                   \
        __SNC_BKPT(_SNC_ISDFLT_BKPT_GROUP(bkpt_group))

#define ___SNC_STEP_BY_STEP_BEGIN(bkpt_group)                                                   \
        {                                                                                       \
                __attribute__((noinline)) bkpt_group bkpt_sbs_control_func(void)                \
                {                                                                               \
                        return snc_bkpt_control_##bkpt_group##_func();                          \
                }                                                                               \
                ASSERT_WARNING(b_ctx->bkpt_sbs_control_func == NULL);                           \
                b_ctx->bkpt_sbs_control_func = bkpt_sbs_control_func;                           \
        }
#define __SNC_STEP_BY_STEP_BEGIN(...) ___SNC_STEP_BY_STEP_BEGIN(__VA_ARGS__)
#define _SNC_STEP_BY_STEP_BEGIN(bkpt_group)                                                     \
        __SNC_STEP_BY_STEP_BEGIN(_SNC_ISDFLT_BKPT_GROUP(bkpt_group))

#define ___SNC_STEP_BY_STEP_END(bkpt_group)                                                     \
        {                                                                                       \
                bkpt_group dummy_group __UNUSED;                                                \
                b_ctx->bkpt_sbs_control_func = NULL;                                            \
        }
#define __SNC_STEP_BY_STEP_END(...) ___SNC_STEP_BY_STEP_END(__VA_ARGS__)
#define _SNC_STEP_BY_STEP_END(bkpt_group)                                                       \
        __SNC_STEP_BY_STEP_END(_SNC_ISDFLT_BKPT_GROUP(bkpt_group))

#define _SNC_STEP_BY_STEP()                                                                     \
        {                                                                                       \
                __attribute__((noinline)) int bkpt_func(bkpt_control_func_t bkpt_control_func)  \
                {                                                                               \
                        volatile int dummy = bkpt_control_func();                               \
                        return dummy;                                                           \
                }                                                                               \
                snc_hw_sys_bkpt(b_ctx, bkpt_func, b_ctx->bkpt_sbs_control_func);                \
        }

#if defined(OS_FREERTOS)
#define _SNC_ASSERT(...)                                                                        \
        if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE) {                                         \
                SENIS_if (__VA_ARGS__) {                                                        \
                SENIS_else {                                                                    \
                        int bkpt_func()                                                         \
                        {                                                                       \
                                taskDISABLE_INTERRUPTS();                                       \
                                hw_watchdog_freeze();                                           \
                                do {} while (1);                                                \
                                return 0;                                                       \
                        }                                                                       \
                        snc_hw_sys_bkpt(b_ctx, bkpt_func, bkpt_func);                           \
                }}                                                                              \
        }
#elif defined OS_BAREMETAL
#define _SNC_ASSERT(...)                                                                        \
        if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE) {                                         \
                SENIS_if (__VA_ARGS__) {                                                        \
                SENIS_else {                                                                    \
                        int bkpt_func()                                                         \
                        {                                                                       \
                                __disable_irq();                                                \
                                GPREG->SET_FREEZE_REG = GPREG_SET_FREEZE_REG_FRZ_SYS_WDOG_Msk;  \
                                hw_sys_assert_trigger_gpio();                                   \
                                do {} while (1);                                                \
                                return 0;                                                       \
                        }                                                                       \
                        snc_hw_sys_bkpt(b_ctx, bkpt_func, bkpt_func);                           \
                }}                                                                              \
        }
#endif /* OS_BAREMETAL */

#endif /* dg_configUSE_SNC_DEBUGGER */

#endif /* dg_configUSE_HW_SENSOR_NODE */


#endif /* SNC_DEBUG_MACROS_H_ */

/**
 * \}
 * \}
 */
