/**
 ****************************************************************************************
 *
 * @file main.c
 *
 * @brief PLT FW reference design
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#include <string.h>
#include <stdbool.h>
#include "osal.h"
#include "resmgmt.h"
#include "ad_ble.h"
#include "hw_gpio.h"
#include "hw_uart.h"
#include "sys_clock_mgr.h"
#include "sys_power_mgr.h"
#include "sys_watchdog.h"
#include "ble_mgr.h"
#include "dgtl.h"
#include "plt_fw.h"
#include "hw_pdc.h"
#include "hw_wkup.h"
#include "platform_devices.h"

#define PATCH_NOT_SET   (0xFFFFFFFF)

#if dg_configUSE_WDOG
__RETAINED_RW int8_t idle_task_wdog_id = -1;
#endif

/* Provided by linker script */
extern char __patchable_params;

/*
 * Perform any application specific hardware configuration.  The clocks,
 * memory, etc. are configured before main() is called.
 */
static void prvSetupHardware( void );

static OS_TASK xHandle;

/* Task priorities */
#define mainTEMPLATE_TASK_PRIORITY              ( OS_TASK_PRIORITY_NORMAL )
#define mainTASK_STACK_SIZE                     800

void plt_task(void *pvParameters);

static void patch_config_uart(void)
{
        uint32_t *pparams = (void *) &__patchable_params;

        /* Get UART parameters from patchable area, if their value is not 0xffffffff. */
        if (pparams[0] != PATCH_NOT_SET) {
                platform_dgtl_io_conf.tx.port = (HW_GPIO_PORT) pparams[0];
        }

        if (pparams[1] != PATCH_NOT_SET) {
                platform_dgtl_io_conf.tx.pin = (HW_GPIO_PIN) pparams[1];
        }

        if (pparams[2] != PATCH_NOT_SET) {
                platform_dgtl_io_conf.rx.port = (HW_GPIO_PORT) pparams[2];
        }

        if (pparams[3] != PATCH_NOT_SET) {
                platform_dgtl_io_conf.rx.pin = (HW_GPIO_PIN) pparams[3];
        }

        if (pparams[4] != PATCH_NOT_SET)
        {
                HW_UART_BAUDRATE *baud_rate = &platform_dgtl_uart_driver_conf.hw_conf.baud_rate;

                switch (pparams[4])
                {
                case 4800:
                        *baud_rate = HW_UART_BAUDRATE_4800;
                        break;
                case 9600:
                        *baud_rate = HW_UART_BAUDRATE_9600;
                        break;
                case 14400:
                        *baud_rate = HW_UART_BAUDRATE_14400;
                        break;
                case 19200:
                        *baud_rate = HW_UART_BAUDRATE_19200;
                        break;
                case 28800:
                        *baud_rate = HW_UART_BAUDRATE_28800;
                        break;
                case 38400:
                        *baud_rate = HW_UART_BAUDRATE_38400;
                        break;
                case 57600:
                        *baud_rate = HW_UART_BAUDRATE_57600;
                        break;
                case 115200:
                        *baud_rate = HW_UART_BAUDRATE_115200;
                        break;
                case 230400:
                        *baud_rate = HW_UART_BAUDRATE_230400;
                        break;
                case 500000:
                        *baud_rate = HW_UART_BAUDRATE_500000;
                        break;
                case 1000000:
                        *baud_rate = HW_UART_BAUDRATE_1000000;
                        break;
                }
        }
}
/**
 * @brief System Initialization and creation of the BLE task
 */
