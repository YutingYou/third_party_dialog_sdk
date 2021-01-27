/**
 * \addtogroup BSP_MEMORY_DEFAULTS
 * \{
 *
 * \addtogroup MEMORY_LAYOUT_SETTINGS Memory Layout Configuration Settings
 *
 * \brief Memory Layout Configuration Settings
 * \{
 *
 */
/**
 ****************************************************************************************
 *
 * @file bsp_memory_defaults_da1469x.h
 *
 * @brief Board Support Package. Device-specific system configuration default values.
 *
 * Copyright (C) 2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifndef BSP_MEMORY_DEFAULTS_DA1469X_H_
#define BSP_MEMORY_DEFAULTS_DA1469X_H_


#ifdef dg_config_RETAINED_UNINIT_SECTION_SIZE
#error "dg_config_RETAINED_UNINIT_SECTION_SIZE is deprecated! "\
       "Please use dg_configRETAINED_UNINIT_SECTION_SIZE instead (no underscore)!"
#endif

/**
 * \brief Size of the RETAINED_RAM_UNINIT section, in bytes.
 *
 * This section is not initialized during startup by either the bootloader or
 * the application. It can be therefore used to maintain debug or other relevant
 * information that will no be lost after reset. It should be guaranteed that
 * both the bootloader (if any) and the application are using the same value for
 * this option (or otherwise the booloader can corrupt the contents of the section).
 * To use this section for a specific variable, use the __RETAINED_UNINIT attribute.
 */
#ifndef dg_configRETAINED_UNINIT_SECTION_SIZE
#define dg_configRETAINED_UNINIT_SECTION_SIZE           (128)
#endif

/**
 * \brief Code size in QSPI projects for DA1469x.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configQSPI_CODE_SIZE_AA
#define dg_configQSPI_CODE_SIZE_AA                      (370 * 1024) /* Take into account CMI firmware size */
#endif

/**
 * \brief Maximum size (in bytes) of image in the QSPI flash.
 *
 * The image in the QSPI flash contains the text (code + const data) and any other initialized data.
 *
 * \note This size should not be larger than the flash partition where the image is stored.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configQSPI_MAX_IMAGE_SIZE
#define dg_configQSPI_MAX_IMAGE_SIZE                    ( IMAGE_PARTITION_SIZE )
#endif

#if dg_configQSPI_MAX_IMAGE_SIZE < dg_configQSPI_CODE_SIZE_AA
#error "dg_configQSPI_MAX_IMAGE_SIZE cannot be smaller than dg_configQSPI_CODE_SIZE_AA"
#endif

/**
 * \brief RAM-block size in cached mode for DA1469x.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configQSPI_CACHED_RAM_SIZE_AA
# ifdef dg_configENABLE_MTB
/* reserve last 8KiB of RAM for MTB */
#  define dg_configQSPI_CACHED_RAM_SIZE_AA              (504 * 1024)
# else
#  define dg_configQSPI_CACHED_RAM_SIZE_AA              (512 * 1024)
# endif
#endif

/**
 * \brief Code and RAM size in RAM projects for DA1469xAA.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configRAM_CODE_SIZE_AA
/* CODE and RAM are merged into a single RAM section */
#define dg_configRAM_CODE_SIZE_AA                       (512 * 1024)
#endif /* dg_configRAM_CODE_SIZE_AA */

#if (dg_configOPTIMAL_RETRAM == 0)
# undef  dg_configMEM_RETENTION_MODE
# define dg_configMEM_RETENTION_MODE                    (0)
#endif

/**
 * \brief Retention memory configuration.
 *
 * 16 bits field; each couple of bits controls whether the relevant memory block will be retained (0) or not (1).
 * -  bits 0-1   : SYSRAM1
 * -  bits 2-3   : SYSRAM2
 * -  bits 4-5   : SYSRAM3
 * -  bits 6-7   : SYSRAM4
 * -  bits 8-9   : SYSRAM5
 * -  bits 9-10  : SYSRAM6
 * -  bits 11-12 : SYSRAM7
 * -  bits 13-14 : SYSRAM8
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 *
 */
#ifndef dg_configMEM_RETENTION_MODE
#define dg_configMEM_RETENTION_MODE                     (0)
#endif

#if (dg_configCODE_LOCATION == NON_VOLATILE_IS_FLASH)
# if (dg_configEXEC_MODE == MODE_IS_CACHED)
#  define CODE_SIZE                                     dg_configQSPI_CODE_SIZE_AA
#  define RAM_SIZE                                      dg_configQSPI_CACHED_RAM_SIZE_AA
# else // MIRRORED
#  error "QSPI mirrored mode is not supported!"
# endif
#elif (dg_configCODE_LOCATION == NON_VOLATILE_IS_NONE)
# if (dg_configEXEC_MODE == MODE_IS_CACHED)
#  pragma message "RAM cached mode is not supported! Resetting to RAM (mirrored) mode!"
#  undef dg_configEXEC_MODE
#  define dg_configEXEC_MODE                            MODE_IS_RAM
# endif
# define CODE_SIZE                                      dg_configRAM_CODE_SIZE_AA
# if (CODE_SZ > 512)
#  error "The used CODE_SZ value exceed the total amount of RAM!"
# endif
#else
# error "Unknown configuration..."
#endif /* dg_configCODE_LOCATION */


#endif /* BSP_MEMORY_DEFAULTS_DA1469X_H_ */

/**
 * \}
 * \}
 */
