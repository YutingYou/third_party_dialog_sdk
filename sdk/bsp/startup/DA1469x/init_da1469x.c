/*
 * Copyright (C) 2019-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 */



#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/errno.h>
#include "sdk_defs.h"
#include "interrupts.h"
#include "hw_bod.h"
#include "hw_cache.h"
#include "hw_clk.h"
#include "hw_cpm.h"
#include "hw_gpio.h"
#include "hw_memctrl.h"
#include "hw_otpc.h"
#include "hw_pdc.h"
#include "hw_pd.h"
#include "hw_rtc.h"
#include "hw_qspi.h"
#include "hw_sys.h"
#include "qspi_automode.h"
#include "sys_tcs.h"
#include "sys_trng.h"


#if dg_configUSE_CLOCK_MGR
#include "sys_clock_mgr.h"
#include "../../system/sys_man/sys_clock_mgr_internal.h"
#endif

#if dg_configFPGA_AD9361_RADIO
int ad9361_radio_init(void);
#endif

/*
 * Linker symbols
 *
 * Note: if any of them is missing, please correct your linker script. Please refer to the linker
 * script of pxp_reporter.
 */
extern uint32_t __copy_table_start__;
extern uint32_t __copy_table_end__;
extern uint32_t __zero_table_start__;
extern uint32_t __zero_table_end__;
extern uint8_t end;
extern uint8_t __HeapLimit;

/*
 * Global variables
 */
__RETAINED_RW static uint8_t *heapend = &end;
__RETAINED_RW uint32_t SystemLPClock = dg_configXTAL32K_FREQ;   /*!< System Low Power Clock Frequency (LP Clock) */


/**
 * @brief  Memory safe implementation of newlib's _sbrk().
 *
 */
__LTO_EXT
void *_sbrk(int incr)
{
        uint8_t *newheapstart;

        if (heapend + incr > &__HeapLimit) {
                /* Hitting this, means that the value of _HEAP_SIZE is too small.
                 * The value of incr is in stored_incr at this point. By checking the equation
                 * above, it is straightforward to determine the missing space.
                 */
                volatile int stored_incr __UNUSED;

                stored_incr = incr;
                ASSERT_ERROR(0);

                errno = ENOMEM;
                return (void *)-1;
        }

        newheapstart = heapend;
        heapend += incr;

        return newheapstart;
}

/**
* \brief  SDK implementation of stdlib's rand().
*
*/
int rand(void)
{
        return (int) sys_trng_rand();
}

/**
* \brief  SDK implementation of stdlib's srand().
*
*/
void srand (unsigned __seed)
{
}

/*
 * Dialog default priority configuration table.
 *
 * Content of this table will be applied at system start.
 *
 * \note If interrupt priorities provided by Dialog need to be changed, do not modify this file.
 * Create similar table with SAME name instead somewhere inside code without week attribute,
 * and it will be used instead of table below.
 */
#pragma weak __dialog_interrupt_priorities
INTERRUPT_PRIORITY_CONFIG_START(__dialog_interrupt_priorities)
        PRIORITY_0, /* Start interrupts with priority 0 (highest) */
                /*
                 * Note: Interrupts with priority 0 are not
                 * allowed to perform OS calls.
                 */
        PRIORITY_1, /* Start interrupts with priority 1 */
                CMAC2SYS_IRQn,
                CRYPTO_IRQn,
                RFDIAG_IRQn,
        PRIORITY_2, /* Start interrupts with priority 2 */
                SNC_IRQn,
                DMA_IRQn,
                I2C_IRQn,
                I2C2_IRQn,
                SPI_IRQn,
                SPI2_IRQn,
                GPADC_IRQn,
                SDADC_IRQn,
                SRC_IN_IRQn,
                SRC_OUT_IRQn,
                TRNG_IRQn,
        PRIORITY_3, /* Start interrupts with priority 3 */
                SysTick_IRQn,
                UART_IRQn,
                UART2_IRQn,
                UART3_IRQn,
                MRM_IRQn,
                XTAL32M_RDY_IRQn,
                PLL_LOCK_IRQn,
                CHARGER_STATE_IRQn,
                CHARGER_ERROR_IRQn,
                LCD_CONTROLLER_IRQn,
                KEY_WKUP_GPIO_IRQn,
                GPIO_P0_IRQn,
                GPIO_P1_IRQn,
                TIMER_IRQn,
