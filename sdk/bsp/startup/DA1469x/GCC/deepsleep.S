/*
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 */

                .syntax unified
                .arch   armv8-m.main


                .equ RESET_STAT_REG,0x500000BC
                .equ PDC_BASE,0x50000200
                .equ PDC_PENDING_CM33_REG_OFFSET,0x8c
                .equ PDC_ACKNOWLEDGE_REG_OFFSET,0x80
                .equ SCB_SHCSR_xFAULTENA_MASK, ((1<<19) | (1<<18) | (1<<17) | (1<<16))

                .equ SCB_BASE,  0xE000ED00
                .equ CPACR,     0xE000ED88

                .equ FPCCR,     0xE000EF34
                .equ FPDSCR,    0xE000EF3C

#if (dg_configENABLE_MTB)
                .equ MTB_POSITION_REG,          0xE0043000
                .equ MTB_MASTER_REG,            0xE0043004
                .equ MTB_FLOW_REG,              0xE0043008
#endif

/*
 *  The processor state is stored here before going to deep sleep,
 *  so that it is restored after wakeup.
 */
                .section retention_mem_zi
                .align 3
                .globl  __retained_state

                /* reserve space for stacking, in case an exception occurs while restoring the state */
                .space  64
__retained_state:
.stored_PRIMASK:
                .space  4
.stored_R4:
                .space  4
.stored_R5:
                .space  4
.stored_R6:
                .space  4
.stored_R7:
                .space  4
.stored_R8:
                .space  4
.stored_R9:
                .space  4
.stored_R10:
                .space  4
.stored_R11:
                .space  4
.stored_R12:
                .space  4
.stored_LR:
                .space  4
.stored_MSP:
                .space  4
.stored_PSP:
                .space  4
.stored_CONTROL:
                .space  4
.stored_ISER:
                .space  4
.stored_ISER1:
                .space  4
.stored_PRIORITY0:
                .space  4
.stored_PRIORITY1:
                .space  4
.stored_PRIORITY2:
                .space  4
.stored_PRIORITY3:
                .space  4
.stored_PRIORITY4:
                .space  4
.stored_PRIORITY5:
                .space  4
.stored_PRIORITY6:
                .space  4
.stored_PRIORITY7:
                .space  4
.stored_PRIORITY8:
                .space  4
.stored_PRIORITY9:
                .space  4
.stored_SCR:
                .space  4
.stored_SHPR1:
                .space  4
.stored_SHPR2:
                .space  4
.stored_SHPR3:
                .space  4
.stored_SHCSR_xFAULTENA:
                .space  4

.stored_CPACR:          .space  4
.stored_FPCCR:          .space  4
.stored_FPDSCR:         .space  4

#if (dg_configENABLE_MTB)
.stored_MTB_POSITION:   .space  4
.stored_MTB_MASTER:     .space  4
.stored_MTB_FLOW:       .space  4
#endif

                .size   __retained_state, . - __retained_state


/* Store processor state and attempt to enter sleep mode.
 *
 * This function stores processor state in retained memory and uses WFI to
 * enter sleep mode (processor registers are not retained during sleep).
 *
 * It returns a value inidicating whether sleep mode was entered or not.
 *
 * The return to the context of its caller is performed indirectly through
 * function goto_deepsleep_return:
 *
 *   - If sleep mode is not entered then goto_deepsleep() will call
 *     goto_deepsleep_return(0) to return to its caller.
 *
 *   - If sleep mode is entered then upon wake-up the reset handler will jump
 *     to wakeup_from_deepsleep() which will restore processor state and then
 *     call goto_deepsleep_return(1) to return to the context which originally
 *     called goto_deepsleep().
 *
 * As a last step before executing the WFI instruction, this function resets
 * RESET_STAT_REG so that upon wake-up the reset handler will be able to detect
 * that we have a wake-up instead of a reset.
 *
 * Processor state is stored at __retained_state. The stack is not modified.
 */
 #if (dg_configCODE_LOCATION == NON_VOLATILE_IS_NONE)
                .section .text
#else
                .section text_retained
#endif
                .thumb
                .thumb_func
                .align  1
                .globl  goto_deepsleep
                .type   goto_deepsleep, %function
