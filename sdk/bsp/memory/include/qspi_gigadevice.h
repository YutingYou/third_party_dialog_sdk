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
 * @file qspi_gigadevice.h
 *
 * @brief QSPI flash driver for Gigadevice flashes - common code
 *
 * Copyright (C) 2016-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifndef _QSPI_GIGADEVICE_H_
#define _QSPI_GIGADEVICE_H_

#define GIGADEVICE_ID    0xC8

#include "sdk_defs.h"
#include "qspi_common.h"

#define GIGADEVICE_PERFORMANCE_MODE       1       // bootrom does not support Macronix performance mode

#define GD_ERASE_PROGRAM_SUSPEND        0x75
#define GD_ERASE_PROGRAM_RESUME         0x7A
#define GD_READ_STATUS_REGISTER_1       0x35
#define GD_WRITE_STATUS_REGISTER        0x01

#define GD_STATUS_QE_BIT                9 // Quad Enable
#define GD_STATUS_QE_MASK               (1 << GD_STATUS_QE_BIT)

/* Suspend */
#define GD_STATUS_SUS1_BIT             7 // Erase suspend bit
#define GD_STATUS_SUS1_MASK            (1 << GD_STATUS_SUS1_BIT)
#define GD_STATUS_SUS2_BIT             2 // Program suspend bit
#define GD_STATUS_SUS2_MASK            (1 << GD_STATUS_SUS2_BIT)

__STATIC_FORCEINLINE __UNUSED uint8_t flash_gd_read_status_register_1(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ uint8_t status;
        uint8_t cmd[] = { GD_READ_STATUS_REGISTER_1 };

        flash_transact(id, cmd, 1, &status, 1);

        return status;
}

__STATIC_FORCEINLINE __UNUSED void flash_gd_write_status_register(HW_QSPIC_ID id, uint16_t value)
{
        uint8_t cmd[] = { GD_WRITE_STATUS_REGISTER, value, value >> 8 };

        flash_write(id, cmd, 3);

        /* Wait for the Flash to process the command */
        while (flash_is_busy(id));
}

__STATIC_FORCEINLINE void flash_gd_enable_quad_mode(HW_QSPIC_ID id)
{
        uint16_t status;

        status = flash_read_status_register(id);
        status |= flash_gd_read_status_register_1(id) << 8;
        if (!(status & GD_STATUS_QE_MASK)) {
                flash_write_enable(id);
                flash_gd_write_status_register(id, status | GD_STATUS_QE_MASK);
        }
}

__RETAINED_CODE static bool flash_gd_is_suspended(HW_QSPIC_ID id)
{
        __DBG_QSPI_VOLATILE__ uint8_t status;

        status = flash_gd_read_status_register_1(id);
        return (status & (GD_STATUS_SUS1_MASK | GD_STATUS_SUS2_MASK)) != 0;
}

#endif /* _QSPI_GIGADEVICE_H_ */
/**
 * \}
 * \}
 */
