/**
 ****************************************************************************************
 *
 * @file hw_memctrl.c
 *
 * @brief Implementation of the Memory Controller Low Level Driver.
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#include "hw_memctrl.h"

void hw_memctrl_reset(void)
{
        MEMCTRL->CMI_CODE_BASE_REG = 0;
        MEMCTRL->CMI_DATA_BASE_REG = 0;
        MEMCTRL->CMI_SHARED_BASE_REG = 0;
        MEMCTRL->CMI_END_REG = 0x1FF << 10;
        MEMCTRL->SNC_BASE_REG = 0;
}

void hw_memctrl_config_cmac_region(uint32_t code_base_addr, uint32_t data_base_addr, uint32_t shared_base_addr, uint32_t end_addr)
{
        ASSERT_ERROR((code_base_addr % 1024) == 0);
        ASSERT_ERROR((data_base_addr % 4) == 0);
        ASSERT_ERROR((shared_base_addr % 1024) == 0);
        ASSERT_ERROR((end_addr & 0x3FF) == 0x3FF);

        MEMCTRL->CMI_CODE_BASE_REG = code_base_addr;
        MEMCTRL->CMI_DATA_BASE_REG = data_base_addr;
        MEMCTRL->CMI_SHARED_BASE_REG = shared_base_addr;
        MEMCTRL->CMI_END_REG = end_addr;
}

void hw_memctrl_config_snc_region(uint32_t snc_base_addr)
{
        ASSERT_ERROR(snc_base_addr % 4 == 0);

        MEMCTRL->SNC_BASE_REG = snc_base_addr;
}

void hw_memctrl_config_master_priorities(MEMCTRL_PRIO syscpu_prio, uint8_t syscpu_max_stall_cycles,
                                        MEMCTRL_PRIO dma_prio, uint8_t dma_max_stall_cycles,
                                        MEMCTRL_PRIO snc_prio, uint8_t snc_max_stall_cycles)
{
        ASSERT_ERROR((syscpu_max_stall_cycles > 0) && (syscpu_max_stall_cycles < 16));
        ASSERT_ERROR((dma_max_stall_cycles > 0) && (dma_max_stall_cycles < 16));
        ASSERT_ERROR((snc_max_stall_cycles > 0) && (snc_max_stall_cycles < 16));

        REG_SETF(MEMCTRL, MEM_PRIO_REG, AHB_PRIO, syscpu_prio);
        REG_SETF(MEMCTRL, MEM_STALL_REG, AHB_MAX_STALL, syscpu_max_stall_cycles);

        REG_SETF(MEMCTRL, MEM_PRIO_REG, AHB2_PRIO, dma_prio);
        REG_SETF(MEMCTRL, MEM_STALL_REG, AHB2_MAX_STALL, dma_max_stall_cycles);

        REG_SETF(MEMCTRL, MEM_PRIO_REG, SNC_PRIO, snc_prio);
        REG_SETF(MEMCTRL, MEM_STALL_REG, SNC_MAX_STALL, snc_max_stall_cycles);
}