#if !defined(OS_FREERTOS)
                TIMER2_IRQn,
#endif
                TIMER3_IRQn,
                TIMER4_IRQn,
                CAPTIMER_IRQn,
                RTC_IRQn,
                RTC_EVENT_IRQn,
                MOTOR_CONTROLLER_IRQn,
                LRA_IRQn,
                USB_IRQn,
                PCM_IRQn,
                VBUS_IRQn,
                DCDC_IRQn,
        PRIORITY_4, /* Start interrupts with priority 4 */
        PRIORITY_5, /* Start interrupts with priority 5 */
        PRIORITY_6, /* Start interrupts with priority 6 */
        PRIORITY_7, /* Start interrupts with priority 7 */
        PRIORITY_8, /* Start interrupts with priority 8 */
        PRIORITY_9, /* Start interrupts with priority 9 */
        PRIORITY_10, /* Start interrupts with priority 10 */
        PRIORITY_11, /* Start interrupts with priority 11 */
        PRIORITY_12, /* Start interrupts with priority 12 */
        PRIORITY_13, /* Start interrupts with priority 13 */
        PRIORITY_14, /* Start interrupts with priority 14 */
        PRIORITY_15, /* Start interrupts with priority 15 (lowest) */
#if defined(OS_FREERTOS)
                TIMER2_IRQn,
#endif
INTERRUPT_PRIORITY_CONFIG_END

void set_interrupt_priorities(const int8_t prios[])
{
        uint32_t old_primask, iser, iser2;
        int i = 0;
        uint32_t prio = 0;

        // Assign all bit for preemption to be preempt priority bits.
        // (required by FreeRTOS)
        NVIC_SetPriorityGrouping(0);

        /*
         * We shouldn't change the priority of an enabled interrupt.
         *  1. Globally disable interrupts, saving the global interrupts disable state.
         *  2. Explicitly disable all interrupts, saving the individual interrupt enable state.
         *  3. Set interrupt priorities.
         *  4. Restore individual interrupt enables.
         *  5. Restore global interrupt enable state.
         */
        old_primask = __get_PRIMASK();
        __disable_irq();
        iser  = NVIC->ISER[0];
        iser2 = NVIC->ISER[1];
        NVIC->ICER[0] = iser;
        NVIC->ICER[1] = iser2;

        for (i = 0; prios[i] != PRIORITY_TABLE_END; ++i) {
                switch (prios[i]) {
                case PRIORITY_0:
                case PRIORITY_1:
                case PRIORITY_2:
                case PRIORITY_3:
                case PRIORITY_4:
                case PRIORITY_5:
                case PRIORITY_6:
                case PRIORITY_7:
                case PRIORITY_8:
                case PRIORITY_9:
                case PRIORITY_10:
                case PRIORITY_11:
                case PRIORITY_12:
                case PRIORITY_13:
                case PRIORITY_14:
                case PRIORITY_15:
                        prio = prios[i] - PRIORITY_0;
                        break;
                default:
                        NVIC_SetPriority(prios[i], prio);
                        break;
                }
        }

        NVIC->ISER[0] = iser;
        NVIC->ISER[1] = iser2;
        __set_PRIMASK(old_primask);

        // enable Usage-, Bus-, and MMU Fault
        SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk
                   |  SCB_SHCSR_BUSFAULTENA_Msk
                   |  SCB_SHCSR_MEMFAULTENA_Msk;
}

/**
 * Check whether the code has been compiled for a chip version that is compatible with the chip
 * it runs on.
 */
static bool is_compatible_chip_version(void)
{
        uint32_t rev = CHIP_VERSION->CHIP_REVISION_REG;
        uint32_t step = CHIP_VERSION->CHIP_TEST1_REG;

        if (rev == 'A' && step == 'B') {
                return true;
        }
        return false;
}


#if dg_configUSE_CLOCK_MGR == 0

void XTAL32M_Ready_Handler(void)
{
        ASSERT_WARNING(hw_clk_is_xtalm_started());
        if (dg_configXTAL32M_SETTLE_TIME_IN_USEC == 0) {
                hw_clk_xtalm_update_rdy_cnt();
        }
}

void PLL_Lock_Handler(void)
{
        ASSERT_WARNING(REG_GETF(CRG_XTAL, PLL_SYS_STATUS_REG, PLL_LOCK_FINE));
}

