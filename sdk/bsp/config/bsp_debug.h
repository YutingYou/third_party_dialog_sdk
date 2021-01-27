/**
 * \addtogroup PLA_BSP_CONFIG
 * \{
 * \addtogroup BSP_DEBUG Debug Configuration
 * \brief Board debug support configuration definitions
 * \{
*/

/**
 ****************************************************************************************
 *
 * @file bsp_debug.h
 *
 * @brief Board Support Package. Debug Configuration.
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BSP_DEBUG_H_
#define BSP_DEBUG_H_

/**
 * \addtogroup DEBUG_SETTINGS
 *
 * \brief Debugging settings
 *
 * \{
 */
/* -------------------------------------- Debug settings ---------------------------------------- */

/**
 * \brief Enable debugger
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configENABLE_DEBUGGER
#define dg_configENABLE_DEBUGGER                (1)
#endif

/**
 * \brief Enable FreeRTOS thread aware debugging
 *
 * - 0 : Disabled
 * - 1 : Enabled
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configFREERTOS_ENABLE_THREAD_AWARENESS
#if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE)
#define dg_configFREERTOS_ENABLE_THREAD_AWARENESS (1)
#else
#define dg_configFREERTOS_ENABLE_THREAD_AWARENESS (0)
#endif
#endif

/**
 * \brief Enable CMAC debugger
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configENABLE_CMAC_DEBUGGER
#define dg_configENABLE_CMAC_DEBUGGER           (1)
#endif

/**
 * \brief Use SW cursor
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUSE_SW_CURSOR
#define dg_configUSE_SW_CURSOR                  (0)
#define SW_CURSOR_PORT                          (0)
#define SW_CURSOR_PIN                           (0)
#else
#               if !defined SW_CURSOR_PORT  &&  !defined SW_CURSOR_PIN
#                       define SW_CURSOR_PORT   (0)
#                       define SW_CURSOR_PIN    (16)
#               endif
#endif

/* ---------------------------------------------------------------------------------------------- */

/**
 * \}
 */

/**
 * \addtogroup DEBUG_SETTINGS
 * \{
 * \addtogroup SYSTEM_VIEW
 *
 * \brief System View configuration settings
 * \{
 */

/* ----------------------------- Segger System View configuration ------------------------------- */

/**
 * \brief Segger's System View
 *
 * When enabled the application should also call SEGGER_SYSVIEW_Conf() to enable system monitoring.
 * configTOTAL_HEAP_SIZE should be increased by dg_configSYSTEMVIEW_STACK_OVERHEAD bytes for each system task.
 * For example, if there are 8 system tasks configTOTAL_HEAP_SIZE should be increased by
 * (8 * dg_configSYSTEMVIEW_STACK_OVERHEAD) bytes.
 *
 * - 0 : Disabled
 * - 1 : Enabled
 *
 * \bsp_default_note{\bsp_config_option_app,}
 *
 */
#ifndef dg_configSYSTEMVIEW
#define dg_configSYSTEMVIEW                     (0)
#endif

/**
 * \brief Stack size overhead when System View API is used
 *
 * All thread stack sizes plus the the stack of IRQ handlers will be increased by that amount
 * to avoid stack overflow when System View is monitoring the system.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configSYSTEMVIEW_STACK_OVERHEAD
#define dg_configSYSTEMVIEW_STACK_OVERHEAD      (256)
#endif

/*
 * Enable/Disable System View monitoring time critical interrupt handlers (BLE, CPM, USB).
 * Disabling ISR monitoring could help reducing assertions triggered by System View monitoring overhead.
 *
 */

/**
 * \brief Let System View monitor BLE related ISRs (BLE_GEN_Handler / BLE_WAKEUP_LP_Handler).
 *
 * - 0 : Disabled
 * - 1 : Enabled
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configSYSTEMVIEW_MONITOR_BLE_ISR
#define dg_configSYSTEMVIEW_MONITOR_BLE_ISR     (1)
#endif

/**
 * \brief Let System View monitor CPM related ISRs (SWTIM1_Handler / WKUP_GPIO_Handler).
 *
 * - 0 : Disabled
 * - 1 : Enabled
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configSYSTEMVIEW_MONITOR_CPM_ISR
#define dg_configSYSTEMVIEW_MONITOR_CPM_ISR     (1)
#endif

/**
 * \brief Let System View monitor USB related ISRs (USB_Handler / VBUS_Handler).
 *
 * - 0 : Disabled
 * - 1 : Enabled
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configSYSTEMVIEW_MONITOR_USB_ISR
#define dg_configSYSTEMVIEW_MONITOR_USB_ISR     (1)
#endif
/* ---------------------------------------------------------------------------------------------- */

