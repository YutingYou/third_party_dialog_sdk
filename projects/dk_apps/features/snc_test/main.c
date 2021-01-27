/**
 ****************************************************************************************
 *
 * @file main.c
 *
 * @brief Sensor Node Controller Demo application tasks
 *
 * Copyright (C) 2017-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "peripheral_setup.h"
#include "platform_devices.h"

#include "osal.h"
#include "resmgmt.h"
#include "hw_cpm.h"
#include "hw_gpio.h"
#include "hw_pd.h"
#include "hw_watchdog.h"
#include "sys_clock_mgr.h"
#include "sys_power_mgr.h"
#include "ad_snc.h"
#include "hw_pdc.h"
#include "ad_spi.h"
#include "ad_i2c.h"

#include "hw_wkup.h"
#include "hw_rtc.h"

#include "snc_defs.h"
#include "snc_queues.h"

#include "adxl362.h"
#include "adxl362_ucodes.h"
#include "adxl362_snc_demo.h"

#include "bh1750.h"
#include "bh1750_ucodes.h"
#include "bh1750_snc_demo.h"

#include "24fc256.h"
#include "24fc256_snc_demo.h"
#include "24fc256_ucodes.h"

/* Task priorities */
#define mainSNC_TASK_PRIORITY           ( OS_TASK_PRIORITY_NORMAL )

/******** Notification Bitmasks *************/
#define ADXL362_FIFO_NOTIFICATION       ( 1 << 0 )
#define BH1750_NOTIFICATION             ( 1 << 1 )
#define EEPROM_WRITE_NOTIFICATION       ( 1 << 2 )
#define EEPROM_READ_NOTIFICATION        ( 1 << 3 )

#define nEEPROM_DEMO
#define ADXL362_DEMO
#define BH1750_DEMO
#define BH1750_DEMO_MODE                (1) /* 0: data collect, 1: data collect with queue, 2: notifier */

__RETAINED static OS_TASK prvSNCTask_h;

/*
 * Perform any application specific hardware configuration.  The clocks,
 * memory, etc. are configured before main() is called.
 */
static void prvSetupHardware(void);
/*
 * Task functions
 */
static void prvSNCTask(void *pvParameters);

static OS_TASK xHandle;

static void system_init(void *pvParameters)
{
        OS_BASE_TYPE status;

        REG_SETF(GPREG, DEBUG_REG, SYS_CPU_FREEZE_EN, 0);

#if defined CONFIG_RETARGET
        extern void retarget_init(void);
#endif /* CONFIG_RETARGET */

        /*
         * Prepare clocks. Note: cm_cpu_clk_set() and cm_sys_clk_set() can be called only from a
         * task since they will suspend the task until the XTAL32M has settled and, maybe, the PLL
         * is locked.
         */
        cm_sys_clk_init(sysclk_XTAL32M);
        cm_apb_set_clock_divider(apb_div1);
        cm_ahb_set_clock_divider(ahb_div1);
        cm_lp_clk_init();

        /* Prepare the hardware to run this demo */
        prvSetupHardware();

#if defined CONFIG_RETARGET
        retarget_init();
#endif /* CONFIG_RETARGET */

        /* Set the desired sleep mode */
        pm_sleep_mode_set(pm_mode_active);

        /* Create main SNC demo task  */
        status = OS_TASK_CREATE( "SNC_APP",             /* The text name assigned to the task, for
                                                           debug only; not used by the kernel. */
                        prvSNCTask,                     /* The function that implements the task. */
                        NULL,                           /* The parameter passed to the task. */
                        1024 * OS_STACK_WORD_SIZE,      /* Stack size allocated for the task in bytes. */
                        mainSNC_TASK_PRIORITY,          /* The priority assigned to the task. */
                        prvSNCTask_h );                 /* The task handle. */
        OS_ASSERT(status == OS_TASK_CREATE_SUCCESS);

        /* The work of the SysInit task is done */
        OS_TASK_DELETE(xHandle);
}

/**
 * @brief Basic initialization and creation of the system initialization task.
 */