/* carry out clock initialization sequence */
static void nortos_clk_setup(void)
{
         hw_clk_enable_lpclk(LP_CLK_IS_RC32K);
         hw_clk_set_lpclk(LP_CLK_IS_RC32K);

         NVIC_ClearPendingIRQ(XTAL32M_RDY_IRQn);
         NVIC_EnableIRQ(XTAL32M_RDY_IRQn);              // Activate XTAL32M Ready IRQ

         hw_clk_enable_sysclk(SYS_CLK_IS_XTAL32M);      // Enable XTAL32M

         /* Wait for XTAL32M to settle */
         while (!hw_clk_is_xtalm_started());

         hw_clk_set_sysclk(SYS_CLK_IS_XTAL32M);

         NVIC_ClearPendingIRQ(PLL_LOCK_IRQn);
         NVIC_EnableIRQ(PLL_LOCK_IRQn);                         // Activate PLL lock IRQ
}

#endif  /* OS_BAREMETAL */


static __RETAINED_CODE void configure_cache(void)
{
#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_FLASH)

        uint32_t product_header_addr;
        uint32_t active_fw_image_addr;
        uint32_t active_fw_size;
        uint32_t cache_len;
        uint32_t scanned_sectors = 0;

        /* Configure cache according to the 'Active FW image address'
         * field of the product header and 'FW Size' field of the active
         * FW image header. */

        /* Product header is located either at start of FLASH or at the sector
         * boundary (0x4000) if a configuration script is used */

        product_header_addr = MEMORY_QSPIF_S_BASE;
        while ((((uint8_t*) product_header_addr)[0] != 0x50) &&
                (((uint8_t*) product_header_addr)[1] != 0x70) &&
                (scanned_sectors < 10)) {
                product_header_addr += 0x1000;
                scanned_sectors++;
        }

        /* Get active_fw_image_addr */
        ASSERT_WARNING(((uint8_t*) product_header_addr)[0] == 0x50);
        ASSERT_WARNING(((uint8_t*) product_header_addr)[1] == 0x70);
        memcpy((uint8_t*) &active_fw_image_addr, &((uint8_t*) product_header_addr)[2], 4);
        active_fw_image_addr += MEMORY_QSPIF_S_BASE;

        /* Get active_fw_size and align it to 64K boundary */
        ASSERT_WARNING(((uint8_t*) active_fw_image_addr)[0] == 0x51);
        ASSERT_WARNING(((uint8_t*) active_fw_image_addr)[1] == 0x71);
        memcpy((uint8_t*) &active_fw_size, &((uint8_t*) active_fw_image_addr)[2], 4);
        active_fw_size += (0x10000 - (active_fw_size % 0x10000)) % 0x10000;

        /* Calculate length of QSPI FLASH cacheable memory
         * (Cached area len will be (cache_len * 64) KBytes, cache_len can be 0 to 512) */
        cache_len = active_fw_size >> 16;

        hw_cache_config(dg_configCACHE_ASSOCIATIVITY, dg_configCACHE_LINESZ, cache_len);

#endif /* (dg_configCODE_LOCATION == NON_VOLATILE_IS_FLASH) */
}

