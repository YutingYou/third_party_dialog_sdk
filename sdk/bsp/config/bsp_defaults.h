/**
 * \addtogroup PLA_BSP_CONFIG
 * \{
 * \addtogroup BSP_CONFIG_DEFAULTS BSP Default Configuration Values
 *
 * \brief Board support package default configuration values
 *
 * The following tags are used to describe the type of each configuration option.
 *
 * - **\bsp_config_option_build**       : To be changed only in the build configuration
 *                                        of the project ("Defined symbols -D" in the
 *                                        preprocessor options).
 *
 * - **\bsp_config_option_app**         : To be changed only in the custom_config*.h
 *                                        project files.
 *
 * - **\bsp_config_option_expert_only** : To be changed only by an expert user.
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file bsp_defaults.h
 *
 * @brief Board Support Package. System configuration default values.
 *
 * Copyright (C) 2015-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BSP_DEFAULTS_H_
#define BSP_DEFAULTS_H_

/* ---------------------------------------------------------------------------------------------- */


/* ----------------------------------- Deprecated Configuration --------------------------------- */

/* Deprecated configuration options must not be defined by the application. */

#define DG_CONFIG_DEPRECATED_MSG " must not be defined by the application. This option is NO LONGER SUPPORTED"
#define DG_CONFIG_NOT_APPLICABLE_MSG " must not be defined by the application. It is NOT APPLICABLE"

#ifdef dg_configPOWER_CONFIG
#error "dg_configPOWER_CONFIG must not be defined by the application. This option is NO LONGER SUPPORTED."\
       " There's a single power configuration setup."
#endif

#ifdef dg_configFORCE_DEEP_SLEEP
#error "dg_configFORCE_DEEP_SLEEP must not be defined by the application. This option is NO LONGER SUPPORTED."\
       " Forcing the system to enter into clockless sleep during deep sleep is no longer supported."
#endif

#ifdef dg_configMEM_RETENTION_MODE_PRESERVE_IMAGE
#error "dg_configMEM_RETENTION_MODE_PRESERVE_IMAGE must not be defined by the application. This option is NO LONGER SUPPORTED."
#endif

#ifdef dg_configUSE_ADC
#error "dg_configUSE_ADC must not be defined by the application. This option is NO LONGER SUPPORTED."\
       " Specify the ADC block you wish to configure (i.e. dg_configUSE_HW_GPADC)."
#endif

#ifdef dg_configCONFIG_HEADER_IN_FLASH
#error "dg_configCONFIG_HEADER_IN_FLASH must not be defined by the application. This option is NO LONGER SUPPORTED."\
       " The values of the trim registers are not anymore taken from the Flash."
#endif

#ifdef dg_configPOWER_EXT_1V8_PERIPHERALS
#error "dg_configPOWER_EXT_1V8_PERIPHERALS must not be defined by the application. This option is NO LONGER SUPPORTED."\
       " Use dg_configPOWER_1V8P instead!"
#endif

#ifdef dg_configUSE_HW_TEMPSENS
#error "dg_configUSE_HW_TEMPSENS must not be defined by the application. This option is NO LONGER SUPPORTED."\
       " Use dg_configUSE_HW_GPADC and HW_GPADC_INPUT_SE_TEMPSENS instead!"
#endif

#ifdef dg_configINITIAL_SLEEP_DELAY_TIME
#pragma message "dg_configINITIAL_SLEEP_DELAY_TIME" DG_CONFIG_DEPRECATED_MSG ". At startup, the system will stay "\
                "active for at least dg_configXTAL32K_SETTLE_TIME before it is allowed to go to sleep."
#endif

#ifdef dg_configPOWER_FLASH
# pragma message "dg_configPOWER_FLASH" DG_CONFIG_DEPRECATED_MSG
# undef  dg_configPOWER_FLASH
#endif

/* --------------------------------- Clock settings -------------------------------------------- */

