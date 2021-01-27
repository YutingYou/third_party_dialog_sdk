/**
 ****************************************************************************************
 *
 * @file qspi_internal.h
 *
 * @brief Access QSPI device when running in auto mode
 *
 * Copyright (C) 2018-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef QSPI_INTERNAL_H_
#define QSPI_INTERNAL_H_

#include "qspi_common.h"

#define FLASH1_AUTODETECT                               (dg_configUSE_HW_QSPI == 1 && dg_configFLASH_AUTODETECT == 1)
# define QSPIC2_DEV_AUTODETECT                          (dg_configUSE_HW_QSPI2 == 1 && dg_configQSPIC2_DEV_AUTODETECT == 1)
#define FLASH_AUTODETECT                                (FLASH1_AUTODETECT || QSPIC2_DEV_AUTODETECT)

/**
 * \brief Define the number of QSPI controllers that can be used in the system
 *
 * Value of 1 means that only first QSPIC can be used
 * Value of 2 means that both QSPIC and QSPIC2 can be used
 */
#define QSPI_CONTROLLER_SUPPORT_NUM                     (1 + dg_configUSE_HW_QSPI2)

/**
 * \brief Reset a QSPI device
 *
 * \param[in] id QSPI controller id
 *
 */
__RETAINED_CODE void qspi_int_reset_device(HW_QSPIC_ID id);

/**
 * \brief Configure a QSPI controller
 *
 * \details This function will set the read instruction, the extra byte to use with it,
 *      the address size and the number of clocks that CS stays high. For Flash devices,
 *      it also sets the erase, suspend/resume, write enable and read status instructions
 *      and it enables the burst break sequence. For RAM devices on the 2nd controller,
 *      it also sets the write instruction and the CS mode and it enables the SRAM mode.
 *
 * \param[in] id QSPI controller id
 *
 */
__RETAINED_CODE void qspi_int_configure(HW_QSPIC_ID id);

/**
 * \brief Activate Flash command entry mode
 *
 * \param[in] id QSPI controller id
 *
 * \note After the call to this function, the QSPI controller is set to manual mode and the Flash
 *        access to single mode.
 *
 * \warning The function must be called with interrupts disabled.
 *
 */
__RETAINED_CODE void qspi_int_activate_command_entry_mode(HW_QSPIC_ID id);

/**
 * \brief Deactivate Flash command entry mode
 *
 * \param[in] id QSPI controller id
 *
 * \note After the call to this function, the QSPI controller is set to auto mode and the Flash
 *        access to quad mode (if QUAD_MODE is 1).
 *
 * \warning The function must be called with interrupts disabled.
 *
 */
__RETAINED_CODE void qspi_int_deactivate_command_entry_mode(HW_QSPIC_ID id);

#if (dg_configDISABLE_BACKGROUND_FLASH_OPS == 0)
/**
 * \brief Erase a sector of the Flash in manual mode
 *
 * \param[in] addr The address of the sector to be erased.
 *
 * \warning This function does not block until the Flash has processed the command! The QSPI
 *        controller is left to manual mode after the call to this function. The function must be
 *        called with interrupts disabled.
 *
 */
__RETAINED_CODE void qspi_int_erase_sector_manual_mode(uint32_t addr);

/**
 * \brief Program data into a page of the Flash in manual mode
 *
 * \param[in] addr The address of the Flash where the data will be written. It may be anywhere in a
 *        page.
 * \param[in] buf Pointer to the beginning of the buffer that contains the data to be written.
 * \param[in] len The number of bytes to be written.
 *
 * \return The number of bytes written.
 *
 * \warning The boundary of the page where addr belongs to, will not be crossed! The caller should
 *        issue another qspi_int_program_page_manual_mode() call in order to write the remaining data
 *        to the next page. The QSPI controller is left to manual mode after the call to this
 *        function. The function must be called with interrupts disabled.
 *
 */
__RETAINED_CODE uint32_t qspi_int_program_page_manual_mode(uint32_t addr, const uint8_t *buf,
                                                      uint32_t len);
#endif

#endif /* QSPI_INTERNAL_H_ */
