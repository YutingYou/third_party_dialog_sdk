/**
 * \addtogroup PLA_BSP_CONFIG
 * \{
 * \addtogroup BSP_MEMORY_DEFAULTS Memory Default Configuration Values
 *
 * \brief BSP memory default configuration values
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
 * @file bsp_memory_defaults.h
 *
 * @brief Board Support Package. Memory Configuration file default values.
 *
 * Copyright (C) 2018-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BSP_MEMORY_DEFAULTS_H_
#define BSP_MEMORY_DEFAULTS_H_

#define PARTITION2(...)
#include "partition_table.h"
#undef PARTITION2

/* ---------------------------------- Heap size configuration ----------------------------------- */

/**
 * \brief Heap size for used libc malloc()
 *
 * Specifies the amount of RAM that will be used as heap for libc malloc() function.
 * It can be configured in bare metal projects to match application's requirements.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef __HEAP_SIZE
# if defined(CONFIG_RETARGET) || defined(CONFIG_RTT)
#  define __HEAP_SIZE                                   0x0600
# else
#  define __HEAP_SIZE                                   0x0100
# endif
#endif


/* ---------------------------------------------------------------------------------------------- */

/* --------------------------------- Stack size configuration ----------------------------------- */

/**
 * \brief Stack size for main() function and interrupt handlers.
 *
 * Specifies the amount of RAM that will be used as stack for the main() function and the interrupt
 * handlers.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef __STACK_SIZE
#define __STACK_SIZE                                    0x0200
#endif

/* ---------------------------------------------------------------------------------------------- */

/*************************************************************************************************\
 * Default configuration for retention RAM
 */
#if !defined(RELEASE_BUILD) && (dg_configOPTIMAL_RETRAM == 1)
/* WARNING: retRAM optimizations are disabled in DEBUG builds! */
# undef dg_configOPTIMAL_RETRAM
# define dg_configOPTIMAL_RETRAM                        (0)
#elif (dg_configEXEC_MODE != MODE_IS_CACHED)
/* WARNING: retRAM optimizations are not applicable in MIRRORED mode! */
# undef dg_configOPTIMAL_RETRAM
# define dg_configOPTIMAL_RETRAM                        (0)
#endif

/* -------------------------- INCLUDE MEMORY LAYOUT CONFIGURATION ------------------------------- */

#include "bsp_memory_defaults_da1469x.h"

#endif /* BSP_MEMORY_DEFAULTS_H_ */
/**
 * \}
 *
 * \}
 */
