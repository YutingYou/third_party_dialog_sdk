/**
 ****************************************************************************************
 *
 * @file main.c
 *
 * @brief LRA application
 *
 * Demo application for driving LRA vibration motors.
 *
 * Copyright (C) 2018-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdbool.h>
#include <stdio.h>
#include "hw_gpio.h"
#include "hw_wkup.h"
#include "osal.h"
#include "resmgmt.h"
#include "sys_clock_mgr.h"
#include "ad_haptic.h"
#include "platform_devices.h"


#define mainHAPTIC_TASK_PRIORITY              ( OS_TASK_PRIORITY_NORMAL )

static void prvSetupHardware(void);
static void Haptic_Task( void *pvParameters );

static OS_TASK xHandle;
static ad_haptic_handle_t haptic_handle;
__RETAINED_RW static OS_EVENT toggle_button_event;
__RETAINED_RW static uint8_t lra_state;

/**
 * @brief Toggle button callback function
 *
 * Signal Haptic_task in order to switch Haptic operation.
 */
static void toggle_button_cb(void)
{
        hw_wkup_reset_interrupt();
        OS_EVENT_SIGNAL_FROM_ISR(toggle_button_event);
}

/**
 * @brief Application-specific System initialization
 *
 * Perform any application-specific hardware configuration and create application tasks.
 * Basic system initialization has already been performed on startup.
 */
static void system_init(void *pvParameters)
{
        OS_TASK task_h = NULL;

        cm_sys_clk_init(sysclk_PLL96);
        cm_apb_set_clock_divider(apb_div1);
        cm_ahb_set_clock_divider(ahb_div1);
        cm_lp_clk_init();

        /* Prepare the hardware to run this demo. */
        prvSetupHardware();

        /* Set the desired sleep mode. */
        pm_sleep_mode_set(pm_mode_extended_sleep);

        /* Set the desired wakeup mode. */
        pm_set_sys_wakeup_mode(pm_sys_wakeup_mode_fast);

        /* Program Wake-up Controller to react to the button. */
        hw_wkup_init(NULL);
        hw_wkup_set_debounce_time(10);
        hw_wkup_configure_pin(KEY1_PORT, KEY1_PIN, 1,
                              KEY1_MODE == HW_GPIO_MODE_INPUT_PULLUP ? HW_WKUP_PIN_STATE_LOW
                                                                     : HW_WKUP_PIN_STATE_HIGH);
        hw_wkup_register_key_interrupt(toggle_button_cb, 1);
        hw_wkup_enable_irq();

#ifdef CONFIG_RETARGET
        void retarget_init(void);
        retarget_init();
#endif

        OS_TASK_CREATE( "HapticTask",                  /* The text name assigned to the task, for
                                                          debug only; not used by the kernel. */
                        Haptic_Task,                   /* The function that implements the task. */
                        NULL,                          /* The parameter passed to the task. */
                        configMINIMAL_STACK_SIZE * OS_STACK_WORD_SIZE * 2,
                                                       /* The number of bytes to allocate to the
                                                           stack of the task. */
                        mainHAPTIC_TASK_PRIORITY,      /* The priority assigned to the task. */
                        task_h );                      /* The task handle */
        OS_ASSERT(task_h);

        /* The work of the SysInit task is done. */
        OS_TASK_DELETE(xHandle);
}

static void sequence_complete(ad_haptic_user_async_cb_data_t data, AD_HAPTIC_PLAYBACK_END_TYPE end_type)
{
}

