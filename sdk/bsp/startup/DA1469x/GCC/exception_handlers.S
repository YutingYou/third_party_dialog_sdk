/*
 * Copyright (C) 2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 */


#define SYS_CTRL_REG                            0x50000024
#define SYS_CTRL_REG_DEBUGGER_ENABLE_MSK        0x80
#define SYS_CTRL_REG_REMAP_ADR0_MSK             0x7

#define WATCHDOG_CTRL_REG                       0x50000704
#define WATCHDOG_CTRL_REG_WDOG_FREEZE_EN_MSK    0x4
#define SYS_WDOG_WATCHDOG_CTRL_REG_NMI_RST_Msk  0x1

#define RESET_FREEZE_REG                        0x50040304
#define RESET_FREEZE_REG_FRZ_SYS_WDOG_MSK       0x08

#define SET_FREEZE_REG                          0x50040300
#define SET_FREEZE_REG_FRZ_SYS_WDOG_MSK         0x08

/* Micro Trace Buffer registers */
#define MTB_POSITION_REG                        0xE0043000
#define MTB_FLOW_REG                            0xE0043008
#define MTB_MASTER_REG                          0xE0043004
#define MTB_MASTER_REG_ENABLE_MSK               0x80000009

        	.syntax	unified
        	.arch	armv8-m.main

#if ( (dg_configSKIP_MAGIC_CHECK_AT_START == 0) && (dg_configIMAGE_SETUP == PRODUCTION_MODE) )
#error "Building Production image with MAGIC Value check enabled! Possible security risk..."
#endif

        	.equ RESET_STAT_REG, 0x500000BC

#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_NONE)
        	.section .text
#else
        	.section text_retained
#endif
        	.thumb
        	.thumb_func
        	.align	2
        	.globl	Wakeup_Reset_Handler
        	.type	Wakeup_Reset_Handler, %function
Wakeup_Reset_Handler:
                /*
                 *  Determine if this is a cold reset (e.g. POR, watchdog reset etc.) or
                 *  a wakeup from deep sleep.
                 *
                 *  If RESET_STAT_REG = 0, it is a wakeup from deep sleep without state retention
                 *  since RESET_STAT_REG is zeroed by goto_deepsleep().
                 *  In that case, the Reset_Handler jumps to wakeup_from_deepsleep,
                 *  instead of going through the normal startup procedure.
                 */
                ldr     r1, =RESET_STAT_REG
                ldr     r2, [r1]
                cbnz	r2, .cold_reset            /* if RESET_STAT_REG != 0 then jump to .cold_reset */

                ldr     r3, =wakeup_from_deepsleep
                bx      r3
.cold_reset:
                ldr     r3, =Reset_Handler
                bx      r3
                .size	Wakeup_Reset_Handler, . - Wakeup_Reset_Handler


                .section text_reset
                .thumb
                .thumb_func
                .align  2
                .globl  Reset_Handler
                .type   Reset_Handler, %function
Reset_Handler:
                ldr     r0, =0x20000004
                ldr     r1, =Wakeup_Reset_Handler
                str     r1, [r0]

#if (dg_configENABLE_MTB)
                ldr     r0, =MTB_POSITION_REG
                ldr     r1, =0
                str     r1, [r0]
                ldr     r0, =MTB_FLOW_REG
                ldr     r1, =0
                str     r1, [r0]
                ldr     r0, =MTB_MASTER_REG
                ldr     r1, =MTB_MASTER_REG_ENABLE_MSK
                str     r1, [r0]
#endif

#if (dg_configSKIP_MAGIC_CHECK_AT_START == 0)

/* If the Magic Word {0xDEADBEEF, 0xDEADBEEF, 0xDEADBEEF, 0xDEAD10CC} is found at 0x20010000 then the
 * execution will block for a while in order to give time to a debugger to attach. */
                ldr     r4, =0x20010000
                ldmia   r4!, {r0-r3}
                ldr     r4, =0xDEADBEEF
                cmp     r0, r4
                bne     .code_starts
                cmp     r1, r4
                bne     .code_starts
                cmp     r2, r4
                bne     .code_starts
                ldr     r4, =0xDEAD10CC
                cmp     r3, r4
                bne     .code_starts
                /* Enable debugger */
                ldr     r4, =SYS_CTRL_REG
                ldrh    r5, [r4]
                mov     r6, r5
                movs    r1, #SYS_CTRL_REG_DEBUGGER_ENABLE_MSK
                orrs    r5, r5, r1
                strh    r5, [r4]
                /* Block for 2'' */
                ldr     r0, =0x150000
