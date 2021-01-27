/**
 ****************************************************************************************
 *
 * @file config.c
 *
 * @brief Configure system.
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/**
\addtogroup BSP
\{
\addtogroup SYSTEM
\{
\addtogroup Start-up
\{
*/

#include <stdbool.h>
#include <stdio.h>
#include "sdk_defs.h"
#ifndef OS_BAREMETAL
#include "osal.h"
#endif
#ifdef CONFIG_RETARGET
#       include <stddef.h>
#       include "hw_uart.h"
#       include "hw_gpio.h"
#       include "hw_sys.h"
# if  (dg_configUSE_CONSOLE == 1)
# include "../adapters/src/sys_platform_devices_internal.h"
#       include "console.h"
# endif

#elif defined CONFIG_RTT
#       include <stdarg.h>
#       include "SEGGER_RTT.h"
#elif defined CONFIG_NO_PRINT

#elif dg_configSYSTEMVIEW
#       include <string.h>
#       include <stdarg.h>
#       include "SEGGER_SYSVIEW.h"
#       include "SEGGER_SYSVIEW_ConfDefaults.h"
#endif
#include "interrupts.h"



#if defined CONFIG_RETARGET

#ifndef CONFIG_RETARGET_UART
        #define CONFIG_RETARGET_UART            SER1_UART

        #define CONFIG_RETARGET_UART_TX_PORT    SER1_TX_PORT
        #define CONFIG_RETARGET_UART_TX_PIN     SER1_TX_PIN
        #define CONFIG_RETARGET_UART_TX_MODE    SER1_TX_MODE
        #define CONFIG_RETARGET_UART_TX_FUNC    SER1_TX_FUNC

        #define CONFIG_RETARGET_UART_RX_PORT    SER1_RX_PORT
        #define CONFIG_RETARGET_UART_RX_PIN     SER1_RX_PIN
        #define CONFIG_RETARGET_UART_RX_MODE    SER1_RX_MODE
        #define CONFIG_RETARGET_UART_RX_FUNC    SER1_RX_FUNC
#endif

#ifndef CONFIG_RETARGET_UART_BAUDRATE
#       define CONFIG_RETARGET_UART_BAUDRATE    HW_UART_BAUDRATE_115200
#endif

#ifndef CONFIG_RETARGET_UART_DATABITS
#       define CONFIG_RETARGET_UART_DATABITS    HW_UART_DATABITS_8
#endif

#ifndef CONFIG_RETARGET_UART_STOPBITS
#       define CONFIG_RETARGET_UART_STOPBITS    HW_UART_STOPBITS_1
#endif

#ifndef CONFIG_RETARGET_UART_PARITY
#       define CONFIG_RETARGET_UART_PARITY      HW_UART_PARITY_NONE
#endif

#define RETARGET_UART_IS_CONFIGURED_FLAG        (0x15)

void retarget_init(void)
{
#if dg_configUSE_CONSOLE
        console_init(&sys_platform_console_controller_conf);
#endif /* dg_configUSE_CONSOLE */
}

#if !dg_configUSE_CONSOLE

static void retarget_reinit(void)
{
        uart_config uart_init = {
                .baud_rate = CONFIG_RETARGET_UART_BAUDRATE,
                .data      = CONFIG_RETARGET_UART_DATABITS,
                .stop      = CONFIG_RETARGET_UART_STOPBITS,
                .parity    = CONFIG_RETARGET_UART_PARITY,
                .use_dma   = 0,
                .use_fifo  = 1,
                .rx_dma_channel = HW_DMA_CHANNEL_0,
                .tx_dma_channel = HW_DMA_CHANNEL_1,
        };

        hw_uart_init(CONFIG_RETARGET_UART, &uart_init);
        hw_uart_write_scr(CONFIG_RETARGET_UART, RETARGET_UART_IS_CONFIGURED_FLAG);
}

