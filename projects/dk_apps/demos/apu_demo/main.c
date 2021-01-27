/**
 ****************************************************************************************
 *
 * @file main.c
 *
 * @brief APU demo application.
 *
 * Copyright (C) 2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdio.h>
#include <stdbool.h>

#include "osal.h"
#include "resmgmt.h"
#include "hw_gpio.h"
#include "hw_watchdog.h"
#include "sys_clock_mgr.h"
#include "sys_power_mgr.h"

#include "periph_setup.h"
#include "audio_task.h"
#include "DA7218_driver.h"

/* Task priorities */
#define mainTEMPLATE_TASK_PRIORITY              ( OS_TASK_PRIORITY_NORMAL )

/*
 * Perform any application specific hardware configuration.  The clocks,
 * memory, etc. are configured before main() is called.
 */
static void prvSetupHardware( void );

static OS_TASK xHandle;

static void system_init( void *pvParameters )
{
#if defined CONFIG_RETARGET
        extern void retarget_init(void);
#endif

        cm_sys_clk_init(sysclk_XTAL32M);

        cm_apb_set_clock_divider(apb_div1);
        cm_ahb_set_clock_divider(ahb_div1);
        cm_lp_clk_init();

        /* Prepare the hardware to run this demo. */
        prvSetupHardware();

#if defined CONFIG_RETARGET
        retarget_init();
#endif

        /* Set the desired sleep mode. */
        pm_sleep_mode_set(pm_mode_extended_sleep);

        /* Initialize Codec7218 */
        DA7218_Init();

        /* Enable Audio Codec7218 */
        DA7218_Enable();

        /* Start main task here (text menu available via UART1 to control application) */
        OS_TASK_CREATE( "Audio task",                   /* The text name assigned to the task, for
                                                           debug only; not used by the kernel. */
                        audio_task,                     /* The function that implements the task. */
                        NULL,                           /* The parameter passed to the task. */
                        256 * OS_STACK_WORD_SIZE,       /* The number of bytes to allocate to the
                                                           stack of the task. */
                        mainTEMPLATE_TASK_PRIORITY,     /* The priority assigned to the task. */
                        context_demo_apu.audio_task );  /* The task handle */
        OS_ASSERT(context_demo_apu.audio_task);

        /* the work of the SysInit task is done */
        OS_TASK_DELETE( xHandle );
}

/**
 * @brief Template main creates a SysInit task, which creates a Template task
 */
int main( void )
{
        OS_BASE_TYPE status;

        /* Start the two tasks as described in the comments at the top of this
        file. */
        status = OS_TASK_CREATE("SysInit",              /* The text name assigned to the task, for
                                                           debug only; not used by the kernel. */
                        system_init,                    /* The System Initialization task. */
                        ( void * ) 0,                   /* The parameter passed to the task. */
                        2 * configMINIMAL_STACK_SIZE * OS_STACK_WORD_SIZE,
                                                        /* The number of bytes to allocate to the
                                                           stack of the task. */
                        OS_TASK_PRIORITY_HIGHEST,       /* The priority assigned to the task. */
                        xHandle );                      /* The task handle */
        OS_ASSERT(status == OS_TASK_CREATE_SUCCESS);



        /* Start the tasks and timer running. */
        vTaskStartScheduler();

        /* If all is well, the scheduler will now be running, and the following
        line will never be reached.  If the following line does execute, then
        there was insufficient FreeRTOS heap memory available for the idle and/or
        timer tasks to be created.  See the memory management section on the
        FreeRTOS web site for more details. */
        for ( ;; );

}

/**
 * @brief Initialize the peripherals domain after power-up.
 *
 */
static void periph_init(void)
{
#if DEMO_PDM_MIC || DEMO_PDM_RECORD_PLAYBACK
        hw_gpio_pad_latch_enable(PDM_CLK_PIN);
        hw_gpio_configure_pin(PDM_CLK_PIN, HW_GPIO_MODE_OUTPUT, HW_GPIO_FUNC_PDM_CLK, false);

        hw_gpio_pad_latch_enable(PDM_DATA_PIN);
        hw_gpio_configure_pin(PDM_DATA_PIN, HW_GPIO_MODE_INPUT, HW_GPIO_FUNC_PDM_DATA, false);

        hw_gpio_pad_latch_enable(PDM_MIC_PE);
        hw_gpio_configure_pin(PDM_MIC_PE, HW_GPIO_MODE_OUTPUT, HW_GPIO_FUNC_GPIO, false);
#endif

        hw_gpio_pad_latch_enable(PCM_CLK_PIN);
        hw_gpio_pad_latch_enable(PCM_FSC_PIN);

        hw_gpio_configure_pin(PCM_CLK_PIN, HW_GPIO_MODE_OUTPUT, HW_GPIO_FUNC_PCM_CLK, false);
        hw_gpio_configure_pin(PCM_FSC_PIN, HW_GPIO_MODE_OUTPUT, HW_GPIO_FUNC_PCM_FSC, false);

        hw_gpio_pad_latch_enable(PCM_DO_PIN);
        hw_gpio_configure_pin(PCM_DO_PIN, HW_GPIO_MODE_OUTPUT, HW_GPIO_FUNC_PCM_DO, false);

#if DEMO_PCM_MIC || DEMO_PCM_RECORD_PLAYBACK
        hw_gpio_pad_latch_enable(PCM_DI_PIN);
        hw_gpio_configure_pin(PCM_DI_PIN, HW_GPIO_MODE_INPUT, HW_GPIO_FUNC_PCM_DI, false);
#endif

#if DEMO_PDM_RECORD_PLAYBACK || DEMO_PCM_RECORD_PLAYBACK
        hw_gpio_pad_latch_enable(BTN_PIN);
        hw_gpio_configure_pin(BTN_PIN, KEY1_MODE, KEY1_FUNC, false);
#endif

}