int main(void)
{
        OS_BASE_TYPE status;

        /* Create SysInit task. */
        status = OS_TASK_CREATE("SysInit",              /* The text name assigned to the task, for
                                                           debug only; not used by the kernel. */
                        system_init,                    /* The System Initialization task. */
                        ( void * ) 0,                   /* The parameter passed to the task. */
                        configMINIMAL_STACK_SIZE * OS_STACK_WORD_SIZE, /* Stack size allocated for the task in bytes. */
                        OS_TASK_PRIORITY_HIGHEST,       /* The priority assigned to the task. */
                        xHandle );                      /* The task handle. */
        OS_ASSERT(status == OS_TASK_CREATE_SUCCESS);

        /* Start the tasks and timer running. */
        vTaskStartScheduler();

        /* If all is well, the scheduler will now be running, and the following
         line will never be reached.  If the following line does execute, then
         there was insufficient FreeRTOS heap memory available for the idle and/or
         timer tasks     to be created.  See the memory management section on the
         FreeRTOS web site for more details. */
        for (;;);
}

#ifdef ADXL362_DEMO
static uint32_t adxl_ucode_id;
/**
 * @brief ADXL362 INT1 SNC notification callback
 *
 */
static void snc_demo_app_adxlint1_cb(void)
{
        OS_TASK_NOTIFY(prvSNCTask_h, ADXL362_FIFO_NOTIFICATION, OS_NOTIFY_SET_BITS);
}
#endif /* ADXL362_DEMO */

#ifdef BH1750_DEMO
/**
 * @brief BH1750 SNC local FIFO has samples notification callback
 *
 */
static void snc_demo_app_bh1750_cb(void)
{
        OS_TASK_NOTIFY(prvSNCTask_h, BH1750_NOTIFICATION, OS_NOTIFY_SET_BITS);
}
#endif /* BH1750_DEMO */

#ifdef EEPROM_DEMO
#define EEPROM_TEST_DATA_SIZE   1024
static uint32_t e_writer_ucode_id;
static uint32_t e_reader_ucode_id;

/* The source test buffer that contains the Lorem Ipsum text
 *
 * Lorem ipsum dolor sit amet, consectetur adipiscing elit. Donec d
 * ignissim lacus a posuere maximus. Aliquam erat volutpat. Proin e
 * lementum tellus ut accumsan feugiat. Duis vel felis nisi. Curabi
 * tur feugiat risus eu lacinia volutpat. Ut viverra arcu finibus m
 * attis dictum. Pellentesque interdum mi a felis tempus auctor. Nu
 * nc et pharetra mi. In ullamcorper bibendum urna, ut congue urna
 * egestas eget. In hac habitasse platea dictumst. Morbi luctus tri
 * stique urna, non convallis ante euismod vitae. Aliquam cursus ma
 * uris non turpis facilisis, posuere ullamcorper velit cursus. Mau
 * ris vel volutpat metus. Pellentesque dictum quis ligula id euism
 * od. In vestibulum olor vel metus tincidunt, in sollicitudin risu
 * s tincidunt. Aenean tristique nisi quam, a ultricies risus tempu
 * s a. Morbi turpis velit, scelerisque quis neque vulputate, susci
 * pit porttitor metus. Mauris pulvinar dapibus velit, in suscipit
 * mauris. Morbi sodales est massa, nec tempor est consectetur et.
 * Fusce non enim aliquet, blandit dui at, viverra nulla. Cras sed.
 *
 * */