static void system_init( void *pvParameters )
{
        OS_TASK plt_handle;
        OS_BASE_TYPE status;

        /* Prepare clocks. Note: cm_cpu_clk_set() and cm_sys_clk_set() can be called only from a
         * task since they will suspend the task until the XTAL16M/XTAL32M has settled and, maybe,
         * the PLL is locked.
         */
        cm_sys_clk_init(sysclk_XTAL32M);
        cm_apb_set_clock_divider(apb_div1);
        cm_ahb_set_clock_divider(ahb_div1);
        cm_lp_clk_init();

        /*
         * Initialize platform watchdog
         */
        sys_watchdog_init();

#if dg_configUSE_WDOG
        // Register the Idle task first.
        idle_task_wdog_id = sys_watchdog_register(false);
        ASSERT_WARNING(idle_task_wdog_id != -1);
        sys_watchdog_configure_idle_id(idle_task_wdog_id);
#endif


        /* Prepare the hardware to run this demo. */
        prvSetupHardware();

        /* Set the desired sleep mode. */
        pm_sleep_mode_set(pm_mode_active);

        /* Initialize BLE Manager */
        ble_mgr_init();

        /* start the test app task */
        status = OS_TASK_CREATE("PltTask",              /* The text name assigned to the task - for debug only as it is not used by the kernel. */
                       plt_task,                        /* The function that implements the task. */
                       NULL,                            /* The parameter passed to the task. */
                       mainTASK_STACK_SIZE,             /* The size of the stack to allocate to the task. */
                       mainTEMPLATE_TASK_PRIORITY,      /* The priority assigned to the task. */
                       plt_handle);                     /* The task handle. */

        OS_ASSERT(status == OS_TASK_CREATE_SUCCESS);

        /* Initialize DGTL */
        dgtl_init();

        OS_TASK_DELETE( xHandle );
}


/**
 * @brief External BLE Host demo main creates the BLE Adapter and Serial Adapter tasks.
 */
int main( void )
{
        OS_BASE_TYPE status;


        /* Set UART's RX/TX lines and baudrate if given in patch area */
        patch_config_uart();

        /* Start the two tasks as described in the comments at the top of this
        file. */
        status = OS_TASK_CREATE("SysInit",                /* The text name assigned to the task - for debug only as it is not used by the kernel. */
                                system_init,              /* The System Initialization task. */
                                ( void * ) 0,             /* The parameter passed to the task. */
                                1200,                     /* The size of the stack to allocate to the task. */
                                OS_TASK_PRIORITY_HIGHEST, /* The priority assigned to the task. */
                                xHandle);                 /* The task handle is not required, so NULL is passed. */
        OS_ASSERT(status == OS_TASK_CREATE_SUCCESS);

        /* Start the tasks and timer running. */
        vTaskStartScheduler();

        /* If all is well, the scheduler will now be running, and the following
        line will never be reached.  If the following line does execute, then
        there was insufficient FreeRTOS heap memory available for the idle and/or
        timer tasks     to be created.  See the memory management section on the
        FreeRTOS web site for more details. */
        for ( ;; );
}

static void periph_init(void)
{


#if CONFIG_USE_HW_FLOW_CONTROL == 1
        HW_GPIO_SET_PIN_FUNCTION(SER1_RTS);
        HW_GPIO_SET_PIN_FUNCTION(SER1_CTS);
#endif /* CONFIG_USE_HW_FLOW_CONTROL */
}

static void prvSetupHardware( void )
{

        /* Init hardware */
        pm_system_init(periph_init);

}

/**
 * @brief Malloc fail hook
 */
void vApplicationMallocFailedHook( void )
{
        /* vApplicationMallocFailedHook() will only be called if
        configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
        function that will get called if a call to OS_MALLOC() fails.
        OS_MALLOC() is called internally by the kernel whenever a task, queue,
        timer or semaphore is created.  It is also called by various parts of the
        demo application.  If heap_1.c or heap_2.c are used, then the size of the
        heap available to OS_MALLOC() is defined by configTOTAL_HEAP_SIZE in
        FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
        to query the size of free heap space that remains (although it does not
        provide information on how the remaining heap might be fragmented). */
        ASSERT_ERROR(0);
}

/**
 * @brief Application idle task hook
 */
void vApplicationIdleHook( void )
{
        /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
           to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
           task. It is essential that code added to this hook function never attempts
           to block in any way (for example, call OS_QUEUE_GET() with a block time
           specified, or call OS_DELAY()).  If the application makes use of the
           OS_TASK_DELETE() API function (as this demo application does) then it is also
           important that vApplicationIdleHook() is permitted to return to its calling
           function, because it is the responsibility of the idle task to clean up
           memory allocated by the kernel to any task that has since been deleted. */

#if dg_configUSE_WDOG
        sys_watchdog_notify(idle_task_wdog_id);
#endif
}

/**
 * @brief Application stack overflow hook
 */
void vApplicationStackOverflowHook( OS_TASK pxTask, char *pcTaskName )
{
        ( void ) pcTaskName;
        ( void ) pxTask;

        /* Run time stack overflow checking is performed if
        configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
        function is called if a stack overflow is detected. */
        ASSERT_ERROR(0);
}

/**
 * @brief Application tick hook
 */
void vApplicationTickHook( void )
{
}