/**
 * \}
 * \}
 */

/* --------------------------------- DEBUG GPIO handling macros --------------------------------- */

#define DBG_GET_SET_PAD_LATCH_REG(name)   *(((volatile uint32_t *)(&name##_SET_REG) == (volatile uint32_t *)(&GPIO->P0_SET_DATA_REG)) ? &CRG_TOP->P0_SET_PAD_LATCH_REG : &CRG_TOP->P1_SET_PAD_LATCH_REG)
#define DBG_GET_RESET_PAD_LATCH_REG(name) *(((volatile uint32_t *)(&name##_SET_REG) == (volatile uint32_t *)(&GPIO->P0_SET_DATA_REG)) ? &CRG_TOP->P0_RESET_PAD_LATCH_REG : &CRG_TOP->P1_RESET_PAD_LATCH_REG)
#define DBG_TOGGLE_PIN_PAD_LATCH(name)    DBG_GET_SET_PAD_LATCH_REG(name) = name##_PIN; \
                                          DBG_GET_RESET_PAD_LATCH_REG(name) = name##_PIN
#define DBG_SET_PIN_REG(name)             name##_SET_REG = name##_PIN
#define DBG_RESET_PIN_REG(name)           name##_RESET_REG = name##_PIN

#define DBG_CONFIGURE(flag, name, func)                                          \
{                                                                                \
        if (flag == 1) {                                                         \
                ASSERT_WARNING(REG_GETF(CRG_TOP, SYS_STAT_REG, COM_IS_UP) == 1); \
                name##_MODE_REG = 0x300 + func;                                  \
                DBG_TOGGLE_PIN_PAD_LATCH(name);                                  \
        }                                                                        \
}

#define DBG_CONFIGURE_HIGH(flag, name)                                           \
{                                                                                \
        if (flag == 1) {                                                         \
                ASSERT_WARNING(REG_GETF(CRG_TOP, SYS_STAT_REG, COM_IS_UP) == 1); \
                name##_MODE_REG = 0x300;                                         \
                DBG_SET_PIN_REG(name);                                           \
                DBG_TOGGLE_PIN_PAD_LATCH(name);                                  \
        }                                                                        \
}

#define DBG_CONFIGURE_LOW(flag, name)                                            \
{                                                                                \
        if (flag == 1) {                                                         \
                ASSERT_WARNING(REG_GETF(CRG_TOP, SYS_STAT_REG, COM_IS_UP) == 1); \
                name##_MODE_REG = 0x300;                                         \
                DBG_RESET_PIN_REG(name);                                         \
                DBG_TOGGLE_PIN_PAD_LATCH(name);                                  \
        }                                                                        \
}

#define DBG_SET_HIGH(flag, name)                                                 \
{                                                                                \
        if (flag == 1) {                                                         \
                ASSERT_WARNING(REG_GETF(CRG_TOP, SYS_STAT_REG, COM_IS_UP) == 1); \
                name##_MODE_REG = 0x300;                                         \
                DBG_SET_PIN_REG(name);                                           \
                DBG_TOGGLE_PIN_PAD_LATCH(name);                                  \
        }                                                                        \
}

#define DBG_SET_LOW(flag, name)                                                  \
{                                                                                \
        if (flag == 1) {                                                         \
                ASSERT_WARNING(REG_GETF(CRG_TOP, SYS_STAT_REG, COM_IS_UP) == 1); \
                name##_MODE_REG = 0x300;                                         \
                DBG_RESET_PIN_REG(name);                                         \
                DBG_TOGGLE_PIN_PAD_LATCH(name);                                  \
        }                                                                        \
}


/* ---------------------------------------------------------------------------------------------- */


/* ---------------------------------- HardFault or NMI event ------------------------------------ */

#ifndef EXCEPTION_DEBUG
#define EXCEPTION_DEBUG                         (0)     // Requires GPIO config.
#endif

/* ---------------------------------------------------------------------------------------------- */


/* --------------------------------- Clock and Power Manager ------------------------------------ */


#ifndef PWR_MGR_DEBUG
#define PWR_MGR_DEBUG                           (0)
#endif

#ifndef SYS_TIM_DEBUG
#define SYS_TIM_DEBUG                           (0)
#endif