const uint8_t e_src_buff[EEPROM_TEST_DATA_SIZE] = {
        'L', 'o', 'r', 'e', 'm', ' ', 'i', 'p', 's', 'u', 'm', ' ', 'd', 'o', 'l', 'o',
        'r', ' ', 's', 'i', 't', ' ', 'a', 'm', 'e', 't', ',', ' ', 'c', 'o', 'n', 's',
        'e', 'c', 't', 'e', 't', 'u', 'r', ' ', 'a', 'd', 'i', 'p', 'i', 's', 'c', 'i',
        'n', 'g', ' ', 'e', 'l', 'i', 't', '.', ' ', 'D', 'o', 'n', 'e', 'c', ' ', 'd',
        'i', 'g', 'n', 'i', 's', 's', 'i', 'm', ' ', 'l', 'a', 'c', 'u', 's', ' ', 'a',
        ' ', 'p', 'o', 's', 'u', 'e', 'r', 'e', ' ', 'm', 'a', 'x', 'i', 'm', 'u', 's',
        '.', ' ', 'A', 'l', 'i', 'q', 'u', 'a', 'm', ' ', 'e', 'r', 'a', 't', ' ', 'v',
        'o', 'l', 'u', 't', 'p', 'a', 't', '.', ' ', 'P', 'r', 'o', 'i', 'n', ' ', 'e',
        'l', 'e', 'm', 'e', 'n', 't', 'u', 'm', ' ', 't', 'e', 'l', 'l', 'u', 's', ' ',
        'u', 't', ' ', 'a', 'c', 'c', 'u', 'm', 's', 'a', 'n', ' ', 'f', 'e', 'u', 'g',
        'i', 'a', 't', '.', ' ', 'D', 'u', 'i', 's', ' ', 'v', 'e', 'l', ' ', 'f', 'e',
        'l', 'i', 's', ' ', 'n', 'i', 's', 'i', '.', ' ', 'C', 'u', 'r', 'a', 'b', 'i',
        't', 'u', 'r', ' ', 'f', 'e', 'u', 'g', 'i', 'a', 't', ' ', 'r', 'i', 's', 'u',
        's', ' ', 'e', 'u', ' ', 'l', 'a', 'c', 'i', 'n', 'i', 'a', ' ', 'v', 'o', 'l',
        'u', 't', 'p', 'a', 't', '.', ' ', 'U', 't', ' ', 'v', 'i', 'v', 'e', 'r', 'r',
        'a', ' ', 'a', 'r', 'c', 'u', ' ', 'f', 'i', 'n', 'i', 'b', 'u', 's', ' ', 'm',
        'a', 't', 't', 'i', 's', ' ', 'd', 'i', 'c', 't', 'u', 'm', '.', ' ', 'P', 'e',
        'l', 'l', 'e', 'n', 't', 'e', 's', 'q', 'u', 'e', ' ', 'i', 'n', 't', 'e', 'r',
        'd', 'u', 'm', ' ', 'm', 'i', ' ', 'a', ' ', 'f', 'e', 'l', 'i', 's', ' ', 't',
        'e', 'm', 'p', 'u', 's', ' ', 'a', 'u', 'c', 't', 'o', 'r', '.', ' ', 'N', 'u',
        'n', 'c', ' ', 'e', 't', ' ', 'p', 'h', 'a', 'r', 'e', 't', 'r', 'a', ' ', 'm',
        'i', '.', ' ', 'I', 'n', ' ', 'u', 'l', 'l', 'a', 'm', 'c', 'o', 'r', 'p', 'e',
        'r', ' ', 'b', 'i', 'b', 'e', 'n', 'd', 'u', 'm', ' ', 'u', 'r', 'n', 'a', ',',
        ' ', 'u', 't', ' ', 'c', 'o', 'n', 'g', 'u', 'e', ' ', 'u', 'r', 'n', 'a', ' ',
        'e', 'g', 'e', 's', 't', 'a', 's', ' ', 'e', 'g', 'e', 't', '.', ' ', 'I', 'n',
        ' ', 'h', 'a', 'c', ' ', 'h', 'a', 'b', 'i', 't', 'a', 's', 's', 'e', ' ', 'p',
        'l', 'a', 't', 'e', 'a', ' ', 'd', 'i', 'c', 't', 'u', 'm', 's', 't', '.', ' ',
        'M', 'o', 'r', 'b', 'i', ' ', 'l', 'u', 'c', 't', 'u', 's', ' ', 't', 'r', 'i',
        's', 't', 'i', 'q', 'u', 'e', ' ', 'u', 'r', 'n', 'a', ',', ' ', 'n', 'o', 'n',
        ' ', 'c', 'o', 'n', 'v', 'a', 'l', 'l', 'i', 's', ' ', 'a', 'n', 't', 'e', ' ',
        'e', 'u', 'i', 's', 'm', 'o', 'd', ' ', 'v', 'i', 't', 'a', 'e', '.', ' ', 'A',
        'l', 'i', 'q', 'u', 'a', 'm', ' ', 'c', 'u', 'r', 's', 'u', 's', ' ', 'm', 'a',
        'u', 'r', 'i', 's', ' ', 'n', 'o', 'n', ' ', 't', 'u', 'r', 'p', 'i', 's', ' ',
        'f', 'a', 'c', 'i', 'l', 'i', 's', 'i', 's', ',', ' ', 'p', 'o', 's', 'u', 'e',
        'r', 'e', ' ', 'u', 'l', 'l', 'a', 'm', 'c', 'o', 'r', 'p', 'e', 'r', ' ', 'v',
        'e', 'l', 'i', 't', ' ', 'c', 'u', 'r', 's', 'u', 's', '.', ' ', 'M', 'a', 'u',
        'r', 'i', 's', ' ', 'v', 'e', 'l', ' ', 'v', 'o', 'l', 'u', 't', 'p', 'a', 't',
        ' ', 'm', 'e', 't', 'u', 's', '.', ' ', 'P', 'e', 'l', 'l', 'e', 'n', 't', 'e',
        's', 'q', 'u', 'e', ' ', 'd', 'i', 'c', 't', 'u', 'm', ' ', 'q', 'u', 'i', 's',
        ' ', 'l', 'i', 'g', 'u', 'l', 'a', ' ', 'i', 'd', ' ', 'e', 'u', 'i', 's', 'm',
        'o', 'd', '.', ' ', 'I', 'n', ' ', 'v', 'e', 's', 't', 'i', 'b', 'u', 'l', 'u',
        'm', ' ', 'o', 'l', 'o', 'r', ' ', 'v', 'e', 'l', ' ', 'm', 'e', 't', 'u', 's',
        ' ', 't', 'i', 'n', 'c', 'i', 'd', 'u', 'n', 't', ',', ' ', 'i', 'n', ' ', 's',
        'o', 'l', 'l', 'i', 'c', 'i', 't', 'u', 'd', 'i', 'n', ' ', 'r', 'i', 's', 'u',
        's', ' ', 't', 'i', 'n', 'c', 'i', 'd', 'u', 'n', 't', '.', ' ', 'A', 'e', 'n',
        'e', 'a', 'n', ' ', 't', 'r', 'i', 's', 't', 'i', 'q', 'u', 'e', ' ', 'n', 'i',
        's', 'i', ' ', 'q', 'u', 'a', 'm', ',', ' ', 'a', ' ', 'u', 'l', 't', 'r', 'i',
        'c', 'i', 'e', 's', ' ', 'r', 'i', 's', 'u', 's', ' ', 't', 'e', 'm', 'p', 'u',
        's', ' ', 'a', '.', ' ', 'M', 'o', 'r', 'b', 'i', ' ', 't', 'u', 'r', 'p', 'i',
        's', ' ', 'v', 'e', 'l', 'i', 't', ',', ' ', 's', 'c', 'e', 'l', 'e', 'r', 'i',
        's', 'q', 'u', 'e', ' ', 'q', 'u', 'i', 's', ' ', 'n', 'e', 'q', 'u', 'e', ' ',
        'v', 'u', 'l', 'p', 'u', 't', 'a', 't', 'e', ',', ' ', 's', 'u', 's', 'c', 'i',
        'p', 'i', 't', ' ', 'p', 'o', 'r', 't', 't', 'i', 't', 'o', 'r', ' ', 'm', 'e',
        't', 'u', 's', '.', ' ', 'M', 'a', 'u', 'r', 'i', 's', ' ', 'p', 'u', 'l', 'v',
        'i', 'n', 'a', 'r', ' ', 'd', 'a', 'p', 'i', 'b', 'u', 's', ' ', 'v', 'e', 'l',
        'i', 't', ',', ' ', 'i', 'n', ' ', 's', 'u', 's', 'c', 'i', 'p', 'i', 't', ' ',
        'm', 'a', 'u', 'r', 'i', 's', '.', ' ', 'M', 'o', 'r', 'b', 'i', ' ', 's', 'o',
        'd', 'a', 'l', 'e', 's', ' ', 'e', 's', 't', ' ', 'm', 'a', 's', 's', 'a', ',',
        ' ', 'n', 'e', 'c', ' ', 't', 'e', 'm', 'p', 'o', 'r', ' ', 'e', 's', 't', ' ',
        'c', 'o', 'n', 's', 'e', 'c', 't', 'e', 't', 'u', 'r', ' ', 'e', 't', '.', ' ',
        'F', 'u', 's', 'c', 'e', ' ', 'n', 'o', 'n', ' ', 'e', 'n', 'i', 'm', ' ', 'a',
        'l', 'i', 'q', 'u', 'e', 't', ',', ' ', 'b', 'l', 'a', 'n', 'd', 'i', 't', ' ',
        'd', 'u', 'i', ' ', 'a', 't', ',', ' ', 'v', 'i', 'v', 'e', 'r', 'r', 'a', ' ',
        'n', 'u', 'l', 'l', 'a', '.', ' ', 'C', 'r', 'a', 's', ' ', 's', 'e', 'd', '.'
};

