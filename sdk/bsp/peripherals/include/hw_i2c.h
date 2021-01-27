/**
 * \addtogroup PLA_DRI_PER_COMM
 * \{
 * \addtogroup HW_I2C I2C Driver
 * \{
 * \brief I2C Controller
 */

/**
 ****************************************************************************************
 *
 * @file hw_i2c.h
 *
 * @brief Definition of API for the I2C Low Level Driver.
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_I2C_H_
#define HW_I2C_H_


#if dg_configUSE_HW_I2C

#include <stdbool.h>
#include <stdint.h>
#include <sdk_defs.h>

/* I2C Base Address */
#define IBA(id)                       ((I2C_Type *)id)

/**
 * \brief TX/RX FIFO depth
 *
 */
#define I2C_FIFO_DEPTH   (4)

/**
 * \brief Wrapper to perform controller setup
 *
 * Controller will be disabled, then any code given as \p seq is executed and controller is enabled
 * again.
 *
 */
#define I2C_SETUP(id, seq)           \
        do {                         \
                hw_i2c_disable(id);  \
                seq;                 \
                hw_i2c_enable(id);   \
        } while (0);

/**
 * \brief I2C controller instance
 *
 */
#define HW_I2C1         ((void *)I2C_BASE)
#define HW_I2C2         ((void *)I2C2_BASE)
typedef void * HW_I2C_ID;

/*
 * Flags passed to read/write operations
 */
#define HW_I2C_F_NONE                   0x00000000      /**< No special command for the operation                          */
#define HW_I2C_F_WAIT_FOR_STOP          0x00000001      /**< Operation will wait until stop condition occurs               */
#define HW_I2C_F_ADD_STOP               0x00000002      /**< Add stop condition after read or write                        */
#define HW_I2C_F_ADD_RESTART            0x00000004      /**< Add Restart condition at the start of read or write           */

/**
 * \brief I2C high speed master code.
 *  This bit field holds the value of the I2C HS mode master code.
 *  HS-mode master codes are reserved 8-bit codes (00001xxx)
 *  that are not used for slave addressing or other purposes.
 *  Each master has its unique master code.
 *  Up to eight high-speed mode masters can be present on the same I2C bus system.
 *  Valid values are from 0 to 7.
 *  This can be written only when the I2C interface is disabled,
 *  which corresponds to the IC_ENABLE[0] register being set to 0.
 *  Writes at other times have no effect.
 *
 */
#define HW_I2C_I2C1_MADDR       0x01
#define HW_I2C_I2C2_MADDR       0x05

/**
 * \brief I2C abort source
 *
 */
typedef enum {
   HW_I2C_ABORT_NONE = 0,                                                                 /**< no abort occured */
   HW_I2C_ABORT_7B_ADDR_NO_ACK = I2C_I2C_TX_ABRT_SOURCE_REG_ABRT_7B_ADDR_NOACK_Msk,       /**< address byte of 7-bit address was not acknowledged by any slave */
   HW_I2C_ABORT_10B_ADDR1_NO_ACK = I2C_I2C_TX_ABRT_SOURCE_REG_ABRT_10ADDR1_NOACK_Msk,     /**< 1st address byte of the 10-bit address was not acknowledged by any slave */
   HW_I2C_ABORT_10B_ADDR2_NO_ACK = I2C_I2C_TX_ABRT_SOURCE_REG_ABRT_10ADDR2_NOACK_Msk,     /**< 2nd address byte of the 10-bit address was not acknowledged by any slave */
   HW_I2C_ABORT_TX_DATA_NO_ACK = I2C_I2C_TX_ABRT_SOURCE_REG_ABRT_TXDATA_NOACK_Msk,        /**< data were not acknowledged by slave */
   HW_I2C_ABORT_GENERAL_CALL_NO_ACK = I2C_I2C_TX_ABRT_SOURCE_REG_ABRT_GCALL_NOACK_Msk,    /**< General Call sent but no slave acknowledged */
   HW_I2C_ABORT_GENERAL_CALL_READ = I2C_I2C_TX_ABRT_SOURCE_REG_ABRT_GCALL_READ_Msk,       /**< trying to read from bus after General Call */
   HW_I2C_ABORT_START_BYTE_ACK = I2C_I2C_TX_ABRT_SOURCE_REG_ABRT_SBYTE_ACKDET_Msk,        /**< START condition acknowledged by slave */
   HW_I2C_ABORT_10B_READ_NO_RESTART = I2C_I2C_TX_ABRT_SOURCE_REG_ABRT_10B_RD_NORSTRT_Msk, /**< read command in 10-bit addressing mode with RESTART disabled */
   HW_I2C_ABORT_MASTER_DISABLED = I2C_I2C_TX_ABRT_SOURCE_REG_ABRT_MASTER_DIS_Msk,         /**< master operation initiated with master mode disabled */
   HW_I2C_ABORT_ARBITRATION_LOST = I2C_I2C_TX_ABRT_SOURCE_REG_ARB_LOST_Msk,               /**< bus arbitration lost */
   HW_I2C_ABORT_SLAVE_FLUSH_TX_FIFO = I2C_I2C_TX_ABRT_SOURCE_REG_ABRT_SLVFLUSH_TXFIFO_Msk,/**< (slave mode) request for data with data already in TX FIFO - used to flush data in TX FIFO */
   HW_I2C_ABORT_SLAVE_ARBITRATION_LOST = I2C_I2C_TX_ABRT_SOURCE_REG_ABRT_SLV_ARBLOST_Msk, /**< (slave mode) bus lost when transmitting to master */
   HW_I2C_ABORT_SLAVE_IN_TX = I2C_I2C_TX_ABRT_SOURCE_REG_ABRT_SLVRD_INTX_Msk,             /**< (slave mode) request for data replied with read request */
   HW_I2C_ABORT_SW_ERROR                                                                  /**< abort due to software error */
} HW_I2C_ABORT_SOURCE;

/**
 * \brief I2C interface speed
 *
 */
typedef enum {
        HW_I2C_SPEED_STANDARD = 0,  /**< 100kb/s */
        HW_I2C_SPEED_FAST,          /**< 400kb/s */
        HW_I2C_SPEED_HIGH           /**< 3.4Mb/s */
} HW_I2C_SPEED;

/**
 * \brief I2C mode
 *
 */
typedef enum {
        HW_I2C_MODE_MASTER = 0,  /**< master mode */
        HW_I2C_MODE_SLAVE        /**< slave mode */
} HW_I2C_MODE;

/**
 * \brief I2C addressing mode
 *
 */
typedef enum {
        HW_I2C_ADDRESSING_7B = 0,  /**< 7-bit addressing */
        HW_I2C_ADDRESSING_10B      /**< 10-bit addressing */
} HW_I2C_ADDRESSING;

/**
 * \brief Callback events when working as slave
 *
 */
