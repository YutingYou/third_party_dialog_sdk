/**
 ****************************************************************************************
 *
 * @file vector_table.S
 *
 * @brief Interrupt Vector Table and Patch Table
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

                .syntax	unified
                .arch	armv8-m.main


/*
;<h> Stack Configuration
;  <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
;</h>
*/
#if (dg_configSYSTEMVIEW == 0)
        #ifdef __STACK_SIZE
                .equ    Stack_Size, __STACK_SIZE
        #else
                .equ    Stack_Size, 0xC00
        #endif
#else
        #ifdef __STACK_SIZE
                .equ    Stack_Size, (__STACK_SIZE + dg_configSYSTEMVIEW_STACK_OVERHEAD)
        #else
                .equ    Stack_Size, (0xC00 + dg_configSYSTEMVIEW_STACK_OVERHEAD)
        #endif
#endif /* (dg_configSYSTEMVIEW == 0) */
                .section .stack
                .align	3
                .globl	__StackTop
                .globl	__StackLimit
__StackLimit:
                .space	Stack_Size
                .size	__StackLimit, . - __StackLimit
__StackTop:
                .size	__StackTop, . - __StackTop


/*
;<h> Heap Configuration
;  <o> Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
;</h>
*/
#ifdef __HEAP_SIZE
        .equ    Heap_Size, __HEAP_SIZE
#else
        .equ    Heap_Size, 0
#endif

                .if      Heap_Size != 0                     /* Heap is provided */
                .section .heap
                .align	3
                .globl	__HeapBase
                .globl	__HeapLimit
__HeapBase:
                .space	Heap_Size
                .size	__HeapBase, . - __HeapBase
__HeapLimit:
                .size	__HeapLimit, . - __HeapLimit
                .endif

                .section .isr_vector, "a"
                .align 2
                .globl	__isr_vector
                .globl  __isr_vector_end
                .globl  __isr_vector_size
__isr_vector:
                .long   __StackTop              /* Top of Stack */
                .long   Reset_Handler           /* Reset Handler */
                .long   NMI_Handler             /* NMI Handler */
                .long   HardFault_Handler       /* Hard Fault Handler */
                .long   MemManage_Handler       /* MPU Fault Handler */
                .long   BusFault_Handler        /* Bus Fault Handler */
                .long   UsageFault_Handler      /* Usage Fault Handler */
                .long   SecureFault_Handler     /* Secure Fault Handler */
                .long   0                       /* Reserved */
                .long   0                       /* Reserved */
                .long   0                       /* Reserved */
                .long   SVC_Handler             /* SVCall Handler */
                .long   DebugMon_Handler        /* Debug Monitor Handler */
                .long   0                       /* Reserved */
                .long   PendSV_Handler          /* PendSV Handler */
                .long   SysTick_Handler         /* SysTick Handler */

                /* External interrupts */
                .long   Sensor_Node_Handler             /*  0: Sensor Node Controller        */
                .long   DMA_Handler                     /*  1: General Purpose DMA           */
                .long   Charger_State_Handler           /*  2: Charger Error interrupt request */
                .long   Charger_Error_Handler           /*  3: Charger FSM interrupt request */
                .long   CMAC2SYS_Handler                /*  4: CMAC and mailbox              */
                .long   UART_Handler                    /*  5: UART                          */
                .long   UART2_Handler                   /*  6: UART2                         */
                .long   UART3_Handler                   /*  7: UART3                         */
                .long   I2C_Handler                     /*  8: I2C                           */
                .long   I2C2_Handler                    /*  9: I2C2                          */
                .long   SPI_Handler                     /* 10: SPI                           */
                .long   SPI2_Handler                    /* 11: SPI2                          */
                .long   PCM_Handler                     /* 12: PCM                           */
                .long   SRC_In_Handler                  /* 13: SRC Input                     */
                .long   SRC_Out_Handler                 /* 14: SRC Output                    */
                .long   USB_Handler                     /* 15: USB                           */
                .long   Timer_Handler                   /* 16: Timer                         */
                .long   Timer2_Handler                  /* 17: Timer2                        */
                .long   RTC_Handler                     /* 18: RTC                           */
                .long   Key_Wkup_GPIO_Handler           /* 19: Debounced button pressed      */
                .long   PDC_Handler                     /* 20: PDC                           */
                .long   VBUS_Handler                    /* 21: VBUS Presence                 */
                .long   MRM_Handler                     /* 22: Cache Miss Rate Monitor       */
                .long   Motor_Controller_Handler        /* 23: Motor and mailbox             */
                .long   TRNG_Handler                    /* 24: True Random Number Generation */
                .long   DCDC_Handler                    /* 25: DC-DC timeout expiration      */
                .long   XTAL32M_Ready_Handler           /* 26: XTAL32M osc trimmed & settled */
                .long   ADC_Handler                     /* 27: Analog-Digital Converter      */
                .long   SDADC_Handler                   /* 28: Analog-Digital Converter 2    */
                .long   Crypto_Handler                  /* 29: Crypto (AES or HASH)          */
                .long   CAPTIMER_Handler                /* 30: GPIO triggered Timer1 Capture */
                .long   RFDIAG_Handler                  /* 31: Baseband/Radio diagnostics    */
                .long   LCD_Controller_Handler          /* 32: Parallel LCD Controller       */
                .long   PLL_Lock_Handler                /* 33: DLL/PLL locked @96MHz         */
                .long   Timer3_Handler                  /* 34: Timer3                        */
                .long   Timer4_Handler                  /* 35: Timer4                        */
                .long   LRA_Handler                     /* 36: LRA                           */
                .long   RTC_Event_Handler               /* 37: RTC                           */
                .long   GPIO_P0_Handler                 /* 38: GPIO_P0                       */
                .long   GPIO_P1_Handler                 /* 39: GPIO_P1                       */

