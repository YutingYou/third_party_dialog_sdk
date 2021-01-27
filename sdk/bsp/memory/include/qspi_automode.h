/**
 * \addtogroup PLA_BSP_SYSTEM
 * \{
 * \addtogroup PLA_MEMORY
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file qspi_automode.h
 *
 * @brief Access QSPI device when running in auto mode
 *
 * Copyright (C) 2016-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef QSPI_AUTOMODE_H_
#define QSPI_AUTOMODE_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sdk_defs.h>
#include "hw_qspi.h"
#include "hw_clk.h"
/*
 * Debug options
 */
#if __DBG_QSPI_ENABLED
#define __DBG_QSPI_VOLATILE__           volatile
#pragma message "Automode: Debugging is on!"
#else
#define __DBG_QSPI_VOLATILE__
#endif

/*
 * Defines (generic)
 */
#define FLASH_SECTOR_SIZE       (0x1000)

/* Macros to put functions that need to be copied to ram in one section (retained) */
typedef struct qspi_ucode_s {
       const uint32_t *code;
       uint8_t size;
} qspi_ucode_t;


/*
 * Flash specific defines
 */

/**
 * \brief Delay after SUSPEND command (in μsec)
 *
 */
#ifndef FLASH_SUS_DELAY
#define FLASH_SUS_DELAY                 (20)
#endif

/**
 * \brief Delay after RESUME command (in μsec)
 *
 */
#ifndef FLASH_RES_DELAY
#define FLASH_RES_DELAY                 (20)
#endif

/**
 * \brief Delay after RESET command (in μsec)
 *
 */
#ifndef FLASH_RST_DELAY
#define FLASH_RST_DELAY                 (12000)
#endif

/**
 * \brief Initialize background Flash operations
 */
void qspi_operations_init(void);

/**
 * \brief Execute any pending QSPI program / erase operations
 *
 * \return true if write operation has been completed
 */
__RETAINED_CODE bool qspi_process_operations(void);

/**
 * \brief Suspend a Flash program or erase operation
 *
 * \details This function will try to suspend an ongoing program or erase procedure. Note that the
 *        program or erase procedure may have been completed before the suspend command is processed
 *        by the Flash. In this case the SUS bit will be left to 0.
 *
 * \warning After the call to this function, the QSPI controller is set to auto mode and the Flash
 *        access to quad mode (if QUAD_MODE is 1). The function must be called with interrupts
 *        disabled.
 */
__RETAINED_CODE void qspi_check_and_suspend_operations(void);

/**
 * \brief Notify tasks waiting for Flash operations that they have been completed
 *
 */
__RETAINED_CODE  void qspi_process_completed_operations(void);

/**
 * \brief Check if there is a pending background Flash operation
 *
 * \return true if a background flash operation is pending, otherwise false
 */
bool qspi_is_op_pending(void);

/**
 * \brief Write flash memory
 *
 * This function allows to write up to page size of data to flash.
 * If size is greater than page size, flash can wrap data and overwrite content of page.
 * It's possible to write less then page size.
 * Memory should be erased before.
 *
 * \note: Do not pass buf pointing to QSPI mapped memory.
 *
 * \param [in] addr offset in flash to write data to
 * \param [in] buf pointer to data to write
 * \param [in] size number of bytes to write
 *
 * return number of bytes written
 *
 */
uint32_t qspi_automode_write_flash_page(uint32_t addr, const uint8_t *buf, uint32_t size);

/**
 * \brief Erase flash sector
 *
 * \param [in] addr starting offset of sector
 */
void qspi_automode_erase_flash_sector(uint32_t addr);

/**
 * \brief Erase whole chip
 */
void qspi_automode_erase_chip(void);

/**
 * \brief Read memory
 *
 * \param [in] addr starting offset
 * \param [out] buf buffer to read data to
 * \param [in] len number of bytes to read
 *
 * \returns number of bytes read
 */
uint32_t qspi_automode_read(uint32_t addr, uint8_t *buf, uint32_t len);

/**
 * \brief Get address of memory
 *
 * \param [in] addr starting offset
 *
 * \returns address in CPU address space where data is located
 */
const void *qspi_automode_addr(uint32_t addr);

/**
 * \brief Power up flash
 */
__RETAINED_CODE void qspi_automode_flash_power_up(void);

