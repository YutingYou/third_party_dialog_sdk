/**
 * \addtogroup MIDDLEWARE
 * \{
 * \addtogroup MIDDLEWARE_CONFIG_DEFAULTS
 *
 * \brief Middleware default configuration values
 *
 * The following tags are used to describe the type of each configuration option.
 *
 * - **\bsp_config_option_build**        : To be changed only in the build configuration
 *                                                of the project ("Defined symbols -D" in the
 *                                                preprocessor options).
 *
 * - **\bsp_config_option_app**          : To be changed only in the custom_config*.h
 *                                                project files.
 *
 * - **\bsp_config_option_expert_only**  : To be changed only by an expert user.
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file middleware_defaults.h
 *
 * @brief Middleware. System Configuration file default values.
 *
 * Copyright (C) 2018-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef MIDDLEWARE_DEFAULTS_H_
#define MIDDLEWARE_DEFAULTS_H_

/**
 * \addtogroup ADAPTER_SELECTION Adapters enabled by default
 *
 * \brief Adapter selection
 *
 * When enabled the specific adapter is included in the compilation of the SDK.
 * - 0 : Disabled
 * - 1 : Enabled
 *
 * The default option can be overridden in the application configuration file.
 *
 * \{
   Adapter                        | Setting                                | Default option
   ------------------------------ | -------------------------------------- | :------------------:
   Table not yet fixed | dg_configXXXXX_ADAPTER                   | 1
 *
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */


/* -------------------------------- Adapters (ad_*) selection -------------------------------- */

#ifndef dg_configFLASH_ADAPTER
#define dg_configFLASH_ADAPTER                  (1)
#endif

#ifndef dg_configI2C_ADAPTER
#define dg_configI2C_ADAPTER                    (0)
#endif

#ifndef dg_configNVMS_ADAPTER
#define dg_configNVMS_ADAPTER                   (1)
#endif

#ifndef dg_configNVMS_FLASH_CACHE
#define dg_configNVMS_FLASH_CACHE               (0)
#endif

#ifndef dg_configNVMS_VES
#define dg_configNVMS_VES                       (1)
#endif

#ifndef dg_configSPI_ADAPTER
#define dg_configSPI_ADAPTER                    (0)
#endif

#ifndef dg_configUART_ADAPTER
#define dg_configUART_ADAPTER                   (0)
#endif

#ifndef dg_configGPADC_ADAPTER
#define dg_configGPADC_ADAPTER                  (0)
#endif

#ifndef dg_configSDADC_ADAPTER
#define dg_configSDADC_ADAPTER                  (0)
#endif

#ifdef dg_configTEMPSENS_ADAPTER
#error "Configuration option dg_configTEMPSENS_ADAPTER  is no longer supported"
#endif

#ifdef dg_configBATTERY_ADAPTER
#error "Configuration option dg_configBATTERY_ADAPTER  is no longer supported"
#endif

#ifndef dg_configNVPARAM_ADAPTER
#define dg_configNVPARAM_ADAPTER                (0)
#endif

#ifndef dg_configNVPARAM_APP_AREA
#define dg_configNVPARAM_APP_AREA               (0)
#endif

#ifndef dg_configCRYPTO_ADAPTER
#define dg_configCRYPTO_ADAPTER                 (1)
#endif

#ifndef dg_configKEYBOARD_SCANNER_ADAPTER
#define dg_configKEYBOARD_SCANNER_ADAPTER       (0)
#endif

#ifndef dg_configSYS_AUDIO_MGR
#define dg_configSYS_AUDIO_MGR                  (0)
#endif


#ifndef dg_configSNC_ADAPTER
#define dg_configSNC_ADAPTER                    (0)
#endif

#ifndef dg_configISO7816_ADAPTER
#define dg_configISO7816_ADAPTER                (0)
#endif

#ifndef dg_configLCDC_ADAPTER
#define dg_configLCDC_ADAPTER                   (0)
#endif