/**
 * \addtogroup CLOCK_SETTINGS Clock Settings (Low Power, XTAL etc.)
 *
 * \brief Settings for the different clock-types of the chip
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 * \{
 */

/**
 * \brief Source of Low Power clock used (LP_CLK_IS_ANALOG, LP_CLK_IS_DIGITAL)
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configLP_CLK_SOURCE
#define dg_configLP_CLK_SOURCE                          LP_CLK_IS_ANALOG
#endif

#if (dg_configUSE_LP_CLK == LP_CLK_ANY)
#error "LP_CLK_ANY is not currently supported!"
#endif

/**
 * \brief Low Power clock used (LP_CLK_32000, LP_CLK_32768, LP_CLK_RCX)
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUSE_LP_CLK
#define dg_configUSE_LP_CLK                             LP_CLK_RCX
#endif

#if ((dg_configLP_CLK_SOURCE == LP_CLK_IS_DIGITAL) && (dg_configUSE_LP_CLK == LP_CLK_RCX))
#error "When the LP source is digital (External), the option LP_CLK_RCX is invalid!"
#endif

/**
 * \brief External LP type
 *
 * - 0: a crystal is connected to XTAL32Kp and XTALK32Km
 * - 1: a digital clock is provided.
 *
 * \note the frequency of the digital clock must be 32KHz or 32.768KHz and be always running.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configEXT_LP_IS_DIGITAL
#define dg_configEXT_LP_IS_DIGITAL                      (0)
#endif

/**
 * \brief Minimum sleep time
 *
 *  No power savings if we enter sleep when the sleep time is less than N LP cycles.
 *
 *  \note It should be ~3msec but this may vary.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configMIN_SLEEP_TIME
# if ((dg_configUSE_LP_CLK == LP_CLK_32000) || (dg_configUSE_LP_CLK == LP_CLK_32768))
#  define dg_configMIN_SLEEP_TIME                       (33*3)  // 3 msec
# elif (dg_configUSE_LP_CLK == LP_CLK_RCX)
#  define dg_configMIN_SLEEP_TIME                       cm_rcx_us_2_lpcycles_low_acc(3000)  // 3 msec
# else /* LP_CLK_ANY */
       /* Must be defined in the custom_config_<>.h file. It should be ~3msec but this may vary. */
# endif
#endif

/**
 * \}
 */

/* ----------------------------------- Image Configuration -------------------------------------- */

/**
 * \addtogroup IMAGE_CONFIGURATION_SETTINGS Image configuration settings
 *
 * \brief Image configuration settings
 *
 * \{
 */

/**
 * \brief The motherboard revision we compile for.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configBLACK_ORCA_MB_REV
#define dg_configBLACK_ORCA_MB_REV                      BLACK_ORCA_MB_REV_D
#endif

/**
 * \brief Controls how the image is built.
 *
 *  - DEVELOPMENT_MODE: various debugging options are included.
 *  - PRODUCTION_MODE: all code used for debugging is removed.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configIMAGE_SETUP
#define dg_configIMAGE_SETUP                            DEVELOPMENT_MODE
#endif

/**
 * \brief When set to 1, the delay at the start of execution of the Reset_Handler is skipped.
 *
 * \details This delay is added in order to facilitate proper programming of the Flash or QSPI\n
 *        launcher invocation. Without it, there is no guarantee that the execution of the image\n
 *        will not proceed, altering the default configuration of the system from the one that the\n
 *        bootloader leaves it.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configSKIP_MAGIC_CHECK_AT_START
#define dg_configSKIP_MAGIC_CHECK_AT_START              (0)
#endif

/**
 * \brief When set to 1, the QSPI copy will be emulated when in DEVELOPMENT_MODE (Not Applicable!).
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configEMULATE_QSPI_COPY
#define dg_configEMULATE_QSPI_COPY                      (0)
#endif

/**
 * \brief Code execution mode
 *
 *  - MODE_IS_RAM
 *  - MODE_IS_MIRRORED
 *  - MODE_IS_CACHED
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configEXEC_MODE
#define dg_configEXEC_MODE                              MODE_IS_RAM
#endif

/**
 * \brief Code location
 *
 * - NON_VOLATILE_IS_OTP
 * - NON_VOLATILE_IS_FLASH
 * - NON_VOLATILE_IS_NONE (RAM)
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configCODE_LOCATION
#define dg_configCODE_LOCATION                          NON_VOLATILE_IS_NONE
#endif

/**
 * \}
 */