#ifndef PWR_MGR_FUNCTIONAL_DEBUG
#define PWR_MGR_FUNCTIONAL_DEBUG                (0)     // Requires GPIO config.
#endif

#ifndef PWR_MGR_USE_TIMING_DEBUG
#define PWR_MGR_USE_TIMING_DEBUG                (0)     // Requires GPIO config.
#endif

#ifndef CLK_MGR_USE_TIMING_DEBUG
#define CLK_MGR_USE_TIMING_DEBUG                (0)     // Requires GPIO config.
#endif


/* Controls which RAM blocks will be retained when the MEASURE_SLEEP_CURRENT test mode is used
 * (optional). */
#ifndef dg_configTESTMODE_RETAIN_RAM
#define dg_configTESTMODE_RETAIN_RAM            (0x1F)
#endif

/* Controls whether the Cache will be retained when the MEASURE_SLEEP_CURRENT test mode is used
 * (optional). */
#ifndef dg_configTESTMODE_RETAIN_CACHE
#define dg_configTESTMODE_RETAIN_CACHE          (0)
#endif

/* Controls whether the ECC RAM will be retained when the MEASURE_SLEEP_CURRENT test mode is used
 * (optional). */
#ifdef dg_config_TESTMODE_RETAIN_ECCRAM
#error "dg_config_TESTMODE_RETAIN_ECCRAM is no longer supported. "
       "Use dg_configTESTMODE_RETAIN_ECCRAM instead (no underscore)!"
#endif

#ifndef dg_configTESTMODE_RETAIN_ECCRAM
#define dg_configTESTMODE_RETAIN_ECCRAM        (0)
#endif

/* ---------------------------------------------------------------------------------------------- */


/* --------------------------------------- USB Charger ------------------------------------------ */


#ifndef SYS_CHARGER_TIMING_DEBUG
#define SYS_CHARGER_TIMING_DEBUG                (0)
#endif


/* ---------------------------------------------------------------------------------------------- */


/* ------------------------------------------- BLE ---------------------------------------------- */


#ifndef BLE_ADAPTER_DEBUG
#define BLE_ADAPTER_DEBUG                       (0)     // Requires GPIO config.
#endif

#define BLE_RX_EN_FUNC                          (57)

#ifndef BLE_WINDOW_STATISTICS
#define BLE_WINDOW_STATISTICS                   (0)
#endif

#ifndef BLE_SLEEP_PERIOD_DEBUG
#define BLE_SLEEP_PERIOD_DEBUG                  (0)     // Requires logging and window statistics.
#endif

#ifndef BLE_WAKEUP_MONITOR_PERIOD
#define BLE_WAKEUP_MONITOR_PERIOD               (1024)
#endif

#ifndef BLE_MAX_MISSES_ALLOWED
#define BLE_MAX_MISSES_ALLOWED                  (0)
#endif

#ifndef BLE_MAX_DELAYS_ALLOWED
#define BLE_MAX_DELAYS_ALLOWED                  (0)
#endif

#ifndef BLE_SSP_DEBUG
#define BLE_SSP_DEBUG                           (0)
#endif

/* ---------------------------------------------------------------------------------------------- */


/* ------------------------------------------ Flash --------------------------------------------- */
#ifndef FLASH_DEBUG
#define FLASH_DEBUG                             (0)     // Requires GPIO config.
#endif

#ifndef __DBG_QSPI_ENABLED
#define __DBG_QSPI_ENABLED                      (0)
#endif

/* ---------------------------------------------------------------------------------------------- */


/* ------------------------------------------ Common -------------------------------------------- */
#ifndef CMN_TIMING_DEBUG
#define CMN_TIMING_DEBUG                        (0)     // Requires GPIO config.
#endif
/* ---------------------------------------------------------------------------------------------- */

/* ------------------------------------ GPIO configuration -------------------------------------- */

/* Enable/Disable GPIO pin assignment conflict detection
 */
#define DEBUG_GPIO_ALLOC_MONITOR_ENABLED        (0)


/* Exception handling debug configuration
 *
 */
#if (EXCEPTION_DEBUG == 0)
// Dummy values to suppress compiler errors
#define EXCEPTIONDBG_MODE_REG                   *(volatile int *)0x20000000
#define EXCEPTIONDBG_SET_REG                    *(volatile int *)0x20000000
#define EXCEPTIONDBG_RESET_REG                  *(volatile int *)0x20000000
#define EXCEPTIONDBG_PIN                        (0)