#ifndef dg_configPMU_ADAPTER
#define dg_configPMU_ADAPTER                    (1)
#endif

#ifndef dg_configHAPTIC_ADAPTER
#define dg_configHAPTIC_ADAPTER                 (0)
#elif dg_configHAPTIC_ADAPTER
#if (!(dg_configUSE_HW_ERM || dg_configUSE_HW_LRA))
#error "Configuration option dg_configHAPTIC_ADAPTER also requires either the dg_configUSE_HW_LRA or the dg_configUSE_HW_ERM configuration options to be set."
#endif
#endif

#ifdef dg_configRF_ADAPTER
# error "dg_configRF_ADAPTER is not supported in DA14690 devices"
#endif


/* ---------------------------------------------------------------------------------------------- */

/**
 * \}
 */


/**
 * \addtogroup CONSOLE_IO_SETTINGS Console I/O Settings
 *
 * \brief Console IO configuration settings
 *
 * \{
   Description                               | Setting                    | Default option
   ----------------------------------------- | -------------------------- | :---------------:
   Enable serial console module              | dg_configUSE_CONSOLE       | 0
   Enable serial console stubbed API         | dg_configUSE_CONSOLE_STUBS | 0
   Enable Command Line Interface module      | dg_configUSE_CLI           | 0
   Enable Command Line Interface stubbed API | dg_configUSE_CLI_STUBS     | 0

   \see console.h cli.h

   \note CLI module requires dg_configUSE_CONSOLE to be enabled.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */

/* -------------------------------------- Console IO configuration settings --------------------- */

#ifndef dg_configUSE_CONSOLE
#define dg_configUSE_CONSOLE                    (0)
#endif

#ifndef dg_configUSE_CONSOLE_STUBS
#define dg_configUSE_CONSOLE_STUBS              (0)
#endif

#ifndef dg_configUSE_CLI
#define dg_configUSE_CLI                        (0)
#endif

#ifndef dg_configUSE_CLI_STUBS
#define dg_configUSE_CLI_STUBS                  (0)
#endif
/* ---------------------------------------------------------------------------------------------- */

/**
 * \}
 */

/* ----------------------------- DGTL ----------------------------------------------------------- */

/**
 * \brief Enable D.GTL interface
 *
 * When this macro is enabled, the DGTL framework is available for use.
 * The framework must furthermore be initialized in the application using
 * dgtl_init(). Additionally, the UART adapter must be initialized accordingly.
 *
 * Please see sdk/middleware/dgtl/include/ for further DGTL configuration
 * (in dgtl_config.h) and API.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 *
 */
#ifndef dg_configUSE_DGTL
#define dg_configUSE_DGTL                       (0)
#endif

/**
 * \addtogroup MIDDLEWARE_DEBUG_SETTINGS Debug Settings
 *
 * \{
 */
/* -------------------------------------- Debug settings ---------------------------------------- */

/**
 * \brief Enable task monitoring.
 *
 * \note Task monitoring can only be enabled if RTT or RETARGET is enabled
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configENABLE_TASK_MONITORING
#define dg_configENABLE_TASK_MONITORING         (0)
#endif

/**
 * \brief Enable Micro Trace Buffer
 *
 * \note MTB is available only on Cortex-M33.
 * \bsp_default_note{\bsp_config_option_app,}
 *
 */
#ifndef dg_configENABLE_MTB
#define dg_configENABLE_MTB                     (0)
#endif


/* ---------------------------------------------------------------------------------------------- */

/* ---------------------------------- OS related configuration ---------------------------------- */

/**
 * \brief Monitor OS heap allocations
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configTRACK_OS_HEAP
#define dg_configTRACK_OS_HEAP                  (0)
#endif

/* ---------------------------------------------------------------------------------------------- */

/**
 * \}
 */

/* ---------------------------------- SYSTEM CONFIGURATION ------------------------------------ */
/**
 * \brief Enable gpadc monitoring.
 *
 * \note The application must not explicitly set dg_configUSE_SYS_ADC to 1.\n
 *       Use instead dg_configRF_ENABLE_RECALIBRATION\n
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */

