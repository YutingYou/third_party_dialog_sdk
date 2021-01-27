/**
 * \addtogroup BSP
 * \{
 * \addtogroup SYSTEM
 * \{
 * \addtogroup DMA
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file hw_dma.c
 *
 * @brief Implementation of the DMA Low Level Driver.
 *
 * Copyright (C) 2015-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#if dg_configUSE_HW_DMA

#include <hw_gpio.h>
#include <hw_dma.h>

#if (dg_configSYSTEMVIEW)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif

static struct hw_dma_callback_data {
        hw_dma_transfer_cb callback;
        void *user_data;
} dma_callbacks_user_data[8];

#define DMA_CHN_REG(reg, chan) ((volatile uint32 *)(&(reg)) + ((chan) * 8))

/**
 * \brief Initialize DMA Channel
 *
 * \param [in] channel_setup pointer to struct of type DMA_Setup
 *
 */
void hw_dma_channel_initialization(DMA_setup *channel_setup)
{
        volatile dma_size_t *dma_x_len_reg;
        volatile dma_size_t *dma_x_int_reg;
        volatile uint32 *dma_x_ctrl_reg;
        volatile uint32 *dma_x_a_start_reg;
        volatile uint32 *dma_x_b_start_reg;
        uint32 src_address;
        uint32 dest_address;

        /* Make sure the DMA channel length is not zero */
        ASSERT_WARNING(channel_setup->length > 0);
        ASSERT_ERROR(channel_setup->channel_number < HW_DMA_CHANNEL_INVALID);

        /* If Secure mode is enabled only the DMA channel 7 can be used by security adapter */
        if (REG_GETF(CRG_TOP, SECURE_BOOT_REG, PROT_AES_KEY_READ) &&
            channel_setup->dest_address >= AES_HASH_BASE          &&
            channel_setup->dest_address <= (AES_HASH_BASE + 0x100) )  {
                ASSERT_ERROR(channel_setup->channel_number == HW_DMA_CHANNEL_7);
        }


        // Look up DMAx_CTRL_REG address
        dma_x_ctrl_reg = DMA_CHN_REG(DMA->DMA0_CTRL_REG, channel_setup->channel_number);

        // Look up DMAx_A_START_REG address
        dma_x_a_start_reg = DMA_CHN_REG(DMA->DMA0_A_START_REG, channel_setup->channel_number);

        // Look up DMAx_B_START_REG address
        dma_x_b_start_reg = DMA_CHN_REG(DMA->DMA0_B_START_REG, channel_setup->channel_number);

        // Look up DMAX_LEN_REG address
        dma_x_len_reg = DMA_CHN_REG(DMA->DMA0_LEN_REG, channel_setup->channel_number);

        // Look up DMAX_INT
        dma_x_int_reg = DMA_CHN_REG(DMA->DMA0_INT_REG, channel_setup->channel_number);

        // Make sure DMA channel is disabled first
        REG_SET_FIELD(DMA, DMA0_CTRL_REG, DMA_ON, *dma_x_ctrl_reg, HW_DMA_STATE_DISABLED);

        // Set DMAx_CTRL_REG width provided settings, but do not start the channel.
        // Start the channel with the "dma_channel_enable" function separately.
        *dma_x_ctrl_reg =
                  channel_setup->bus_width |
                  channel_setup->dreq_mode |
                  channel_setup->burst_mode |
                  /*Always enable bus error detection. */
                  REG_MSK(DMA, DMA0_CTRL_REG, BUS_ERROR_DETECT) |
                  channel_setup->b_inc |
                  channel_setup->a_inc |
                  channel_setup->circular |
                  channel_setup->dma_prio |
                  channel_setup->dma_idle |
                  channel_setup->dma_init;

        if (channel_setup->irq_enable == HW_DMA_IRQ_STATE_ENABLED) {
                GLOBAL_INT_DISABLE();
                DMA->DMA_INT_MASK_REG |= 1 << channel_setup->channel_number;
                GLOBAL_INT_RESTORE();
        } else {
                GLOBAL_INT_DISABLE();
                DMA->DMA_INT_MASK_REG &= ~(1 << channel_setup->channel_number);
                GLOBAL_INT_RESTORE();
        }

        // Set DMA_REQ_MUX_REG for the requested channel / trigger combination
        if (channel_setup->dma_req_mux != HW_DMA_TRIG_NONE) {
                switch (channel_setup->channel_number) {
                case HW_DMA_CHANNEL_0:
                case HW_DMA_CHANNEL_1:
                        GLOBAL_INT_DISABLE();
                        REG_SETF(DMA, DMA_REQ_MUX_REG, DMA01_SEL, channel_setup->dma_req_mux);
                        GLOBAL_INT_RESTORE();
                        break;
                case HW_DMA_CHANNEL_2:
                case HW_DMA_CHANNEL_3:
                        GLOBAL_INT_DISABLE();
                        REG_SETF(DMA, DMA_REQ_MUX_REG, DMA23_SEL, channel_setup->dma_req_mux);
                        GLOBAL_INT_RESTORE();
                        break;
                case HW_DMA_CHANNEL_4:
                case HW_DMA_CHANNEL_5:
                        GLOBAL_INT_DISABLE();
                        REG_SETF(DMA, DMA_REQ_MUX_REG, DMA45_SEL, channel_setup->dma_req_mux);
                        GLOBAL_INT_RESTORE();
                        break;
                case HW_DMA_CHANNEL_6:
                case HW_DMA_CHANNEL_7:
                        GLOBAL_INT_DISABLE();
                        REG_SETF(DMA, DMA_REQ_MUX_REG, DMA67_SEL, channel_setup->dma_req_mux);
                        GLOBAL_INT_RESTORE();
                        break;
                default:
                        break;
                }

                if (channel_setup->dma_req_mux != HW_DMA_TRIG_PCM_RXTX && channel_setup->dma_req_mux != HW_DMA_TRIG_SRC_RXTX) {
                        /*
                        * When different DMA channels are used for same device it is important
                        * that only one trigger is set for specific device at a time.
                        * Having same trigger for different channels can cause unpredictable results.
                        * Following code also should help when SPI1 is assigned to non 0 channel.
                        * The audio triggers (src and pcm) are an exception, as they use 2 pairs each for DMA access
                        */
                        GLOBAL_INT_DISABLE();
                        switch (channel_setup->channel_number) {
                        case HW_DMA_CHANNEL_6:
                        case HW_DMA_CHANNEL_7:
                                if (REG_GETF(DMA, DMA_REQ_MUX_REG, DMA45_SEL) == channel_setup->dma_req_mux) {
                                        REG_SETF(DMA, DMA_REQ_MUX_REG, DMA45_SEL, HW_DMA_TRIG_NONE);
                                }
                                /* no break */
                        case HW_DMA_CHANNEL_4:
                        case HW_DMA_CHANNEL_5:
                                if (REG_GETF(DMA, DMA_REQ_MUX_REG, DMA23_SEL) == channel_setup->dma_req_mux) {
                                        REG_SETF(DMA, DMA_REQ_MUX_REG, DMA23_SEL, HW_DMA_TRIG_NONE);
                                }
                                /* no break */
                        case HW_DMA_CHANNEL_2:
                        case HW_DMA_CHANNEL_3:
                                if (REG_GETF(DMA, DMA_REQ_MUX_REG, DMA01_SEL) == channel_setup->dma_req_mux) {
                                        REG_SETF(DMA, DMA_REQ_MUX_REG, DMA01_SEL, HW_DMA_TRIG_NONE);
                                }
                                break;
                        case HW_DMA_CHANNEL_0:
                        case HW_DMA_CHANNEL_1:
                        default:
                                break;
                        }
                        GLOBAL_INT_RESTORE();
                }
        }


        if ((channel_setup->dma_req_mux == HW_DMA_TRIG_USB_RXTX) &&
                (channel_setup->channel_number & 1)) {//odd channels used for TX
                REG_SET_FIELD(DMA, DMA0_CTRL_REG, REQ_SENSE, *dma_x_ctrl_reg, 1);
        }

        src_address = black_orca_phy_addr(channel_setup->src_address);
        dest_address = black_orca_phy_addr(channel_setup->dest_address);

        // Set source address registers
        if (IS_QSPIF_ADDRESS(src_address)) {
                /* Peripherals access QSPI through a different address range compared to the CPU */
                src_address += 0x20000000;
        }
        *dma_x_a_start_reg = src_address;
        // Set destination address registers
        *dma_x_b_start_reg = dest_address;

        // Set IRQ number of transfers
        if (channel_setup->irq_nr_of_trans > 0) {
                // If user explicitly set this number use it
                *dma_x_int_reg = channel_setup->irq_nr_of_trans - 1;
        } else {
                // If user passed 0, use transfer length to fire interrupt after transfer ends
                *dma_x_int_reg = channel_setup->length - 1;
        }

        // Set the transfer length
        *dma_x_len_reg = (channel_setup->length) - 1;

        if (channel_setup->irq_enable)
                dma_callbacks_user_data[channel_setup->channel_number].callback = channel_setup->callback;
        else
                dma_callbacks_user_data[channel_setup->channel_number].callback = NULL;
        dma_callbacks_user_data[channel_setup->channel_number].user_data = channel_setup->user_data;
}