#else


// Tick
#define EXCEPTIONDBG_MODE_REG                   GPIO->P0_30_MODE_REG
#define EXCEPTIONDBG_SET_REG                    GPIO->P0_SET_DATA_REG
#define EXCEPTIONDBG_RESET_REG                  GPIO->P0_RESET_DATA_REG
#define EXCEPTIONDBG_PIN                        (1 << 30)


#endif /* EXCEPTION_DEBUG */

/* Functional debug configuration
 *
 * Note that GPIO overlapping is allowed if the tracked events are discrete and the initial GPIO
 * configuration is the same! No checking is performed for erroneous configuration though!
 *
 */

#if (PWR_MGR_FUNCTIONAL_DEBUG == 0)
// Dummy values to suppress compiler errors
#define PWRDBG_TICK_MODE_REG                    *(volatile int *)0x20000000
#define PWRDBG_TICK_SET_REG                     *(volatile int *)0x20000000
#define PWRDBG_TICK_RESET_REG                   *(volatile int *)0x20000000
#define PWRDBG_TICK_PIN                         (0)

#define PWRDBG_POWERUP_MODE_REG                 *(volatile int *)0x20000000
#define PWRDBG_POWERUP_SET_REG                  *(volatile int *)0x20000000
#define PWRDBG_POWERUP_RESET_REG                *(volatile int *)0x20000000
#define PWRDBG_POWERUP_PIN                      (0)

#else

// Tick
#define PWRDBG_TICK_MODE_REG                    GPIO->P0_19_MODE_REG
#define PWRDBG_TICK_SET_REG                     GPIO->P0_SET_DATA_REG
#define PWRDBG_TICK_RESET_REG                   GPIO->P0_RESET_DATA_REG
#define PWRDBG_TICK_PIN                         (1 << 19)

// Active / Sleep
#define PWRDBG_POWERUP_MODE_REG                 GPIO->P0_21_MODE_REG
#define PWRDBG_POWERUP_SET_REG                  GPIO->P0_SET_DATA_REG
#define PWRDBG_POWERUP_RESET_REG                GPIO->P0_RESET_DATA_REG
#define PWRDBG_POWERUP_PIN                      (1 << 21)

#endif /* PWR_MGR_FUNCTIONAL_DEBUG */


/* Timing debug configuration
 *
 * Note that in this mode the pad latches are removed immediately after the execution resumes from
 * the __WFI(). Because of this, it is not advised to use this feature in projects that use GPIOS.
 * Nevertheless, in case it is used, make sure that the "peripheral initialization" is also done
 * at that point, modifying sys_power_mgr.c accordingly.
 *
 * Note also that GPIO overlapping is allowed if the tracked events are discrete and the initial
 * GPIO configuration is the same! No checking is performed for erroneous configuration though!
 *
 */

#if (PWR_MGR_USE_TIMING_DEBUG == 0)
// Dummy values to suppress compiler errors
#define PWRDBG_SLEEP_ENTER_MODE_REG             *(volatile int *)0x20000000
#define PWRDBG_SLEEP_ENTER_SET_REG              *(volatile int *)0x20000000
#define PWRDBG_SLEEP_ENTER_RESET_REG            *(volatile int *)0x20000000
#define PWRDBG_SLEEP_ENTER_PIN                  (0)

#define PWRDBG_SLEEP_EXIT_MODE_REG              *(volatile int *)0x20000000
#define PWRDBG_SLEEP_EXIT_SET_REG               *(volatile int *)0x20000000
#define PWRDBG_SLEEP_EXIT_RESET_REG             *(volatile int *)0x20000000
#define PWRDBG_SLEEP_EXIT_PIN                   (0)

#else

// Power manager: sleep or idle entry (until __WFI() is called)
#define PWRDBG_SLEEP_ENTER_MODE_REG             GPIO->P0_17_MODE_REG
#define PWRDBG_SLEEP_ENTER_SET_REG              GPIO->P0_SET_DATA_REG
#define PWRDBG_SLEEP_ENTER_RESET_REG            GPIO->P0_RESET_DATA_REG
#define PWRDBG_SLEEP_ENTER_PIN                  (1 << 17)

// Power manager: sleep or idle exit
#define PWRDBG_SLEEP_EXIT_MODE_REG              GPIO->P0_18_MODE_REG
#define PWRDBG_SLEEP_EXIT_SET_REG               GPIO->P0_SET_DATA_REG
#define PWRDBG_SLEEP_EXIT_RESET_REG             GPIO->P0_RESET_DATA_REG
#define PWRDBG_SLEEP_EXIT_PIN                   (1 << 18)