#ifndef dg_configUSE_SYS_ADC
#define dg_configUSE_SYS_ADC                    (0)
#endif

/**
 * \brief When set to 1, the sys charger service is used to charge the battery.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configUSE_SYS_CHARGER
#define dg_configUSE_SYS_CHARGER                (0)
#endif


/**
 * \brief When set to 1, the sys usb service is used to manage:
 *        - VBUS attach / detach and  USB suspend / resume operations.
 *        - Notifications towards SDK and applications.
 *        - Suspend / resume sleep.
 *        - Suspend / resume DC/DC if in use.
 *
 * \note The service is automatically enabled when charging or USB enumeration
 *       are involved (see dg_configUSE_SYS_CHARGER , dg_configUSE_USB_ENUMERATION).
 *       It's recommended to be enabled when:
 *       - The power supply is a non rechargeable battery and the application is not
 *         interested in USB enumeration.
 *       - The only source of power supply is VBUS e.g a USB dongle.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configUSE_SYS_USB
#define dg_configUSE_SYS_USB                    (0)
#endif

/* ---------------------------------------------------------------------------------------------- */

/* ----------------------------------- Driver dependencies -------------------------------------- */

/* If USB charger is enebled, we need to enable GPADC and Battery adapters as well */
#if (dg_configUSE_USB_CHARGER == 1)
#undef dg_configGPADC_ADAPTER
#define dg_configGPADC_ADAPTER                  (1)
#endif

/*
 * SYS_CHARGER will alse enable SYS_USB which implements the USB and VBUS interrupt handlers
 */
#if (dg_configUSE_SYS_CHARGER == 1)
#undef dg_configUSE_HW_USB_CHARGER
#define dg_configUSE_HW_USB_CHARGER             (1)
#undef dg_configUSE_HW_CHARGER
#define dg_configUSE_HW_CHARGER                 (1)
#undef dg_configUSE_HW_USB
#define dg_configUSE_HW_USB                     (1)
#undef dg_configUSE_SYS_USB
#define dg_configUSE_SYS_USB                    (1)
#endif

/*
 * For USB_ENUMERATION to work, SYS_USB is also enabled, and library usb_lib needs to be linked
 * with the active project
 */
#if (dg_configUSE_USB_ENUMERATION == 1)
  #undef dg_configUSE_SYS_USB
  #define dg_configUSE_SYS_USB                  (1)
  #ifndef dg_configUSB_SUSPEND_MODE
    #define dg_configUSB_SUSPEND_MODE               USB_SUSPEND_MODE_NONE
  #elif (dg_configUSB_SUSPEND_MODE == USB_SUSPEND_MODE_PAUSE)
      #error "USB_SUSPEND_MODE_PAUSE is currently not supported"
  #endif
#endif

/*
 * SYS_USB can be explicitly enabled, even if dg_configUSE_SYS_CHARGER and dg_configUSE_USB_ENUMERATION
 * are not used. This will enable the VBUS and USB interrupt handlers, but no functionality will be
 * attached to them
 */
#if (dg_configUSE_SYS_USB == 1)
#undef dg_configUSE_HW_USB
#define dg_configUSE_HW_USB                     (1)
#endif

/* If RF recalibration is enabled or RCX is used as low power clock, we need to enable SNC and GPADC as well */
#if (dg_configRF_ENABLE_RECALIBRATION || (dg_configUSE_LP_CLK == LP_CLK_RCX && defined(OS_FREERTOS)))
# undef dg_configUSE_HW_GPADC
# define dg_configUSE_HW_GPADC                  (1)
# undef dg_configGPADC_ADAPTER
# define dg_configGPADC_ADAPTER                 (1)
#  if (dg_configRF_ENABLE_RECALIBRATION)
#   undef dg_configUSE_SYS_ADC
#   define dg_configUSE_SYS_ADC                 (1)
#  endif
#  undef dg_configUSE_SNC_HW_GPADC
#  define dg_configUSE_SNC_HW_GPADC             (1)
#  undef dg_configUSE_HW_SENSOR_NODE
#  define dg_configUSE_HW_SENSOR_NODE           (1)
#  undef dg_configSNC_ADAPTER
#  define dg_configSNC_ADAPTER                  (1)
#  undef dg_configUSE_STATIC_IO_CONFIG
#  define dg_configUSE_STATIC_IO_CONFIG         (1)
#endif