void printf_settings(sys_audio_device_t *dev, device_direction_t dir)
{
        if (dir == INPUT_DEVICE) {
                printf("\n\r>>> Input device: ");
        } else {
                printf("\n\r>>> Output device: ");
        }

        switch (dev->device_type) {
        case AUDIO_PDM:
                printf("PDM <<<\n\r");
                printf("1. Mode:                     %s\n\r", ((dev->pdm_param.mode == MODE_SLAVE) ? "Slave" : "Master"));
                printf("2. Clock frequency:          %ld Hz\n\r", dev->pdm_param.clk_frequency);
                printf("3. Channels recorded:        %s\n\r", PRINTF_RECORDED_CHANNELS(dev->pdm_param.channel));
                if (dir == INPUT_DEVICE) {
                        printf("4. In delay:                 %d\n\r", dev->pdm_param.in_delay);
                } else {
                        printf("4. Out delay:                %d\n\r", dev->pdm_param.out_delay);
                }
                break;
        case AUDIO_PCM:
                printf("PCM <<<\n\r");
                printf("1.  Mode:                    %s\n\r", ((dev->pcm_param.mode == MODE_SLAVE) ? "Slave" : "Master"));
                printf("2.  Format:                  %s\n\r", ((dev->pcm_param.format == PCM_MODE) ? "PCM" :
                                                               (dev->pcm_param.format == I2S_MODE) ? "I2S" :
                                                               (dev->pcm_param.format == IOM2_MODE)? "IOM2": "TDM"));
                printf("3.  Sample rate:             %ld Hz\n\r", dev->pcm_param.sample_rate);
                printf("4.  Total channel number:    %d\n\r", dev->pcm_param.total_channel_num);
                printf("5.  Channel delay:           %d\n\r", dev->pcm_param.channel_delay);
                printf("6.  Bits depth:              %d\n\r", dev->pcm_param.bits_depth);
                printf("7.  Enable dithering:        %d\n\r", dev->pcm_param.enable_dithering);
                printf("8.  FSC delay:               %s\n\r", (dev->pcm_param.fsc_delay == HW_PCM_FSC_STARTS_1_CYCLE_BEFORE_MSB_BIT) ?
                       "HW_PCM_FSC_STARTS_1_CYCLE_BEFORE_MSB_BIT": "HW_PCM_FSC_STARTS_SYNCH_TO_MSB_BIT");
                printf("9.  Inverted FSC polarity:   %d\n\r", dev->pcm_param.inverted_fsc_polarity);
                printf("10. Inverted_clock polarity: %d\n\r", dev->pcm_param.inverted_clk_polarity);
                printf("11. Cycles per bit:          %s\n\r", (dev->pcm_param.cycle_per_bit == HW_PCM_ONE_CYCLE_PER_BIT) ?
                       "HW_PCM_ONE_CYCLE_PER_BIT": "HW_PCM_TWO_CYCLE_PER_BIT");
                printf("12. FSC length:              %d\n\r", dev->pcm_param.fsc_length);
                if (dev->pcm_param.fsc_length >= dev->pcm_param.total_channel_num * (dev->pcm_param.bits_depth / 8)) {
                        printf("\n\r\n\r>>> Warning fsc_length time is bigger than total channel num <<<\n\r");
                }
                break;
        case AUDIO_MEMORY:
                printf("MEMORY <<<\n\r");
                printf("1. Sample rate:              %ld Hz\n\r", dev->memory_param.sample_rate);
                printf("2. Stereo:                   %s\n\r", dev->memory_param.stereo ? "Yes" : "No");
                printf("3. Bits depth:               %d\n\r", dev->memory_param.bits_depth);
                break;
        default :
                break;
        }
}

/**
 * @brief Hardware Initialization
 */
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