/* this function configures the PDC table only the first time it is called */
static void configure_pdc(void)
{
        uint32_t pdc_entry_index __UNUSED;
        bool no_syscpu_pdc_entries = true;
        NVIC_DisableIRQ(PDC_IRQn);
        NVIC_ClearPendingIRQ(PDC_IRQn);

#if defined(CONFIG_USE_BLE) || (dg_configUSE_SYS_CHARGER) || (dg_configENABLE_DEBUGGER == 1)
        /* Set up PDC entry for CMAC2SYS IRQ or VBUS IRQ or debugger */
        pdc_entry_index = hw_pdc_add_entry(HW_PDC_LUT_ENTRY_VAL(
                                                HW_PDC_TRIG_SELECT_PERIPHERAL,
                                                HW_PDC_PERIPH_TRIG_ID_COMBO,
                                                HW_PDC_MASTER_CM33,
                                                (dg_configENABLE_XTAL32M_ON_WAKEUP ? HW_PDC_LUT_ENTRY_EN_XTAL : 0)));
        hw_pdc_set_pending(pdc_entry_index);
        hw_pdc_acknowledge(pdc_entry_index);
        no_syscpu_pdc_entries = false;
#endif

#if defined(CONFIG_USE_BLE)
        /*
         * Set up PDC entry for CMAC wakeup from MAC timer.
         * This entry is also used for the SYS2CMAC mailbox interrupt.
         */
        pdc_entry_index = hw_pdc_add_entry(HW_PDC_LUT_ENTRY_VAL(
                                                        HW_PDC_TRIG_SELECT_PERIPHERAL,
                                                        HW_PDC_PERIPH_TRIG_ID_MAC_TIMER,
                                                        HW_PDC_MASTER_CMAC,
                                                        HW_PDC_LUT_ENTRY_EN_XTAL));

        hw_pdc_set_pending(pdc_entry_index);
        hw_pdc_acknowledge(pdc_entry_index);
#endif

#if defined(OS_FREERTOS)
        /* FreeRTOS Timer requires PD_TIM to be always on */
        REG_SETF(CRG_TOP, PMU_CTRL_REG, TIM_SLEEP, 0);
        while (REG_GETF(CRG_TOP, SYS_STAT_REG, TIM_IS_UP) == 0) {
        }

        /* Add PDC entry to wakeup from Timer2 */
        pdc_entry_index = hw_pdc_add_entry(HW_PDC_LUT_ENTRY_VAL(
                                                HW_PDC_TRIG_SELECT_PERIPHERAL,
                                                HW_PDC_PERIPH_TRIG_ID_TIMER2,
                                                HW_PDC_MASTER_CM33,
                                                (dg_configENABLE_XTAL32M_ON_WAKEUP ? HW_PDC_LUT_ENTRY_EN_XTAL : 0)));
        hw_pdc_set_pending(pdc_entry_index);
        hw_pdc_acknowledge(pdc_entry_index);
        no_syscpu_pdc_entries = false;
#endif

        /* Let SYSCPU goto sleep when needed */
        if (!no_syscpu_pdc_entries) {
                REG_SETF(CRG_TOP, PMU_CTRL_REG, SYS_SLEEP, 1);
        }

        /* clear the PDC IRQ since it will be pending here */
        NVIC_ClearPendingIRQ(PDC_IRQn);
}

#if dg_configUSE_CLOCK_MGR && dg_configUSE_HW_RTC
/* this function configures the RTC clock and RTC_KEEP_RTC_REG*/
void configure_rtc(void)
{
        uint16_t div_int;
        uint16_t div_frac;

#if (dg_configUSE_LP_CLK == LP_CLK_RCX)
        div_int = rcx_clock_hz / 100;
        div_frac = 10 * (rcx_clock_hz - (div_int * 100));
#else
        div_int = dg_configXTAL32K_FREQ / 100;
        div_frac = 10 * (dg_configXTAL32K_FREQ - (div_int * 100));
#endif
        hw_rtc_clk_config(RTC_DIV_DENOM_1000, div_int, div_frac);

        hw_rtc_set_keep_reg_on_reset(true);
}
#endif
/**
 * Basic system setup.
 *
 * @brief  Setup the AMBA clocks. Ensure proper alignment of copy and zero table entries.
 *
 * @note   No variable initialization should take place here, since copy & zero tables
 *         have not yet been initialized yet and any modifications to variables will
 *         be discarded. For the same reason, functions that initialize or are
 *         using initialized variables should not be called from here.
 */