.delay_loop:
                subs    r0, #1
                bgt     .delay_loop
                /* Make sure that this will happen only once! */
                ldr     r4, =0x20010000
                movs    r0, #0
                str     r0, [r4]
                /* Restore debugger setting */
                ldr     r4, =SYS_CTRL_REG
                strh    r6, [r4]

.code_starts:
#endif

/* RAM projects should disable the cache controller */
#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_NONE)
                ldr     r4, =SYS_CTRL_REG
                ldrh    r0, [r4]
                movs    r1, #1
                lsls    r1, r1, #10
                bics    r0, r0, r1
                /* Update SYS_CTRL_REG. */
                strh    r0, [r4]
#endif

                bl	SystemInitPre

/*
 *  Between symbol address __copy_table_start__ and __copy_table_end__,
 *  there are array of triplets, each of which specify:
 *    offset 0: LMA of start of a section to copy from
 *    offset 4: VMA of start of a section to copy to
 *    offset 8: size of the section to copy. Must be multiply of 4
 *
 *  All addresses must be aligned to 4 bytes boundary.
 */
        	ldr	r4, =__copy_table_start__
        	ldr	r5, =__copy_table_end__

.L_loop0:
        	cmp	r4, r5
        	bge	.L_loop0_done
        	ldr	r1, [r4]
        	ldr	r2, [r4, #4]
        	ldr	r3, [r4, #8]

/*
 *  Copy (src(r1), dst(r2), len(r3))
 */
        	push    {r4, r5}
.L_loop0_0:
        	subs	r3, #4
        	blt     .L_loop0_end
        	ldr     r4, [r1], #4
        	str     r4, [r2], #4
        	lsls    r4, r3, #26	/* while (r3 % 64) */
        	bne     .L_loop0_0

.L_loop0_1:
        	subs    r3, #64
        	blt     .L_loop0_end
        	ldmia   r1!, {r4-r11}
        	stmia   r2!, {r4-r11}
        	ldmia   r1!, {r4-r11}
        	stmia   r2!, {r4-r11}
        	b       .L_loop0_1

.L_loop0_end:
        	pop    {r4, r5}

        	adds	r4, #12
        	b       .L_loop0

.L_loop0_done:

/*
 *  Between symbol address __copy_table_start__ and __copy_table_end__,
 *  there are array of tuples specifying:
 *    offset 0: Start of a BSS section
 *    offset 4: Size of this BSS section. Must be multiply of 4
 */
        	ldr	r3, =__zero_table_start__
        	ldr	r4, =__zero_table_end__

.L_loop2:
        	cmp	r3, r4
        	bge	.L_loop2_done
        	ldr	r1, [r3]
        	ldr	r2, [r3, #4]

/*
 *  Zero (from(r1), size(r2))
 */
        	push    {r4}
        	mov     r4, 0
        	mov     r5, r4
        	mov     r6, r4
        	mov     r7, r4
        	mov     r8, r4
        	mov     r9, r4
        	mov     r10, r4
        	mov     r11, r4
.L_loop2_0:
        	subs	r2, #4
        	blt     .L_loop2_end
        	str     r4, [r1], #4
        	lsls    r0, r2, #26 /* while (r2 % 64) */
        	bne     .L_loop2_0

.L_loop2_1:
        	subs	r2, #64
        	blt     .L_loop2_end
        	stmia   r1!, {r4-r11}
        	stmia   r1!, {r4-r11}
        	b       .L_loop2_1

.L_loop2_end:
        	pop    {r4}

        	adds	r3, #8
        	b       .L_loop2
.L_loop2_done:

                bl	SystemInit

        	bl	_start

        	.pool
        	.size	Reset_Handler, . - Reset_Handler

#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_FLASH)
                .section text_retained
#endif
                .align  2
                .thumb
                .thumb_func
                .globl  NMI_Handler
                .type   NMI_Handler, %function
NMI_Handler:
                ldr     r1, =NMI_HandlerC
                movs    r0, #4
                mov     r2, lr
                tst     r0, r2
                beq     NMI_stacking_using_MSP
                mrs     r0, psp
                b       stack_check
NMI_stacking_using_MSP:
                mrs     r0, msp
                b       stack_check
                .size   NMI_Handler, . - NMI_Handler

                .align  2
                .thumb
                .thumb_func
                .globl  HardFault_Handler
                .type   HardFault_Handler, %function
HardFault_Handler:
                ldr     r1, =HardFault_HandlerC
                movs    r0, #4
                mov     r2, lr
                tst     r0, r2
                beq     HF_stacking_using_MSP
                mrs     r0, psp
                b       stack_check
HF_stacking_using_MSP:
                mrs     r0, msp
stack_check:
                /* If address 0x0 is remapped to SYSRAM (SYS_CTRL_REG[REMAP_ADR0] is 0x3),
                 * check if SP (r0) belongs to one of the following ranges:
                 * - [0x0, 0x80000)
                 * - [0x800000, 0x880000)
                 * - [0x20000000, 0x20080000)
                 * Else, check if SP (r0) belongs to one of the following ranges:
                 * - [0x800000, 0x880000)
                 * - [0x20000000, 0x20080000)
                 *
                 * If dg_configUSE_HW_QSPI2 is defined, also check the following range,
                 * in case PSRAM is present:
                 * - [0x32000000, 0x34000000)
                 */
                ldr     r2, =SYS_CTRL_REG
                ldr     r2, [r2]
                and     r2, r2, #SYS_CTRL_REG_REMAP_ADR0_MSK
                cmp     r2, #3
                beq     stack_check_range_0x0
                b       stack_check_range_0x800000

stack_check_range_0x0:
                /* Check if SP (r0) belongs to [0x0, 0x80000)  */
                ldr     r2, =0x80000-32
                cmp     r0, r2
                bgt 	stack_check_range_0x800000
                bx      r1 /* Valid SP */

stack_check_range_0x800000:
                /* Check if SP (r0) belongs to [0x800000, 0x880000) */
                ldr     r2, =0x800000
                cmp     r0, r2
                blt     Wrong_SP
                ldr     r2, =0x880000-32
                cmp     r0, r2
                bgt 	stack_check_range_0x20000000
                bx      r1 /* Valid SP */

stack_check_range_0x20000000:
                /* Check if SP (r0) belongs to [0x20000000, 0x20080000) */
                ldr     r2, =0x20000000
                cmp     r0, r2
                blt     Wrong_SP
                ldr     r2, =0x20080000-32
                cmp     r0, r2
#if dg_configUSE_HW_QSPI2
                bgt     stack_check_range_0x32000000
#else
                bgt     Wrong_SP
#endif
                bx      r1 /* Valid SP */

#if dg_configUSE_HW_QSPI2
                stack_check_range_0x32000000:
                /* Check if SP (r0) belongs to [0x32000000, 0x34000000) */
                ldr     r2, =0x32000000
                cmp     r0, r2
                blt     Wrong_SP
                ldr     r2, =0x34000000-32
                cmp     r0, r2
                bgt     Wrong_SP
                bx      r1 /* Valid SP */
#endif

Wrong_SP:
#if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE)
                /*
                * Set       WATCHDOG_CTRL_REG[WDOG_FREEZE_EN]
                * and reset WATCHDOG_CTRL_REG[NMI_RST]
                * so that watchdog can be frozen.
                */
                ldr     r0, =WATCHDOG_CTRL_REG
                ldr     r1, =WATCHDOG_CTRL_REG_WDOG_FREEZE_EN_MSK
                str     r1, [r0]
                /* Freeze WDog at all times */
                ldr     r0, =SET_FREEZE_REG
                ldr     r1, =SET_FREEZE_REG_FRZ_SYS_WDOG_MSK
                strh    r1, [r0, #0]
                /* Enable debugger at all times */
                ldr     r4, =SYS_CTRL_REG
                ldrh    r0, [r4, #0]
                movs    r1, #SYS_CTRL_REG_DEBUGGER_ENABLE_MSK
                orrs    r0, r0, r1
                /* Update SYS_CTRL_REG */
                strh    r0, [r4, #0]
#else
                /* Unfreeze WDog at all times */
                ldr     r0, =RESET_FREEZE_REG
                ldr     r1, =RESET_FREEZE_REG_FRZ_SYS_WDOG_MSK
                strh    r1, [r0, #0]
#if (dg_configENABLE_DEBUGGER == 1)
                /* Enable debugger */
                ldr     r4, =SYS_CTRL_REG
                ldrh    r0, [r4, #0]
                movs    r1, #SYS_CTRL_REG_DEBUGGER_ENABLE_MSK
                orrs    r0, r0, r1
                /* Update SYS_CTRL_REG */
                strh    r0, [r4, #0]
#endif
#endif /* (dg_configIMAGE_SETUP == DEVELOPMENT_MODE) */
                /* Wait for the WDog to hit or a debug session to start */
                b       .
                .size   HardFault_Handler, . - HardFault_Handler

        	.end

