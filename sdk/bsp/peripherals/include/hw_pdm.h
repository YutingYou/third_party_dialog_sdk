/**
 * \addtogroup PLA_DRI_PER_AUDIO
 * \{
 * \addtogroup HW_PDM_AUDIO PDM Audio Interface Driver
 * \{
 * \brief PDM LLD provides a serial audio connection for 1 stereo or 2 mono input devices
 *  or outputs devices.
 */

/**
 ****************************************************************************************
 *
 * @file hw_pdm.h
 *
 * @brief Definition of API for the PDM Low Level Driver.
 *
 * Copyright (C) 2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_PDM_H_
#define HW_PDM_H_


#if dg_configUSE_HW_PDM

#include "sdk_defs.h"

/**
 * \brief PDM Master/Slave mode
 *
 */
typedef enum {
        HW_PDM_SLAVE_MODE = 0, /**< PDM Interface in slave mode */
        HW_PDM_MASTER_MODE     /**< PDM Interface in master mode */
} HW_PDM_MODE;

/**
 * \brief PDM input delay
 *
 */
typedef enum {
        HW_PDM_DI_NO_DELAY = 0, /**< no PDM input delay */
        HW_PDM_DI_4_NS_DELAY,   /**< 4ns PDM input delay */
        HW_PDM_DI_8_NS_DELAY,   /**< 8ns PDM input delay */
        HW_PDM_DI_12_NS_DELAY   /**< 12ns PDM input delay */
} HW_PDM_DI_DELAY;

/**
 * \brief PDM output delay
 *
 */
typedef enum {
        HW_PDM_DO_NO_DELAY = 0, /**< no delay */
        HW_PDM_DO_8_NS_DELAY,   /**< 8ns PDM output delay */
        HW_PDM_DO_12_NS_DELAY,  /**< 12ns PDM output delay */
        HW_PDM_DO_16_NS_DELAY   /**< 16ns PDM output delay */
} HW_PDM_DO_DELAY;

/**
 * \brief PDM output channel configuration
 *
 */
typedef enum {
        HW_PDM_CHANNEL_NONE = 0,  /**< No PDM output - no output */
        HW_PDM_CHANNEL_R,         /**< Right channel only PDM output (falling edge of PDM_CLK) */
        HW_PDM_CHANNEL_L,         /**< Left channel only PDM output (rising edge of PDM_CLK) */
        HW_PDM_CHANNEL_LR         /**< Left and Right channel PDM output */
} HW_PDM_CHANNEL_CONFIG;


/**
 * \brief Get input delay in PDM interface
 *
 * \return Additional delay (in ns) from the PDM data input pad to
 *            the PDM interface. Available delay values are:
 * - no delay,
 * - 4 ns delay,
 * - 8 ns delay,
 * - 12 ns delay.
 */
__STATIC_INLINE HW_PDM_DI_DELAY hw_pdm_get_input_delay()
{
        return(REG_GETF(APU, SRC1_CTRL_REG, SRC_PDM_DI_DEL));
}

/**
 * \brief Get output delay in PDM interface
 *
 * \return additional delay (in ns) from the PDM interface to the PDM data outpu pad.
 *  Available delay values are:
 * - no delay,
 * - 8 ns delay,
 * - 12 ns delay,
 * - 16 ns delay.
 */
__STATIC_INLINE HW_PDM_DO_DELAY hw_pdm_get_output_delay()
{
        return(REG_GETF(APU, SRC1_CTRL_REG, SRC_PDM_DO_DEL));
}

/**
 * \brief Get PDM output channel configuration
 *
 * \return channel_conf Output configuration of the PDM output interface.
 * Available values for PDM output configuration are:
 * \parblock
 *      ::HW_PDM_CHANNEL_NONE<br>
 *      There is no data on the PDM output interface.
 *
 *      ::HW_PDM_CHANNEL_R<br>
 *      Data stream at the output of the PDM interface is available only on the right channel.
 *
 *      ::HW_PDM_CHANNEL_L<br>
 *      Data stream at the output of the PDM interface is available only on the left channel.
 *
 *      ::HW_PDM_CHANNEL_LR<br>
 *      Data stream at the output of the PDM interface is available on both left and right channel.
 * \endparblock
 */
__STATIC_INLINE HW_PDM_CHANNEL_CONFIG hw_pdm_get_output_channel_config()
{
        return(REG_GETF(APU, SRC1_CTRL_REG, SRC_PDM_MODE));
}

/**
 * \brief Get PDM Master/Slave mode
 *
 * \return The PDM mode, HW_PDM_SLAVE_MODE or HW_PDM_MASTER_MODE
 *
 */
__STATIC_INLINE HW_PDM_MODE hw_pdm_get_mode()
{
        return(REG_GETF(CRG_PER, PDM_DIV_REG, PDM_MASTER_MODE));
}

/**
 * \brief Get PDM status. Supported only for Master mode
 *
 * \return  PDM status
 *
 */
__STATIC_INLINE bool hw_pdm_get_status()
{
        if (hw_pdm_get_mode() == HW_PDM_SLAVE_MODE) {
                return true;
        }

        return(REG_GETF(CRG_PER, PDM_DIV_REG, CLK_PDM_EN));
}

/**
 * \brief Get PDM clock divider.
 *
 * \return  PDM clock divider
 *
 */