/**
 * @brief EEPROM SNC uCode writer notification callback
 *
 */
void snc_demo_app_eeprom_write_cb(void)
{
        OS_TASK_NOTIFY(prvSNCTask_h, EEPROM_WRITE_NOTIFICATION, OS_NOTIFY_SET_BITS);
}

/**
 * @brief EEPROM SNC uCode reader notification callback
 *
 */
void snc_demo_app_eeprom_read_cb(void)
{
        OS_TASK_NOTIFY(prvSNCTask_h, EEPROM_READ_NOTIFICATION, OS_NOTIFY_SET_BITS);
}

/**
 * @brief Prints an EEPROM page
 *
 * \param [in] pageBuff a pointer to the buffer which contains the page data
 * \param [in] len      the data length
 *
 */
static void _printout_eeprom_page(uint8_t *pageBuff, uint16_t len)
{
        uint16_t i;
        for (i = 0; i < len; i++) {
                if (i % (EEPROM_24FC256_PAGE_SIZE / 8) == 0)
                        printf("\r\n");
                printf("%c", pageBuff[i]);
        }
        printf("\r\n");
}

/**
 * @brief Helper function that reads from the EEPROM test source buffer
 *        and pushes pages into the CM33-to-SNC queue of the EEPROM writer uCode-Block
 *        as long as the queue is NOT empty
 *
 * \param [in] pageAddr the address of the page that the pushed data is going to be written to
 *
 * \return uint16_t the next page address to be pushed to the EEPROM writer
 */
