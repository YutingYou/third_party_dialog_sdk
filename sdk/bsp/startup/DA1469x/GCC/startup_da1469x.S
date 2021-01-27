/**************************************************************************//**
 * @file     startup_da1469x.S
 * @brief    CMSIS Core Device Startup File for DA1469x Device
 * @version  V1.1.0
 * @date     17. May 2019
 ******************************************************************************/
/*
 * Copyright (c) 2009-2019 Arm Limited. All rights reserved
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/* Copyright (c) 2017-2019 Modified by Dialog Semiconductor */

/*
;//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------
*/


#define SYS_CTRL_REG                            0x50000024
#define SYS_CTRL_REG_DEBUGGER_ENABLE_MSK        0x80

        	.syntax	unified
        	.arch	armv8-m.main

                .section text_reset
                .text
                .align  2
        	.thumb_func
                .type   Default_Handler, %function
        	.weak	Default_Handler
Default_Handler:
                /* Enable debugger */
                ldr     r1, =SYS_CTRL_REG
                ldrh    r2, [r1, #0]
                movs    r3, #SYS_CTRL_REG_DEBUGGER_ENABLE_MSK
                orrs    r2, r2, r3
                strh    r2, [r1, #0]
        	b	.
        	.size	Default_Handler, . - Default_Handler

/* Macro to define default exception/interrupt handlers.
 * Default handler are weak symbols with an endless loop.
 * They can be overwritten by real handlers.
 */
        	.macro	def_irq_handler	handler_name
        	.weak	\handler_name
        	.set	\handler_name, Default_Handler
        	.endm

        	.thumb
        	.thumb_func
        	.align	1
                .type   SVC_Handler, %function
        	.weak	SVC_Handler
SVC_Handler:
        	b	.
        	.size	SVC_Handler, . - SVC_Handler
                def_irq_handler MemManage_Handler
                def_irq_handler BusFault_Handler
                def_irq_handler UsageFault_Handler
                def_irq_handler SecureFault_Handler
                def_irq_handler DebugMon_Handler
                def_irq_handler PendSV_Handler
                def_irq_handler SysTick_Handler

                def_irq_handler Sensor_Node_Handler
                def_irq_handler DMA_Handler
                def_irq_handler Charger_State_Handler
                def_irq_handler Charger_Error_Handler
                def_irq_handler CMAC2SYS_Handler
                def_irq_handler UART_Handler
                def_irq_handler UART2_Handler
                def_irq_handler UART3_Handler
                def_irq_handler I2C_Handler
                def_irq_handler I2C2_Handler
                def_irq_handler SPI_Handler
                def_irq_handler SPI2_Handler
                def_irq_handler PCM_Handler
                def_irq_handler SRC_In_Handler
                def_irq_handler SRC_Out_Handler
                def_irq_handler USB_Handler
                def_irq_handler Timer_Handler
                def_irq_handler Timer2_Handler
                def_irq_handler RTC_Handler
                def_irq_handler Key_Wkup_GPIO_Handler
                def_irq_handler PDC_Handler
                def_irq_handler VBUS_Handler
                def_irq_handler MRM_Handler
                def_irq_handler Motor_Controller_Handler
                def_irq_handler TRNG_Handler
                def_irq_handler DCDC_Handler
                def_irq_handler XTAL32M_Ready_Handler
                def_irq_handler ADC_Handler
                def_irq_handler SDADC_Handler
                def_irq_handler Crypto_Handler
                def_irq_handler CAPTIMER_Handler
                def_irq_handler RFDIAG_Handler
                def_irq_handler LCD_Controller_Handler
                def_irq_handler PLL_Lock_Handler
                def_irq_handler Timer3_Handler
                def_irq_handler Timer4_Handler
                def_irq_handler LRA_Handler
                def_irq_handler RTC_Event_Handler
                def_irq_handler GPIO_P0_Handler
                def_irq_handler GPIO_P1_Handler
                def_irq_handler RESERVED40_Handler
                def_irq_handler RESERVED41_Handler
                def_irq_handler RESERVED42_Handler
                def_irq_handler RESERVED43_Handler
                def_irq_handler RESERVED44_Handler
                def_irq_handler RESERVED45_Handler
                def_irq_handler RESERVED46_Handler
                def_irq_handler RESERVED47_Handler

        	.end
