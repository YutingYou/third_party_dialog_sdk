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
 * @file qspi_apmemory.h
 *
 * @brief QSPI driver for AP Memory PSRAMs - common code
 *
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *
 ****************************************************************************************
 */
#ifndef _QSPI_APMEMORY_H_
#define _QSPI_APMEMORY_H_

#include "sdk_defs.h"

#define APMEMORY_ID                      0x0D

#define APM_CMD_ENTER_QUAD               0x35
#define APM_CMD_EXIT_QUAD                0xF5
#define APM_CMD_RESET_ENABLE             0x66
#define APM_CMD_RESET_CMD                0x99


__RETAINED_CODE static void psram_initialize(HW_QSPIC_ID id)
{
        /*
         * If we initialize rst_cmd during declaration (e.g uint8_t cmd[] = { 0x66, 0x99 };),
         * compiler adds it to .rodata (flash). We need to run qspi_int_reset_device() from RAM
         * so we declare it without initialization and fill it later.
         */
        uint8_t cmd[2];

        cmd[0] = APM_CMD_RESET_ENABLE;
        cmd[1] = APM_CMD_RESET_CMD;

        flash_write(id, &cmd[0], 1);
        flash_write(id, &cmd[1], 1);
}

__RETAINED_CODE static uint8_t psram_get_dummy_bytes(HW_QSPIC_ID id)
{
        return 2;
}

#endif /* _QSPI_APMEMORY_H_ */
/**
 * \}
 * \}
 */