/* ----------------------------------- System Configuration ------------------------------------- */

/**
 * \addtogroup SYSTEM_CONFIGURATION_SETTINGS System configuration settings
 *
 * \brief Generic System Configuration Settings
 *
 * \{
 */

/**
 * \brief Image copy time
 *
 * The number of LP cycles needed for the application's image data to be copied from the OTP
 * (or QSPI) to the RAM in mirrored mode.
 *
 * \warning MUST BE SMALLER THAN #dg_configMIN_SLEEP_TIME !!!
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#if (dg_configEXEC_MODE != MODE_IS_MIRRORED)
# undef dg_configIMAGE_COPY_TIME
# define dg_configIMAGE_COPY_TIME                       (0)
#elif !defined(dg_configIMAGE_COPY_TIME)
# if ((dg_configUSE_LP_CLK == LP_CLK_32000) || (dg_configUSE_LP_CLK == LP_CLK_32768))
#  define dg_configIMAGE_COPY_TIME                      (64)
# elif (dg_configUSE_LP_CLK == LP_CLK_RCX)
#  define dg_configIMAGE_COPY_TIME                      cm_rcx_us_2_lpcycles(1950)
# endif
#endif

/**
 * \brief Watchdog Service
 *
 * - 1: enabled
 * - 0: disabled
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUSE_WDOG
#define dg_configUSE_WDOG                               (0)
#endif

/**
 * \brief Brown-out Detection
 *
 * - 1: used
 * - 0: not used
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUSE_BOD
#define dg_configUSE_BOD                                (1)
#endif

/**
 * \brief Reset value for Watchdog.
 *
 * See WATCHDOG_REG:WDOG_VAL field.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configWDOG_RESET_VALUE
#define dg_configWDOG_RESET_VALUE                       (0xFF)
#endif

/**
 * \brief Watchdog notify interval
 *
 * Interval (in milliseconds) for common timer which can be used to trigger tasks in order to notify
 * watchdog. Can be set to 0 in order to disable timer code entirely.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configWDOG_NOTIFY_TRIGGER_TMO
#define dg_configWDOG_NOTIFY_TRIGGER_TMO                (0)
#endif

/**
 * \brief Abort a clock modification if it will cause an error to the SysTick counter
 *
 * - 1: on
 * - 0: off
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configABORT_IF_SYSTICK_CLK_ERR
#define dg_configABORT_IF_SYSTICK_CLK_ERR               (0)
#endif

/**
 * \brief Maximum adapters count
 *
 * Should be equal to the number of Adapters used by the Application. It can be larger (up to 254)
 * than needed, at the expense of increased Retention Memory requirements. It cannot be 0.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configPM_MAX_ADAPTERS_CNT
#define dg_configPM_MAX_ADAPTERS_CNT                    (16)
#endif

/**
 * \brief Maximum sleep defer time
 *
 * The maximum time sleep can be deferred via a call to pm_defer_sleep_for(). It is in clock cycles
 * in the case of the XTAL32K and in usec in the case of RCX.
 * \note  It should be > 3.5msec.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configPM_MAX_ADAPTER_DEFER_TIME
# if ((dg_configUSE_LP_CLK == LP_CLK_32000) || (dg_configUSE_LP_CLK == LP_CLK_32768))
#  define dg_configPM_MAX_ADAPTER_DEFER_TIME            (128)
# elif (dg_configUSE_LP_CLK == LP_CLK_RCX)
#  define dg_configPM_MAX_ADAPTER_DEFER_TIME            cm_rcx_us_2_lpcycles(4000)
# else /* LP_CLK_ANY */
       /* Must be defined in the custom_config_<>.h file. It should be > 3.5msec. */
