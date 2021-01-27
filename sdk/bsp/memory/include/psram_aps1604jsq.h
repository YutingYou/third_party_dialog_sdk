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
 * @file psram_aps1604jsq.h
 *
 * @brief QSPI ram driver for aps1604jsq
 *
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 *
 ****************************************************************************************
 */
#ifndef _PSRAM_APS1604JSQ_H_

#define _PSRAM_APS1604JSQ_H_

#define APS1604JSQ       0x5D

#ifndef APS1604_SIZE
#define APS1604_SIZE     0x1A
#endif

#include "qspi_common.h"
#include <qspi_apmemory.h>

static void psram_aps1604jsq_sys_clock_cfg(HW_QSPIC_ID id, sys_clk_t sys_clk);
static const qspi_flash_config_t psram_aps1604jsq_config = {
        .manufacturer_id               = APMEMORY_ID,
        .device_type                   = APS1604JSQ,
        .device_density                = APS1604_SIZE,
        .initialize                    = psram_initialize,
        .sys_clk_cfg                   = psram_aps1604jsq_sys_clock_cfg,
        .get_dummy_bytes               = psram_get_dummy_bytes,
        .break_seq_size                = HW_QSPI_BREAK_SEQ_SIZE_1B,
        .address_size                  = HW_QSPI_ADDR_SIZE_24,
        .send_once                     = 0,
        .fast_read_opcode              = CMD_FAST_READ_QUAD,
        .enter_qpi_opcode              = APM_CMD_ENTER_QUAD,
        .extra_byte                    = 0x00,
        .qpi_mode                      = true,
        .is_ram                        = true,
        .burst_len                     = HW_QSPI2_MEMBLEN_64,
        .cs_high_t                     = 8,
        .cs_high_t_en                  = true,
        .memory_size                   = MEMORY_SIZE_16Mb,
};

__RETAINED_CODE static void psram_aps1604jsq_sys_clock_cfg(HW_QSPIC_ID id, sys_clk_t sys_clk)
{
        uint16_t qspi_clk_div;
        uint16_t qspi_t_cem_cc;

        switch (hw_qspi_get_div(id)) {
        case HW_QSPI_DIV_1:
                qspi_clk_div = 1;
                break;
        case HW_QSPI_DIV_2:
                qspi_clk_div = 2;
                break;
        case HW_QSPI_DIV_4:
                qspi_clk_div = 4;
                break;
        case HW_QSPI_DIV_8:
                qspi_clk_div = 8;
                break;
        default:
                ASSERT_WARNING(0);
                return;
        }
        if (sys_clk == sysclk_PLL96) {
                qspi_t_cem_cc = psram_aps1604jsq_config.cs_high_t
                        * (dg_configPLL96M_FREQ / 1000000) / qspi_clk_div;
        } else {
                qspi_t_cem_cc = psram_aps1604jsq_config.cs_high_t
                        * (dg_configXTAL32M_FREQ / 1000000) / qspi_clk_div;
        }
        REG_SETF(QSPIC2, QSPIC2_MEMBLEN_REG, QSPIC_T_CEM_CC, MIN(qspi_t_cem_cc, 0x3FF));
}
#endif /* _PSRAM_APS1604JSQ_H_ */
/**
 * \}
 * \}
 */