static uint16_t _push_eeprom_page_to_writer(uint16_t pageAddr)
{
        uint8_t e_page_buff[EEPROM_24FC256_PAGE_SIZE + 2];
        uint8_t *pSrc;
        while (!ad_snc_queue_is_full(e_writer_ucode_id, AD_SNC_QUEUE_TYPE_CM33_SNC)) {

                pSrc = (uint8_t*)e_src_buff + pageAddr;

                // Fill in the address bytes
                e_page_buff[0] = pageAddr >> 8;
                e_page_buff[1] = pageAddr;

                // Copy the data that are going to be pushed into the queue
                memcpy(&e_page_buff[2], pSrc, EEPROM_24FC256_PAGE_SIZE);

                // Push the data to be written and trigger the writer uCode
                ad_snc_queue_push(e_writer_ucode_id, e_page_buff, sizeof(e_page_buff), 0);
                ad_snc_pdc_set_pending(e_writer_ucode_id);

                // Increment the page address
                pageAddr = (pageAddr + EEPROM_24FC256_PAGE_SIZE);
        }

        return pageAddr;
}

/**
 * @brief Helper function that pushes page addresses that are wanted to be read
 *        by the EEPROM reader uCode-Block as long as the SNC-to-CM33 queue is NOT full
 *
 * \param [in] pageAddr the address of the page that the EEPROM reader uCode-Block
 *                      is going to read from
 *
 * \return uint16_t the next page address to be pushed to the EEPROM reader
 */
static uint16_t _push_eeprom_page_address_to_reader(uint16_t pageAddr)
{
        uint8_t e_addr_buff[2];
        while (!ad_snc_queue_is_full(e_reader_ucode_id, AD_SNC_QUEUE_TYPE_CM33_SNC)) {

                // Fill in the address bytes
                e_addr_buff[0] = pageAddr >> 8;
                e_addr_buff[1] = pageAddr;

                // Push the data to be written and trigger the writer uCode
                ad_snc_queue_push(e_reader_ucode_id, e_addr_buff, sizeof(e_addr_buff), 0);

                // Increment the page address
                pageAddr = (EEPROM_24FC256_PAGE_SIZE + pageAddr) % EEPROM_TEST_DATA_SIZE;
        }

        return pageAddr;
}

#endif /* EEPROM_DEMO */

#if defined(EEPROM_DEMO) || defined(BH1750_DEMO)
/**
 * @brief RTC initialization function
 *
 * This function initializes RTC to produce PDC events periodically in a fixed interval
 *
 * \param [in] pdcIntervalms The PDC event interval in ms (must be multiple of 10)
 *
 */
static void _snc_demo_rtc_init(uint32_t pdcIntervalms)
{
        ASSERT_WARNING((pdcIntervalms > 0) && ((pdcIntervalms % 10) == 0));

        hw_rtc_config_pdc_evt_t cfg = { 0 };

        /* Enable the RTC PDC event */
        cfg.pdc_evt_en = true;

        /* Set the RTC PDC event period */
        cfg.pdc_evt_period = (pdcIntervalms / 10) - 1;

        /* Configure the RTC event controller */
        hw_rtc_config_RTC_to_PDC_evt(&cfg);

        /* Enable the RTC peripheral clock */
        hw_rtc_clock_enable();

        /* Start the RTC */
        hw_rtc_time_start();
}
#endif /* EEPROM_DEMO || BH1750_DEMO */

