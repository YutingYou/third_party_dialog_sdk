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
 * @file qspi_gd25le32.h
 *
 * @brief QSPI flash driver for the GigaDevice GD25LE32
 *
 * Copyright (C) 2018-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifndef _QSPI_GD25LE32_H_
#define _QSPI_GD25LE32_H_

#ifndef GIGADEVICE_ID
#define GIGADEVICE_ID  0xC8
#endif

#ifndef GD25LE_SERIES
#define GD25LE_SERIES  0x60
#endif

#define GD25LE32_SIZE  0x16

#include "qspi_common.h"
#include "sdk_defs.h"

#include "qspi_gigadevice.h"

// Flash power up/down timings
#define GD25LE32_POWER_DOWN_DELAY_US          20

#define GD25LE32_RELEASE_POWER_DOWN_DELAY_US  20

#define GD25LE32_POWER_UP_DELAY_US            1800


#if (dg_configFLASH_POWER_OFF == 1)
/**
 * \brief uCode for handling the QSPI FLASH activation from power off.
 */
        /*
         * Delay 1800usec
         * 0x01   // CMD_NBYTES = 0, CMD_TX_MD = 0 (Single), CMD_VALID = 1
         * 0x80   // CMD_WT_CNT_LS --> 1800000 / 62.5 = 28800 = 1800usec
         * 0x70   // CMD_WT_CNT_MS
         * Exit from Fast Read mode
         * 0x09   // CMD_NBYTES = 1, CMD_TX_MD = 0 (Single), CMD_VALID = 1
         * 0x00   // CMD_WT_CNT_LS = 0
         * 0x00   // CMD_WT_CNT_MS = 0
         * 0xFF   // Enable Reset
         * (up to 16 words)
         */
        const uint32_t GD25LE32_ucode_wakeup[] = {
                0x09000001 | (((uint16_t)(GD25LE32_POWER_UP_DELAY_US*1000/62.5) & 0xFFFF) << 8),
                0x00FF0000,
        };
#elif (dg_configFLASH_POWER_DOWN == 1)
/**
 * \brief uCode for handling the QSPI FLASH release from power-down.
 */
        /*
         * 0x09   // CMD_NBYTES = 1, CMD_TX_MD = 0 (Single), CMD_VALID = 1
         * 0x40   // CMD_WT_CNT_LS --> 20000 / 62.5 = 320   // 20usec
         * 0x01   // CMD_WT_CNT_MS
         * 0xAB   // Release Power Down
         * (up to 16 words)
         */
        const uint32_t GD25LE32_ucode_wakeup[] = {
                0xAB000009 | (((uint16_t)(GD25LE32_RELEASE_POWER_DOWN_DELAY_US*1000/62.5) & 0xFFFF) << 8),
        };
#else
/**
 * \brief uCode for handling the QSPI FLASH exit from the "Continuous Read Mode".
 */
        /*
         * 0x25   // CMD_NBYTES = 4, CMD_TX_MD = 2 (Quad), CMD_VALID = 1
         * 0x00   // CMD_WT_CNT_LS = 0
         * 0x00   // CMD_WT_CNT_MS = 0
         * 0xFF
         * 0xFF
         * 0xFF
         * 0xFF
         */
        const uint32_t GD25LE32_ucode_wakeup[] = {
                0xFF000025,
                0x00FFFFFF,
        };
#endif


static void flash_gd25le32_initialize(HW_QSPIC_ID id);
static void flash_gd25le32_sys_clock_cfg(HW_QSPIC_ID id, sys_clk_t sys_clk);
static uint8_t flash_gd25le32_get_dummy_bytes(HW_QSPIC_ID id);

static const qspi_flash_config_t flash_gd25le32_config = {
        .manufacturer_id               = GIGADEVICE_ID,
        .device_type                   = GD25LE_SERIES,
        .device_density                = GD25LE32_SIZE,
        .is_suspended                  = flash_gd_is_suspended,
        .initialize                    = flash_gd25le32_initialize,
        .sys_clk_cfg                   = flash_gd25le32_sys_clock_cfg,
        .get_dummy_bytes               = flash_gd25le32_get_dummy_bytes,
        .break_seq_size                = HW_QSPI_BREAK_SEQ_SIZE_1B,
        .address_size                  = HW_QSPI_ADDR_SIZE_24,
        .fast_read_opcode              = CMD_FAST_READ_QUAD,
        .page_program_opcode           = CMD_QUAD_PAGE_PROGRAM,
        .page_qpi_program_opcode       = CMD_QPI_PAGE_PROGRAM,
        .erase_opcode                  = CMD_SECTOR_ERASE,
        .erase_suspend_opcode          = GD_ERASE_PROGRAM_SUSPEND,
        .erase_resume_opcode           = GD_ERASE_PROGRAM_RESUME,
        .quad_page_program_address     = false,
        .read_erase_progress_opcode    = CMD_READ_STATUS_REGISTER,
        .erase_in_progress_bit         = FLASH_STATUS_BUSY_BIT,
        .erase_in_progress_bit_high_level = true,
#if GIGADEVICE_PERFORMANCE_MODE
        .send_once                     = 1,
        .extra_byte                    = 0x20,
#else
        .send_once                     = 0,
        .extra_byte                    = 0x00,
#endif
        .ucode_wakeup                  = {GD25LE32_ucode_wakeup, sizeof(GD25LE32_ucode_wakeup)},
        .power_down_delay              = GD25LE32_POWER_DOWN_DELAY_US,
        .release_power_down_delay      = GD25LE32_RELEASE_POWER_DOWN_DELAY_US,
        .power_up_delay                = GD25LE32_POWER_UP_DELAY_US,
        .is_ram                        = false,
        .qpi_mode                      = false,
        .enter_qpi_opcode              = CMD_ENTER_QPI_MODE,
        .memory_size                   = MEMORY_SIZE_32Mb, /* 32M-bit Serial Flash */

};

__RETAINED_CODE static void flash_gd25le32_initialize(HW_QSPIC_ID id)
{
        flash_gd_enable_quad_mode(id);
}

__RETAINED_CODE static void flash_gd25le32_sys_clock_cfg(HW_QSPIC_ID id, sys_clk_t sys_clk)
{
}

__RETAINED_CODE static uint8_t flash_gd25le32_get_dummy_bytes(HW_QSPIC_ID id)
{
        return 2;
}

#endif /* _QSPI_GD25LE32_H_ */
/**
 * \}
 * \}
 */
