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
 * @file qspi_mx25u3235.h
 *
 * @brief QSPI flash driver for the Macronix MX25U3235
 *
 * Copyright (C) 2016-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifndef _QSPI_MX25U3235_H_
#define _QSPI_MX25U3235_H_

#ifndef MACRONIX_ID
#define MACRONIX_ID         0xC2
#endif

#ifndef MX25U_MX66U_SERIES
#define MX25U_MX66U_SERIES  0x25
#endif

#define MX25U3235_SIZE      0x36

#include "qspi_common.h"
#include "sdk_defs.h"

#include "qspi_macronix.h"

// Flash power up/down timings
#define MX25U3235_POWER_DOWN_DELAY_US          10

#define MX25U3235_RELEASE_POWER_DOWN_DELAY_US  30

#define MX25U3235_POWER_UP_DELAY_US            800

#if (dg_configFLASH_POWER_OFF == 1)
/**
 * \brief uCode for handling the QSPI FLASH activation from power off.
 */
        /*
         * Delay 3000usec
         * 0x01   // CMD_NBYTES = 0, CMD_TX_MD = 0 (Single), CMD_VALID = 1
         * 0x80   // CMD_WT_CNT_LS = 0x80 --> 3000000 / 62.5 = 48000 // 3000usec
         * 0xBB   // CMD_WT_CNT_MS = 0xBB
         * Exit from Fast Read mode
         * 0x11   // CMD_NBYTES = 2, CMD_TX_MD = 0 (Single), CMD_VALID = 1
         * 0x00   // CMD_WT_CNT_LS = 0
         * 0x00   // CMD_WT_CNT_MS = 0
         * 0xFF   // Enable Reset
         * 0xFF   // Enable Reset
         * (up to 16 words)
         */
        const uint32_t mx25u3235_ucode_wakeup[] = {
                0x11000001 | (((uint16_t)(MX25U3235_POWER_UP_DELAY_US*1000/62.5) & 0xFFFF) << 8),
                0xFFFF0000,
        };
#elif (dg_configFLASH_POWER_DOWN == 1)
/**
 * \brief uCode for handling the QSPI FLASH release from power-down.
 */
        /*
         * 0x09   // CMD_NBYTES = 1, CMD_TX_MD = 0 (Single), CMD_VALID = 1
         * 0xD0   // CMD_WT_CNT_LS = 0xD0 --> 45000 / 62.5 = 720   // 45usec for worst case (MX25R3235F)
         * 0x02   // CMD_WT_CNT_MS = 0x02
         * 0xAB   // Release Power Down
         * (up to 16 words)
         */
        const uint32_t mx25u3235_ucode_wakeup[] = {
                0xAB000009 | (((uint16_t)(MX25U3235_RELEASE_POWER_DOWN_DELAY_US*1000/62.5) & 0xFFFF) << 8),
        };
#else
/**
 * \brief uCode for handling the QSPI FLASH exit from the "Continuous Read Mode".
 */
        /*
         * 0x45   // CMD_NBYTES = 8, CMD_TX_MD = 2 (Quad), CMD_VALID = 1
         * 0x00   // CMD_WT_CNT_LS = 0
         * 0x00   // CMD_WT_CNT_MS = 0
         * 0xFF
         * 0xFF
         * 0xFF
         * 0xFF
         * 0xFF
         * 0xFF
         * 0xFF
         * 0xFF
         */
        const uint32_t mx25u3235_ucode_wakeup[] = {
                0xFF000025,
                0x00FFFFFF,
        };

        const uint32_t mx25u3235_ucode_wakeup_32bit_addressing[] = {
                0xFF000045,
                0xFFFFFFFF,
                0x00FFFFFF,
        };

#endif

static void flash_mx25u3235_initialize(HW_QSPIC_ID id);
static void flash_mx25u3235_sys_clock_cfg(HW_QSPIC_ID id, sys_clk_t sys_clk);
static uint8_t flash_mx25u3235_get_dummy_bytes(HW_QSPIC_ID id);

static const qspi_flash_config_t flash_mx25u3235_config = {
        .manufacturer_id               = MACRONIX_ID,
        .device_type                   = MX25U_MX66U_SERIES,
        .device_density                = MX25U3235_SIZE,
        .is_suspended                  = flash_mx_is_suspended,
        .initialize                    = flash_mx25u3235_initialize,
        .sys_clk_cfg                   = flash_mx25u3235_sys_clock_cfg,
        .get_dummy_bytes               = flash_mx25u3235_get_dummy_bytes,
        .break_seq_size                = HW_QSPI_BREAK_SEQ_SIZE_1B,
        .address_size                  = HW_QSPI_ADDR_SIZE_24,
        .fast_read_opcode              = CMD_FAST_READ_QUAD,
        .page_program_opcode           = MX_QUAD_IO_PAGE_PROGRAM,
        .erase_opcode                  = CMD_SECTOR_ERASE,
        .erase_suspend_opcode          = MX_ERASE_PROGRAM_SUSPEND,
        .erase_resume_opcode           = MX_ERASE_PROGRAM_RESUME,
        .quad_page_program_address     = true,
        .read_erase_progress_opcode    = CMD_READ_STATUS_REGISTER,
        .erase_in_progress_bit         = FLASH_STATUS_BUSY_BIT,
        .erase_in_progress_bit_high_level = true,
#if MACRONIX_PERFORMANCE_MODE
        .send_once                     = 1,
        .extra_byte                    = 0xA5,
#else
        .send_once                     = 0,
        .extra_byte                    = 0x00,
#endif
        .ucode_wakeup                  = {mx25u3235_ucode_wakeup, sizeof(mx25u3235_ucode_wakeup)},
        .power_down_delay              = MX25U3235_POWER_DOWN_DELAY_US,
        .release_power_down_delay      = MX25U3235_RELEASE_POWER_DOWN_DELAY_US,
        .power_up_delay                = MX25U3235_POWER_UP_DELAY_US,
        .qpi_mode                      = false,
        .is_ram                        = false,
        .memory_size                   = MEMORY_SIZE_32Mb, /* 32Mb bits */
};

__RETAINED_CODE static void flash_mx25u3235_initialize(HW_QSPIC_ID id)
{
        flash_mx_enable_quad_mode(id);
}

__RETAINED_CODE static void flash_mx25u3235_sys_clock_cfg(HW_QSPIC_ID id, sys_clk_t sys_clk)
{
}

__RETAINED_CODE static uint8_t flash_mx25u3235_get_dummy_bytes(HW_QSPIC_ID id)
{
        return 2;
}

#endif /* _QSPI_MX25U3235_H_ */
/**
 * \}
 * \}
 */