goto_deepsleep:


                /* Store PRIMASK and disable interrupts */
                mrs     r0, PRIMASK
                cpsid   i
                ldr     r3, =.stored_PRIMASK
                stm     r3!, {r0}

                /*
                 * Acknowledge all PDC entries pending for CM33
                 * NOTE: only r0, r1, r2 are touched in order to do this
                 */
                ldr   r0, =PDC_BASE           // PDC registers base address
                ldr   r1, [r0, PDC_PENDING_CM33_REG_OFFSET]     // r1 = PDC->PDC_PENDING_CM33_REG

                movs    r0, #0                  // loop init: r0 = i = 0
                b.n     .L_loop_cond
.L_loop_next:
                adds    r0, #1                  // i = i + 1
.L_loop_cond:
                cmp     r0, #15
                bgt.n   .L_loop_exit            // if i > 15 then exit loop

                movs    r2, #1
                lsls    r2, r0                  // r2 = 1 << i
                tst     r1, r2                  // check if bit at pos i is set
                beq.n   .L_loop_next            // if not then continue loop
                                                // else
                ldr     r2, =PDC_BASE         // r2 = PDC regs base = 0x50000200
                str.w   r0, [r2, PDC_ACKNOWLEDGE_REG_OFFSET]    // PDC->PDC_ACKNOWLEDGE_REG = i

                b.n     .L_loop_next            // continue loop
