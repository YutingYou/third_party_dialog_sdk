/**
 ****************************************************************************************
 *
 * @file main.c
 *
 * @brief ERM application
 *
 * Demo application for driving ERM vibration motors.
 *
 * Copyright (C) 2018-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "hw_gpio.h"
#include "hw_watchdog.h"
#include "hw_wkup.h"
#include "osal.h"
#include "resmgmt.h"
#include "sys_clock_mgr.h"
#include "sys_power_mgr.h"
#include "hw_haptic.h"


/*
 * ERM vibration motor config
 */
#define ERM_MAX_NOM_V     3000
#define ERM_MAX_ABS_V     3300
#define ERM_R             60

/*
 * Battery voltage (in mV)
 */
#define V_BAT     3000

/*
 * ERM drive level (when switched on) (range of 0 to 1.0)
 */
#define ERM_DRIVE_LEVEL     0.75

/*
 * ERM drive polarity (HDRVP or HDRVM output pin)
 */
#define ERM_DRIVE_PIN     HW_HAPTIC_ERM_OUTPUT_HDRVP

static void prvSetupHardware(void);

static OS_TASK xHandle;
static bool state;

/**
 * @brief Toggle button callback function
 *
 * Start/Stop ERM.
 */
static void toggle_button_cb(void)
{
        hw_wkup_reset_interrupt();
        state ^= 1;
        hw_haptic_set_state(state);
}

/**
 * @brief Application-specific System initialization
 *
 * Perform any application-specific hardware configuration and create application tasks.
 * Basic system initialization has already been performed on startup.
 */
static void system_init(void *pvParameters)
{
        cm_sys_clk_init(sysclk_XTAL32M);
        cm_apb_set_clock_divider(apb_div1);
        cm_ahb_set_clock_divider(ahb_div1);
        cm_lp_clk_init();

        /* Prepare the hardware to run this demo. */
        prvSetupHardware();

        /* Program WKUPCT to react to the button. */
        hw_wkup_init(NULL);
        hw_wkup_set_debounce_time(10);
        hw_wkup_configure_pin(KEY1_PORT, KEY1_PIN, 1,
                              KEY1_MODE == HW_GPIO_MODE_INPUT_PULLUP ? HW_WKUP_PIN_STATE_LOW
                                                                     : HW_WKUP_PIN_STATE_HIGH);
        hw_wkup_register_key_interrupt(toggle_button_cb, 1);
        hw_wkup_enable_irq();

        /* Calculate ERM configuration parameters */
        uint16_t D_nom_max, D_abs_max;

        D_nom_max = HW_HAPTIC_CONV_VOLT_TO_DUTYCYCLE(ERM_MAX_NOM_V, V_BAT, ERM_R);
        D_abs_max = HW_HAPTIC_CONV_VOLT_TO_DUTYCYCLE(ERM_MAX_ABS_V, V_BAT, ERM_R);


        /* Initialize Haptic Driver/Controller. */
        haptic_config_t erm_cfg = {
                .duty_cycle_nom_max = D_nom_max,
                .duty_cycle_abs_max = D_abs_max,
                .signal_out = ERM_DRIVE_PIN,
        };
        hw_haptic_init(&erm_cfg);

        /* Set initial drive state. */
        state = false;

        /* Set drive level. */
        hw_haptic_set_drive_level(ERM_DRIVE_LEVEL * HW_HAPTIC_UFIX16_ONE, HW_HAPTIC_DRIVE_LEVEL_REF_NOM_MAX);

        printf("\r\nThis is an ERM application.\r\nPress K1 button for starting/stopping the ERM...\r\n");

        /* Apply initial drive state. */
        hw_haptic_set_state(HW_HAPTIC_STATE_IDLE);

        /* The work of the SysInit task is done. */
        OS_TASK_DELETE(xHandle);
}

/**
 * @brief Main function
 *
 * Create system_init task and start Scheduler.
 */
int main(void)
{
        OS_BASE_TYPE status;

        status = OS_TASK_CREATE("SysInit",              /* The text name assigned to the task, for
                                                           debug only; not used by the kernel. */
                        system_init,                    /* The System Initialization task. */
                        ( void * ) 0,                   /* The parameter passed to the task. */
                        configMINIMAL_STACK_SIZE * OS_STACK_WORD_SIZE,
                                                        /* The number of bytes to allocate to the
                                                           stack of the task. */
                        OS_TASK_PRIORITY_HIGHEST,       /* The priority assigned to the task. */
                        xHandle );                      /* The task handle */
        OS_ASSERT(status == OS_TASK_CREATE_SUCCESS);

        /* Start the tasks and timer running. */
        vTaskStartScheduler();
        /* If all is well, the scheduler will now be running, and the following
         line will never be reached. If the following line does execute, then
         there was insufficient FreeRTOS heap memory available for the idle and/or
         timer tasks to be created. See the memory management section on the
         FreeRTOS web site for more details. */
        for (;;);
}

/**
 * @brief Application-specific peripherals initialization
 *
 * To be performed every time after wake-up.
 */
static void periph_init(void)
{
}

/**
 * @brief Application-specific hardware initialization
 */
static void prvSetupHardware(void)
{
        /* Initialize hardware. */
        pm_system_init(periph_init);

        /* Configure PIN P0_6 for the button. */
        HW_GPIO_SET_PIN_FUNCTION(KEY1);
        HW_GPIO_PAD_LATCH_ENABLE(KEY1);
        HW_GPIO_PAD_LATCH_DISABLE(KEY1);
}
/**
 * @brief Malloc fail hook
 */
void vApplicationMallocFailedHook(void)
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
        ASSERT_ERROR(0);                ;
}
/**
 * @brief Application idle task hook
 */
void vApplicationIdleHook(void)
{
        /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
         to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
         task.  It is essential that code added to this hook function never attempts
         to block in any way (for example, call OS_QUEUE_GET() with a block time
         specified, or call OS_DELAY()).  If the application makes use of the
         OS_TASK_DELETE() API function (as this demo application does) then it is also
         important that vApplicationIdleHook() is permitted to return to its calling
         function, because it is the responsibility of the idle task to clean up
         memory allocated by the kernel to any task that has since been deleted. */
}
/**
 * @brief Application stack overflow hook
 */
void vApplicationStackOverflowHook( OS_TASK pxTask, char *pcTaskName)
{
        (void)pcTaskName;
        (void)pxTask;
        /* Run time stack overflow checking is performed if
         configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
        function is called if a stack overflow is detected. */
        ASSERT_ERROR(0);
}
/**
 * @brief Application tick hook
 */
void vApplicationTickHook(void)
{
}