__STATIC_INLINE bool uart_needs_initialization()
{
        if (CONFIG_RETARGET_UART == HW_UART1) {
                return (!REG_GETF(CRG_COM, CLK_COM_REG, UART_ENABLE)
                        || (hw_uart_read_scr(HW_UART1) != RETARGET_UART_IS_CONFIGURED_FLAG));
        } else if (CONFIG_RETARGET_UART == HW_UART2) {
                return (!REG_GETF(CRG_COM, CLK_COM_REG, UART2_ENABLE)
                        || (hw_uart_read_scr(HW_UART2) != RETARGET_UART_IS_CONFIGURED_FLAG));
        } else if (CONFIG_RETARGET_UART == HW_UART3) {
                return (!REG_GETF(CRG_COM, CLK_COM_REG, UART3_ENABLE)
                        || (hw_uart_read_scr(HW_UART3) != RETARGET_UART_IS_CONFIGURED_FLAG));
        }
}

__USED
int _write (int fd, char *ptr, int len)
{
        hw_sys_pd_com_enable();
        HW_GPIO_SET_PIN_FUNCTION(CONFIG_RETARGET_UART_TX);
        HW_GPIO_PAD_LATCH_ENABLE(CONFIG_RETARGET_UART_TX);

        /* Enable UART if it's not enabled - can happen after exiting sleep */
        if (uart_needs_initialization()) {
                retarget_reinit();
        }

        /* Write "len" of char from "ptr" to file id "fd"
         * Return number of char written. */
        hw_uart_send(CONFIG_RETARGET_UART, ptr, len, NULL, NULL);

        while (hw_uart_is_busy(CONFIG_RETARGET_UART)) {}
        HW_GPIO_PAD_LATCH_DISABLE(CONFIG_RETARGET_UART_TX);
        hw_sys_pd_com_disable();

        return len;
}

__USED
int _read (int fd, char *ptr, int len)
{
        int ret = 0;

        hw_sys_pd_com_enable();
        HW_GPIO_SET_PIN_FUNCTION(CONFIG_RETARGET_UART_RX);
        HW_GPIO_PAD_LATCH_ENABLE(CONFIG_RETARGET_UART_RX);

        if (uart_needs_initialization()) {
                retarget_reinit();
        }

        /*
         * we need to wait for anything to read and return since stdio will assume EOF when we just
         * return 0 from _read()
         */
        while (!hw_uart_is_data_ready(CONFIG_RETARGET_UART)) {

#ifndef OS_BAREMETAL
                /*
                 * Use some short sleep to give a time for the Idle task to make its work
                 * e.g. freeing memory in OS e.g. deleting task if it is not needed anymore.
                 */
                OS_DELAY(2);
#endif
        }

        /* and now read as much as possible */
        while (hw_uart_is_data_ready(CONFIG_RETARGET_UART) && ret < len) {
                ptr[ret++] = hw_uart_read(CONFIG_RETARGET_UART);
        }

        HW_GPIO_PAD_LATCH_DISABLE(CONFIG_RETARGET_UART_RX);
        hw_sys_pd_com_disable();

        return ret;
}

void _ttywrch(int ch)
{
        _write(1 /* STDOUT */, (char*) &ch, 1);
}
#endif /* !dg_configUSE_CONSOLE */

/* defined CONFIG_RETARGET */

#elif defined CONFIG_RTT

/*
 * override libc printf()
 */
extern int SEGGER_RTT_vprintf(unsigned BufferIndex, const char * sFormat, va_list * pParamList);
int printf(const char *__restrict format, ...) __attribute__((format (printf, 1, 2)));

int printf(const char *__restrict format, ...)
{
        int ret;
        va_list param_list;

        va_start(param_list, format);
        ret = SEGGER_RTT_vprintf(0, format, &param_list);
        va_end(param_list);
        return ret;
}


/*
 *       _write()
 *
 * Function description
 *   Low-level write function.
 *   libc subroutines will use this system routine for output to all files,
 *   including stdout.
 *   Write data via RTT.
 */
int _write(int file, char *ptr, int len) {
        (void) file;  /* Not used, avoid warning */
        SEGGER_RTT_Write(0, ptr, len);
        return len;
}