__STATIC_INLINE uint8_t hw_pdm_get_clk_div() {

        return(REG_GETF(CRG_PER, PDM_DIV_REG, PDM_DIV));
}

/**
 * \brief Get the status of swap of the channels on the PDM input source
 *
 * \return true when input PDM channels are swapped otherwise false
 *
 */
__STATIC_INLINE bool hw_pdm_get_in_channel_swap()
{
        return(REG_GETF(APU, SRC1_CTRL_REG, SRC_PDM_IN_INV));
}

/**
 * \brief Get the status of swap of the channels on the PDM output source
 *
 * \return true when output PDM channels are swapped otherwise false
 *
 */
__STATIC_INLINE bool hw_pdm_get_out_channel_swap()
{
        return(REG_GETF(APU, SRC1_CTRL_REG, SRC_PDM_OUT_INV));
}

/**
 * \brief Enable PDM block system clock source used only for Master mode
 *
 * Enable the PDM clock source.
 * PDM_DIV must be set before or together with CLK_PDM_EN.
 */
__STATIC_INLINE void hw_pdm_enable(void)
{
        REG_SET_BIT(CRG_PER, PDM_DIV_REG, CLK_PDM_EN);
}

/**
 * \brief Disable PDM block system clock source
 *
 * Disable the PDM clock source.
 */
__STATIC_INLINE void hw_pdm_disable(void)
{
        REG_CLR_BIT(CRG_PER, PDM_DIV_REG, CLK_PDM_EN);
}

/**
 * \brief Initialize PDM clock
 *
 * \param[in] frequency requested frequency in the range 125490...4000000 (Hz) of PDM clock
 *              for default clock DIVN.
 *
 *  * \return achieved frequency (Hz) of PDM clock
 */
uint32_t hw_pdm_request_clk(uint32_t frequency);

/**
 * \brief Set input delay in PDM interface
 *
 * \param[in] delay Additional delay (in ns) from the PDM data input pad to
 *            the PDM interface. Available delay values are:
 * - no delay,
 * - 4 ns delay,
 * - 8 ns delay,
 * - 12 ns delay.
 */
__STATIC_INLINE void hw_pdm_set_input_delay(HW_PDM_DI_DELAY delay)
{
        REG_SETF(APU, SRC1_CTRL_REG, SRC_PDM_DI_DEL, delay);
}

/**
 * \brief Set output delay in PDM interface
 *
 * \param[in] delay additional delay (in ns) from the PDM interface to the PDM data outpu pad.
 *  Available delay values are:
 * - no delay,
 * - 8 ns delay,
 * - 12 ns delay,
 * - 16 ns delay.
 */
__STATIC_INLINE void hw_pdm_set_output_delay(HW_PDM_DO_DELAY delay)
{
        REG_SETF(APU, SRC1_CTRL_REG, SRC_PDM_DO_DEL, delay);
}

/**
 * \brief Set PDM output channel configuration
 *
 * \param[in] channel_conf Output configuration of the PDM output interface.
 * Available values for PDM output configuration are:
 * \parblock
 *      ::HW_PDM_CHANNEL_NONE<br>
 *      There is no data on the PDM output interface.
 *
 *      ::HW_PDM_CHANNEL_R<br>
 *      Data stream at the output of the PDM interface is available only on the right channel.
 *
 *      ::HW_PDM_CHANNEL_L<br>
 *      Data stream at the output of the PDM interface is available only on the left channel.
 *
 *      ::HW_PDM_CHANNEL_LR<br>
 *      Data stream at the output of the PDM interface is available on both left and right channel.
 * \endparblock
 */
__STATIC_INLINE void hw_pdm_set_output_channel_config(HW_PDM_CHANNEL_CONFIG channel_conf)
{
        REG_SETF(APU, SRC1_CTRL_REG, SRC_PDM_MODE, channel_conf);
}

/**
 * \brief Set PDM Master/Slave mode
 *
 * \param[in] mode The PDM mode, HW_PDM_SLAVE_MODE or HW_PDM_MASTER_MODE
 *
 */
__STATIC_INLINE void hw_pdm_set_mode(HW_PDM_MODE mode)
{
        REG_SETF(CRG_PER, PDM_DIV_REG, PDM_MASTER_MODE, mode);
}

/**
 * \brief Swap left and right channel on the PDM input source
 *
 * \param[in] swap true when input PDM channels are swapped otherwise false
 *
 */
__STATIC_INLINE void hw_pdm_set_in_channel_swap(bool swap)
{
        if (swap == true) {
                REG_SET_BIT(APU, SRC1_CTRL_REG, SRC_PDM_IN_INV);
        } else {
                REG_CLR_BIT(APU, SRC1_CTRL_REG, SRC_PDM_IN_INV);
        }
}

/**
 * \brief Swap left and right channel on the PDM output source
 *
 * \param[in] swap true when output PDM channels are swapped otherwise false
 *
 */
__STATIC_INLINE void hw_pdm_set_out_channel_swap(bool swap)
{
        if (swap == true) {
                REG_SET_BIT(APU, SRC1_CTRL_REG, SRC_PDM_OUT_INV);
        } else {
                REG_CLR_BIT(APU, SRC1_CTRL_REG, SRC_PDM_OUT_INV);
        }
}

#endif /* dg_configUSE_HW_PDM */
#endif /* HW_PDM_H_ */

/**
 * \}
 * \}
 */