/**
 *
 * \brief RTC-to-PDC event period, in 10ms
 *
 * When RCX is set as the low power clock or RF calibration is enabled and Real Time Clock is used
 * dg_configRTC_PDC_EVENT_PERIOD defines the RTC-to-PDC event period that will be used for RTC event controller
 * configuration.
 *
 * \bsp_default_note{\bsp_config_option_expert_only,}
 *
 */
#if (dg_configRF_ENABLE_RECALIBRATION || (dg_configUSE_LP_CLK == LP_CLK_RCX && defined(OS_FREERTOS)))
#ifndef dg_configRTC_PDC_EVENT_PERIOD
#define dg_configRTC_PDC_EVENT_PERIOD   (100)   // in 10ms
#endif
#else
#ifdef dg_configRTC_PDC_EVENT_PERIOD
#undef dg_configRTC_PDC_EVENT_PERIOD
#endif
#endif

/**
 *
 * \brief RCX calibration polling interval counter
 *
 * dg_configRCX_CAL_POLL_INT defines the number of RTC-to-PDC events that SYS_RCX ucode should wait before executing its code.
 *
 * \bsp_default_note{\bsp_config_option_expert_only,}
 *
 */
#if (dg_configUSE_LP_CLK == LP_CLK_RCX && defined(OS_FREERTOS))
#ifndef dg_configRCX_CAL_POLL_INT
#define dg_configRCX_CAL_POLL_INT       (1)     // in dg_configRTC_PDC_EVENT_PERIOD * 10ms - calibration interval = 1sec
#endif
#endif

/**
 *
 * \brief RF calibration polling interval counter
 *
 * dg_configRF_CAL_POLL_INT defines the number of RTC-to-PDC events that SYS_ADC ucode should wait before executing its code
 * when RF calibration is enabled.
 *
 * \bsp_default_note{\bsp_config_option_expert_only,}
 *
 */
#if (dg_configRF_ENABLE_RECALIBRATION)
#ifndef dg_configRF_CAL_POLL_INT
#define dg_configRF_CAL_POLL_INT        (1)     // in dg_configRTC_PDC_EVENT_PERIOD * 10ms - calibration interval = 1sec
#endif
#endif

/* If RF is enabled, we need to enable GPADC adapter as well */
#if  dg_configRF_ADAPTER
#undef dg_configGPADC_ADAPTER
#define dg_configGPADC_ADAPTER                  (1)
#endif

/*
 * \brief Enable the RTC correction mechanism
 *
 * When RCX is set as the low power clock and Real Time Clock is used (i.e. dg_configUSE_HW_RTC is defined),
 * setting this macro to 1 enables the RTC correction mechanism.
 */
#if (dg_configUSE_LP_CLK == LP_CLK_RCX && defined(OS_FREERTOS) && dg_configUSE_HW_RTC)
#ifndef dg_configRTC_CORRECTION
#define dg_configRTC_CORRECTION (1)
#endif
#else
#if dg_configRTC_CORRECTION
# pragma message "dg_configRTC_CORRECTION is only used in RTOS based projects when RCX is set as low power clock. Forcing to 0."
#undef dg_configRTC_CORRECTION
#define dg_configRTC_CORRECTION (0)
#endif
#endif
/* ---------------------------------------------------------------------------------------------- */

#endif /* MIDDLEWARE_DEFAULTS_H_ */

/**
\}
\}
*/