int _read(int fd, char *ptr, int len)
{
        int ret = 1;

        /*
         * we need to return at least one character from this call as otherwise stdio functions
         * will assume EOF on file and won't read from it anymore.
         */
        ptr[0] = SEGGER_RTT_WaitKey();

        if (len > 1) {
                ret += SEGGER_RTT_Read(0, ptr + 1, len - 1);
        }

        return ret;
}

int _putc(int a)
{
        char *ptr = (char *)&a;
        int ret;
        ret = SEGGER_RTT_Write(0, ptr, 1);
        return ret;
}

/* defined CONFIG_RTT */

#elif dg_configSYSTEMVIEW

extern void _VPrintHost(const char* s, U32 Options, va_list* pParamList);

int printf(const char *__restrict format, ...) __attribute__((format (printf, 1, 2)));
int printf(const char *__restrict format, ...)
{
        va_list ParamList;
        va_start(ParamList, format);
        _VPrintHost(format, SEGGER_SYSVIEW_LOG, &ParamList);
        va_end(ParamList);
        return 0;
}


/*
 *       _write()
 *
 * Function description
 *   Low-level write function.
 *   libc subroutines will use this system routine for output to all files,
 *   including stdout.
 *   Write data via RTT.
 */
int _write(int file, char *ptr, int len) {
        (void) file;  /* Not used, avoid warning */
        static char send_buf[SEGGER_SYSVIEW_MAX_STRING_LEN - 1];
        int send_len;

        /*
         * Messages bigger than SEGGER_SYSVIEW_MAX_STRING_LEN are not supported by
         * systemview, so only the first SEGGER_SYSVIEW_MAX_STRING_LEN chars will
         * be actually sent to host.
         */
        send_len = (sizeof(send_buf) - 1 > len) ? len : sizeof(send_buf) - 1 ;
        memcpy(send_buf, ptr, send_len);
        send_buf[send_len] = '\0';
        SEGGER_SYSVIEW_Print(send_buf);

        return len;
}

int _read(int fd, char *ptr, int len)
{
        int ret = 1;

        /*
         * we need to return at least one character from this call as otherwise stdio functions
         * will assume EOF on file and won't read from it anymore.
         */
        ptr[0] = 0;
        return ret;
}

/* defined dg_configSYSTEMVIEW */

#elif defined CONFIG_NO_PRINT || !defined CONFIG_CUSTOM_PRINT

/* CONFIG_NO_PRINT, by default */

/*
 *       _write()
 *
 * Function description
 *   Low-level write function.
 *   libc subroutines will use this system routine for output to all files,
 *   including stdout.
 *   Empty stub.
 */
int _write(int file, char *ptr, int len) {
        return len;
}

int _read(int fd, char *ptr, int len)
{
        int ret = 1;

        /*
         * we need to return at least one character from this call as otherwise stdio functions
         * will assume EOF on file and won't read from it anymore.
         */
        ptr[0] = 0;
        return ret;
}

/*
 * override libc printf()
 *
 * empty stub
 */
int printf(const char *__restrict format, ...) __attribute__((format (printf, 1, 2)));
int printf(const char *__restrict format, ...)
{
        return 0;
}

int puts(const char *s)
{
        return EOF;
}

#endif


/*
 * System configuration checks
 */
#ifdef PRINT_POWER_RAIL_SETUP

# define PPRS_THEME "\n\n******* 1V8 and 1V8P power rails & Flash operational mode configuration *******\n"

# define PPRS_1V8_TITLE "1V8 rail:\n"
# if (dg_configPOWER_1V8_ACTIVE == 1)
#  define PPRS_1V8_ACTIVE "\tactive: on\n"
# elif (dg_configPOWER_1V8_ACTIVE == 0)
#  define PPRS_1V8_ACTIVE "\tactive: off\n"
# else
#  define PPRS_1V8_ACTIVE "\tactive: sw defined\n"
# endif
# if (dg_configPOWER_1V8_SLEEP == 1)
#  define PPRS_1V8_SLEEP "\tsleep: on\n"
# elif (dg_configPOWER_1V8_SLEEP == 0)
#  define PPRS_1V8_ACTIVE "\tsleep: off\n"
# else
#  define PPRS_1V8_SLEEP "\tsleep: sw defined\n"
# endif