__isr_vector_end:
                .equ     __isr_vector_size, __isr_vector - __isr_vector
                .size	__isr_vector, . - __isr_vector

                /*
                 * Patch Table
                 * Notice: it crosses the 0x100 boundary
                 *         in cached mode, the first entries will be copied to
                 *         (and accessed from) RAM and others will only lie on
                 *         QSPI flash or OTP.
                 *
                 * In C declare: extern uint32_t __patch_table_start[];
                 */
                .section .patch_table, "a"
                .align 4
                .globl	__patch_table_start
__patch_table_start:
                /*
                 * 28 32-bit entries, initialized to "pointer to Default_Patch_Code"
                 *
                 * NOTE: we assume that SYS_CTRL_REG.REMAP_INTVECT == 1
                 */
                .long Default_Patch_Code_Handler        /*  0, in RAM */
                .long Default_Patch_Code_Handler        /*  1, in RAM */
                .long Default_Patch_Code_Handler        /*  2, in RAM */
                .long Default_Patch_Code_Handler        /*  3, in RAM */
                .long Default_Patch_Code_Handler        /*  4, in RAM */
                .long Default_Patch_Code_Handler        /*  5, in RAM */
                .long Default_Patch_Code_Handler        /*  6, in RAM */
                .long Default_Patch_Code_Handler        /*  7, in RAM */
                .long Default_Patch_Code_Handler        /*  8, in RAM */
                .long Default_Patch_Code_Handler        /*  9, in RAM */
                .long Default_Patch_Code_Handler        /* 10, in RAM */
                .long Default_Patch_Code_Handler        /* 11, in RAM */
                /*
                 * Chip version: AD
                 * Entries 12, 13, 14 and 15 are unusable and skipped in the QSPI image,
                 * to account for the overhead of the QSPI header (16 bytes).
                 *
                 * Chip version: AA, AC, AE
                 * Entries 14 and 15 are unusable and skipped in the QSPI image,
                 * to account for the overhead of the QSPI header (8 bytes).
                 *
                 * The mkimage tool is responsible for stripping the proper entries from the QSPI image.
                 */
                .long Default_Patch_Code_Handler        /* 12 */
                .long Default_Patch_Code_Handler        /* 13 */
                .long 0xDEADBEEF                        /* 14 */
                .long 0xDEADBEEF                        /* 15 */
                .size	__patch_table_start, . - __patch_table_start

__patch_table_end:

                .section .default_patch_code_handler_section, "a"
                .globl Default_Patch_Code_Handler
                .align	1
                .thumb
                .thumb_func
                .type	Default_Patch_Code_Handler, %function
Default_Patch_Code_Handler:
                bkpt	#0
                bkpt	#0