typedef enum {
        HW_I2C_EVENT_READ_REQUEST = 0,  /**< Data read request from master */
        HW_I2C_EVENT_DATA_READY,        /**< Data written by master */
        HW_I2C_EVENT_TX_ABORT,          /**< TX FIFO abort */
        HW_I2C_EVENT_RX_OVERFLOW,       /**< RX FIFO overflow, some data are lost */
        HW_I2C_EVENT_INVALID            /**< Invalid event */
} HW_I2C_EVENT;

/**
 * \brief I2C interrupt source
 *
 * Can be used as bitmask.
 *
 */
typedef enum {
        HW_I2C_INT_RX_UNDERFLOW = I2C_I2C_INTR_STAT_REG_R_RX_UNDER_Msk,     /**< attempt to read from empty RX FIFO has been made */
        HW_I2C_INT_RX_OVERFLOW = I2C_I2C_INTR_STAT_REG_R_RX_OVER_Msk,       /**< RX FIFO is full but new data are incoming and being discarded */
        HW_I2C_INT_RX_FULL = I2C_I2C_INTR_STAT_REG_R_RX_FULL_Msk,           /**< RX FIFO level is equal or above threshold set by hw_i2c_set_rx_fifo_threshold() */
        HW_I2C_INT_TX_OVERFLOW = I2C_I2C_INTR_STAT_REG_R_TX_OVER_Msk,       /**< attempt to write to TX FIFO which is already full */
        HW_I2C_INT_TX_EMPTY = I2C_I2C_INTR_STAT_REG_R_TX_EMPTY_Msk,         /**< TX FIFO level is  equal or below threshold set by hw_i2c_set_tx_fifo_threshold() */
        HW_I2C_INT_READ_REQUEST = I2C_I2C_INTR_STAT_REG_R_RD_REQ_Msk,       /**< (slave only) I2C master attempts to read data */
        HW_I2C_INT_TX_ABORT = I2C_I2C_INTR_STAT_REG_R_TX_ABRT_Msk,          /**< TX cannot be completed \sa hw_get_abort_source() \sa hw_i2c_reset_abort_source() */
        HW_I2C_INT_RX_DONE = I2C_I2C_INTR_STAT_REG_R_RX_DONE_Msk,           /**< (slave only) I2C master did not acknowledge transmitted byte */
        HW_I2C_INT_ACTIVITY = I2C_I2C_INTR_STAT_REG_R_ACTIVITY_Msk,         /**< any I2C activity occurred */
        HW_I2C_INT_STOP_DETECTED = I2C_I2C_INTR_STAT_REG_R_STOP_DET_Msk,    /**< STOP condition occurred */
        HW_I2C_INT_START_DETECTED = I2C_I2C_INTR_STAT_REG_R_START_DET_Msk,  /**< START/RESTART condition occurred */
        HW_I2C_INT_GENERAL_CALL = I2C_I2C_INTR_STAT_REG_R_GEN_CALL_Msk      /**< (slave only) General Call address received */
} HW_I2C_INT;

/**
 * \brief DMA transfer type
 *
 */
typedef enum {
        HW_I2C_DMA_TRANSFER_WRITE,
        HW_I2C_DMA_TRANSFER_MASTER_READ,
        HW_I2C_DMA_TRANSFER_SLAVE_READ,
} HW_I2C_DMA_TRANSFER;

/**
 * \brief Callback called on interrupt from I2C controller
 *
 * \param [in] id I2C controller instance
 * \param [in] mask interrupt events mask
 *
 */
typedef void (*hw_i2c_interrupt_cb)(HW_I2C_ID id, uint16_t mask);

/**
 * \brief Callback called upon completion of read or write in non-blocking mode (FIFO or DMA)
 *
 * This is a common callback type, which can be used with all non-deprecated API
 *
 * \param [in] id I2C controller instance
 * \param [in] cb_data data passed by user along with callback
 * \param [in] len number of bytes transferred. In case of write failure this number is equal
 *             to the number of bytes written to I2C TX FIFO until the failure occurred.
 * \param [in] success operation status
 *
 */
typedef void (*hw_i2c_complete_cb)(HW_I2C_ID id, void *cb_data, uint16_t len, bool success);

/**
 * \brief Callback called on DMA operation completed
 *
 * \param [in] id I2C controller instance
 * \param [in] cb_data user data passed to hw_i2c_prepare_dma(), hw_i2c_write_buffer_dma()
 *             or hw_i2c_read_buffer_dma()
 * \param [in] len length of data in buffer
 *
 * \deprecated it is used by deprecated API, consider switching to API that uses
 *             \sa hw_i2c_complete_cb
 *
 */
typedef /*DEPRECATED*/ void (*hw_i2c_dma_completed_handler_cb)(HW_I2C_ID id, void *cb_data,
                uint16_t len);

/**
 * \brief Callback called on event when in slave mode
 *
 * \param [in] id I2C controller instance
 * \param [in] event event identifier
 *
 */
typedef void (*hw_i2c_event_cb)(HW_I2C_ID id, HW_I2C_EVENT event);

/**
 * \brief I2C configuration
 *
 */
typedef struct {
        /** I2C clock (SCL) settings, refer to datasheet for details. Set to 0 for default values to be used. */
        struct {
                uint16_t ss_hcnt;       /**< standard speed I2C clock (SCL) high count */
                uint16_t ss_lcnt;       /**< standard speed I2C clock (SCL) low count  */
                uint16_t fs_hcnt;       /**< fast speed I2C clock (SCL) high count     */
                uint16_t fs_lcnt;       /**< fast speed I2C clock (SCL) low count      */
                uint16_t hs_hcnt;       /**< high speed I2C clock (SCL) high count     */
                uint16_t hs_lcnt;       /**< high speed I2C clock (SCL) low count      */
                } clock_cfg;

        HW_I2C_SPEED        speed;      /**< bus speed */
        HW_I2C_MODE         mode;       /**< mode of operation */
        HW_I2C_ADDRESSING   addr_mode;  /**< addressing mode */
        uint16_t            address;    /**< target slave address in master mode or controller address in slave mode */
        hw_i2c_event_cb     event_cb;   /**< slave event callback (only valid in slave mode) */
} i2c_config;

/**
 * \brief Write a value to an I2C register field
 *
 * \param [in] id I2C controller instance
 * \param [in] reg the I2C register
 * \param [in] field the I2C register field
 * \param [in] val value to be written
 *
 * \sa HW_I2C_REG_GETF
 *
 */