#ifdef ADXL362_DEMO

/**
 * @brief TIMER1 initialization function for timer capture mode
 *
 * This function initializes TIMER1 in timer capture mode and associates it with ADXL362 pin
 * interrupt in order to be able to obtain a timestamp each time the sensor has samples in its FIFO
 */
static void _snc_demo_timer1_capture_init(void)
{
        timer_config cfg = {
                .clk_src = HW_TIMER_CLK_SRC_EXT,
                .prescaler = 31,
                .mode = HW_TIMER_MODE_TIMER,
                .timer = {
                        .direction = HW_TIMER_DIR_UP,
                        .free_run = true,
                        .reload_val = 0,
                        .gpio1 = HW_TIMER_GPIO_PIN_17,
                        .trigger1 = HW_TIMER_TRIGGER_FALLING,
                },
        };

        hw_timer_init(HW_TIMER, &cfg);
        hw_timer_register_int(HW_TIMER, NULL);

        /* Disable NVIC interrupt */
        NVIC_DisableIRQ(TIMER_IRQn);

        hw_timer_enable(HW_TIMER);
}
#endif /* ADXL362_DEMO */

/**
 * @brief prvSNCTask task initializes an ambient light sensor (BH1750), an I2C EEPROM
 *        and an accelerometer sensor (ADXL362) as well as the Sensor Node. For each external IC
 *        the task registers microcodes (uCodes)
 */