void SystemInitPre(void) __attribute__((section("text_reset")));
void SystemInitPre(void)
{
        /*
         * Enable M33 debugger.
         */
        if (dg_configENABLE_DEBUGGER) {
                ENABLE_DEBUGGER;
        } else {
                DISABLE_DEBUGGER;
        }

        /*
         * Enable CMAC debugger.
         */
        if (dg_configENABLE_CMAC_DEBUGGER) {
                ENABLE_CMAC_DEBUGGER;
        } else {
                DISABLE_CMAC_DEBUGGER;
        }

        /*
         * Bandgap has already been set by the bootloader.
         * Use fast clocks from now on.
         */
        hw_clk_set_hclk_div(0);
        hw_clk_set_pclk_div(0);

        /*
         * Disable pad latches
         */
        hw_gpio_pad_latch_disable_all();

        /*
         * Make sure we are running on a chip version that the code has been built for.
         */
        ASSERT_WARNING_UNINIT(is_compatible_chip_version());

        /*
         * Ensure 4-byte alignment for all elements of each entry in the Copy Table.
         * If any of the assertions below hits, please correct your linker script
         * file accordingly!
         */
        if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE) {
                uint32_t *p;

                for (p = &__copy_table_start__; p < &__copy_table_end__; p += 3) {
                        ASSERT_WARNING_UNINIT( (p[0] & 0x3) == 0 );     // from
                        ASSERT_WARNING_UNINIT( (p[1] & 0x3) == 0 );     // to
                        ASSERT_WARNING_UNINIT( (p[2] & 0x3) == 0 );     // size
                }
        }

        /*
         * Ensure 4-byte alignment for all elements of each entry in the Data Table.
         * If any of the assertions below hits, please correct your linker script
         * file accordingly!
         */
        if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE) {
                uint32_t *p;

                for (p = &__zero_table_start__; p < &__zero_table_end__; p += 2) {
                        ASSERT_WARNING_UNINIT( (p[0] & 0x3) == 0 );    // start at
                        ASSERT_WARNING_UNINIT( (p[1] & 0x3) == 0 );    // size
                }
        }

        /*
         * Clear all PDC entries and make sure SYS_SLEEP is 0.
         */
        REG_SETF(CRG_TOP, PMU_CTRL_REG, SYS_SLEEP, 0);
        hw_pdc_lut_reset();

        /*
         * Reset memory controller.
         */
        hw_memctrl_reset();

        /*
         * Initialize power domains
         */
        GLOBAL_INT_DISABLE();
        REG_SETF(CRG_TOP, PMU_CTRL_REG, RADIO_SLEEP, 1);
        while (!REG_GETF(CRG_TOP, SYS_STAT_REG, RAD_IS_DOWN));
        REG_SETF(CRG_TOP, PMU_CTRL_REG, PERIPH_SLEEP, 1);
        while (!REG_GETF(CRG_TOP, SYS_STAT_REG, PER_IS_DOWN));
        REG_SETF(CRG_TOP, PMU_CTRL_REG, COM_SLEEP, 1);
        while (!REG_GETF(CRG_TOP, SYS_STAT_REG, COM_IS_DOWN));
        /*
         * PD_TIM is kept active so that XTAL and PLL registers
         * can be programmed properly in SystemInit.
         */
        REG_SETF(CRG_TOP, PMU_CTRL_REG, TIM_SLEEP, 0);
        while (!REG_GETF(CRG_TOP, SYS_STAT_REG, TIM_IS_UP));
        GLOBAL_INT_RESTORE();

        /*
         * Keep CMAC core under reset
         */
        REG_SETF(CRG_TOP, CLK_RADIO_REG, CMAC_CLK_ENABLE, 0);
        REG_SETF(CRG_TOP, CLK_RADIO_REG, CMAC_SYNCH_RESET, 1);

        /*
         * Disable unused peripherals
         */
#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_FLASH)
        /*
         * Booter has already set QSPI_ENABLE to 1 and it must not
         * be disabled since we are executing from flash.
         */
#else
        /*
         * Since we are executing from RAM, QSPI can be disabled.
         */
        REG_SETF(CRG_TOP, CLK_AMBA_REG, QSPI_ENABLE, 0);
#endif

        REG_SETF(CRG_TOP, CLK_AMBA_REG, QSPI2_ENABLE, 0);
        REG_SETF(CRG_TOP, CLK_AMBA_REG, AES_CLK_ENABLE, 0);
        REG_SETF(CRG_TOP, CLK_AMBA_REG, TRNG_CLK_ENABLE, 0);
        REG_SETF(CRG_TOP, CLK_AMBA_REG, OTP_ENABLE, 0);
}