void hw_dma_channel_update_source(HW_DMA_CHANNEL channel, void *addr, dma_size_t length,
                                                                        hw_dma_transfer_cb cb)
{
        uint32_t phy_addr = black_orca_phy_addr((uint32_t) addr);

        dma_callbacks_user_data[channel].callback = cb;

        // Look up DMAx_A_START_REG address
        volatile uint32 *dma_x_a_start_reg = DMA_CHN_REG(DMA->DMA0_A_START_REG, channel);

        // Look up DMAX_LEN_REG address
        volatile dma_size_t *dma_x_len_reg = DMA_CHN_REG(DMA->DMA0_LEN_REG, channel);

        // Look up DMAX_INT_REG address
        volatile dma_size_t *dma_x_int_reg = DMA_CHN_REG(DMA->DMA0_INT_REG, channel);

        if (IS_QSPIF_ADDRESS(phy_addr)) {
                /* Peripherals access QSPI through a different address range compared to the CPU */
                phy_addr += 0x20000000;
        }
        // Set source address registers
        *dma_x_a_start_reg = phy_addr;

        *dma_x_int_reg = length - 1;

        // Set the transfer length
        *dma_x_len_reg = length - 1;
}

void hw_dma_channel_update_destination(HW_DMA_CHANNEL channel, void *addr, dma_size_t length,
                                                                        hw_dma_transfer_cb cb)
{
        uint32_t phy_addr = black_orca_phy_addr((uint32_t) addr);

        /* If Secure mode is enabled only the DMA channel 7 can be used by security adapter */
        if (REG_GETF(CRG_TOP, SECURE_BOOT_REG, PROT_AES_KEY_READ) &&
            phy_addr >= AES_HASH_BASE && phy_addr <= (AES_HASH_BASE + 0x100) )  {
                ASSERT_ERROR(channel == HW_DMA_CHANNEL_7);
        }


        dma_callbacks_user_data[channel].callback = cb;

        // Look up DMAx_B_START_REG address
        volatile uint32 *dma_x_b_start_reg = DMA_CHN_REG(DMA->DMA0_B_START_REG, channel);

        // Look up DMAX_LEN_REG address
        volatile dma_size_t *dma_x_len_reg = DMA_CHN_REG(DMA->DMA0_LEN_REG, channel);
        volatile dma_size_t *dma_x_int_reg = DMA_CHN_REG(DMA->DMA0_INT_REG, channel);

        // Set destination address register
        *dma_x_b_start_reg = phy_addr;

        *dma_x_int_reg = length - 1;

        // Set the transfer length
        *dma_x_len_reg = length - 1;
}