static void prvSNCTask(void *pvParameters)
{
        uint32_t notif, rtn;
#ifdef ADXL362_DEMO
        adxl362_meas_t acc_meas;
        uint32_t adxl_bytes;
        static uint8_t adxl_data[132];
        uint32_t adxl_time_s;
#endif /* ADXL362_DEMO */

#ifdef EEPROM_DEMO
        static uint8_t e_r_page_buff[EEPROM_24FC256_PAGE_SIZE];
        uint16_t e_pages_pushed = 0, e_nxt_addr = 0;
        uint32_t e_pages_popped;
#endif /* EEPROM_DEMO */

#ifdef BH1750_DEMO
#if BH1750_DEMO_MODE == 0
        static uint32_t bh1750_samples[UCODE_BH1750_MAX_NUM_OF_SAMPLES * BH1750_SAMPLE_SIZE];
        uint32_t bh1750_samples_notify = 5;
        uint32_t bh1750_samples_num;
#endif
#if BH1750_DEMO_MODE == 1
        static uint8_t bh1750_samples_bytes[UCODE_BH1750_MAX_NUM_OF_SAMPLES * BH1750_SAMPLE_SIZE];
        uint32_t bh1750_samples_notify = 5;
        uint32_t bh1750_bytes_num;
        uint32_t bh1750_time_s;
#endif
#if BH1750_DEMO_MODE == 2
        uint32_t b1750_sample_value;
        uint32_t bh1750_notif_state;
#endif
#endif /* BH1750_DEMO */

        printf("SNC Test APP \n\r");

#ifdef ADXL362_DEMO
        _snc_demo_timer1_capture_init();

        /* ADXL362 INT1 pin registration as a wakeup source */
        hw_wkup_init(NULL);
        hw_wkup_configure_port(ADXL362_INT_1_PORT, 0 << ADXL362_INT_1_PIN, 1 << ADXL362_INT_1_PIN,
                0 << ADXL362_INT_1_PIN);
        hw_wkup_clear_status(ADXL362_INT_1_PORT, 1 << ADXL362_INT_1_PIN);

        /* ADXL362 initializations and uCode registration */
        adxl_ucode_id = snc_demo_adxl362_init(snc_demo_app_adxlint1_cb);
#endif /* ADXL362_DEMO */

#if defined(EEPROM_DEMO) || defined(BH1750_DEMO)
        _snc_demo_rtc_init(200);
#endif /* EEPROM_DEMO || BH1750_DEMO */

#ifdef BH1750_DEMO
/* BH1750 initializations */
#if BH1750_DEMO_MODE == 0
        uint32_t __UNUSED bh1750_ucode_id = snc_demo_bh1750_init(snc_demo_app_bh1750_cb,0,bh1750_samples_notify);
#endif
#if BH1750_DEMO_MODE == 1
        uint32_t __UNUSED bh1750_ucode_id = snc_demo_bh1750_init(snc_demo_app_bh1750_cb,1,bh1750_samples_notify);
#endif
#if BH1750_DEMO_MODE == 2
        ucode_bh1750_set_threshold_low(30, 10);
        ucode_bh1750_set_threshold_high(80, 10);
        uint32_t __UNUSED bh1750_ucode_id = snc_demo_bh1750_init(snc_demo_app_bh1750_cb,2,0);
#endif
#endif /* BH1750_DEMO */

#ifdef EEPROM_DEMO
        // EEPROM writer uCode initializations and registration
        e_writer_ucode_id = snc_demo_24fc256_writer_init(snc_demo_app_eeprom_write_cb);

        // Start pushing pages into the CM33-to-SNC queue of the EEPROM writer uCode-Block
        e_nxt_addr = _push_eeprom_page_to_writer(e_nxt_addr);
#endif

        for (;;) {
                /* Event / Notification Loop */
                rtn = OS_TASK_NOTIFY_WAIT(0x0, OS_TASK_NOTIFY_ALL_BITS, &notif, OS_TASK_NOTIFY_FOREVER);
                configASSERT(rtn == OS_TASK_NOTIFY_SUCCESS);

#ifdef ADXL362_DEMO
                // Check if we received a FIFO "watermark" notification from the ADXL362
                if (notif & ADXL362_FIFO_NOTIFICATION) {
                        while (ad_snc_queue_pop(adxl_ucode_id, (uint8_t*)adxl_data, &adxl_bytes,
                               &adxl_time_s)) {
                                printf("Acc Samples : %lu, TS: 0x%06lx\r\n", adxl_bytes / 2,
                                        adxl_time_s);

                                // Print the acquired samples
                                for (uint32_t i = 0; i < (adxl_bytes / 2) / 3; i++) {
                                        acc_meas = ADXL362_ConvertXYZ(adxl_data + i * 6);
                                        printf("x: %d, y: %d, z: %d\r\n",
                                                acc_meas.accX, acc_meas.accY, acc_meas.accZ);
                                }
                        }
                }
#endif /* ADXL362_DEMO */

#ifdef EEPROM_DEMO
                // Check if we received an EEPROM Write Notification
                if (notif & EEPROM_WRITE_NOTIFICATION) {
                        ++e_pages_pushed;
                        if (e_nxt_addr < EEPROM_TEST_DATA_SIZE) {
                                e_nxt_addr = _push_eeprom_page_to_writer(e_nxt_addr);
                        } else if (e_pages_pushed == (EEPROM_TEST_DATA_SIZE / EEPROM_24FC256_PAGE_SIZE)) {
                                printf("uCode EEPROM writer finished writing all the pages\r\n");
                                printf("Initializing EEPROM Reader uCode\r\n");
                                // Initialize and register the EEPROM reader uCode
                                e_reader_ucode_id =
                                        snc_demo_24fc256_reader_init(snc_demo_app_eeprom_read_cb);

                                // Fill the EEPROM reader queue with the desired
                                // base addresses to read from
                                e_nxt_addr = 0;
                                e_nxt_addr = _push_eeprom_page_address_to_reader(e_nxt_addr);

                        }
                }

                // Check if we received an EEPROM Read Notification
                if (notif & EEPROM_READ_NOTIFICATION) {
                        if (ad_snc_queue_pop(e_reader_ucode_id, (uint8_t*)e_r_page_buff, &e_pages_popped, NULL)) {
                                printf("\r\nSNC reads an EEPROM page!\r\n");
                                _printout_eeprom_page(e_r_page_buff, e_pages_popped);

                                // Push another page address for the SNC to read
                                // if there is space in the queue
                                e_nxt_addr = _push_eeprom_page_address_to_reader(e_nxt_addr);
                        }
                }
#endif /* EEPROM_DEMO */

#ifdef BH1750_DEMO
                /* Check if we received a BH1750 Notification */
                if (notif & BH1750_NOTIFICATION) {
#if  BH1750_DEMO_MODE==0
                        printf("\r\nAmbient light samples!\r\n");
                        /* Get the recently acquired number of samples from the local FIFO */
                        bh1750_samples_num = ucode_bh1750_get_fifo_num_of_samples();

                        /* Read the last updated samples to a temporary buffer and print them out */
                        ucode_bh1750_get_fifo_samples(bh1750_samples);
                        for (uint32_t i = 0; i < bh1750_samples_num; i += BH1750_SAMPLE_SIZE) {
                                printf("%lu lx\r\n", bh1750_raw_meas_to_lx(bh1750_samples[i] << 8 |
                                        bh1750_samples[i+1]));
                        }
#endif
#if BH1750_DEMO_MODE==1
                        printf("\r\nAmbient light Notification!\r\n");
                        while (ad_snc_queue_pop(bh1750_ucode_id, (uint8_t*)bh1750_samples_bytes, &bh1750_bytes_num,
                               &bh1750_time_s)) {

                                /* Print the acquired samples */
                                for (uint32_t i = 0; i < bh1750_bytes_num/2; i += BH1750_SAMPLE_SIZE) {
                                        printf("%lu lx\r\n", bh1750_raw_meas_to_lx((uint16_t)bh1750_samples_bytes[i] <<8 |
                                                bh1750_samples_bytes[i+1]));
                                }

                        }
#endif
#if BH1750_DEMO_MODE==2
                        bh1750_notif_state = bh1750_get_ambient_light_state(&b1750_sample_value);
                        if (bh1750_notif_state == AMBIENT_LIGHT_STATE_NORMAL) {
                                printf("\r\nAmbient light state is NORMAL! ");
                                printf("%lu lx\r\n", bh1750_raw_meas_to_lx(b1750_sample_value));
                        }
                        if (bh1750_notif_state == AMBIENT_LIGHT_STATE_HIGH) {
                                printf("\r\nAmbient light state is HIGH! ");
                                printf("%lu lx\r\n", bh1750_raw_meas_to_lx(b1750_sample_value));
                        }
                        if (bh1750_notif_state == AMBIENT_LIGHT_STATE_LOW) {
                                printf("\r\nAmbient light state is LOW! ");
                                printf("%lu lx\r\n", bh1750_raw_meas_to_lx(b1750_sample_value));
                        }
#endif
                }
#endif /* BH1750_DEMO */
        }
}

