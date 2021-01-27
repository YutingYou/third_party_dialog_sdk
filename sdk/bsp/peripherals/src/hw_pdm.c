/**
 ****************************************************************************************
 *
 * @file hw_pdm.c
 *
 * @brief Implementation of the PDM/Audio Low Level Driver.
 *
 * Copyright (C) 2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configUSE_HW_PDM
#include <hw_pdm.h>

uint32_t hw_pdm_request_clk(uint32_t frequency)
{
        uint32_t div;

        /* Translate main clk frequency and requested frequency to proper divider */
        div = (dg_configDIVN_FREQ / frequency);

        /* Calculate the achievable frequency */
        if (dg_configXTAL32M_FREQ % frequency) {
                frequency = (dg_configXTAL32M_FREQ / div);
        }

        /* PDM_CLK frequency according to specification is in the range of 62.5 kHz - 4 MHz */
        ASSERT_WARNING((frequency >= 62500) && (frequency <= 4000000));

        ASSERT_WARNING((div & ~(CRG_PER_PDM_DIV_REG_PDM_DIV_Msk >>
                                CRG_PER_PDM_DIV_REG_PDM_DIV_Pos)) == 0);

        REG_SETF(CRG_PER, PDM_DIV_REG, PDM_DIV, div);

        return frequency;
}

#endif /* dg_configUSE_HW_PDM */