void hw_dma_channel_update_int_ix(HW_DMA_CHANNEL channel, uint16_t int_ix)
{
        volatile dma_size_t *dma_x_int_reg = DMA_CHN_REG(DMA->DMA0_INT_REG, channel);
        *dma_x_int_reg = int_ix;
}

/**
 * \brief Enable or disable a DMA channel
 *
 * \param [in] channel_number DMA channel number to start/stop
 * \param [in] dma_on enable/disable DMA channel
 *
 */
void hw_dma_channel_enable(HW_DMA_CHANNEL channel_number, HW_DMA_STATE dma_on)
{
        // Look up DMAx_CTRL_REG address
        volatile dma_size_t *dma_x_ctrl_reg = DMA_CHN_REG(DMA->DMA0_CTRL_REG, channel_number);

        if (dma_on == HW_DMA_STATE_ENABLED) {
                uint16_t dma_ctrl = *dma_x_ctrl_reg;

                REG_SET_FIELD(DMA, DMA0_CTRL_REG, DMA_ON, dma_ctrl, 1);
                if (dma_callbacks_user_data[channel_number].callback) {
                        GLOBAL_INT_DISABLE();
                        DMA->DMA_INT_MASK_REG |= 1 << channel_number;
                        GLOBAL_INT_RESTORE();
                }
                // Start the chosen DMA channel
                *dma_x_ctrl_reg = dma_ctrl;
                NVIC_EnableIRQ(DMA_IRQn);
        } else {
                // Stop the chosen DMA channel
                REG_SET_FIELD(DMA, DMA0_CTRL_REG, DMA_ON, *dma_x_ctrl_reg, 0);
                GLOBAL_INT_DISABLE();
                DMA->DMA_INT_MASK_REG &= ~(1 << channel_number);
                GLOBAL_INT_RESTORE();
        }
}