# endif
#endif

/**
 * \brief When set to 1, the DCDC is used.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configUSE_DCDC
#define dg_configUSE_DCDC                               (1)
#endif

/**
 * \brief Apply ADC Gain Error correction.
 *
 * - 1: used
 * - 0: not used
 *
 * The default setting is: 1.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configUSE_ADC_GAIN_ERROR_CORRECTION
#define dg_configUSE_ADC_GAIN_ERROR_CORRECTION          (1)
#endif

/**
 * \brief Image copy time
 *
 * The number of LP cycles needed for the application's image data to be copied from the OTP
 * (or QSPI) to the RAM in mirrored mode.
 *
 * \warning MUST BE SMALLER THAN #dg_configMIN_SLEEP_TIME !!!
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#if (dg_configEXEC_MODE != MODE_IS_MIRRORED)
# undef dg_configIMAGE_COPY_TIME
# define dg_configIMAGE_COPY_TIME                       (0)
#elif !defined(dg_configIMAGE_COPY_TIME)
# if ((dg_configUSE_LP_CLK == LP_CLK_32000) || (dg_configUSE_LP_CLK == LP_CLK_32768))
#  define dg_configIMAGE_COPY_TIME                      (64)
# elif (dg_configUSE_LP_CLK == LP_CLK_RCX)
#  define dg_configIMAGE_COPY_TIME                      cm_rcx_us_2_lpcycles(1950)
# else /* LP_CLK_ANY */
       /* Must be defined in the custom_config_<>.h file. */
# endif
#endif

/*
 * \brief Trimmed Configuration Script
 *
 * Enabling this feature the system will always use the trimmed configuration values
 * stored by the TCS in OTP
 *
 * - 1: used
 * - 0: not used
 */
#ifndef dg_configUSE_SYS_TCS
#define dg_configUSE_SYS_TCS                            (1)
#endif

/**
 * \brief When set to 1, the system will go to sleep and never exit allowing for the sleep current to be
 *        measured.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configTESTMODE_MEASURE_SLEEP_CURRENT
#define dg_configTESTMODE_MEASURE_SLEEP_CURRENT         (0)
#endif

/**
 * \}
 */

/* ----------------------------------- USB Configuration ---------------------------------------- */

/**
 * \addtogroup USB_SETTINGS USB configuration settings
 *
 * \brief USB configuration settings
 *
 * \{
 */

/**
 * \brief Controls how the system will behave when the USB i/f is suspended.
 *
 * \details When the USB Node is suspended by the USB Host, the application may have to act in
 *          order to comply with the USB specification (consume less than 2.5mA). The available
 *          options are:
 *          0: do nothing
 *          1: pause system clock => the LP clock is stopped and only VBUS and USB irqs are handled
 *          2: pause application => The system is not paused but the application must stop all
 *             timers and make sure all tasks are blocked.
 *
 *          Both in modes 1 and 2, the application must make sure that all external peripherals are
 *          either powered off or placed in the lowest power consumption mode.
 */
#ifndef dg_configUSB_SUSPEND_MODE
#define dg_configUSB_SUSPEND_MODE                       USB_SUSPEND_MODE_NONE
#endif