void da1469x_SystemInit(void)
{
        REG_SETF(CRG_TOP, POWER_CTRL_REG, LDO_RADIO_ENABLE, 1); // Switch on the RF LDO

        /*
         * Initialize busy status register
         */
        hw_sys_sw_bsr_init();

        /*
         * Apply default priorities to interrupts.
         */
        set_interrupt_priorities(__dialog_interrupt_priorities);

        SystemLPClock = dg_configXTAL32K_FREQ;

#if ((dg_configCODE_LOCATION == NON_VOLATILE_IS_FLASH) && (dg_configEXEC_MODE == MODE_IS_CACHED))
        /* Disable cache before reinitialize QSPI */
        uint32_t cache_len = hw_cache_get_len();
        hw_cache_disable();
#endif

        /* Disable QSPI init after power up */
        hw_qspi_disable_init(HW_QSPIC);
        /* The bootloader may have left the Flash in wrong mode */
        qspi_automode_init();

#if ((dg_configCODE_LOCATION == NON_VOLATILE_IS_FLASH) && (dg_configEXEC_MODE == MODE_IS_CACHED))
        hw_cache_enable(cache_len);
#endif

        /* Already up in SystemInitPre()
         * PD_TIM is kept active here in order to program XTAL and PLL registers*/
        ASSERT_WARNING(hw_pd_check_tim_status());

        /* enable OTP to read TCS values */
        hw_otpc_init();
        hw_otpc_set_speed(HW_OTPC_CLK_FREQ_32MHz);
        hw_otpc_enter_mode(HW_OTPC_MODE_READ);
        /* get TCS values */
        sys_tcs_get_trim_values_from_cs();
        /*
         * Apply trimmed values for xtal32m in case no entry exists in OTP
         */
        hw_sys_apply_default_values();
        /* Close OPT */
        hw_otpc_close();
        configure_cache();

#if defined(CONFIG_RETARGET) || defined(CONFIG_RTT)
        /* This is needed to initialize stdout, so that it can be used by putchar (that doesn't initialize stdout,
         * contrary to printf). Putchar is needed by the Unity test framework
         * This also has the side effect of changing stdout to unbuffered (which seems more reasonable)
         */
        setvbuf(stdout, NULL, _IONBF, 0);
#endif

        /*
         * Keep PD_PER enabled.
         */
        hw_sys_pd_periph_enable();

        /* Default settings to be used if no CS setting is available*/
        CHARGER->CHARGER_TEST_CTRL_REG = DEFAULT_CHARGER_TEST_CTRL_REG;

        /*
         * Apply tcs settings.
         * They need to be re-applied when the blocks they contain are enabled.
         * PD_MEM is by default enabled.
         * PD_AON settings are applied by the booter
         */
        sys_tcs_apply_reg_pairs(SYS_TCS_GROUP_PD_MEM);
        sys_tcs_apply_reg_pairs(SYS_TCS_GROUP_PD_PER);
        /* In non baremetal apps PD_COMM will be opened by the  power manager */
#ifdef OS_BAREMETAL
        hw_sys_pd_com_enable();
        sys_tcs_apply_reg_pairs(SYS_TCS_GROUP_PD_COMM);
#endif
        sys_tcs_apply_reg_pairs(SYS_TCS_GROUP_PD_SYS);
        sys_tcs_apply_reg_pairs(SYS_TCS_GROUP_PD_TMR);

        /*
         * Apply custom trim settings which don't require the respective block to be enabled
         */
        sys_tcs_apply_custom_values(SYS_TCS_GROUP_GP_ADC_SINGLE_MODE, sys_tcs_custom_values_system_cb, NULL);
        sys_tcs_apply_custom_values(SYS_TCS_GROUP_GP_ADC_DIFF_MODE, sys_tcs_custom_values_system_cb, NULL);

        /*
         * Apply preferred settings on top of tcs settings.
         */
        hw_sys_set_preferred_values(HW_PD_AON);
        hw_sys_set_preferred_values(HW_PD_SYS);
        hw_sys_set_preferred_values(HW_PD_TMR);

#if dg_configUSE_CLOCK_MGR
        cm_clk_init_low_level_internal();
#else
        hw_clk_xtalm_configure();
        if (dg_configXTAL32M_SETTLE_TIME_IN_USEC != 0) {
                hw_clk_set_xtalm_settling_time(XTAL32M_USEC_TO_256K_CYCLES(dg_configXTAL32M_SETTLE_TIME_IN_USEC)/8, false);
        }
#endif

        configure_pdc();

#if dg_configUSE_CLOCK_MGR
        // Always enable the XTAL32M
        cm_enable_xtalm();
        while (!cm_poll_xtalm_ready());                 // Wait for XTAL32M to settle
        hw_clk_set_sysclk(SYS_CLK_IS_XTAL32M);          // Set XTAL32M as sys_clk

#if ((dg_configLP_CLK_SOURCE == LP_CLK_IS_ANALOG) && (dg_configUSE_LP_CLK == LP_CLK_RCX))
        /*
         * Note: If the LP clock is the RCX then we have to wait for the XTAL32M to settle
         *       since we need to estimate the frequency of the RCX before continuing
         *       (calibration procedure).
         */
        cm_rcx_calibrate();
        hw_clk_set_lpclk(LP_CLK_IS_RCX);        // Set RCX as the LP clock
#endif
#if dg_configUSE_HW_RTC
        configure_rtc();
#endif
#else
        /* perform clock initialization here, as there is no clock manager to do it later for us */
        nortos_clk_setup();
#endif

        /* Calculate pll_min_current value
         * Apply value to PLL_SYS_CTRL3_REG
         */
        hw_sys_pll_calculate_min_current();
        hw_sys_pll_set_min_current();

        /*
         * BOD protection
         */
#if (dg_configUSE_BOD == 1)
        /* BOD has already been enabled at this point but it must be reconfigured */
        hw_bod_configure();
#else
        hw_bod_deactivate();
#endif




#if dg_configFPGA_AD9361_RADIO
#if defined(CONFIG_USE_BLE)
        /*  We need the SPI port if external radio is used - enable it if needed. */
        if (!hw_pd_check_com_status()) {
                hw_sys_pd_com_enable();
        }

        ASSERT_ERROR(REG_GETF(CRG_TOP, PMU_CTRL_REG, COM_SLEEP) == 0);

        /* Initialize FPGA radio */
        ad9361_radio_init();

        /* Now COM power domain could be disabled - radio has been initialized. */
        hw_sys_pd_com_disable();
        hw_pd_wait_power_down_com();
#endif /* defined(CONFIG_USE_BLE) */
#endif /* dg_configFPGA_AD9361_RADIO */
}