#define HW_I2C_REG_SETF(id, reg, field, val) \
        IBA(id)->reg##_REG = ((IBA(id)->reg##_REG & ~(I2C_##reg##_REG_##field##_Msk)) | \
        ((I2C_##reg##_REG_##field##_Msk) & ((val) << (I2C_##reg##_REG_##field##_Pos))))

/**
 * \brief Get the value of an I2C register field
 *
 * \param [in] id I2C controller instance
 * \param [in] reg the I2C register
 * \param [in] field the I2C register field
 *
 * \sa HW_SPI_SET_RFIELD
 *
 */
#define HW_I2C_REG_GETF(id, reg, field) \
        ((IBA(id)->reg##_REG & (I2C_##reg##_REG_##field##_Msk)) >> (I2C_##reg##_REG_##field##_Pos))

/**
 * \brief Initialize I2C controller
 *
 * I2C controller is disabled, clock and interrupt for I2C component are enabled, all interrupts are
 * masked though. \p cfg can be NULL if no configuration should be performed.
 *
 * \note Even with \p cfg set to NULL, I2C clock (SCL) will be configured using default values.
 *
 * \note The I2C clock source is set to DIVN (16MHz, regardless of PLL or XTAL16M being used).
 *
 * \param [in] id I2C controller instance
 * \param [in] cfg configuration
 *
 */
void hw_i2c_init(HW_I2C_ID id, const i2c_config *cfg);

/**
 * \brief DeInitialize I2C controller
 *
 * \param [in] id I2C controller instance
 *
 */
void hw_i2c_deinit(HW_I2C_ID id);

/**
 * \brief Configure I2C controller
 *
 * Shortcut to configure most common I2C controller parameters.
 *
 * \note Even with \p cfg set to NULL, I2C clock (SCL) will be configured using default values.
 *
 * \param [in] id I2C controller instance
 * \param [in] cfg configuration
 *
 */
void hw_i2c_configure(HW_I2C_ID id, const i2c_config *cfg);

/**
 * \brief Enable I2C controller
 *
 * hw_i2c_init() shall be called before enabling I2C controller.
 *
 * \param [in] id I2C controller instance
 *
 */
__STATIC_INLINE void hw_i2c_enable(HW_I2C_ID id)
{
        HW_I2C_REG_SETF(id, I2C_ENABLE, I2C_EN, 1);
}

/**
 * \brief Disable I2C controller
 *
 * \param [in] id I2C controller instance
 *
 */
__STATIC_INLINE void hw_i2c_disable(HW_I2C_ID id)
{
        HW_I2C_REG_SETF(id, I2C_ENABLE, I2C_EN, 0);
}

/**
 * \brief Get I2C Controller Enable status
 *
 * \return The contents of the I2C_ENABLE_STATUS_REG
 *
 */
__STATIC_INLINE uint16_t hw_i2c_get_enable_status(HW_I2C_ID id)
{
        return IBA(id)->I2C_ENABLE_STATUS_REG;
}

/**
 * \brief Get I2C controller occupied status
 *
 * This function checks if the I2C controller is occupied by an ongoing operation
 * waiting for an interrupt to hit in order to complete the operation.
 *
 * \param [in] id I2C controller instance
 *
 * \return True if requested id is already occupied, false otherwise
 *
 */
bool hw_i2c_is_occupied(HW_I2C_ID id);

/**
 * \brief Register interrupt handler
 *
 * \param [in] id I2C controller instance
 * \param [in] cb callback function
 * \param [in] mask initial bitmask of requested interrupt events
 *
 * \sa hw_i2c_set_int_mask
 *
 */
void hw_i2c_register_int(HW_I2C_ID id, hw_i2c_interrupt_cb cb, uint16_t mask);

/**
 * \brief Unregister interrupt handler
 *
 * This function disables all I2C interrupts by masking them. In addition
 * it clears any pending ones on the ARM core. The status of RAW_INTR_STAT_REG
 * remains unchanged.
 *
 * \param [in] id I2C controller instance
 *
 */
void hw_i2c_unregister_int(HW_I2C_ID id);

/**
 * \brief Set bitmask of requested interrupt events
 *
 * \param [in] id I2C controller instance
 * \param [in] mask bitmask of requested interrupt events
 *
 * \sa I2C_INT
 *
 */
void hw_i2c_set_int_mask(HW_I2C_ID id, uint16_t mask);

/**
 * \brief Get current bitmask of requested interrupt events
 *
 * \param [in] id I2C controller instance
 *
 * \return current bitmask
 *
 * \sa I2C_INT
 *
 */
uint16_t hw_i2c_get_int_mask(HW_I2C_ID id);

/**
 * \brief Set I2C controller mode
 *
 * Can be only set when controller is disabled.
 *
 * \param [in] id I2C controller instance
 * \param [in] mode mode to be set
 *
 */
__STATIC_INLINE void hw_i2c_set_mode(HW_I2C_ID id, HW_I2C_MODE mode)
{
        // default to master mode, if incorrect value specified
        HW_I2C_REG_SETF(id, I2C_CON, I2C_MASTER_MODE, (mode == HW_I2C_MODE_SLAVE) ? 0 : 1);
        HW_I2C_REG_SETF(id, I2C_CON, I2C_SLAVE_DISABLE, (mode == HW_I2C_MODE_SLAVE) ? 0 : 1);
}


/**
 * \brief Get I2C controller master mode status
 *
 * \param [in] id I2C controller instance
 *
 * \return true if controller in master mode
 *
 */
__STATIC_INLINE uint8_t hw_i2c_is_master(HW_I2C_ID id)
{
        return HW_I2C_REG_GETF(id, I2C_CON, I2C_MASTER_MODE);
}

/**
 * \brief Set I2C interface bus speed
 *
 * Can be only set when controller is disabled.
 *
 * \param [in] id I2C controller instance
 * \param [in] speed speed to be set
 *
 */
__STATIC_INLINE void hw_i2c_set_speed(HW_I2C_ID id, HW_I2C_SPEED speed)
{
        // default to standard mode (100kbit/s), if incorrect value specified
        switch (speed) {
        case HW_I2C_SPEED_HIGH:
                HW_I2C_REG_SETF(id, I2C_CON, I2C_SPEED, 3);
                break;
        case HW_I2C_SPEED_FAST:
                HW_I2C_REG_SETF(id, I2C_CON, I2C_SPEED, 2);
                break;
        case HW_I2C_SPEED_STANDARD:
        default:
                HW_I2C_REG_SETF(id, I2C_CON, I2C_SPEED, 1);
                break;
        }
}

/**
 * \brief Set whether RESTART conditions may be sent when acting as master
 *
 * \param [in] id I2C controller instance
 * \param [in] enabled RESTART status to be set
 *
 */
__STATIC_INLINE void hw_i2c_set_restart_enabled(HW_I2C_ID id, bool enabled)
{
        HW_I2C_REG_SETF(id, I2C_CON, I2C_RESTART_EN, !!enabled);
}

/**
 * \brief Set whether General Call should be used to address slaves
 *
 * Can only be changed when controller is disabled.
 *
 * \param [in] id I2C controller instance
 * \param [in] enabled General Call status to be set
 *
 */
__STATIC_INLINE void hw_i2c_set_general_call_enabled(HW_I2C_ID id, bool enabled)
{
        HW_I2C_REG_SETF(id, I2C_TAR, SPECIAL, !!enabled);
        HW_I2C_REG_SETF(id, I2C_TAR, GC_OR_START, !enabled);
}

/**
 * \brief Set target slave addressing mode in master mode
 *
 * Can only be changed when controller is disabled.
 *
 * \param [in] id I2C controller instance
 * \param [in] addr_mode mode of addressing
 *
 */
__STATIC_INLINE void hw_i2c_set_target_addressing_mode(HW_I2C_ID id, HW_I2C_ADDRESSING addr_mode)
{
        // default to 7b addressing, if incorrect value specified
        HW_I2C_REG_SETF(id, I2C_CON, I2C_10BITADDR_MASTER, (addr_mode == HW_I2C_ADDRESSING_10B) ? 1 : 0);
}

/**
 * \brief Set slave addressing mode in slave mode
 *
 * Can be only set when controller is disabled.
 *
 * \param [in] id I2C controller instance
 * \param [in] addr_mode mode of addressing
 *
 */
__STATIC_INLINE void hw_i2c_set_slave_addressing_mode(HW_I2C_ID id, HW_I2C_ADDRESSING addr_mode)
{
        // default to 7b addressing, if incorrect value specified
        HW_I2C_REG_SETF(id, I2C_CON, I2C_10BITADDR_SLAVE, (addr_mode == HW_I2C_ADDRESSING_10B) ? 1 : 0);
}

/**
 * \brief Set slave address in slave mode
 *
 * Can be only set when controller is disabled.
 *
 * \param [in] id I2C controller instance
 * \param [in] address slave address
 *
 */
__STATIC_INLINE void hw_i2c_set_slave_address(HW_I2C_ID id, uint16_t address)
{
        HW_I2C_REG_SETF(id, I2C_SAR, IC_SAR, address);
}

/**
 * \brief Set support for general call acknowledgment
 *
 * When enabled, controller will send ACK for general call address.
 * This applies only to controller working in slave mode.
 *
 * \param [in] id I2C controller instance
 * \param [in] ack acknowledgment status
 *
 */

__STATIC_INLINE void hw_i2c_set_general_call_ack_enabled(HW_I2C_ID id, bool ack)
{
        HW_I2C_REG_SETF(id, I2C_ACK_GENERAL_CALL, ACK_GEN_CALL, !!ack);
}

/**
 * \brief Setup controller for operation in master mode
 *
 * Shortcut for calling hw_i2c_set_mode(), hw_i2c_set_target_addressing_mode() and
 * hw_i2c_set_target_address().
 *
 * \param [in] id I2C controller instance
 * \param [in] addr_mode mode of addressing
 * \param [in] address target slave address
 *
 */
void hw_i2c_setup_master(HW_I2C_ID id, HW_I2C_ADDRESSING addr_mode, uint16_t address);

/**
 * \brief Setup callback function for operation in slave mode
 *
 * \param [in] id I2C controller instance
 * \param [in] cb callback for events
 *
 */
void hw_i2c_set_slave_callback(HW_I2C_ID id, hw_i2c_event_cb cb);

/**
 * \brief Register proper handling for DMA read in slave mode.
 *
 * This function must be called after DMA has been setup for reading in slave mode.
 *
 * \param [in] id I2C controller instance
 *
 * \sa hw_i2c_read_buffer_dma
 * \sa hw_i2c_prepare_dma
 *
 */
void hw_i2c_register_slave_dma_read_callback(HW_I2C_ID id);

/**
 * \brief Setup controller for operation in slave mode
 *
 * Shortcut for calling hw_i2c_set_mode(), hw_i2c_set_slave_addressing_mode() and
 * hw_i2c_set_slave_address().
 *
 * \param [in] id I2C controller instance
 * \param [in] addr_mode mode of addressing
 * \param [in] address slave address
 * \param [in] cb callback for events
 *
 * \sa hw_i2c_set_mode
 * \sa hw_i2c_set_slave_addressing_mode
 * \sa hw_i2c_set_slave_address
 *
 */
void hw_i2c_setup_slave(HW_I2C_ID id, HW_I2C_ADDRESSING addr_mode,
                        uint16_t address, hw_i2c_event_cb cb);

/**
 * \brief Check if controller is busy when operating in master mode
 *
 * \param [in] id I2C controller instance
 *
 * \return busy status
 *
 */
__STATIC_INLINE bool hw_i2c_is_master_busy(HW_I2C_ID id)
{
        return HW_I2C_REG_GETF(id, I2C_STATUS, MST_ACTIVITY);
}

/**
 * \brief Check if controller is busy when operating in slave mode
 *
 * \param [in] id I2C controller instance
 *
 * \return busy status
 *
 */
__STATIC_INLINE bool hw_i2c_is_slave_busy(HW_I2C_ID id)
{
        return HW_I2C_REG_GETF(id, I2C_STATUS, SLV_ACTIVITY);
}

/**
 * \brief Check controller activity
 *
 * \param [in] id I2C controller instance
 *
 * \return busy status
 *
 */
__STATIC_INLINE bool hw_i2c_controler_is_busy(HW_I2C_ID id)
{
        return HW_I2C_REG_GETF(id, I2C_STATUS, I2C_ACTIVITY);
}

/**
 * \brief Check if TX FIFO queue is empty
 *
 * This function should be used to check if all data written to TX FIFO were transmitted.
 *
 * \param [in] id I2C controller instance
 *
 * \return TX FIFO empty status
 *
 */
__STATIC_INLINE bool hw_i2c_is_tx_fifo_empty(HW_I2C_ID id)
{
        return HW_I2C_REG_GETF(id, I2C_STATUS, TFE);
}

/**
 * \brief Check if TX FIFO is not full
 *
 * This function should be used to check if data can be written to TX FIFO.
 *
 * \param [in] id I2C controller instance
 *
 * \return TX FIFO not full status
 *
 */
__STATIC_INLINE bool hw_i2c_is_tx_fifo_not_full(HW_I2C_ID id)
{
        return HW_I2C_REG_GETF(id, I2C_STATUS, TFNF);
}

/**
 * \brief Check if RX FIFO queue is full
 *
 * This function should be used to check if RX FIFO is filled, i.e. subsequent data read will be
 * discarded.
 *
 * \param [in] id I2C controller instance
 *
 * \return RX FIFO full status
 *
 */
__STATIC_INLINE bool hw_i2c_is_rx_fifo_full(HW_I2C_ID id)
{
        return HW_I2C_REG_GETF(id, I2C_STATUS, RFF);
}

/**
 * \brief Check if RX FIFO is not empty
 *
 * This function should be used to check if there are any data received in RX FIFO
 *
 * \param [in] id I2C controller instance
 *
 * \return RX FIFO not empty status
 *
 */
__STATIC_INLINE bool hw_i2c_is_rx_fifo_not_empty(HW_I2C_ID id)
{
        return HW_I2C_REG_GETF(id, I2C_STATUS, RFNE);
}

/**
 * \brief Set target slave address in master mode
 *
 * \param [in] id I2C controller instance
 * \param [in] address slave address
 *
 */
__STATIC_INLINE void hw_i2c_set_target_address(HW_I2C_ID id, uint16_t address)
{
        /* Wait for the master to become IDLE */
        while (hw_i2c_is_master_busy(id));

        /* Now is safe to disable the I2C to change the Target Address */
        hw_i2c_disable(id);

        /* Change the Target Address */
        HW_I2C_REG_SETF(id, I2C_TAR, IC_TAR, address);

        /* Enable again the I2C to use the new address */
        hw_i2c_enable(id);
}

/**
 * \brief Write single byte into TX FIFO
 *
 * It is caller's responsibility to ensure there is free space in TX FIFO before calling this
 * function - either hw_i2c_is_tx_fifo_not_full() or hw_i2c_get_tx_fifo_level() can be used for this
 * purpose.
 *
 * This function can be used in both master and slave modes.
 *
 * \param [in] id I2C controller instance
 * \param [in] byte data to write
 *
 * \warning This function does not check for errors during transmission. Use hw_i2c_write_buffer_sync()
 *          or hw_i2c_write_buffer_async() instead
 *
 */
__STATIC_INLINE void hw_i2c_write_byte(HW_I2C_ID id, uint8_t byte)
{
        IBA(id)->I2C_DATA_CMD_REG = byte & (I2C_I2C_DATA_CMD_REG_I2C_CMD_Msk | I2C_I2C_DATA_CMD_REG_I2C_DAT_Msk);
}

/**
 * \brief Write multiple bytes to I2C slave synchronously
 *
 * Writes array of data to a slave device on the I2C bus via the TX FIFO. This call blocks until
 * the operation completes. In case of failure the function stores the Tx error code to
 * the abrt_code parameter -if provided- and clears the Tx Abort register.
 *
 * \param [in]     id           I2C controller instance
 * \param [in]     data         Address of the buffer containing the data to write
 * \param [in]     len          Length of the data buffer
 * \param [in,out] abrt_code    If not NULL, the status of the operation based on ::HW_I2C_ABORT_SOURCE
 *                              enumeration will be stored here
 * \param [in] flags            Possible values for flags are:
 * \parblock
 *      ::HW_I2C_F_NONE<br>
 *      The function will return only when all bytes have been transmitted.
 *      Possible Tx failure after this point must be checked by the caller. This flag can be used
 *      for consecutive calls where the transaction can resume without stop/start conditions in
 *      between.
 *
 *      ::HW_I2C_F_WAIT_FOR_STOP<br>
 *      The function will return only when all bytes have been transmitted
 *      and a STOP condition has been generated.
 *
 *      ::HW_I2C_F_ADD_STOP<br>
 *      STOP condition will be generated at the end of the transaction. This flag has no effect in
 *      DA14680/1 since STOP condition is automatically generated at the end of the transaction.
 *
 *      ::HW_I2C_F_ADD_RESTART<br>
 *      RESTART condition will be generated at the beginning of the transaction.
 *      HW_I2C_F_ADD_RESTART is not supported in DA14680/1.
 * \endparblock
 *
 * \return The number of bytes written to the Tx FIFO. It is strongly recommended to check
 *         the value of abrt_code to make sure that the number of bytes returned were actually
 *         transmitted to the I2C bus.
 *
 * \note    This function should be used only when operating in master mode.
 *
 * \warning Only for DA14680/1: When calling this function under an OS, it is possible to have a
 *          Tx FIFO underrun which results in a STOP sequence, due to OS preemption.
 *
 */
size_t hw_i2c_write_buffer_sync(HW_I2C_ID id, const uint8_t *data, uint16_t len,
                                HW_I2C_ABORT_SOURCE *abrt_code, uint32_t flags);

/**
 * \brief Write multiple bytes to I2C slave asynchronously
 *
 * Writes array of data to a slave device on the I2C bus via the TX FIFO. This call is non-blocking
 * and specified callback is called upon completion, with the operation status. In case of failure
 * the failure cause must be read from the cb by calling hw_i2c_get_abort_source().
 *
 * \param [in] id       I2C controller instance
 * \param [in] data     Address of the buffer containing the data to write
 * \param [in] len      Length of the data buffer
 * \param [in] cb       Callback to be called upon completion
 * \param [in] cb_data  Data to pass to cb
 * \param [in] flags    Possible values for flags are:
 * \parblock
 *      ::HW_I2C_F_NONE<br>
 *      The cb will be called as soon as the last byte of data is written to the Tx FIFO. Possible
 *      Tx failure after this point must be checked by the caller. This flag can be used for
 *      consecutive calls where the transaction can resume without stop/start conditions in
 *      between.
 *
 *      ::HW_I2C_F_WAIT_FOR_STOP<br>
 *      The callback will be called when all bytes have been transmitted and a STOP condition has
 *      been generated.
 *
 *      ::HW_I2C_F_ADD_STOP<br>
 *      STOP condition will be generated at the end of the transaction. This flag has no effect in
 *      DA14680/1 since STOP condition is automatically generated at the end of the transaction.
 *
 *      ::HW_I2C_F_ADD_RESTART<br>
 *      RESTART condition will be generated at the beginning of the transaction.
 *      HW_I2C_F_ADD_RESTART is not supported in DA14680/1.
 * \endparblock
 *
 * \return 0 in case of success, -1 otherwise.
 *
 * \note    This function should be used only when operating in master mode.
 *
 * \warning This function registers an internal interrupt handler, which overrides any previously
 *          installed handler.
 *          Only for DA14680/1: When calling this function under an OS, it is possible to have a
 *          Tx FIFO underrun which results in a STOP sequence, due to OS preemption.
 *
 * \sa hw_i2c_register_int
 *
 */
int hw_i2c_write_buffer_async(HW_I2C_ID id, const uint8_t *data, uint16_t len,
                              hw_i2c_complete_cb cb, void *cb_data, uint32_t flags);

/**
 * \brief Initiate reading from I2C bus
 *
 * No data is read via this call, only START/RESTART condition is generated on bus if required.
 * Actual data is read by controller and put in RX FIFO which can be read using hw_i2c_read_byte().
 *
 * This function should be only used when operating in master mode.
 *
 * \param [in] id I2C controller instance
 *
 * \sa hw_i2c_read_byte
 *
 */
__STATIC_INLINE void hw_i2c_read_byte_trigger(HW_I2C_ID id)
{
        IBA(id)->I2C_DATA_CMD_REG = I2C_I2C_DATA_CMD_REG_I2C_CMD_Msk;
}

/**
 * \brief Read multiple bytes from I2C slave synchronously
 *
 * This function will read multiple bytes from bus taking care of RX FIFO control. This call blocks
 * until the operation completes. The operation completes when the complete buffer is filled, or in
 * case of a failure. Failures are cleared by the function before returning.
 *
 * \param [in]      id           I2C controller
 * \param [in, out] data         Address of the buffer where data are stored
 * \param [in]      len          Length of the data buffer
 * \param [in,out]  abrt_code    If not NULL, the status of the operation based on ::HW_I2C_ABORT_SOURCE
 *                               enumeration will be stored here
 * \param [in] flags             Possible values for flags are:
 * \parblock
 *      ::HW_I2C_F_NONE<br>
 *      This flag can be used for consecutive calls where the transaction can resume without
 *      stop/start conditions in between.
 *
 *      ::HW_I2C_F_ADD_STOP<br>
 *      STOP condition will be generated at the end of the transaction. This flag has no effect in
 *      DA14680/1 since STOP condition is automatically generated at the end of the transaction.
 *
 *      ::HW_I2C_F_ADD_RESTART<br>
 *      RESTART condition will be generated at the beginning of the transaction.
 *      HW_I2C_F_ADD_RESTART is not supported in DA14680/1.
 * \endparblock
 *
 * \return The number of bytes read
 *
 * \note This function should be used only when operating in master mode.
 *
 * \warning Only for DA14680/1: When calling this function under an OS, it is possible to have a
 *          Tx FIFO underrun which results in a STOP sequence, due to OS preemption.
 *
 */
size_t hw_i2c_read_buffer_sync(HW_I2C_ID id, uint8_t *data, uint16_t len,
                               HW_I2C_ABORT_SOURCE *abrt_code, uint32_t flags);

/**
 * \brief Read multiple bytes from I2C slave asynchronously
 *
 * This function will read multiple bytes from bus taking care of RX FIFO control. This call is
 * non-blocking and specified callback is called upon completion, with the operation status.
 * The operation completes when the complete buffer is filled, or in case of a failure.
 * In case of failure the failure cause must be read from the cb by calling
 * hw_i2c_get_abort_source().
 *
 * \param [in] id I2C controller
 * \param [in, out] data Address of the buffer where data are stored
 * \param [in] len Length of the data buffer
 * \param [in] cb Callback to be called upon completion
 * \param [in] cb_data Data to pass to cb
 * \param [in] flags    Possible values for flags are:
 * \parblock
 *      ::HW_I2C_F_NONE<br>
 *      This flag can be used for consecutive calls where the transaction can resume without
 *      stop/start conditions in between.
 *
 *      ::HW_I2C_F_ADD_STOP<br>
 *      STOP condition will be generated at the end of the transaction. This flag has no effect in
 *      DA14680/1 since STOP condition is automatically generated at the end of the transaction.
 *
 *      ::HW_I2C_F_ADD_RESTART<br>
 *      RESTART condition will be generated at the beginning of the transaction.
 *      HW_I2C_F_ADD_RESTART is not supported in DA14680/1.
 * \endparblock
 *
 * \return 0 in case of success, -1 otherwise.
 *
 * \note This function should be used only when operating in master mode.
 *
 * \warning This function registers an internal interrupt handler, which overrides any previously
 *          installed handler.
 *          Only for DA14680/1: When calling this function under an OS, it is possible to have a
 *          Tx FIFO underrun which results in a STOP sequence, due to OS preemption.
 *
 * \sa hw_i2c_register_int
 *
 */
int hw_i2c_read_buffer_async(HW_I2C_ID id, uint8_t *data, uint16_t len,
                             hw_i2c_complete_cb cb, void *cb_data, uint32_t flags);

/**
 * \brief Write then read multiple bytes from I2C slave.
 *
 * This function allows to perform typical I2C transaction.
 * This call is non-blocking and specified callback is called upon completion, with the operation status.
 * In case of failure the failure cause must be read from the cb by calling hw_i2c_get_abort_source().
 *
 * \param [in]  id              I2C controller
 * \param [in]  w_data          Address of the buffer containing the data to write
 * \param [in]  w_len           Length of the buffer containing the data to write
 * \param [in, out] r_data      Address of the buffer where data will be stored
 * \param [in]  r_len           Length of the buffer where data will be stored
 * \param [in]  cb              Callback to be called upon completion
 * \param [in]  cb_data         Data to pass to cb
 * \param [in] flags            Possible values for flags are:
 * \parblock
 *      ::HW_I2C_F_NONE<br>
 *      This flag can be used for consecutive calls where the transaction can resume without
 *      stop/start conditions in between.
 *
 *      ::HW_I2C_F_ADD_STOP<br>
 *      STOP condition will be generated at the end of the transaction. This flag has no effect in
 *      DA14680/1 since STOP condition is automatically generated at the end of the transaction.
 *
 *      ::HW_I2C_F_ADD_RESTART<br>
 *      RESTART condition will be generated at the beginning of the transaction.
 *      HW_I2C_F_ADD_RESTART is not supported in DA14680/1.
 * \endparblock
 *
 * \return 0 in case of success, -1 otherwise.
 *
 * \note This function should be used only when operating in master mode.
 *
 * \warning This function registers an internal interrupt handler, which overrides any previously
 *          installed handler.
 *          Only for DA14680/1: When calling this function under an OS, it is possible to have a
 *          Tx FIFO underrun which results in a STOP sequence, due to OS preemption.
 *
 * \sa hw_i2c_register_int
 *
 */
int hw_i2c_write_then_read_async(HW_I2C_ID id, const uint8_t *w_data, uint16_t w_len,
                                        uint8_t *r_data, uint16_t r_len, hw_i2c_complete_cb cb,
                                        void *cb_data, uint32_t flags);

/**
 * \brief Read single byte from RX FIFO
 *
 * It is caller's responsibility to ensure there is data to read in RX FIFO before calling this
 * function by checking either hw_i2c_get_rx_fifo_level() or hw_i2c_rx_fifo_not_empty().
 *
 * This function can be used in both master and slave modes.
 *
 * \param [in] id I2C controller instance
 *
 * \return read byte
 *
 * \sa hw_i2c_get_rx_fifo_level
 * \sa hw_i2c_rx_fifo_not_empty
 *
 */
__STATIC_INLINE uint8_t hw_i2c_read_byte(HW_I2C_ID id)
{
        return HW_I2C_REG_GETF(id, I2C_DATA_CMD, I2C_DAT);
}

/**
 * \brief Set threshold level on TX FIFO
 *
 * An interrupt will be generated once number of entries in TX FIFO is less or equal to \p level.
 * This cannot be set to value greater than HW_I2C_FIFO_DEPTH.
 *
 * \param [in] id I2C controller instance
 * \param [in] level threshold
 *
 */
__STATIC_INLINE void hw_i2c_set_tx_fifo_threshold(HW_I2C_ID id, uint8_t level)
{
        HW_I2C_REG_SETF(id, I2C_TX_TL, TX_TL, level);
}

/**
 * \brief Set threshold level on RX FIFO
 *
 * An interrupt will be generated once number of entries in RX FIFO is greater than \p level.
 * This cannot be set to value greater than HW_I2C_FIFO_DEPTH.
 *
 * \param [in] id I2C controller instance
 * \param [in] level threshold
 *
 */
__STATIC_INLINE void hw_i2c_set_rx_fifo_threshold(HW_I2C_ID id, uint8_t level)
{
        HW_I2C_REG_SETF(id, I2C_RX_TL, RX_TL, level);
}

/**
 * \brief Get threshold level on TX FIFO
 *
 * \param [in] id I2C controller instance
 *
 */
__STATIC_INLINE uint8_t hw_i2c_get_tx_fifo_threshold(HW_I2C_ID id)
{
        return HW_I2C_REG_GETF(id, I2C_TX_TL, TX_TL);
}

/**
 * \brief Get threshold level on RX FIFO
 *
 * \param [in] id I2C controller instance
 *
 */
__STATIC_INLINE uint8_t hw_i2c_get_rx_fifo_threshold(HW_I2C_ID id)
{
        return HW_I2C_REG_GETF(id, I2C_RX_TL, RX_TL);
}

/**
 * \brief Get number of bytes in TX FIFO
 *
 * \param [in] id I2C controller instance
 *
 * \return number of bytes
 *
 */
__STATIC_INLINE uint8_t hw_i2c_get_tx_fifo_level(HW_I2C_ID id)
{
        return HW_I2C_REG_GETF(id, I2C_TXFLR, TXFLR);
}

/**
 * \brief Get number of bytes in RX FIFO
 *
 * \param [in] id I2C controller instance

 * \return number of bytes
 *
 */
__STATIC_INLINE uint8_t hw_i2c_get_rx_fifo_level(HW_I2C_ID id)
{
        return HW_I2C_REG_GETF(id, I2C_RXFLR, RXFLR);
}

/**
 * \brief Get interrupt state
 *
 * Interrupt state returned includes only interrupts which are not masked. For raw interrupt status
 * use hw_i2c_get_raw_int_state().
 *
 * \param [in] id I2C controller instance

 * \return interrupt state bitmask
 *
 * \sa I2C_INT
 * \sa hw_i2c_get_raw_int_state
 * \sa hw_i2c_set_intr_mask
 *
 */
__STATIC_INLINE uint16_t hw_i2c_get_int_state(HW_I2C_ID id)
{
        return IBA(id)->I2C_INTR_STAT_REG;
}

/**
 * \brief Get raw interrupt state
 *
 * \param [in] id I2C controller instance
 *
 * \return interrupt state bitmask
 *
 * \sa I2C_INT
 * \sa hw_i2c_get_int_state
 *
 */
__STATIC_INLINE uint16_t hw_i2c_get_raw_int_state(HW_I2C_ID id)
{
        return IBA(id)->I2C_RAW_INTR_STAT_REG;
}

/**
 * \brief Reset all interrupt state
 *
 * This does reset all interrupts which can be reset by software and TX_ABORT status.
 *
 * \warning Although this also clears TX_ABORT it does not reset flushed state on TX FIFO. This has
 * \warning to be cleared manually using hw_i2c_reset_abort_source().
 *
 * \param [in] id I2C controller instance
 *
 * \sa hw_i2c_reset_abort_source
 *
 */
__STATIC_INLINE void hw_i2c_reset_int_all(HW_I2C_ID id)
{
        (void) IBA(id)->I2C_CLR_INTR_REG;
}

/**
 * \brief Reset RX_UNDERFLOW interrupt state
 *
 * Should be used to reset RX_UNDERFLOW interrupt.
 *
 * \param [in] id I2C controller instance
 *
 */
__STATIC_INLINE void hw_i2c_reset_int_rx_underflow(HW_I2C_ID id)
{
        (void) IBA(id)->I2C_CLR_RX_UNDER_REG;
}

/**
 * \brief Reset RX_OVERFLOW interrupt state
 *
 * Should be used to reset RX_OVERFLOW interrupt.
 *
 * \param [in] id I2C controller instance
 *
 */
__STATIC_INLINE void hw_i2c_reset_int_rx_overflow(HW_I2C_ID id)
{
        (void) IBA(id)->I2C_CLR_RX_OVER_REG;
}

/**
 * \brief Reset TX_OVERFLOW interrupt state
 *
 * Should be used to reset TX_OVERFLOW interrupt.
 *
 * \param [in] id I2C controller instance
 *
 */
__STATIC_INLINE void hw_i2c_reset_int_tx_overflow(HW_I2C_ID id)
{
        (void) IBA(id)->I2C_CLR_TX_OVER_REG;
}

/**
 * \brief Reset READ_REQUEST interrupt state
 *
 * Should be used to reset READ_REQUEST interrupt.
 *
 * \param [in] id I2C controller instance
 *
 */
__STATIC_INLINE void hw_i2c_reset_int_read_request(HW_I2C_ID id)
{
        (void) IBA(id)->I2C_CLR_RD_REQ_REG;
}

/**
 * \brief Reset TX_ABORT interrupt state
 *
 * Should be used to reset TX_ABORT interrupt.
 *
 * \param [in] id I2C controller instance
 *
 */
__STATIC_INLINE void hw_i2c_reset_int_tx_abort(HW_I2C_ID id)
{
        (void) IBA(id)->I2C_CLR_TX_ABRT_REG;
}

/**
 * \brief Reset RX_DONE interrupt state
 *
 * Should be used to reset RX_DONE interrupt.
 *
 * \param [in] id I2C controller instance
 *
 */
__STATIC_INLINE void hw_i2c_reset_int_rx_done(HW_I2C_ID id)
{
        (void) IBA(id)->I2C_CLR_RX_DONE_REG;
}

/**
 * \brief Reset ACTIVITY interrupt state
 *
 * Should be used to reset ACTIVITY interrupt.
 *
 * \param [in] id I2C controller instance
 *
 */
__STATIC_INLINE void hw_i2c_reset_int_activity(HW_I2C_ID id)
{
        (void) IBA(id)->I2C_CLR_ACTIVITY_REG;
}

/**
 * \brief Reset START_DETECTED interrupt state
 *
 * Should be used to reset START_DETECTED interrupt.
 *
 * \param [in] id I2C controller instance
 *
 */
__STATIC_INLINE void hw_i2c_reset_int_start_detected(HW_I2C_ID id)
{
        (void) IBA(id)->I2C_CLR_START_DET_REG;
}

/**
 * \brief Reset STOP_DETECTED interrupt state
 *
 * Should be used to reset STOP_DETECTED interrupt.
 *
 * \param [in] id I2C controller instance
 *
 */
__STATIC_INLINE void hw_i2c_reset_int_stop_detected(HW_I2C_ID id)
{
        (void) IBA(id)->I2C_CLR_STOP_DET_REG;
}

/**
 * \brief Reset GENERAL_CALL interrupt state
 *
 * Should be used to reset GENERAL_CALL interrupt.
 *
 * \param [in] id I2C controller instance
 *
 */
__STATIC_INLINE void hw_i2c_reset_int_gen_call(HW_I2C_ID id)
{
        (void) IBA(id)->I2C_CLR_GEN_CALL_REG;
}

/**
 * \brief Get abort source
 *
 * This can be used to retrieve source of TX_ABORT interrupt. TX FIFO is flushed and
 * remains in this state until cleared using hw_i2c_reset_abort_source().
 *
 * \param [in] id I2C controller instance
 *
 * \return abort source bitmask
 *
 * \sa I2C_ABORT_SOURCE
 * \sa hw_i2c_reset_abort_source
 *
 */
__STATIC_INLINE uint16_t hw_i2c_get_abort_source(HW_I2C_ID id)
{
        return IBA(id)->I2C_TX_ABRT_SOURCE_REG & 0x1FFFF;
}

/**
 * \brief Reset abort source
 *
 * This clears TX_ABORT interrupt status and unlocks TX FIFO.
 *
 * \note this is an alias for hw_i2c_reset_int_tx_abort()
 *
 * \param [in] id I2C controller instance
 *
 * \sa I2C_ABORT_SOURCE
 * \sa hw_i2c_reset_int_tx_abort
 *
 */
__STATIC_INLINE void hw_i2c_reset_abort_source(HW_I2C_ID id)
{
        hw_i2c_reset_int_tx_abort(id);
}

/**
 * \brief Aborts I2C transfer
 *
 * This forces master to issue a STOP command and flush the TX fifo.
 *
 * \param [in] id I2C controller instance
 *
 * \warning Can be used only if controller is in master mode
 *
 */
__STATIC_INLINE void hw_i2c_master_abort_transfer(HW_I2C_ID id)
{
        if (hw_i2c_is_master(id)) {
                HW_I2C_REG_SETF(id, I2C_ENABLE, I2C_ABORT, 1);
        }
}


/**
 * \brief Prepares I2C DMA for transfer
 *
 * This "extended functionality" variant of hw_i2c_prepare_dma() adds the \p notify_on_stop
 * parameter.
 *
 * Use hw_i2c_dma_start() to start actual data transfer.
 *
 * \p channel specifies either of channels in RX/TX pair used for transfer, i.e. channel=0|1 means
 * channels 0 and 1 will be used, channel=2|3 means channels 2 and 3 will be used and so on.
 * Once DMA is prepared, no other application should make changes to either of channels.
 *
 * \p data buffer elements are 16-bit wide, this high byte should be 0 for writing and discarded
 * when reading.
 *
 * Callback is called once DMA transfer between buffer and RX/TX FIFOs is completed which means that
 * there could still be activity on I2C bus. Application can check when transfer is completed using
 * other means, i.e. STOP_DETECTED interrupt.
 *
 * \param [in] id I2C controller instance
 * \param [in] channel DMA channel
 * \param [in] data buffer to read from or write to, depends on \p type
 * \param [in] len length of \p buffer
 * \param [in] type type of transfer
 * \param [in] cb callback for transfer completed
 * \param [in] cb_data data to pass to \p cb
 * \param [in] flags Possible values for flags are:
 * \parblock
 *      ::HW_I2C_F_NONE<br>
 *      This flag can be used for consecutive calls where the transaction can resume without
 *      stop/start conditions in between.
 *
 *      ::HW_I2C_F_WAIT_FOR_STOP<br>
 *       \p cb will be called when STOP condition is generated. STOP condition will be generated at
 *       the end of the transaction. If HW_I2C_F_WAIT_FOR_STOP flag is not set \p cb will be
 *       called when the last byte has been written to the Tx FIFO.
 *
 *      ::HW_I2C_F_ADD_STOP<br>
 *      STOP condition will be generated at the end of the transaction. This flag has no effect in
 *      DA14680/1 since STOP condition is automatically generated at the end of the transaction.
 *
 *      ::HW_I2C_F_ADD_RESTART<br>
 *      RESTART condition will be generated at the beginning of the transaction.
 *      HW_I2C_F_ADD_RESTART is not supported in DA14680/1.
 * \endparblock
 *
 * \sa hw_i2c_dma_start
 *
 */
void hw_i2c_prepare_dma(HW_I2C_ID id, uint8_t channel, void *data, uint16_t len,
                HW_I2C_DMA_TRANSFER type, hw_i2c_complete_cb cb, void *cb_data,
                uint32_t flags);


/**
 * \brief Starts DMA transfer
 *
 * Should be called once I2C DMA is setup using hw_i2c_prepare_dma(). Once started, DMA transfer
 * will only finish once previously specified number of bytes is read or written.
 *
 * \param [in] id I2C controller instance
 *
 * \sa hw_i2c_prepare_dma
 *
 */
void hw_i2c_dma_start(HW_I2C_ID id);

/**
 * \brief Write multiple bytes on I2C bus using DMA
 *
 * Shortcut for calling hw_i2c_prepare_dma() and hw_i2c_dma_start().
 *
 * \param [in] id I2C controller instance
 * \param [in] channel DMA channel
 * \param [in] data buffer to write data to
 * \param [in] len length of \p buffer
 * \param [in] cb callback for transfer completed
 * \param [in] cb_data data to pass to \p cb
 * \param [in] flags Possible values for flags are:
 * \parblock
 *      ::HW_I2C_F_NONE<br>
 *      This flag can be used for consecutive calls where the transaction can resume without
 *      stop/start conditions in between.
 *
 *      ::HW_I2C_F_WAIT_FOR_STOP<br>
 *       \p cb will be called when STOP condition is generated. STOP condition will be generated at
 *       the end of the transaction. If HW_I2C_F_WAIT_FOR_STOP flag is not set \p cb will be
 *       called when the last byte has been written to the Tx FIFO.
 *
 *      ::HW_I2C_F_ADD_STOP<br>
 *      STOP condition will be generated at the end of the transaction. This flag has no effect in
 *      DA14680/1 since STOP condition is automatically generated at the end of the transaction.
 *
 *      ::HW_I2C_F_ADD_RESTART<br>
 *      RESTART condition will be generated at the beginning of the transaction.
 *      HW_I2C_F_ADD_RESTART is not supported in DA14680/1.
 * \endparblock
 *
 * \sa hw_i2c_prepare_dma
 * \sa hw_i2c_dma_start
 *
 */
void hw_i2c_write_buffer_dma(HW_I2C_ID id, uint8_t channel, const uint16_t *data, uint16_t len,
                                          hw_i2c_complete_cb cb, void *cb_data, uint32_t flags);


/**
 * \brief Read multiple bytes from I2C bus using DMA
 *
 * This function uses the common callback type, which provides a way to signal whether the read
 * finished successfully.
 *
 * Shortcut for calling hw_i2c_prepare_dma() and hw_i2c_dma_start().
 *
 * \param [in] id I2C controller instance
 * \param [in] channel DMA channel
 * \param [out] data buffer to put data read from I2C bus
 * \param [in] len length of \p buffer
 * \param [in] cb callback for transfer completed
 * \param [in] cb_data data to pass to \p cb
 * \param [in] flags Possible values for flags are:
 * \parblock
 *      ::HW_I2C_F_NONE<br>
 *      This flag can be used for consecutive calls where the transaction can resume without
 *      stop/start conditions in between.
 *
 *      ::HW_I2C_F_ADD_STOP<br>
 *      STOP condition will be generated at the end of the transaction. This flag has no effect in
 *      DA14680/1 since STOP condition is automatically generated at the end of the transaction.
 *
 *      ::HW_I2C_F_ADD_RESTART<br>
 *      RESTART condition will be generated at the beginning of the transaction.
 *      HW_I2C_F_ADD_RESTART is not supported in DA14680/1.
 * \endparblock
 *
 * \sa hw_i2c_prepare_dma
 * \sa hw_i2c_dma_start
 *
 */
void hw_i2c_read_buffer_dma(HW_I2C_ID id, uint8_t channel, uint8_t *data, uint16_t len,
                                   hw_i2c_complete_cb cb, void *cb_data, uint32_t flags);


#endif /* dg_configUSE_HW_I2C */
#endif /* HW_I2C_H_ */

/**
 * \}
 * \}
 */