/**
 * \brief Enable the DMA for reading/writing data to USB EP.\n
 * By default the USB DMA is not enabled.\n
 * To enable the DMA for the USB, set this the macro to value (1) in the custom_config_xxx.h file.\n
 * When the USB DMA is enabled, the default end points with DMA are EP1 and EP2. \n
 * It is possible only one TX and one RX end point to use DMA.\n
 * User can choose a different pair of end points to use the DMA as needed according to app requirements.\n
 * To change the end points using DMA, set in the the custom_config_xxx.h file the desired values for the macros:
 * \par \c dg_configUSB_TX_DMA_EP
 * valid values: 1,3,5\n
 * default value: 1
 * \par \c dg_configUSB_RX_DMA_EP
 * valid values: 2,4,6\n
 * default value: 2
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUSB_DMA_SUPPORT
#define dg_configUSB_DMA_SUPPORT                        (0)
#endif


/**
 * \brief The USB TX end point (D-->H) to enable the DMA.\n
 * User can choose a different pair of end points to use the DMA as needed according to app requirements.\n
 * To change the TX end point using DMA, set in the the custom_config_xxx.h file the desired value for this macros.
 * \par \c dg_configUSB_TX_DMA_EP
 * valid values: 1,3,5\n
 * default value: 1
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUSB_TX_DMA_EP
#define dg_configUSB_TX_DMA_EP                          (1)
#endif

/**
 * \brief The USB RX end point (D-->H) to enable the DMA.\n
 * User can choose a different pair of end points to use the DMA as needed according to app requirements.\n
 * To change the RX end point using DMA, set in the the custom_config_xxx.h file the desired value for this macros.
 * \par \c dg_configUSB_RX_DMA_EP
 * valid values: 2,4,6\n
 * default value: 2
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUSB_RX_DMA_EP
#define dg_configUSB_RX_DMA_EP                          (2)
#endif

/**
 * \}
 */

/* ----------------------------------- Flash Configuration -------------------------------------- */

/**
 * \addtogroup FLASH_SETTINGS Flash configuration settings
 *
 * \brief Flash configuration settings
 *
 * \{
 */

/**
 * \brief The rail from which the Flash is powered, if a Flash is used.
 *
 * - FLASH_IS_NOT_CONNECTED
 * - FLASH_CONNECTED_TO_1V8
 * - FLASH_CONNECTED_TO_1V8P
 * - FLASH_CONNECTED_TO_1V8F
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configFLASH_CONNECTED_TO
#error "dg_configFLASH_CONNECTED_TO is not defined!"
#endif

/**
 * \brief When set to 1, the QSPI FLASH is put to power-down state during sleep.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configFLASH_POWER_DOWN
#define dg_configFLASH_POWER_DOWN                       (0)
#endif

/* Backward compatibility check */

#if defined(dg_configPOWER_FLASH) || defined(dg_configFLASH_POWER_OFF)
#define PRINT_POWER_RAIL_SETUP
#endif

/**
 * \brief Enable the Flash Auto-detection mode for QSPIC
 *
 * \warning THIS WILL GREATLY INCREASE THE CODE SIZE AND RETRAM USAGE!!! MAKE SURE YOUR PROJECT
 *          CAN SUPPORT THIS.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configFLASH_AUTODETECT
#define dg_configFLASH_AUTODETECT                       (0)
#endif

/**
 * \brief Offset of the image if not placed at the beginning of QSPI Flash.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configIMAGE_FLASH_OFFSET
#define dg_configIMAGE_FLASH_OFFSET                     (0)
#endif

/**
 * \brief Set the flash page size.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configFLASH_MAX_WRITE_SIZE
#define dg_configFLASH_MAX_WRITE_SIZE                   (128)
#endif

/**
 * \brief Disable background operations.
 *
 * When enabled, outstanding QSPI operations will take place during sleep time
 * increasing the efficiency.
 *
 * - 1 : Disabled
 * - 0 : Enabled
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configDISABLE_BACKGROUND_FLASH_OPS
#define dg_configDISABLE_BACKGROUND_FLASH_OPS           (0)
#endif

/**
 * \}
 */

/* ----------------------------------- Cache Configuration -------------------------------------- */

/**
 * \addtogroup CACHE_SETTINGS Cache configuration settings
 *
 * \brief Cache configuration settings
 *
 * \{
 */