/**
 * @brief Initialize the peripherals domain after power-up.
 *
 */
static void periph_init(void)
{
}

/**
 * @brief Hardware Initialization
 */
static void prvSetupHardware(void)
{
        /* Init hardware */
        pm_system_init(periph_init);

        hw_sys_pd_com_enable();

#ifdef EEPROM_DEMO
        ad_i2c_io_config(((ad_i2c_controller_conf_t*)MEM_24FC256)->id,
                ((ad_i2c_controller_conf_t*)MEM_24FC256)->io, AD_IO_CONF_OFF);
#endif /* EEPROM_DEMO */

#ifdef ADXL362_DEMO
        hw_gpio_configure_pin(ACC_POWER_ENABLE_PORT, ACC_POWER_ENABLE_PIN,
                              HW_GPIO_MODE_OUTPUT, HW_GPIO_FUNC_GPIO, false);
        hw_gpio_pad_latch_enable(ACC_POWER_ENABLE_PORT, ACC_POWER_ENABLE_PIN);
        hw_gpio_pad_latch_disable(ACC_POWER_ENABLE_PORT, ACC_POWER_ENABLE_PIN);

        hw_gpio_configure_pin(ADXL362_INT_1_PORT, ADXL362_INT_1_PIN,
                              HW_GPIO_MODE_INPUT_PULLUP, HW_GPIO_FUNC_GPIO, true);
        hw_gpio_pad_latch_enable(ADXL362_INT_1_PORT, ADXL362_INT_1_PIN);
        hw_gpio_pad_latch_disable(ADXL362_INT_1_PORT, ADXL362_INT_1_PIN);

        ad_spi_io_config(((ad_spi_controller_conf_t*)ADXL362)->id,
                ((ad_spi_controller_conf_t*)ADXL362)->io, AD_IO_CONF_OFF);
#endif /* ADXL362_DEMO */

#ifdef BH1750_DEMO
        hw_gpio_configure_pin(BH1750_POWER_ENABLE_PORT, BH1750_POWER_ENABLE_PIN,
                              HW_GPIO_MODE_OUTPUT, HW_GPIO_FUNC_GPIO, false);
        hw_gpio_pad_latch_enable(BH1750_POWER_ENABLE_PORT, BH1750_POWER_ENABLE_PIN);
        hw_gpio_pad_latch_disable(BH1750_POWER_ENABLE_PORT, BH1750_POWER_ENABLE_PIN);

        ad_i2c_io_config(((ad_i2c_controller_conf_t*)BH1750)->id,
                ((ad_i2c_controller_conf_t*)BH1750)->io, AD_IO_CONF_OFF);
#endif /* BH1750_DEMO */

        hw_sys_pd_com_disable();
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
        ASSERT_ERROR(0);
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