#  define PPRS_1V8P_TITLE "1V8P rail:\n"
#  if (dg_configPOWER_1V8P_ACTIVE == 1)
#   define PPRS_1V8P_ACTIVE "\tactive: on\n"
#  elif (dg_configPOWER_1V8P_ACTIVE == 0)
#   define PPRS_1V8P_ACTIVE "\tactive: off\n"
#  else
#   define PPRS_1V8P_ACTIVE "\tactive: sw defined\n"
#  endif
#  if (dg_configPOWER_1V8P_SLEEP == 1)
#   define PPRS_1V8P_SLEEP "\tsleep: on\n"
#  elif (dg_configPOWER_1V8P_SLEEP == 0)
#   define PPRS_1V8P_SLEEP "\tsleep: off\n"
#  else
#   define PPRS_1V8P_SLEEP "\tsleep: sw defined\n"
#  endif


# if (dg_configFLASH_CONNECTED_TO == FLASH_CONNECTED_TO_1V8P)
#  define PPRS_FLASH_TITLE "Flash is connected to 1V8P"
#  if (dg_configFLASH_POWER_DOWN)
#   define PPRS_FLASH_POWER_DOWN "\nFlash Power Down mode is on\n"
#  else
#   define PPRS_FLASH_POWER_DOWN "\n"
#  endif
#  define PPRS_FLASH_POWER_OFF "\t"

# elif (dg_configFLASH_CONNECTED_TO == FLASH_CONNECTED_TO_1V8)
#  define PPRS_FLASH_TITLE "Flash is connected to 1V8"
#  if (dg_configFLASH_POWER_OFF)
#   define PPRS_FLASH_POWER_OFF "\nFlash Power Off mode is on"
#  else
#   define PPRS_FLASH_POWER_OFF "\t"
#  endif
#  if (dg_configFLASH_POWER_DOWN)
#   define PPRS_FLASH_POWER_DOWN "\nFlash Power Down mode is on\n"
#  else
#   define PPRS_FLASH_POWER_DOWN "\n"
#  endif

# else
#  define PPRS_FLASH_TITLE "A Flash is not connected"
#  define PPRS_FLASH_POWER_OFF "\t"
#  define PPRS_FLASH_POWER_DOWN "\n"
# endif /* dg_configFLASH_CONNECTED_TO */

# pragma message PPRS_THEME "> " PPRS_1V8_TITLE PPRS_1V8_ACTIVE PPRS_1V8_SLEEP "\n" "> " \
                 PPRS_1V8P_TITLE PPRS_1V8P_ACTIVE PPRS_1V8P_SLEEP "\n" "> " PPRS_FLASH_TITLE PPRS_FLASH_POWER_OFF \
                 PPRS_FLASH_POWER_DOWN

# undef PPRS_THEME
# undef PPRS_1V8_TITLE
# undef PPRS_1V8_ACTIVE
# undef PPRS_1V8_SLEEP
# undef PPRS_1V8P_TITLE
# undef PPRS_1V8P_ACTIVE
# undef PPRS_1V8P_SLEEP
# undef PPRS_FLASH_TITLE
# undef PPRS_FLASH_POWER_OFF
# undef PPRS_FLASH_POWER_DOWN
#endif /* PRINT_POWER_RAIL_SETUP */

#if (dg_configIMAGE_SETUP == PRODUCTION_MODE)
# if (dg_configCODE_LOCATION == NON_VOLATILE_IS_NONE)
#  error "Production mode build: Please define an appropriate code location!"
# endif

#else /* dg_configIMAGE_SETUP == DEVELOPMENT_MODE */
# if (dg_configCODE_LOCATION == NON_VOLATILE_IS_OTP)
#  pragma message"Development mode build: code will be built for OTP execution!"
# endif