__STATIC_INLINE void dma_helper(HW_DMA_CHANNEL channel_number, dma_size_t len, bool stop_dma)
{
        hw_dma_transfer_cb cb;

        NVIC_DisableIRQ(DMA_IRQn);
        cb = dma_callbacks_user_data[channel_number].callback;
        if (stop_dma) {
                dma_callbacks_user_data[channel_number].callback = NULL;
                hw_dma_channel_enable(channel_number, HW_DMA_STATE_DISABLED);
        }
        if (cb) {
                cb(dma_callbacks_user_data[channel_number].user_data, len);
        }
        NVIC_EnableIRQ(DMA_IRQn);
}

bool hw_dma_channel_active(void)
{
        int dma_on;

        dma_on = REG_GETF(DMA, DMA0_CTRL_REG, DMA_ON);
        dma_on |= REG_GETF(DMA, DMA1_CTRL_REG, DMA_ON);
        dma_on |= REG_GETF(DMA, DMA2_CTRL_REG, DMA_ON);
        dma_on |= REG_GETF(DMA, DMA3_CTRL_REG, DMA_ON);
        dma_on |= REG_GETF(DMA, DMA4_CTRL_REG, DMA_ON);
        dma_on |= REG_GETF(DMA, DMA5_CTRL_REG, DMA_ON);
        dma_on |= REG_GETF(DMA, DMA6_CTRL_REG, DMA_ON);
        dma_on |= REG_GETF(DMA, DMA7_CTRL_REG, DMA_ON);

        return (dma_on == 1);
}

bool hw_dma_is_channel_active(HW_DMA_CHANNEL channel_number)
{
        volatile dma_size_t *dma_x_ctrl_reg = DMA_CHN_REG(DMA->DMA0_CTRL_REG, channel_number);
        return REG_GET_FIELD(DMA, DMA0_CTRL_REG, DMA_ON, *dma_x_ctrl_reg);
}

/**
 * \brief Capture DMA Interrupt Handler
 *
 * Calls user interrupt handler
 *
 */
void DMA_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        uint16_t risen;
        uint16_t i;
        volatile dma_size_t *dma_x_len_reg;
        volatile dma_size_t *dma_x_int_reg;
        volatile uint32 *dma_x_ctrl_reg;
        risen = DMA->DMA_INT_STATUS_REG;

        for (i = 0; risen != 0 && i < 8; ++i, risen >>= 1) {
                if (risen & 1) {
                        bool stop;

                        /*
                         * DMAx_INT_REG shows after how many transfers the interrupt
                         * is generated
                         */
                        dma_x_int_reg = DMA_CHN_REG(DMA->DMA0_INT_REG, i);

                        /*
                         * DMAx_LEN_REG shows the length of the DMA transfer
                         */
                        dma_x_len_reg = DMA_CHN_REG(DMA->DMA0_LEN_REG, i);

                        dma_x_ctrl_reg = DMA_CHN_REG(DMA->DMA0_CTRL_REG, i);

                        /*
                         * Stop DMA if:
                         *  - transfer is completed
                         *  - mode is not circular
                         */
                        stop = (*dma_x_int_reg == *dma_x_len_reg)
                                && (!REG_GET_FIELD(DMA, DMA0_CTRL_REG, CIRCULAR, *dma_x_ctrl_reg));
                        DMA->DMA_CLEAR_INT_REG = 1 << i;
                        dma_helper(i, *dma_x_int_reg + 1, stop);
                }
        }

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

void hw_dma_channel_stop(HW_DMA_CHANNEL channel_number)
{
        // Stopping DMA will clear DMAx_IDX_REG so read it before
        volatile dma_size_t *dma_x_idx_reg = DMA_CHN_REG(DMA->DMA0_IDX_REG, channel_number);
        dma_helper(channel_number, *dma_x_idx_reg, true);
}

dma_size_t hw_dma_transfered_bytes(HW_DMA_CHANNEL channel_number)
{
        volatile dma_size_t *dma_x_int_reg = DMA_CHN_REG(DMA->DMA0_IDX_REG, channel_number);
        return *dma_x_int_reg;
}

#endif /* dg_configUSE_HW_DMA */

/**
 * \}
 * \}
 * \}
 */
