/**
 * \addtogroup PLA_DRI_PER_AUDIO
 * \{
 * \addtogroup HW_APU_SRC Audio Processing Unit - Sample Rate Converter
 * \{
 * \brief APU Sample Rate Converter
 */

/**
 ****************************************************************************************
 *
 * @file hw_apu_src.h
 *
 * @brief Definition of the API for the Audio Unit SRC Low Level Driver.
 *
 * Copyright (C) 2019-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_APU_SRC_H_
#define HW_APU_SRC_H_

#if dg_configUSE_HW_APU_SRC

#include <stdbool.h>
#include <stdint.h>
#include "sdk_defs.h"

/**
 * \brief Input/Output direction
 */
typedef enum {
        HW_APU_SRC_IN,  /**< SRC input */
        HW_APU_SRC_OUT, /**< SRC output */
} HW_APU_SRC_DIRECTION;

/**
 * \brief Flow status
 */
typedef enum {
        HW_APU_SRC_FLOW_OK = 0,         /**< No flow errors */
        HW_APU_SRC_FLOW_OVER,           /**< Overflow errors */
        HW_APU_SRC_FLOW_UNDER,          /**< Underflow errors */
        HW_APU_SRC_FLOW_OVER_UNDER,     /**< Both overflow and underflow errors */
} HW_APU_SRC_FLOW_STATUS;

/**
 * \brief Input/Output selection
 */
typedef enum {
        HW_APU_SRC_PCM,         /**< PCM interface */
        HW_APU_SRC_PDM,         /**< PDM interface */
        HW_APU_SRC_REGS,        /**< SRC registers */
} HW_APU_SRC_SELECTION;

/**
 * \brief Clear the SRC over/underflow indications
 *
 * \param[in] io Input/Output selection (IN or OUT)
 */