#endif /* CLK_MGR_USE_TIMING_DEBUG */

#if (CLK_MGR_USE_TIMING_DEBUG == 0)
// Dummy values to suppress compiler errors
#define CLKDBG_LOWER_CLOCKS_MODE_REG            *(volatile int *)0x20000000
#define CLKDBG_LOWER_CLOCKS_SET_REG             *(volatile int *)0x20000000
#define CLKDBG_LOWER_CLOCKS_RESET_REG           *(volatile int *)0x20000000
#define CLKDBG_LOWER_CLOCKS_PIN                 (0)

#define CLKDBG_XTAL32M_SETTLED_MODE_REG         *(volatile int *)0x20000000
#define CLKDBG_XTAL32M_SETTLED_SET_REG          *(volatile int *)0x20000000
#define CLKDBG_XTAL32M_SETTLED_RESET_REG        *(volatile int *)0x20000000
#define CLKDBG_XTAL32M_SETTLED_PIN              (0)

#define CLKDBG_XTAL32M_READY_MODE_REG           *(volatile int *)0x20000000
#define CLKDBG_XTAL32M_READY_SET_REG            *(volatile int *)0x20000000
#define CLKDBG_XTAL32M_READY_RESET_REG          *(volatile int *)0x20000000
#define CLKDBG_XTAL32M_READY_PIN                (0)

#define CLKDBG_PLL_ON_MODE_REG                  *(volatile int *)0x20000000
#define CLKDBG_PLL_ON_SET_REG                   *(volatile int *)0x20000000
#define CLKDBG_PLL_ON_RESET_REG                 *(volatile int *)0x20000000
#define CLKDBG_PLL_ON_PIN                       (0)

#else

// Low clocks
#define CLKDBG_LOWER_CLOCKS_MODE_REG            GPIO->P0_26_MODE_REG
#define CLKDBG_LOWER_CLOCKS_SET_REG             GPIO->P0_SET_DATA_REG
#define CLKDBG_LOWER_CLOCKS_RESET_REG           GPIO->P0_RESET_DATA_REG
#define CLKDBG_LOWER_CLOCKS_PIN                 (1 << 26)

// XTAL32M settling
#define CLKDBG_XTAL32M_SETTLED_MODE_REG         GPIO->P0_27_MODE_REG
#define CLKDBG_XTAL32M_SETTLED_SET_REG          GPIO->P0_SET_DATA_REG
#define CLKDBG_XTAL32M_SETTLED_RESET_REG        GPIO->P0_RESET_DATA_REG
#define CLKDBG_XTAL32M_SETTLED_PIN              (1 << 27)

// XTAL32M Ready IRQ
#define CLKDBG_XTAL32M_READY_MODE_REG           GPIO->P0_28_MODE_REG
#define CLKDBG_XTAL32M_READY_SET_REG            GPIO->P0_SET_DATA_REG
#define CLKDBG_XTAL32M_READY_RESET_REG          GPIO->P0_RESET_DATA_REG
#define CLKDBG_XTAL32M_READY_PIN                (1 << 28)

// PLL is on
#define CLKDBG_PLL_ON_MODE_REG                  GPIO->P0_29_MODE_REG
#define CLKDBG_PLL_ON_SET_REG                   GPIO->P0_SET_DATA_REG
#define CLKDBG_PLL_ON_RESET_REG                 GPIO->P0_RESET_DATA_REG
#define CLKDBG_PLL_ON_PIN                       (1 << 29)

#endif /* CLK_MGR_USE_TIMING_DEBUG */



#if (BLE_ADAPTER_DEBUG == 0)
#define BLEBDG_ADAPTER_MODE_REG                 *(volatile int *)0x20000000
#define BLEBDG_ADAPTER_SET_REG                  *(volatile int *)0x20000000
#define BLEBDG_ADAPTER_RESET_REG                *(volatile int *)0x20000000
#define BLEBDG_ADAPTER_PIN                      (0)

#else


#define BLEBDG_ADAPTER_MODE_REG                 GPIO->P0_31_MODE_REG
#define BLEBDG_ADAPTER_SET_REG                  GPIO->P0_SET_DATA_REG
#define BLEBDG_ADAPTER_RESET_REG                GPIO->P0_RESET_DATA_REG
#define BLEBDG_ADAPTER_PIN                      (1 << 31)