uint32_t black_orca_phy_addr(uint32_t addr)
{
        uint32_t phy_addr;
        uint32_t flash_region_base_offset;
        uint32_t flash_region_size;
        HW_SYS_REMAP_ADDRESS_0 remap_addr0;
        static const uint32 remap[] = {
                MEMORY_ROM_BASE,
                MEMORY_OTP_BASE,
                MEMORY_QSPIF_BASE,
                MEMORY_SYSRAM_BASE,
                MEMORY_QSPIF_S_BASE,
                MEMORY_OTP_BASE,
                MEMORY_CACHERAM_BASE,
                0
        };

        static const uint32 flash_region_sizes[] = {
             32 * 1024 * 1024,
             16 * 1024 * 1024,
             8 * 1024 * 1024,
             4 * 1024 * 1024,
             2 * 1024 * 1024,
             1 * 1024 * 1024,
             512 * 1024,
             256 * 1024,
        };

        remap_addr0 = hw_sys_get_memory_remapping();

        if (remap_addr0 != HW_SYS_REMAP_ADDRESS_0_TO_QSPI_FLASH) {
                if (addr >= MEMORY_REMAPPED_END) {
                        phy_addr = addr;
                } else {
                        phy_addr = addr + remap[remap_addr0];
                }
        } else {
                /* Take into account flash region base, offset and size */
                flash_region_base_offset = REG_GETF(CACHE, CACHE_FLASH_REG, FLASH_REGION_BASE) << CACHE_CACHE_FLASH_REG_FLASH_REGION_BASE_Pos;
                flash_region_base_offset += REG_GETF(CACHE, CACHE_FLASH_REG, FLASH_REGION_OFFSET) << 2;
                flash_region_size = flash_region_sizes[REG_GETF(CACHE, CACHE_FLASH_REG, FLASH_REGION_SIZE)];
                if (addr < MEMORY_REMAPPED_END) {
                        /*
                         * In the remapped region, accesses are only allowed when
                         * 0 <= addr < flash_region_size.
                         */
                        ASSERT_ERROR(addr < flash_region_size);

                        phy_addr = flash_region_base_offset + addr;
                } else if (IS_QSPIF_ADDRESS(addr)) {
                        /*
                         * In QSPI AHB-C bus, accesses are only allowed when
                         * flash_region_base_offset <= addr
                         *   AND
                         * addr < flash_region_base_offset + flash_region_base_offset
                         */
                        ASSERT_ERROR(addr >= flash_region_base_offset);
                        ASSERT_ERROR(addr < flash_region_base_offset + flash_region_size);
                        phy_addr = addr;
                } else {
                        phy_addr = addr;
                }
        }

        return phy_addr;
}