/**
 * \brief Set the size (in bytes) of the QSPI flash cacheable area.
 *
 * All reads from offset 0 up to (not including) offset dg_configCACHEABLE_QSPI_AREA_LEN
 * will be cached. In addition, any writes to this area will trigger cache flushing, to
 * avoid any cache incoherence.
 *
 * The size must be 64KB-aligned, due to the granularity of CACHE_CTRL2_REG[CACHE_LEN].
 *
 * Special values:
 *  *  0 : Turn off cache.
 *  * -1 : Don't configure cacheable area size (i.e. leave as set by booter).
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configCACHEABLE_QSPI_AREA_LEN
#define dg_configCACHEABLE_QSPI_AREA_LEN                (-1)
#endif

/**
 * \brief Set the associativity of the cache.
 *
 * Available values:
 *  0   /// direct-mapped
 *  1   /// 2-way set associative
 *  2   /// 4-way set associative
 *  3   /// leave as set by the ROM booter
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configCACHE_ASSOCIATIVITY
#define dg_configCACHE_ASSOCIATIVITY                    (2)
#endif

/**
 * \brief Set the line size of the cache.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 *
 * Available values:
 *  0   /// 8 bytes
 *  1   /// 16 bytes
 *  2   /// 32 byte
 *  3   /// leave as set by the ROM booter
 */
#ifndef dg_configCACHE_LINESZ
#define dg_configCACHE_LINESZ                           (0)
#endif

/**
 * \}
 */

/* ----------------------------------- UART settings -------------------------------------------- */

/**
 * \addtogroup UART_SETTINGS UART configuration settings
 *
 * \brief UART configuration settings
 *
 * \{
 */

/**
 * \brief Software FIFO support
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUART_SOFTWARE_FIFO
#define dg_configUART_SOFTWARE_FIFO                     (0)
#endif

/**
 * \brief UART1's software FIFO size
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUART1_SOFTWARE_FIFO_SIZE
#define dg_configUART1_SOFTWARE_FIFO_SIZE               (0)
#endif

/**
 * \brief UART2's software FIFO size
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUART2_SOFTWARE_FIFO_SIZE
#define dg_configUART2_SOFTWARE_FIFO_SIZE               (0)
#endif

/**
 * \brief Circular DMA support for RX
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUART_RX_CIRCULAR_DMA
#define dg_configUART_RX_CIRCULAR_DMA                   (0)
#endif

/**
 * \brief UART1's Circular DMA buffer size for RX
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUART1_RX_CIRCULAR_DMA_BUF_SIZE
#define dg_configUART1_RX_CIRCULAR_DMA_BUF_SIZE         (0)
#endif

/**
 * \brief UART2's Circular DMA buffer size for RX
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUART2_RX_CIRCULAR_DMA_BUF_SIZE
#define dg_configUART2_RX_CIRCULAR_DMA_BUF_SIZE         (0)
#endif

/**
 * \}
 */

/* ----------------------------------- RF Configuration ----------------------------------------- */

/**
 * \addtogroup RF_DRIVER_SETTINGS Radio Driver Settings
 *
 * \brief Doxygen documentation is not yet available for this module.
 *        Please check the source code file(s)
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 * \{
 */

/**
 * \brief Set to 1 to enable the recalibration procedure.
 */
#if defined(CONFIG_USE_BLE)
# ifndef dg_configRF_ENABLE_RECALIBRATION
#  define dg_configRF_ENABLE_RECALIBRATION              (1)
# endif
#endif

/**
 * \}
 */

/* ----------------------------------- DEVICE-SPECIFIC CONFIGURATION ---------------------------- */


#include "bsp_defaults_da1469x.h"


/* ----------------------------------- DEBUG CONFIGURATION -------------------------------------- */

#include "bsp_debug.h"

/* ----------------------------------- MEMORY LAYOUT CONFIGURATION ------------------------------ */

#include "bsp_memory_defaults.h"


#endif /* BSP_DEFAULTS_H_ */

/**
\}
\}
*/