# if (dg_configCODE_LOCATION == NON_VOLATILE_IS_FLASH) && (dg_configEMULATE_OTP_COPY == 1)
#  pragma message"Building for Flash code with OTP copy emulation! OTP copy is disabled..."
#  undef dg_configEMULATE_OTP_COPY
#  define dg_configEMULATE_OTP_COPY      0
# endif

# if (dg_configCODE_LOCATION == NON_VOLATILE_IS_FLASH) \
        && (dg_configFLASH_CONNECTED_TO == FLASH_IS_NOT_CONNECTED)
#  error "Building for Flash code but a Flash is not connected!"
# endif

# if dg_configPMU_ADAPTER
#  if (dg_configPOWER_1V8_ACTIVE == 0) && (dg_configPOWER_1V8_SLEEP == 1)
#   error "1V8 rail set to off during active and on during sleep..."
#  endif

#   if (dg_configPOWER_1V8P_ACTIVE == 0) && (dg_configPOWER_1V8P_SLEEP == 1)
#    error "1V8P rail set to off during active and on during sleep..."
#   endif

#  if (dg_configFLASH_CONNECTED_TO == FLASH_CONNECTED_TO_1V8) && (dg_configPOWER_1V8_ACTIVE == 0)
#   error "Flash is connected to the 1V8 rail but the rail is turned off (2)..."
#  endif


#  if (dg_configFLASH_POWER_DOWN == 1)
#   if defined(dg_configFLASH_POWER_OFF) && (dg_configFLASH_POWER_OFF == 1)
#    error "dg_configFLASH_POWER_DOWN and dg_configFLASH_POWER_OFF cannot be both set to 1"
#   endif
#    if ((dg_configFLASH_CONNECTED_TO == FLASH_CONNECTED_TO_1V8P || dg_configFLASH_CONNECTED_TO == FLASH_CONNECTED_TO_1V8F) \
        && (dg_configPOWER_1V8P_SLEEP == 1)) || \
       ((dg_configFLASH_CONNECTED_TO == FLASH_CONNECTED_TO_1V8) && (dg_configPOWER_1V8_SLEEP == 1))
        /* OK */
#    else
#     error "Flash power down is selected but the rail that the Flash is connected to is turned off..."
#    endif
#  endif /* dg_configFLASH_POWER_DOWN */
# endif /* dg_configPMU_ADAPTER */

# if (dg_configUSE_USB == 1) && (dg_configUSE_USB_CHARGER == 0) && (dg_configUSE_USB_ENUMERATION == 0)
#  error "Wrong USB configuration!"
# endif

# if !(defined(OS_FREERTOS) ^ defined(OS_BAREMETAL))
#  error "One (and only one) of OS_FREERTOS or OS_BAREMETAL must be defined"
# endif

# if (dg_configLOG_BLE_STACK_MEM_USAGE == 1 ) && (dg_configIMAGE_SETUP != DEVELOPMENT_MODE)
#  error "dg_configLOG_BLE_STACK_MEM_USAGE must not be set when building for PRODUCTION_MODE "
# endif

#endif /* dg_configIMAGE_SETUP */

#  if dg_configUSE_HW_CLK
#   if ((dg_configXTAL32M_SETTLE_TIME_IN_USEC * dg_configXTAL32K_FREQ * 125 + dg_configRC32M_FREQ_MIN/2) > 0xFFFFFFFFULL)
#    error "dg_configXTAL32M_SETTLE_TIME is too long"
#   endif
#  endif

#if (dg_configNVPARAM_ADAPTER)
# if (dg_configNVMS_ADAPTER != 1)
#  pragma message "NVMS adapter is mandatory to make use of NVPARAM and will be enabled silently"
#  undef dg_configNVMS_ADAPTER
#  define dg_configNVMS_ADAPTER 1
# endif
#endif

/*
 * Error about not supported DK motherboards
 */
# if (dg_configBLACK_ORCA_MB_REV != BLACK_ORCA_MB_REV_D)
#  error "dg_configBLACK_ORCA_MB_REV is set to a value that is not supported by this SDK."
# endif

/**
\}
\}
\}
*/