static void Haptic_Task( void *pvParameters )
{
        extern ad_haptic_controller_conf_t ad_haptic_config;
        ad_haptic_drive_mode_t drive_mode;

        printf("This is an LRA application.\r\nPress K1 button for driving the LRA with one of the prestored waveform sequences/patterns...\r\n");

        OS_EVENT_CREATE(toggle_button_event);

        /* Specify initial drive state. */
        lra_state = 0;

        for (;;) {
                OS_EVENT_WAIT(toggle_button_event,RES_WAIT_FOREVER);
                switch (lra_state) {
                case 0:
                        haptic_handle = ad_haptic_open(&ad_haptic_config);
                        ASSERT_WARNING(haptic_handle != NULL);
                        drive_mode.overdrive = AD_HAPTIC_OVERDRIVE_OFF;
                        drive_mode.freq_ctrl = AD_HAPTIC_FREQ_CTRL_OFF;
                        ASSERT_WARNING(ad_haptic_set_drive_mode(haptic_handle, &drive_mode) == AD_HAPTIC_ERROR_NONE);
                        ASSERT_WARNING(ad_haptic_play_wm_sequence(haptic_handle, 0, false, true, true, sequence_complete, NULL) == AD_HAPTIC_ERROR_NONE);
                        break;
                case 1:
                        drive_mode.overdrive = AD_HAPTIC_OVERDRIVE_OFF;
                        drive_mode.freq_ctrl = AD_HAPTIC_FREQ_CTRL_ON;
                        ASSERT_WARNING(ad_haptic_set_drive_mode(haptic_handle, &drive_mode) == AD_HAPTIC_ERROR_NONE);
                        ASSERT_WARNING(ad_haptic_play_wm_sequence(haptic_handle, 0, false, true, true, sequence_complete, NULL) == AD_HAPTIC_ERROR_NONE);
                        break;
                case 2:
                        ASSERT_WARNING(ad_haptic_play_wm_sequence(haptic_handle, 1, true, true, true, sequence_complete, NULL) == AD_HAPTIC_ERROR_NONE);
                        break;
                case 3:
                        ASSERT_WARNING(ad_haptic_stop_wm_sequence(haptic_handle, true) == AD_HAPTIC_ERROR_NONE);
                        drive_mode.overdrive = AD_HAPTIC_OVERDRIVE_ON;
                        drive_mode.freq_ctrl = AD_HAPTIC_FREQ_CTRL_ON;
                        ASSERT_WARNING(ad_haptic_set_drive_mode(haptic_handle, &drive_mode) == AD_HAPTIC_ERROR_NONE);
                        ASSERT_WARNING(ad_haptic_play_wm_sequence(haptic_handle, 2, true, true, true, sequence_complete, NULL) == AD_HAPTIC_ERROR_NONE);
                        break;
                case 4:
                        ASSERT_WARNING(ad_haptic_stop_wm_sequence(haptic_handle, true) == AD_HAPTIC_ERROR_NONE);
                        drive_mode.overdrive = AD_HAPTIC_OVERDRIVE_OFF;
                        drive_mode.freq_ctrl = AD_HAPTIC_FREQ_CTRL_OFF;
                        ASSERT_WARNING(ad_haptic_set_drive_mode(haptic_handle, &drive_mode) == AD_HAPTIC_ERROR_NONE);
                        ASSERT_WARNING(ad_haptic_set_half_period(haptic_handle, 735) == AD_HAPTIC_ERROR_NONE);
                        ASSERT_WARNING(ad_haptic_set_drive_level(haptic_handle, 0.75 * HW_HAPTIC_UFIX16_ONE + 0.5, HW_HAPTIC_DRIVE_LEVEL_REF_NOM_MAX) == AD_HAPTIC_ERROR_NONE);
                        ASSERT_WARNING(ad_haptic_set_state(haptic_handle, HW_HAPTIC_STATE_ACTIVE) == AD_HAPTIC_ERROR_NONE);
                        break;
                case 5:
                        drive_mode.overdrive = AD_HAPTIC_OVERDRIVE_OFF;
                        drive_mode.freq_ctrl = AD_HAPTIC_FREQ_CTRL_ON;
                        ASSERT_WARNING(ad_haptic_set_drive_mode(haptic_handle, &drive_mode) == AD_HAPTIC_ERROR_NONE);
                        ASSERT_WARNING(ad_haptic_set_drive_level(haptic_handle, 0.6 * HW_HAPTIC_UFIX16_ONE + 0.5, HW_HAPTIC_DRIVE_LEVEL_REF_NOM_MAX) == AD_HAPTIC_ERROR_NONE);
                        ASSERT_WARNING(ad_haptic_set_state(haptic_handle, HW_HAPTIC_STATE_ACTIVE) == AD_HAPTIC_ERROR_NONE);
                        break;
                case 6:
                        ASSERT_WARNING(ad_haptic_close(haptic_handle, true) == AD_HAPTIC_ERROR_NONE);
                        break;
                default:
                        break;
                }
                lra_state += 1;
                if (lra_state == 7) {
                        lra_state = 0;
                }
        }
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
         timer tasks to be created.  See the memory management section on the
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