#endif /* BLE_ADAPTER_DEBUG */

#if (SYS_CHARGER_TIMING_DEBUG == 0)

#define SYS_CHARGER_DBG_VBUS_MODE_REG           *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_VBUS_SET_REG            *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_VBUS_RESET_REG          *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_VBUS_PIN                (0)

#define SYS_CHARGER_DBG_CH_EVT_MODE_REG         *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_CH_EVT_SET_REG          *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_CH_EVT_RESET_REG        *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_CH_EVT_PIN              (0)

#define SYS_CHARGER_DBG_PRE_CH_MODE_REG         *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_PRE_CH_SET_REG          *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_PRE_CH_RESET_REG        *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_PRE_CH_PIN              (0)

#define SYS_CHARGER_DBG_CH_MODE_REG             *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_CH_SET_REG              *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_CH_RESET_REG            *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_CH_PIN                  (0)

#define SYS_CHARGER_DBG_EOC_MODE_REG            *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_EOC_SET_REG             *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_EOC_RESET_REG           *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_EOC_PIN                 (0)

#define SYS_CHARGER_DBG_ENUM_DONE_MODE_REG      *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_ENUM_DONE_SET_REG       *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_ENUM_DONE_RESET_REG     *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_ENUM_DONE_PIN           (0)

#define SYS_CHARGER_DBG_SUS_MODE_REG             *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_SUS_SET_REG              *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_SUS_RESET_REG            *(volatile int *)0x20000000
#define SYS_CHARGER_DBG_SUS_PIN                  (0)

#else

#define SYS_CHARGER_DBG_VBUS_MODE_REG           GPIO->P1_10_MODE_REG
#define SYS_CHARGER_DBG_VBUS_SET_REG            GPIO->P1_SET_DATA_REG
#define SYS_CHARGER_DBG_VBUS_RESET_REG          GPIO->P1_RESET_DATA_REG
#define SYS_CHARGER_DBG_VBUS_PIN                (1 << 10)

#define SYS_CHARGER_DBG_CH_EVT_MODE_REG         GPIO->P1_11_MODE_REG
#define SYS_CHARGER_DBG_CH_EVT_SET_REG          GPIO->P1_SET_DATA_REG
#define SYS_CHARGER_DBG_CH_EVT_RESET_REG        GPIO->P1_RESET_DATA_REG
#define SYS_CHARGER_DBG_CH_EVT_PIN              (1 << 11)

#define SYS_CHARGER_DBG_PRE_CH_MODE_REG         GPIO->P1_12_MODE_REG
#define SYS_CHARGER_DBG_PRE_CH_SET_REG          GPIO->P1_SET_DATA_REG
#define SYS_CHARGER_DBG_PRE_CH_RESET_REG        GPIO->P1_RESET_DATA_REG
#define SYS_CHARGER_DBG_PRE_CH_PIN              (1 << 12)

#define SYS_CHARGER_DBG_CH_MODE_REG             GPIO->P1_13_MODE_REG
#define SYS_CHARGER_DBG_CH_SET_REG              GPIO->P1_SET_DATA_REG
#define SYS_CHARGER_DBG_CH_RESET_REG            GPIO->P1_RESET_DATA_REG
#define SYS_CHARGER_DBG_CH_PIN                  (1 << 13)

#define SYS_CHARGER_DBG_EOC_MODE_REG            GPIO->P1_14_MODE_REG
#define SYS_CHARGER_DBG_EOC_SET_REG             GPIO->P1_SET_DATA_REG
#define SYS_CHARGER_DBG_EOC_RESET_REG           GPIO->P1_RESET_DATA_REG
#define SYS_CHARGER_DBG_EOC_PIN                 (1 << 14)

#define SYS_CHARGER_DBG_ENUM_DONE_MODE_REG      GPIO->P1_15_MODE_REG
#define SYS_CHARGER_DBG_ENUM_DONE_SET_REG       GPIO->P1_SET_DATA_REG
#define SYS_CHARGER_DBG_ENUM_DONE_RESET_REG     GPIO->P1_RESET_DATA_REG
#define SYS_CHARGER_DBG_ENUM_DONE_PIN           (1 << 15)

#define SYS_CHARGER_DBG_SUS_MODE_REG             GPIO->P1_16_MODE_REG
#define SYS_CHARGER_DBG_SUS_SET_REG              GPIO->P1_SET_DATA_REG
#define SYS_CHARGER_DBG_SUS_RESET_REG            GPIO->P1_RESET_DATA_REG
#define SYS_CHARGER_DBG_SUS_PIN                  (1 << 16)


