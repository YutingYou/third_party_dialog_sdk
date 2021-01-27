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
 * @file qspi_common.h
 *
 * @brief QSPI flash driver common definitions
 *
 * Copyright (C) 2016-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifndef _QSPI_COMMON_H_
#define _QSPI_COMMON_H_

#include <stdbool.h>

/*
 * Flash Commands
 *
 * Note: Default command issuing mode is single mode! If commands specific to other modes have to be
 * issued then the mode must be changed!
 */
#define CMD_WRITE_STATUS_REGISTER       0x01
#define CMD_WRITE_DISABLE               0x04
#define CMD_READ_STATUS_REGISTER        0x05
#define CMD_WRITE_ENABLE                0x06
#define CMD_SECTOR_ERASE                0x20
#define CMD_QUAD_PAGE_PROGRAM           0x32
#define CMD_QPI_PAGE_PROGRAM            0x02
#define CMD_BLOCK_ERASE                 0x52
#define CMD_CHIP_ERASE                  0xC7
#define CMD_FAST_READ_QUAD              0xEB
#define CMD_READ_JEDEC_ID               0x9F
#define CMD_EXIT_CONTINUOUS_MODE        0xFF
#define CMD_RELEASE_POWER_DOWN          0xAB
#define CMD_ENTER_POWER_DOWN            0xB9

#define CMD_FAST_READ_QUAD_4B           0xEC
#define CMD_SECTOR_ERASE_4B             0x21
#define CMD_QUAD_PAGE_PROGRAM_4B        0x34

#define CMD_ENTER_QPI_MODE              0x38    // Requires single mode for the command entry!
#define CMD_EXIT_QPI_MODE               0xFF    // Requires quad mode for the command entry!

/*
 * RAM specific commands
 *
 */
#define CMD_WRITE                       0x02
#define CMD_WRITE_QUAD                  0x38

#define MEMORY_SIZE_1Mb                 (1024 * 1024) /* 1Mb memory size in bits*/
#define MEMORY_SIZE_2Mb                 (2 * MEMORY_SIZE_1Mb)
#define MEMORY_SIZE_4Mb                 (4 * MEMORY_SIZE_1Mb)
#define MEMORY_SIZE_8Mb                 (8 * MEMORY_SIZE_1Mb)
#define MEMORY_SIZE_16Mb                (16 * MEMORY_SIZE_1Mb)
#define MEMORY_SIZE_32Mb                (32 * MEMORY_SIZE_1Mb)
#define MEMORY_SIZE_64Mb                (64 * MEMORY_SIZE_1Mb)
#define MEMORY_SIZE_128Mb               (128 * MEMORY_SIZE_1Mb)
#define MEMORY_SIZE_512Mb               (512 * MEMORY_SIZE_1Mb)

/* Erase/Write in progress */
#define FLASH_STATUS_BUSY_BIT           0
#define FLASH_STATUS_BUSY_MASK          (1 << FLASH_STATUS_BUSY_BIT)

/* WE Latch bit */
#define FLASH_STATUS_WEL_BIT            1
#define FLASH_STATUS_WEL_MASK           (1 << FLASH_STATUS_WEL_BIT)

typedef bool (*is_suspended_cb_t)(HW_QSPIC_ID id);
typedef void (*initialize_cb_t)(HW_QSPIC_ID id);
typedef void (*sys_clk_cfg_cb_t)(HW_QSPIC_ID id, sys_clk_t type);
typedef uint8_t (*get_dummy_bytes_cb_t)(HW_QSPIC_ID id);

/**
 * \brief QSPI Flash configuration structure
 *
 * This struct is used to define a driver for a specific QSPI device.
 *
 * \note The struct instance must be declared as static const for this to work
 */