/**
 * \brief Set QSPI Flash into power down mode
 */
__RETAINED_CODE void qspi_automode_flash_power_down(void);

/**
 * \brief Init QSPI controller
 */
__RETAINED_CODE bool qspi_automode_init(void);

#if (dg_configDISABLE_BACKGROUND_FLASH_OPS == 0)
/**
 * \brief Check if a program or sector erase operation is in progress
 *
 * \param[in] id QSPI controller id
 *
 * \return bool True if the BUSY bit is set else false.
 *
 * \warning This function checks the value of the BUSY bit in the Status Register 1 of the Flash. It
 *        is the responsibility of the caller to call the function in the right context. The
 *        function must be called with interrupts disabled.
 *
 */
__RETAINED_CODE bool qspi_check_program_erase_in_progress(HW_QSPIC_ID id);

/**
 * \brief Resume a Flash program or sector erase operation
 *
 * \param[in] id QSPI controller id
 *
 * \warning After the call to this function, the QSPI controller is set to manual mode and the Flash
 *        access to single mode. The function must be called with interrupts disabled.
 *
 */
__RETAINED_CODE void qspi_resume(HW_QSPIC_ID id);

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
__RETAINED_CODE void flash_erase_sector_manual_mode(uint32_t addr);

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
 *        issue another flash_program_page_manual_mode() call in order to write the remaining data
 *        to the next page. The QSPI controller is left to manual mode after the call to this
 *        function. The function must be called with interrupts disabled.
 *
 */
__RETAINED_CODE uint32_t flash_program_page_manual_mode(uint32_t addr, const uint8_t *buf,
                                                        uint32_t len);
#endif

/**
 * \brief Configure Flash and QSPI controller for system clock frequency
 *
 * This function is used to change the Flash configuration of the QSPI controller
 * to work with the system clock frequency defined in sys_clk. Dummy clock
 * cycles could be changed here to support higher clock frequencies.
 * QSPI controller clock divider could also be changed if the Flash
 * maximum frequency is smaller than the system clock frequency.
 * This function must be called before changing system clock frequency.
 *
 * \param [in] sys_clk System clock frequency
 *
 */
__RETAINED_CODE void qspi_automode_sys_clock_cfg(sys_clk_t sys_clk);

/**
 * \brief Get ucode required for wake-up sequence
 *
 * \param [in] id QSPI controller id
 *
 * \return qspi_ucode_t Pointer to the structure containing the ucode and ucode size
 */
const qspi_ucode_t *qspi_automode_get_ucode(HW_QSPIC_ID id);

/**
 * \brief Verified if passed adder is valid and physically available
 *
 * \param [in] addr starting offset
 */
__RETAINED_CODE bool qspi_is_valid_addr(uint32_t addr);

/**
 * \brief Get maximum available memory size for selected controller id
 *
 * \param[in] id QSPI controller id
 *
 * \return Maximum memory size counted in bytes
 */
uint32_t qspi_get_device_size(HW_QSPIC_ID id);

/**
 * \brief Get configuration parameters for selected controller id
 *
 * \param [in]          id                    QSPI controller id
 * \param [out]         manufacturer_id       Manufacturer ID
 * \param [out]         device_type           Device type
 * \param [out]         density               Device density
 *
 * \return true if the device is marked as present, false otherwise
 */
bool qspi_get_config(HW_QSPIC_ID id, uint8_t *manufacturer_id, uint8_t *device_type,
                                                                                uint8_t *density);

/**
 * \brief Read the JEDEC manufacturer ID, device type and device density using command 0x9F
 *
 * \param [in]          id                      QSPI controller id
 * \param [out]         manufacturer_id         Pointer to the variable where the manufacturer ID
 *                                              will be returned
 * \param [out]         device_type             Pointer to the variable where the device type will
 *                                              be returned
 * \param [out]         density                 Pointer to the variable where the device density
 *                                              will be returned
 *
 * \return true if JEDEC Id has been read successfully, false otherwise
 *
 * \warning this function will reset FLASH device connected to the specified QSPI controller
 */
__RETAINED_CODE bool qspi_read_flash_jedec_id(HW_QSPIC_ID id, uint8_t *manufacturer_id,
                                                        uint8_t *device_type, uint8_t *density);

#endif /* QSPI_AUTOMODE_H_ */
/**
 * \}
 * \}
 */