#endif /* SYS_CHARGER_TIMING_DEBUG */

#if (CMN_TIMING_DEBUG == 0)
// Common: Inside critical section (initial configuration: low)
#define CMNDBG_CRITICAL_SECTION_MODE_REG       *(volatile int *)0x20000000
#define CMNDBG_CRITICAL_SECTION_SET_REG        *(volatile int *)0x20000000
#define CMNDBG_CRITICAL_SECTION_RESET_REG      *(volatile int *)0x20000000
#define CMNDBG_CRITICAL_SECTION_PIN            (0)

#else


// Common: Inside critical section
#define CMNDBG_CRITICAL_SECTION_MODE_REG        GPIO->P0_24_MODE_REG
#define CMNDBG_CRITICAL_SECTION_SET_REG         GPIO->P0_SET_DATA_REG
#define CMNDBG_CRITICAL_SECTION_RESET_REG       GPIO->P0_RESET_DATA_REG
#define CMNDBG_CRITICAL_SECTION_PIN             (1 << 24)


#endif /* CMN_TIMING_DEBUG */


/* Flash debug configuration
 *
 */
#if (FLASH_DEBUG == 0)
// Write page (initial configuration: low)
#define FLASHDBG_PAGE_PROG_MODE_REG             *(volatile int *)0x20000000
#define FLASHDBG_PAGE_PROG_SET_REG              *(volatile int *)0x20000000
#define FLASHDBG_PAGE_PROG_RESET_REG            *(volatile int *)0x20000000
#define FLASHDBG_PAGE_PROG_PIN                  (0)

// Program page wait loop (initial configuration: low)
#define FLASHDBG_PAGE_PROG_WL_MODE_REG          *(volatile int *)0x20000000
#define FLASHDBG_PAGE_PROG_WL_SET_REG           *(volatile int *)0x20000000
#define FLASHDBG_PAGE_PROG_WL_RESET_REG         *(volatile int *)0x20000000
#define FLASHDBG_PAGE_PROG_WL_PIN               (0)

// Program page wait loop - pending irq check (initial configuration: low)
#define FLASHDBG_PAGE_PROG_WL_IRQ_MODE_REG      *(volatile int *)0x20000000
#define FLASHDBG_PAGE_PROG_WL_IRQ_SET_REG       *(volatile int *)0x20000000
#define FLASHDBG_PAGE_PROG_WL_IRQ_RESET_REG     *(volatile int *)0x20000000
#define FLASHDBG_PAGE_PROG_WL_IRQ_PIN           (0)

// Suspend op (initial configuration: low)
#define FLASHDBG_SUSPEND_MODE_REG               *(volatile int *)0x20000000
#define FLASHDBG_SUSPEND_SET_REG                *(volatile int *)0x20000000
#define FLASHDBG_SUSPEND_RESET_REG              *(volatile int *)0x20000000
#define FLASHDBG_SUSPEND_PIN                    (0)

// Erase sector cmd (initial configuration: low)
#define FLASHDBG_SECTOR_ERASE_MODE_REG          *(volatile int *)0x20000000
#define FLASHDBG_SECTOR_ERASE_SET_REG           *(volatile int *)0x20000000
#define FLASHDBG_SECTOR_ERASE_RESET_REG         *(volatile int *)0x20000000
#define FLASHDBG_SECTOR_ERASE_PIN               (0)

// Notify task (initial configuration: low)
#define FLASHDBG_TASK_NOTIFY_MODE_REG           *(volatile int *)0x20000000
#define FLASHDBG_TASK_NOTIFY_SET_REG            *(volatile int *)0x20000000
#define FLASHDBG_TASK_NOTIFY_RESET_REG          *(volatile int *)0x20000000
#define FLASHDBG_TASK_NOTIFY_PIN                (0)

// Suspend action (low level) (initial configuration: low)
#define FLASHDBG_SUSPEND_ACTION_MODE_REG        *(volatile int *)0x20000000
#define FLASHDBG_SUSPEND_ACTION_SET_REG         *(volatile int *)0x20000000
#define FLASHDBG_SUSPEND_ACTION_RESET_REG       *(volatile int *)0x20000000
#define FLASHDBG_SUSPEND_ACTION_PIN             (0)

