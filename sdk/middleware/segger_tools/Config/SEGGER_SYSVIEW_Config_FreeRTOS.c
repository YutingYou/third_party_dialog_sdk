  
/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 2015 - 2017  SEGGER Microcontroller GmbH & Co. KG        *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
*                                                                    *
*       SEGGER SystemView * Real-time application analysis           *
*                                                                    *
**********************************************************************
*                                                                    *
* All rights reserved.                                               *
*                                                                    *
* SEGGER strongly recommends to not make any changes                 *
* to or modify the source code of this software in order to stay     *
* compatible with the RTT protocol and J-Link.                       *
*                                                                    *
* Redistribution and use in source and binary forms, with or         *
* without modification, are permitted provided that the following    *
* conditions are met:                                                *
*                                                                    *
* o Redistributions of source code must retain the above copyright   *
*   notice, this list of conditions and the following disclaimer.    *
*                                                                    *
* o Redistributions in binary form must reproduce the above          *
*   copyright notice, this list of conditions and the following      *
*   disclaimer in the documentation and/or other materials provided  *
*   with the distribution.                                           *
*                                                                    *
* o Neither the name of SEGGER Microcontroller GmbH & Co. KG         *
*   nor the names of its contributors may be used to endorse or      *
*   promote products derived from this software without specific     *
*   prior written permission.                                        *
*                                                                    *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND             *
* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,        *
* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF           *
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE           *
* DISCLAIMED. IN NO EVENT SHALL SEGGER Microcontroller BE LIABLE FOR *
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR           *
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT  *
* OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;    *
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF      *
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT          *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE  *
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH   *
* DAMAGE.                                                            *
*                                                                    *
**********************************************************************
*                                                                    *
*       SystemView version: V2.52a                                    *
*                                                                    *
**********************************************************************
-------------------------- END-OF-HEADER -----------------------------

File    : SEGGER_SYSVIEW_Config_FreeRTOS.c
Purpose : Sample setup configuration of SystemView with FreeRTOS.
Revision: $Rev: 7745 $
*/

#if (dg_configSYSTEMVIEW == 1)
#if (DEVICE_FAMILY != DA1469X) && (DEVICE_FAMILY != DA1468X)
#include "FreeRTOS.h"
#include "task.h"
#include "SEGGER_SYSVIEW.h"
#include "sys_timer.h"
#include "interrupts.h"
#include "SEGGER_RTT.h"
extern const SEGGER_SYSVIEW_OS_API SYSVIEW_X_OS_TraceAPI;

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/
// The application name to be displayed in SystemViewer
#define SYSVIEW_APP_NAME        "DemoApp"

// The target device name
#define SYSVIEW_DEVICE_NAME     "DA1469x"

// Frequency of the timestamp. Must match SEGGER_SYSVIEW_GET_TIMESTAMP in SEGGER_SYSVIEW_Conf.h
#define SYSVIEW_TIMESTAMP_FREQ  (configSYSTICK_CLOCK_HZ)

// System Frequency. SystemcoreClock is used in most CMSIS compatible projects.
#define SYSVIEW_CPU_FREQ        configCPU_CLOCK_HZ

// The lowest RAM address used for IDs (pointers)
#define SYSVIEW_RAM_BASE        (MEMORY_SYSRAM_BASE)
/*********************************************************************
*
*       _cbSendSystemDesc()
*
*  Function description
*    Sends SystemView description strings.
*/
static void _cbSendSystemDesc(void)
{
        /*
           * The maximum size of the string passed as argument to SEGGER_SYSVIEW_SendSysDesc()
           * should not exceed SEGGER_SYSVIEW_MAX_STRING_LEN (128) bytes. Values can be comma
           * seperated.
           *
           * More ISR entries could be added but this would result in a slower system and might
           * also affect time critical tasks or trigger assertions.
           *
           * This is because multiple SEGGER_SYSVIEW_SendSysDesc() calls will result in multiple
           * RTT transactions.
           *
           * Note also that _cbSendSystemDesc() is called multiple times from the host PC and not
           * just during initialization, so assertions may occur anytime during SystemView monitoring.
           *
           */
        const char* sys_desc =
                "N="SYSVIEW_APP_NAME",D="SYSVIEW_DEVICE_NAME",O=FreeRTOS,"
                "I#15=SysTick,"
                //"I#16=Sensor_Node,"
                "I#17=DMA,"
                //"I#18=Charger_State,"
                //"I#19=Charger_Error,"
                "I#20=CMAC2SYS,"
                //"I#21=UART,"
                //"I#22=UART2,"
                //"I#23=UART3,"
                //"I#24=I2C,"
                //"I#25=I2C2,"
                //"I#26=SPI,"
                //"I#27=SPI2,"
                //"I#28=PCM,"
                //"I#29=SRC_In,"
                //"I#30=SRC_Out,"
                //"I#31=USB,"
                //"I#32=Timer,"
                "I#33=Timer2,"
                //"I#34=RTC,"
                //"I#35=Key_Wkup_GPIO,"
                //"I#36=PDC,"
                //"I#37=VBUS,"
                //"I#38=MRM,"
                //"I#39=Motor_Controller,"
                //"I#40=TRNG,"
                //"I#41=DCDC,"
                "I#42=XTAL32M_Ready,"
                //"I#43=ADC,"
                //"I#44=ADC2,"
                //"I#45=Crypto,"
                //"I#46=CAPTIMER1,"
                //"I#47=RFDIAG,"
                //"I#48=LCD_Controller,"
                //"I#49=PLL_Lock,"
                //"I#50=Timer3,"
                //"I#51=Timer4,"
                //"I#52=LRA,"
                //"I#53=RTC_Event,"
                //"I#54=GPIO_P0,"
                //"I#55=GPIO_P1"
                "I#56=RSVD";

          SEGGER_SYSVIEW_SendSysDesc(sys_desc);
}

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/
void SEGGER_SYSVIEW_Conf(void)
{
  SEGGER_SYSVIEW_Init(SYSVIEW_TIMESTAMP_FREQ, SYSVIEW_CPU_FREQ,
                      &SYSVIEW_X_OS_TraceAPI, _cbSendSystemDesc);
  SEGGER_SYSVIEW_SetRAMBase(SYSVIEW_RAM_BASE);
}

__RETAINED_CODE
U32 SEGGER_SYSVIEW_X_GetTimestamp()
{
        uint32_t timestamp;
        uint32_t timer_value;

        SEGGER_RTT_LOCK();
        timestamp = (uint32_t) sys_timer_get_timestamp_fromCPM(&timer_value);
        SEGGER_RTT_UNLOCK();

        return timestamp;
}

__RETAINED_CODE
U32 SEGGER_SYSVIEW_X_GetInterruptId(void)
{
        return ((*(U32 *)(0xE000ED04)) & 0x1FF);
}
#endif
#endif /* (dg_configSYSTEMVIEW == 1) */

/*************************** End of file ****************************/