typedef struct qspi_flash_config {

        /* Struct members that should be configured in case of either a Flash or a RAM device*/

        uint8_t manufacturer_id;               /**< JEDEC vendor ID (Cmd 0x9F, 1st byte)
                                                    This (and the device_type/device_density)
                                                    are needed for flash auto detection */
        uint8_t device_type;                   /**< JEDEC device type (Cmd 0x9F, 2nd byte) */
        uint8_t device_density;                /**< JEDEC device type (Cmd 0x9F, 3rd byte) */
        initialize_cb_t initialize;            /**< Callback that is used for device initialization */
        sys_clk_cfg_cb_t sys_clk_cfg;          /**< Callback that is used to perform device configuration when
                                                    system clock is changed (e.g. change dummy bytes or QSPIC
                                                    clock divider */
        get_dummy_bytes_cb_t get_dummy_bytes;  /**< Return the number of dummy bytes currently
                                                    needed (they may e.g. change when the clock
                                                    changes) */
        HW_QSPI_BREAK_SEQ_SIZE break_seq_size; /**< Whether the break sequence, which puts flash out of the
                                                    continuous mode, is one or two bytes long (the break byte is
                                                    0xFF) */
        HW_QSPI_ADDR_SIZE address_size;        /**< Whether the device uses 24- or 32-bit addressing mode */
        uint8_t send_once;                     /**< If set to 1, the "Performance mode" (or burst, or
                                                    continuous; differs per vendor) will be used for read
                                                    accesses. In this mode, the read opcode is only sent
                                                    once, and subsequent accesses only transfer the address */
        uint8_t fast_read_opcode;              /**< Opcode for fast read operation */
        uint8_t enter_qpi_opcode;              /**< Opcode for entering QPI mode */
        uint8_t extra_byte;                    /**< The extra byte to transmit, when in "Performance mode"
                                                    (send once is 1), that tells the flash that it should
                                                    stay in this continuous, performance mode */
        bool qpi_mode;                         /**< Used only in QSPIC2, true if the device operates in QPI mode */
        bool is_ram;                           /**< True if device is RAM, false if device is Flash */
        uint32_t memory_size;                  /**< Maximum capacity, memory size of selected device in Mbits */



        is_suspended_cb_t is_suspended;        /**< Callback that is used to check if device is in erase/program
                                                    suspend state */


        /* Struct members that should be configured only in case of a Flash device*/

        uint8_t erase_opcode;                  /**< erase opcode to use */
        uint8_t erase_suspend_opcode;          /**< erase suspend opcode to use */
        uint8_t erase_resume_opcode;           /**< erase resume opcode to use */
        uint8_t page_program_opcode;           /**< page program opcode to use */
        uint8_t page_qpi_program_opcode;       /**< QPI page program opcode to use */
        bool quad_page_program_address;        /**< If true, the address will be transmitted in
                                                    QUAD mode when writing a page. Otherwise, it
                                                    will be transmitted in single mode */
        uint8_t read_erase_progress_opcode;    /**< The opcode to use to check if erase is in progress
                                                    (Usually the Read Status Reg opcode (0x5)) */
        uint8_t erase_in_progress_bit;         /**< The bit to check when reading the erase progress */
        bool erase_in_progress_bit_high_level; /**< The active state (true: high, false: low) of the bit
                                                    above */
        qspi_ucode_t ucode_wakeup;             /**< The QSPIC microcode to use to setup the flash on wakeup. This
                                                    is automatically used by the QSPI Controller after wakeup, and
                                                    before CPU starts code execution. This is different based on
                                                    whether flash was active, in deep power down or completely off
                                                    while the system was sleeping */
        uint16_t power_down_delay;             /**< This is the time, in usec, needed for the flash to go to power
                                                    down, after the Power Down command is issued */
        uint16_t release_power_down_delay;     /**< This is the time, in usec, needed for the flash to exit the power
                                                    down mode, after the Release Power Down command is issued */
        uint16_t power_up_delay;               /**< This is the time, in usec, needed for the flash to power-up */


        /* Struct members that should be configured only in case of a RAM device*/

        HW_QSPI2_MEMBLEN burst_len;            /**< Used only in QSPIC2. The length of the wrapping burst that the
                                                    external memory device is capable to implement */
        uint8_t cs_high_t;                     /**< Used only in QSPIC2. The maximum allowed time tCEM for which the
                                                    QSPIC_CS can stay active (in usec) */
        bool cs_high_t_en;                     /**< Used only in QSPIC2. True if control of the tCEM time is enabled */

} qspi_flash_config_t;

#endif /* _QSPI_COMMON_H_ */
/**
 * \}
 * \}
 */