// Resume op (initial configuration: low)
#define FLASHDBG_RESUME_MODE_REG                *(volatile int *)0x20000000
#define FLASHDBG_RESUME_SET_REG                 *(volatile int *)0x20000000
#define FLASHDBG_RESUME_RESET_REG               *(volatile int *)0x20000000
#define FLASHDBG_RESUME_PIN                     (0)

#else



// Write page (initial configuration: low)
#define FLASHDBG_PAGE_PROG_MODE_REG             GPIO->P1_01_MODE_REG
#define FLASHDBG_PAGE_PROG_SET_REG              GPIO->P1_SET_DATA_REG
#define FLASHDBG_PAGE_PROG_RESET_REG            GPIO->P1_RESET_DATA_REG
#define FLASHDBG_PAGE_PROG_PIN                  (1 << 1)

// Program page wait loop (initial configuration: low)
#define FLASHDBG_PAGE_PROG_WL_MODE_REG          GPIO->P1_02_MODE_REG
#define FLASHDBG_PAGE_PROG_WL_SET_REG           GPIO->P1_SET_DATA_REG
#define FLASHDBG_PAGE_PROG_WL_RESET_REG         GPIO->P1_RESET_DATA_REG
#define FLASHDBG_PAGE_PROG_WL_PIN               (1 << 2)

// Program page wait loop - pending irq check (initial configuration: low)
#define FLASHDBG_PAGE_PROG_WL_IRQ_MODE_REG      GPIO->P1_03_MODE_REG
#define FLASHDBG_PAGE_PROG_WL_IRQ_SET_REG       GPIO->P1_SET_DATA_REG
#define FLASHDBG_PAGE_PROG_WL_IRQ_RESET_REG     GPIO->P1_RESET_DATA_REG
#define FLASHDBG_PAGE_PROG_WL_IRQ_PIN           (1 << 3)

// Suspend op (initial configuration: low)
#define FLASHDBG_SUSPEND_MODE_REG               GPIO->P1_04_MODE_REG
#define FLASHDBG_SUSPEND_SET_REG                GPIO->P1_SET_DATA_REG
#define FLASHDBG_SUSPEND_RESET_REG              GPIO->P1_RESET_DATA_REG
#define FLASHDBG_SUSPEND_PIN                    (1 << 4)

// Erase sector cmd (initial configuration: low)
#define FLASHDBG_SECTOR_ERASE_MODE_REG          GPIO->P1_05_MODE_REG
#define FLASHDBG_SECTOR_ERASE_SET_REG           GPIO->P1_SET_DATA_REG
#define FLASHDBG_SECTOR_ERASE_RESET_REG         GPIO->P1_RESET_DATA_REG
#define FLASHDBG_SECTOR_ERASE_PIN               (1 << 5)

// Notify task (initial configuration: low)
#define FLASHDBG_TASK_NOTIFY_MODE_REG           GPIO->P1_06_MODE_REG
#define FLASHDBG_TASK_NOTIFY_SET_REG            GPIO->P1_SET_DATA_REG
#define FLASHDBG_TASK_NOTIFY_RESET_REG          GPIO->P1_RESET_DATA_REG
#define FLASHDBG_TASK_NOTIFY_PIN                (1 << 6)

// Suspend action (low level) (initial configuration: low)
#define FLASHDBG_SUSPEND_ACTION_MODE_REG        GPIO->P1_07_MODE_REG
#define FLASHDBG_SUSPEND_ACTION_SET_REG         GPIO->P1_SET_DATA_REG
#define FLASHDBG_SUSPEND_ACTION_RESET_REG       GPIO->P1_RESET_DATA_REG
#define FLASHDBG_SUSPEND_ACTION_PIN             (1 << 7)

// Resume op (initial configuration: low)
#define FLASHDBG_RESUME_MODE_REG                GPIO->P1_08_MODE_REG
#define FLASHDBG_RESUME_SET_REG                 GPIO->P1_SET_DATA_REG
#define FLASHDBG_RESUME_RESET_REG               GPIO->P1_RESET_DATA_REG
#define FLASHDBG_RESUME_PIN                     (1 << 8)



#endif /* FLASH_DEBUG */


/* Enables the logging of stack (RW) heap memories usage.
 *
 * The feature shall only be enabled in development/debug mode
 */
#ifndef dg_configLOG_BLE_STACK_MEM_USAGE
#define dg_configLOG_BLE_STACK_MEM_USAGE                (0)
#endif



#endif /* BSP_DEBUG_H_ */

/**
\}
\}
*/