.L_loop_exit:

                /* store R4-R12, LR */
                stm     r3!, {r4-r12, lr}

                /* Store MSP, PSP, CONTROL */
                mrs     r0, msp
                mrs     r1, psp
                mrs     r2, control
                stm     r3!, {r0-r2}

                /* Store NVIC->ISER[0..1] */
                ldr     r0, =0xE000E100
                ldm     r0, {r4-r5}
                stm     r3!, {r4-r5}

                /* Save values of NVIC->IPR[0..39] */
                ldr     r0, =0xE000E400         // NVIC->IPR[] base address
                ldm     r0, {r2, r4-r12}
                stm     r3!, {r2, r4-r12}


                /*
                 * Store
                 *      SCB->SCR
                 *      SCB->SHPR1..3
                 *      SCB->SHCSR (<x>FAULTENA fields only)
                 */
                ldr     r0, =SCB_BASE           // address of SCB
                ldr     r4, [r0, #0x10]         // value of SCB->SCR
                ldr     r5, [r0, #0x18]         // value of SCB->SHPR1
                ldr     r6, [r0, #0x1C]         // value of SCB->SHPR2
                ldr     r7, [r0, #0x20]         // value of SCB->SHPR3
                ldr     r8, [r0, #0x24]         // value of SCB->SHCSR
                and     r8, SCB_SHCSR_xFAULTENA_MASK // keep only the <x>FAULTENA fields
                stm     r3!, {r4-r8}

                mov     r9, r0                  // save address of SCB for later
                mov     r10, r4                 // save value of SCB->SCR for later

                /* Store CPACR, FPCCR, FPDSCR */
                ldr     r4, [r0, #(CPACR - SCB_BASE)]   // load CPACR
                and     r4, (0xF << 20)                 // keep only CP11 and CP10
                ldr     r5, [r0, #(FPCCR - SCB_BASE)]   // load FPCCR
                and     r5, (0xF << 28)                 // keep only ASPEN, LSPEN, CLRONRET
                ldr     r6, [r0, #(FPDSCR - SCB_BASE)]  // load FPDSCR
                stm     r3!, {r4-r6}

#if (dg_configENABLE_MTB)
                /* Store MTB  */
                ldr     r0, =#MTB_POSITION_REG          // load MTB_POSITION_REG address
                ldm     r0, {r4-r6}                     // value of MTB_POSITION_REG MTB_MASTER_REG, MTB_FLOW_REG
                stm     r3!, {r4-r6}
#endif

                /*
                 *  Set RESET_STAT_REG = 0
                 */
                ldr     r1, =RESET_STAT_REG
                movs    r0, 0
                str     r0, [r1]

                /* Set SCR.SLEEPDEEP */
                mov     r0, r10                 // r10 = saved SCB->SCR value
                orr     r0, #4
                str     r0, [r9, #0x10]         // r9 = SCB start address

                wfi

                /* we reach this point if deep sleep didn't actually happen */

                /* Restore SCR.SLEEPDEEP */
                str     r10, [r9, #0x10]

                /* Return 0 to the caller of goto_deepsleep() */
                mov     r0, #0
                b       goto_deepsleep_return

                .pool
                .size   goto_deepsleep, . - goto_deepsleep


/* Restore processor state and return to the caller of goto_deepsleep().
 *
 * The reset handler jumps to this function if it detects a wake-up instead of
 * a reset.
 */
                .thumb
                .thumb_func
                .align  1
                .globl  wakeup_from_deepsleep
                .type   wakeup_from_deepsleep, %function

wakeup_from_deepsleep:
                /* disable interrupts, final state will be set when PRIMASK is restored */
                cpsid   i

                /* set SP to a safe value, we have reserved some space for this case */
                ldr     r0, =__retained_state
                mov     sp, r0


                /*
                 * restore values of NVIC->ISER, NVIC->PRIORITY{0,1,2},
                 *                   SCB->SCR, SCB->SHR[0] (SHPR2) and SCB->SHR[1] (SHPR3)
                 */

                ldr     r0, =.stored_ISER

                /* Restore NVIC->ISER[0..1] */
                ldm     r0!, {r4-r5}
                ldr     r1, =0xE000E100         // address of NVIC->ISER[0]
                stm     r1, {r4-r5}

                /* Restore values of NVIC->IPR[0..39] */
                ldm     r0!, {r2, r4-r12}
                ldr     r1, =0xE000E400         // NVIC->IPR[] base address
                stm     r1, {r2, r4-r12}

                /* Restore
                 *      SCB->SCR,
                 *      SCB->SHRPR1..3
                 *      SCB->SHCSR (<x>FAULTENA fields only)
                 */
                ldm     r0!, {r4-r8}
                ldr     r1, =SCB_BASE
                str     r4, [r1, #0x10]         // SCB->SCR
                str     r5, [r1, #0x18]         // SCB->SHPR1
                str     r6, [r1, #0x1C]         // SCB->SHPR2
                str     r7, [r1, #0x20]         // SCB->SHPR3
                // r8 = saved SHCSR.<x>FAULTENA fields
                ldr     r2, [r1, #0x24]         // read SCB->SHCSR
                orr     r2, r8                  // modify
                str     r2, [r1, #0x24]         // write SCB->SHCSR

                /* r1 = SCB_BASE */

                /* Restore
                 *      CPACR
                 *      FPCCR
                 *      FPDSCR
                 */
                ldm     r0!, {r4-r6}
                /* r4 = .stored_CPACR */
                ldr     r2, [r1, #(CPACR - SCB_BASE)]   // read CPACR
                orr     r2, r4                          // CPACR |= .stored_CPACR
                str     r2, [r1, #(CPACR - SCB_BASE)]   // write back
                /* r5 = .stored_FPCCR */
                ldr     r2, [r1, #(FPCCR - SCB_BASE)]   // read FPCCR
                orr     r2, r5                          // FPCCR |= .stored_FPCCR
                str     r2, [r1, #(FPCCR - SCB_BASE)]   // write back
                /* r6 = .stored_FPDSCR */
                str     r6, [r1, #(FPDSCR - SCB_BASE)]

#if (dg_configENABLE_MTB)
                /* restore MTB */
                ldm     r0!, {r4-r6}
                ldr     r2, =#MTB_MASTER_REG
                str		r5,	[r2]
                ldr     r2, =#MTB_POSITION_REG
                str		r4,	[r2]
                ldr     r2, =#MTB_FLOW_REG
                str		r6,	[r2]
#endif

                /* restore MSP, PSP, CONTROL */
                ldr     r0, =.stored_MSP
                ldm     r0, {r1-r3}
                msr     msp, r1
                msr     psp, r2
                msr     control, r3

                /* Return 1 to the caller of goto_deepsleep() */
                mov     r0, #1
                b       goto_deepsleep_return

                .pool
                .size   wakeup_from_deepsleep, . - wakeup_from_deepsleep

/*
 * Return to the caller of goto_deepsleep().
 *
 * Parameters:
 *      int sleep_was_entered: the value to be returned by goto_deepsleep() to its caller
 */
                .thumb
                .thumb_func
                .align  1
                .type   goto_deepsleep_return, %function

goto_deepsleep_return:
                /* restore R4-R12, LR */
                ldr     r1, =.stored_R4
                ldm     r1, {r4-r12, lr}

                /* Restore PRIMASK */
                ldr     r2, [r1, #(.stored_PRIMASK - .stored_R4)]
                msr     PRIMASK, r2

                /* Interrupts might be enabled now */

                bx      lr

                .pool
                .size   goto_deepsleep_return, . - goto_deepsleep_return
