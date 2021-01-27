/**
 ****************************************************************************************
 *
 * @file hw_apu_src.c
 *
 * @brief Implementation of the Audio Unit SRC Low Level Driver.
 *
 * Copyright (C) 2019-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#if dg_configUSE_HW_APU_SRC
#include "hw_apu_src.h"

/**
 * \brief Calculate the SRC sampling frequency and the IIR filters setting
 *
 * \param[in] sample_rate Sampling rate in Hz with allowed values:
 *                        8000, 11025, 16000, 22050, 32000, 44100, 48000, 96000, 192000
 * \param[in] divider     SRC clock divider (1..255)
 *
 * \param[out] iir_setting The calculated Input Up-Sampling or Output Down-Sampling IIR filters
 *              setting
 *
 * \return The calculated sampling frequency (in Hz)
 */
static uint32_t hw_apu_src_calc_sampling_frequency(uint32_t sample_rate, uint8_t divider,
                                                                               uint8_t *iir_setting)
{
        ASSERT_WARNING(divider > 0);
        ASSERT_WARNING(iir_setting != NULL);
        switch (sample_rate) {
                case 8000:
                case 11025:
                case 16000:
                case 22050:
                case 32000:
                case 44100:
                case 48000:
                        *iir_setting = 0;
                          break;
                case 96000:
                        *iir_setting = 1;
                        break;
                case 192000:
                        *iir_setting = 3;
                        break;
                default:
                        ASSERT_WARNING(0);
                        return 2; // reserved
                }

        sample_rate /= (*iir_setting) + 1;
        sample_rate /= 100;
        uint32_t sampling_frequency = 4096 * sample_rate * divider;
        ASSERT_WARNING(sampling_frequency <= 0xFFFFFF);
        return sampling_frequency & 0xFFFFFF;
}

uint8_t hw_apu_src_calculate_divider(uint16_t src_clk)
{
        const uint16_t divn_clk = dg_configDIVN_FREQ / 1000;

        if (divn_clk % src_clk) {
                ASSERT_WARNING(0);
                return 0 ;
        }

       return (divn_clk / src_clk);
}

void hw_apu_src_init(uint16_t src_clk, uint32_t in_sample_rate, uint32_t out_sample_rate)
{
        uint8_t divider;
        uint8_t iir_setting;
        uint32_t val;

        const uint16_t divn_clk = dg_configDIVN_FREQ / 1000;

         if (divn_clk % src_clk) {
                 ASSERT_WARNING(0);
                 return;
         }

         divider = divn_clk / src_clk;

         val = CRG_PER->SRC_DIV_REG;
         REG_SET_FIELD(CRG_PER, SRC_DIV_REG, SRC_DIV, val, divider);
         REG_SET_FIELD(CRG_PER, SRC_DIV_REG, CLK_SRC_EN, val, 1);
         CRG_PER->SRC_DIV_REG = val;

        if (in_sample_rate > 0) {
                uint32_t sampling_frequency = hw_apu_src_calc_sampling_frequency(in_sample_rate,
                        divider, &iir_setting);
                REG_SETF(APU, SRC1_IN_FS_REG, SRC_IN_FS, sampling_frequency);
                val = APU->SRC1_CTRL_REG;
                REG_SET_FIELD(APU, SRC1_CTRL_REG, SRC_IN_DS, val, iir_setting);
                APU->SRC1_CTRL_REG = val;
        }

        if (out_sample_rate > 0) {
                uint32_t sampling_frequency = hw_apu_src_calc_sampling_frequency(out_sample_rate,
                        divider, &iir_setting);
                REG_SETF(APU, SRC1_OUT_FS_REG, SRC_OUT_FS, sampling_frequency);
                val = APU->SRC1_CTRL_REG;
                REG_SET_FIELD(APU, SRC1_CTRL_REG, SRC_OUT_US, val, iir_setting);
                APU->SRC1_CTRL_REG = val;
        }
}

void hw_apu_src_clear_flow_error()
{
        // Clear input data registers
        hw_apu_src_write_input(1, 0);
        hw_apu_src_write_input(2, 0);

        HW_APU_SRC_CLEAR_FLOW_ERROR(IN);
        HW_APU_SRC_CLEAR_FLOW_ERROR(OUT);
}

HW_APU_SRC_FLOW_STATUS hw_apu_src_get_flow_status(HW_APU_SRC_DIRECTION direction)
{
        HW_APU_SRC_FLOW_STATUS status = HW_APU_SRC_FLOW_OK;
        switch (direction) {
        case HW_APU_SRC_IN:
                if (REG_GETF(APU, SRC1_CTRL_REG, SRC_IN_OVFLOW)) {
                        status |= HW_APU_SRC_FLOW_OVER;
                }
                if (REG_GETF(APU, SRC1_CTRL_REG, SRC_IN_UNFLOW)) {
                        status |= HW_APU_SRC_FLOW_UNDER;
                }
                HW_APU_SRC_CLEAR_FLOW_ERROR(IN);
                break;
        case HW_APU_SRC_OUT:
                if (REG_GETF(APU, SRC1_CTRL_REG, SRC_OUT_OVFLOW)) {
                        status |= HW_APU_SRC_FLOW_OVER;
                }
                if (REG_GETF(APU, SRC1_CTRL_REG, SRC_OUT_UNFLOW)) {
                        status |= HW_APU_SRC_FLOW_UNDER;
                }
                HW_APU_SRC_CLEAR_FLOW_ERROR(OUT);
                break;
        default:
                ASSERT_WARNING(0);
        }
        return status;
}

#endif /* dg_configUSE_HW_APU_SRC */