#define HW_APU_SRC_CLEAR_FLOW_ERROR(io) \
        while (REG_GETF(APU, SRC1_CTRL_REG, SRC_##io##_OVFLOW)|| \
                REG_GETF(APU, SRC1_CTRL_REG, SRC_##io##_UNFLOW)) { \
                REG_SET_BIT(APU, SRC1_CTRL_REG, SRC_##io##_FLOWCLR); \
        } \
        REG_CLR_BIT(APU, SRC1_CTRL_REG, SRC_##io##_FLOWCLR);

/* *************************************************************************
 *
 *                       ENABLE-DISABLE FUNCTIONS
 *
 * ************************************************************************* */

/**
 * \brief Disable SRC
 */
__STATIC_INLINE void hw_apu_src_disable(void)
{
        REG_CLR_BIT(APU, SRC1_CTRL_REG, SRC_EN);
}

/**
 * \brief Enable SRC
 */
__STATIC_INLINE void hw_apu_src_enable(void)
{
        /*
         * The under/overflows occur due to the reconfiguration can be
         * ignored, so we disable under/overflow notifications until
         * (SRC_IN_OK == 1 && SRC_OUT_OK == 1).
         */
        REG_SET_BIT(APU, SRC1_CTRL_REG, SRC_IN_FLOWCLR);
        REG_SET_BIT(APU, SRC1_CTRL_REG, SRC_OUT_FLOWCLR);

        REG_SET_BIT(APU, SRC1_CTRL_REG, SRC_EN);
        while (REG_GETF(APU, SRC1_CTRL_REG, SRC_IN_OK) == 0 &&
                REG_GETF(APU, SRC1_CTRL_REG, SRC_OUT_OK) == 0);

        REG_CLR_BIT(APU, SRC1_CTRL_REG, SRC_IN_FLOWCLR);
        REG_CLR_BIT(APU, SRC1_CTRL_REG, SRC_OUT_FLOWCLR);
}

/**
 * \brief Disable SRC FIFO. On each SRC request, one sample is serviced.
 */
__STATIC_INLINE void hw_apu_src_disable_fifo(void)
{
        REG_CLR_BIT(APU, SRC1_CTRL_REG, SRC_FIFO_ENABLE);
}

/**
 * \brief Enable SRC FIFO. FIFO is used to store samples from/to SRC
 *
 * \param[in] direction The SRC FIFO direction.
 *                       HW_APU_SRC_IN  - FIFO is used to store samples from memory to SRC
 *                       HW_APU_SRC_OUT - FIFO is used to store samples from SRC to memory
 */
__STATIC_INLINE void hw_apu_src_enable_fifo(HW_APU_SRC_DIRECTION direction)
{
        switch (direction) {
        case HW_APU_SRC_IN:
                REG_CLR_BIT(APU, SRC1_CTRL_REG, SRC_FIFO_DIRECTION);
                break;
        case HW_APU_SRC_OUT:
                REG_SET_BIT(APU, SRC1_CTRL_REG, SRC_FIFO_DIRECTION);
                break;
        default:
                ASSERT_WARNING(0);
        }
        REG_SET_BIT(APU, SRC1_CTRL_REG, SRC_FIFO_ENABLE);
}

/**
 * \brief Check if SRC FIFO is enabled. FIFO is used to store samples from/to SRC
 *
 * \return  True if it is enabled or False if it is disabled
 */
__STATIC_INLINE bool hw_apu_src_is_fifo_enabled()
{
        return(REG_GETF(APU, SRC1_CTRL_REG, SRC_FIFO_ENABLE));
}

/* *************************************************************************
 *
 *                              SET FUNCTIONS
 *
 * ************************************************************************* */

/**
 * \brief Set the AMODE
 *
 * \param[in] direction              Input/Output direction of data flow allowed values:
 *                                       HW_APU_SRC_IN, HW_APU_SRC_OUT
 */
__STATIC_INLINE void hw_apu_src_set_automode(HW_APU_SRC_DIRECTION direction)
{
        if (direction == HW_APU_SRC_IN) {
                REG_SET_BIT(APU, SRC1_CTRL_REG, SRC_IN_AMODE);
        } else {
                REG_SET_BIT(APU, SRC1_CTRL_REG, SRC_OUT_AMODE);
        }
}

/**
 * \brief Clear the AMODE
 *
 * \param[in] direction              Input/Output direction of data flow allowed values:
 *                                       HW_APU_SRC_IN, HW_APU_SRC_OUT
 */
__STATIC_INLINE void hw_apu_src_set_manual_mode(HW_APU_SRC_DIRECTION direction)
{
        if (direction == HW_APU_SRC_IN) {
                REG_CLR_BIT(APU, SRC1_CTRL_REG, SRC_IN_AMODE);
        } else {
                REG_CLR_BIT(APU, SRC1_CTRL_REG, SRC_OUT_AMODE);
        }
}

/**
 * \brief Select the SRC input
 *
 * \param[in] input The SRC input
 */
__STATIC_INLINE void hw_apu_src_select_input(HW_APU_SRC_SELECTION input)
{
        uint32_t val = APU->APU_MUX_REG;

        switch (input) {
        case HW_APU_SRC_PDM:
                REG_SET_FIELD(APU, APU_MUX_REG, SRC1_MUX_IN, val, 0);
                REG_SET_FIELD(APU, APU_MUX_REG, PDM1_MUX_IN, val, 1);
                break;
        case HW_APU_SRC_PCM:
                REG_SET_FIELD(APU, APU_MUX_REG, SRC1_MUX_IN, val, 1);
                REG_CLR_FIELD(APU, APU_MUX_REG, PDM1_MUX_IN, val);
                break;
        case HW_APU_SRC_REGS:
                REG_SET_FIELD(APU, APU_MUX_REG, SRC1_MUX_IN, val, 2);
                REG_CLR_FIELD(APU, APU_MUX_REG, PDM1_MUX_IN, val);
                break;
        default:
                ASSERT_WARNING(0);
        }

        APU->APU_MUX_REG = val;
}

/**
 * \brief Write data to an input SRC register
 *
 * \param[in] stream The input stream (1 or 2)
 * \param[in] value  The data to be written
 */
__STATIC_INLINE void hw_apu_src_write_input(uint8_t stream, uint32_t value)
{
        switch (stream) {
        case 1:
                REG_SETF(APU, SRC1_IN1_REG, SRC_IN, value);
                break;
        case 2:
                REG_SETF(APU, SRC1_IN2_REG, SRC_IN, value);
                break;
        default:
                ASSERT_WARNING(0);
        }
}

/* *************************************************************************
 *
 *                              GET FUNCTIONS
 *
 * ************************************************************************* */
/**
 * \brief Get the mode
 *
 * \param[in] direction              Input/Output direction of data flow allowed values:
 *                                       HW_APU_SRC_IN, HW_APU_SRC_OUT
 * \return mode                        0 for manual mode, 1 for automatic mode
 */
__STATIC_INLINE bool hw_apu_src_get_mode(HW_APU_SRC_DIRECTION direction)
{
        if (direction == HW_APU_SRC_IN) {
                return REG_GETF(APU, SRC1_CTRL_REG, SRC_IN_AMODE);
        } else {
                return REG_GETF(APU, SRC1_CTRL_REG, SRC_OUT_AMODE);
        }
}

/**
 * \brief Read data from an output SRC register
 *
 * \param[in] stream The output stream (1 or 2)
 *
 * \return The data read
 */
__STATIC_INLINE uint32_t hw_apu_src_read_output(uint8_t stream)
{
        switch (stream) {
        case 1:
                return REG_GETF(APU, SRC1_OUT1_REG, SRC_OUT);
        case 2:
                return REG_GETF(APU, SRC1_OUT2_REG, SRC_OUT);
        default:
                ASSERT_WARNING(0);
                return 0;
        }
}

/**
 * \brief Get SRC1 status
 *
 * \return  SRC1 status
 *
 */
__STATIC_INLINE bool hw_apu_src_get_status()
{
        return (REG_GETF(APU, SRC1_CTRL_REG, SRC_EN));
}
/**
 * \brief Check if SRC flow errors have occurred and clear the indication
 *
 * \param[in] direction Input/Output direction
 *
 * \return The flow status
 */
HW_APU_SRC_FLOW_STATUS hw_apu_src_get_flow_status(HW_APU_SRC_DIRECTION direction);

/**
 * \brief Initialize the SRC
 *
 * Configure the SRC sampling frequencies, the input down-sampler and output up-sampler IIR filters,
 * the conversion modes, the divider of the internally generated clock and enable the clock
 *
 * \param[in]  src_clk        SRC clock in kHz with allowed values (in kHz):
 *                            128,  160,  200,  250,  256,  320,  400,  500,  640,   800,
 *                            1000, 1280, 1600, 2000, 3200, 4000, 6400, 8000, 16000, 32000
 * \param[in] in_sample_rate  Input sampling rate in Hz with allowed values:
 *                            0, 8000, 11025, 16000, 22050, 32000, 44100, 48000, 96000, 192000
 *
 * \param[in] out_sample_rate Output sampling rate in Hz with allowed values:
 *                              0, 8000, 11025, 16000, 22050, 32000, 44100, 48000, 96000, 192000
 */
void hw_apu_src_init(uint16_t src_clk,  uint32_t in_sample_rate, uint32_t out_sample_rate);

void hw_apu_src_clear_flow_error(void);
#endif /* dg_configUSE_HW_APU_SRC */
#endif /* HW_APU_SRC_H_ */
/**
 * \}
 * \}
 */
