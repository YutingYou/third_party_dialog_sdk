/**
 * \addtogroup PLA_DRI_PER_COMM
 * \{
 * \addtogroup HW_UART UART 1/2/3 Driver
 * \{
 * \brief UART Controller
 */

/**
 ****************************************************************************************
 *
 * @file hw_uart.h
 *
 * @brief Definition of API for the UART Low Level Driver.
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_UART_H_
#define HW_UART_H_

#if dg_configUSE_HW_UART

#include <stdbool.h>
#include <stdint.h>
#include <sdk_defs.h>
#include "hw_dma.h"

#ifndef HW_UART_USE_DMA_SUPPORT
#  define HW_UART_USE_DMA_SUPPORT	1
#endif

#define UBA(id)         ((UART2_Type *)id)
#define HW_UART1        ((uint16_t *)UART_BASE)
#define HW_UART2        ((uint16_t *)UART2_BASE)
#define HW_UART3        ((uint16_t *)UART3_BASE)
typedef uint16_t * HW_UART_ID;


/**
 * \brief Get the mask of a field of a UART register.
 *
 * \param [in] instance identifies the UART instance to use (empty: UART, 2: UART2)
 * \param [in] reg is the register to access
 * \param [in] field is the register field to access
 *
 * \note The stripped register name should be provided, e.g.:
 *        - to get UART_UART_USR_REG_UART_BUSY_Msk, use
 *              HW_UART_REG_FIELD_MASK(, USR, UART_BUSY)
 *        - to get UART2_UART2_SFE_REG_UART_SHADOW_FIFO_ENABLE_Msk, use
 *              HW_UART_REG_FIELD_MASK(2, SFE, UART_SHADOW_FIFO_ENABLE)
 *
 */
#define HW_UART_REG_FIELD_MASK(instance, reg, field) \
        (UART##instance##_UART##instance##_##reg##_REG_##field##_Msk)

/**
 * \brief Get the bit position of a field of a UART register.
 *
 * \param [in] instance identifies the UART instance to use (empty: UART, 2: UART2)
 * \param [in] reg is the register to access
 * \param [in] field is the register field to access
 *
 * \note The stripped register name should be provided, e.g.:
 *        - to get UART_UART_USR_REG_UART_BUSY_Pos, use
 *              HW_UART_REG_FIELD_POS(, USR, UART_BUSY)
 *        - to get UART2_UART2_SFE_REG_UART_SHADOW_FIFO_ENABLE_Pos, use
 *              HW_UART_REG_FIELD_POS(2, SFE, UART_SHADOW_FIFO_ENABLE)
 *
 */
#define HW_UART_REG_FIELD_POS(instance, reg, field) \
        (UART##instance##_UART##instance##_##reg##_REG_##field##_Pos)

/**
 * \brief Get the value of a field of a UART register.
 *
 * \param [in] id identifies UART to use
 * \param [in] reg is the register to access
 * \param [in] field is the register field to write
 *
 * \return the value of the register field
 *
 */
#define HW_UART_REG_GETF(id, reg, field) \
                ((UBA(id)->UART2_##reg##_REG & (UART2_UART2_##reg##_REG_##field##_Msk)) >> (UART2_UART2_##reg##_REG_##field##_Pos))

/**
 * \brief Set the value of a field of a UART register.
 *
 * \param [in] id identifies UART to use
 * \param [in] reg is the register to access
 * \param [in] field is the register field to write
 * \param [in] new_val is the value to write
 *
 */
#define HW_UART_REG_SETF(id, reg, field, new_val) \
                UBA(id)->UART2_##reg##_REG = ((UBA(id)->UART2_##reg##_REG & ~(UART2_UART2_##reg##_REG_##field##_Msk)) | \
                ((UART2_UART2_##reg##_REG_##field##_Msk) & ((new_val) << (UART2_UART2_##reg##_REG_##field##_Pos))))



/*
 * ENUMERATION DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief Interrupt Identification codes
 *
 */
typedef enum {
        HW_UART_INT_MODEM_STAT         = 0,
        HW_UART_INT_NO_INT_PEND        = 1,
        HW_UART_INT_THR_EMPTY          = 2,
        HW_UART_INT_RECEIVED_AVAILABLE = 4,
        HW_UART_INT_RECEIVE_LINE_STAT  = 6,
        HW_UART_INT_BUSY_DETECTED      = 7,
        HW_UART_INT_TIMEOUT            = 12,
} HW_UART_INT;

/**
 * \brief Baud rates dividers
 *
 * The defined values comprise the values of 3 registers: DLH, DLL, DLF.
 * The encoding of the values for each register is:
 *
 * +--------+--------+--------+--------+
 * | unused |   DLH  |   DLL  |   DLF  |
 * +--------+--------+--------+--------+
 *
 */
typedef enum {
        HW_UART_BAUDRATE_1000000   = 0x00000200,
        HW_UART_BAUDRATE_500000    = 0x00000400,
        HW_UART_BAUDRATE_230400    = 0x0000080b,
        HW_UART_BAUDRATE_115200    = 0x00001106,
        HW_UART_BAUDRATE_57600     = 0x0000220c,
        HW_UART_BAUDRATE_38400     = 0x00003401,
        HW_UART_BAUDRATE_28800     = 0x00004507,
        HW_UART_BAUDRATE_19200     = 0x00006803,
        HW_UART_BAUDRATE_14400     = 0x00008a0e,
        HW_UART_BAUDRATE_9600      = 0x0000d005,
        HW_UART_BAUDRATE_4800      = 0x0001a00b,
} HW_UART_BAUDRATE;

/**
 * \brief Character format
 *
 */
typedef enum {
        HW_UART_DATABITS_5        = 0,
        HW_UART_DATABITS_6        = 1,
        HW_UART_DATABITS_7        = 2,
        HW_UART_DATABITS_8        = 3,
} HW_UART_DATABITS;

/**
 * \brief Parity
 *
 */
typedef enum {
        HW_UART_PARITY_NONE     = 0,
        HW_UART_PARITY_ODD      = 1,
        HW_UART_PARITY_EVEN     = 3,
} HW_UART_PARITY;

/**
 * \brief Stop bits
 *
 */
typedef enum {
        HW_UART_STOPBITS_1 = 0,
        /**< The number of stop bits is 1.5 if a character format with 5 bit is chosen */
        HW_UART_STOPBITS_2 = 1,         /**< Stop bit 2 */
} HW_UART_STOPBITS;

/**
 * \brief Uart errors
 *
 */
typedef enum {
        HW_UART_ERR_NOERROR     = 0,
        HW_UART_ERR_OE          = 1,             /**< Overrun error */
        HW_UART_ERR_PE          = 2,             /**< Parity error */
        HW_UART_ERR_FE          = 3,             /**< Framing error */
        HW_UART_ERR_BI          = 4,             /**< Break interrupt indication*/
        HW_UART_ERR_RFE         = 5,             /**< Receive FIFO error */
} HW_UART_ERROR;

/**
 * \brief Configuration errors
 *
 */
typedef enum {
        HW_UART_CONFIG_ERR_NOERR        = 0,
        HW_UART_CONFIG_ERR_RX_FIFO      = 1,         /**< RX FIFO level different than RX burst mode*/
        HW_UART_CONFIG_ERR_TX_FIFO      = 2,         /**< TX FIFO level different than TX burst mode */
        HW_UART_CONFIG_ERR_RXTX_FIFO    = 3,         /**< RX and TX FIFO level different than TX RX burst mode */
        HW_UART_CONFIG_ERR_RX_SIZE      = 4,         /**< Received buffer size is not aligned with burst mode */
        HW_UART_CONFIG_ERR_TX_SIZE      = 5,         /**< Transmitted buffer size is not aligned with burst mode */
} HW_UART_CONFIG_ERR;

/**
 * \brief UART configuration structure definition
 *
 */
typedef struct {
        // Baud rate divisor
        HW_UART_BAUDRATE        baud_rate;
        HW_UART_DATABITS        data:2;
        HW_UART_PARITY          parity:2;
        HW_UART_STOPBITS        stop:1;
        uint8_t                 auto_flow_control:1;
        uint8_t                 use_dma:1;
        uint8_t                 use_fifo:1;

        HW_DMA_CHANNEL          tx_dma_channel:4;
        HW_DMA_CHANNEL          rx_dma_channel:4;
} uart_config;

/**
 * \brief UART configuration structure definition (extended version)
 *
 */
typedef struct {
        // Baud rate divisor
        HW_UART_BAUDRATE        baud_rate;
        HW_UART_DATABITS        data:2;
        HW_UART_PARITY          parity:2;
        HW_UART_STOPBITS        stop:1;
        uint8_t                 auto_flow_control:1;
        uint8_t                 use_dma:1;
        uint8_t                 use_fifo:1;
        uint8_t                 tx_fifo_tr_lvl:2;
        uint8_t                 rx_fifo_tr_lvl:2;
        uint8_t                 tx_dma_burst_lvl:2; /* 0 no burst mode, 1 burst size of 4, 2 burst size of 8 */
        uint8_t                 rx_dma_burst_lvl:2; /* 0 no burst mode, 1 burst size of 4, 2 burst size of 8 */
        HW_DMA_CHANNEL          tx_dma_channel:4;
        HW_DMA_CHANNEL          rx_dma_channel:4;
} uart_config_ex;

#ifdef HW_UART_ENABLE_USER_ISR
/**
 * \brief User defined interrupt function
 *
 * User code does not need to handle interrupts. Interrupts are handled by the driver.
 * If for some reason the user needs to handle interrupts differently, it is possible to
 * set a different ISR via hw_uart_set_isr(uart_id, user_isr).
 * In that case, the user-defined ISR must handle all UART interrupts as required.
 *
 * \sa hw_uart_set_isr
 *
 */
typedef void (*hw_uart_interrupt_isr)(void);

/**
 * \brief Setup user defined interrupt function for uart
 *
 * \param [in] uart identifies UART
 * \param [in] isr user defined interrupt handler
 *
 */
void hw_uart_set_isr(HW_UART_ID uart, hw_uart_interrupt_isr isr);
#endif

typedef void (*hw_uart_tx_callback)(void *user_data, uint16_t written);
typedef void (*hw_uart_rx_callback)(void *user_data, uint16_t read);
typedef void (*hw_uart_err_callback)(void *user_data, HW_UART_ERROR error);

//======================= Status functions ====================================

/**
 * \brief Check if a serial transfer is in progress
 *
 * \return true or false
 */
__STATIC_INLINE bool hw_uart_is_busy(HW_UART_ID uart)
{
        return HW_UART_REG_GETF(uart, USR, UART_BUSY);
}

//===================== Read/Write functions ===================================

/**
 * \brief Read one byte from UART
 *
 * This is a blocking function to read single byte from UART.
 * UART must be enabled before.
 *
 * \param [in] uart identifies UART to use
 *
 * \return the received byte
 *
 */
uint8_t hw_uart_read(HW_UART_ID uart);

/**
 * \brief Write one byte to UART
 *
 * This function waits till output FIFO has empty space according to
 * threshold level, and puts byte to be transmitted.
 *
 * \param [in] uart identifies UART to use
 * \param [in] data byte to be written
 *
 */
void hw_uart_write(HW_UART_ID uart, uint8_t data);

/**
 * \brief Stop asynchronous read from UART
 *
 * If there is outstanding read on given UART, it will be stopped.
 * Callback function will be fired with data already present in buffer.
 * Callback may be called from interrupt in the meantime but will be called
 * only once.
 *
 * \param [in] uart identifies UART to use
 *
 * \return number of bytes actually received
 *
 */
uint16_t hw_uart_abort_receive(HW_UART_ID uart);

/**
 * \brief Get number of bytes currently received by asynchronous read
 *
 * If there is outstanding read on given UART, the function returns the number of
 * bytes currently received.
 *
 * \param [in] uart identifies UART to use
 *
 */
uint16_t hw_uart_peek_received(HW_UART_ID uart);

/**
 * \brief Get number of bytes currently sent by asynchronous read
 *
 * If there is outstanding sending on given UART, the function returns  the number of
 * bytes currently sent.
 *
 * \param [in] uart identifies UART to use
 *
 */
uint16_t hw_uart_peek_transmitted(HW_UART_ID uart);

/**
 * \brief Read receive buffer register
 *
 * \param [in] uart identifies UART to use
 *
 * \return the read byte
 *
 */
__STATIC_INLINE uint8_t hw_uart_rxdata_getf(HW_UART_ID uart)
{
        // Read element from the receive FIFO
        return UBA(uart)->UART2_RBR_THR_DLL_REG;
}

/**
 * \brief Write byte to the transmit holding register
 *
 * \param [in] uart identifies UART to use
 * \param [in] data byte to be written
 *
 */
__STATIC_INLINE void hw_uart_txdata_setf(HW_UART_ID uart, uint8_t data)
{
        // Write data to the transmit FIFO
        UBA(uart)->UART2_RBR_THR_DLL_REG = data;
}

/**
 * \brief Writes number of bytes to UART synchronously
 *
 * This function finishes when all data is put in FIFO. It does not wait for data to be transmitted
 * out from UART. Call \sa hw_uart_is_tx_fifo_empty() to wait for transmission to finish.
 * This function does not use interrupts, nor DMA.
 *
 * \param [in] uart identifies UART to use
 * \param [in] data data to be written
 * \param [in] len number of bytes to be written
 *
 */
void hw_uart_write_buffer(HW_UART_ID uart, const void *data, uint16_t len);

/**
 * \brief Write number of bytes to UART
 *
 * \param [in] uart identifies UART to use
 * \param [in] data data to be written
 * \param [in] len number of bytes to be written
 * \param [in] cb function to call from interrupt when transmission ends
 * \param [in] user_data parameter passed to cb function
 * \return success or no success of sending:
 *      HW_UART_CONFIG_ERR_NOERR = successful transmit
 *      HW_UART_CONFIG_ERR_TX_SIZE = unsuccessful transmit, dma burst mode level incorrect
 *
 */
HW_UART_CONFIG_ERR hw_uart_send(HW_UART_ID uart, const void *data, uint16_t len, hw_uart_tx_callback cb,
                                                                                void *user_data);

/**
 * \brief Read number of bytes from UART synchronously
 *
 * This function waits to receive \p len bytes from UART.
 * This function does not use interrupts, nor DMA, it just waits for all data to arrive.
 *
 * \param [in] uart identifies UART to use
 * \param [out] data buffer for incoming data
 * \param [in] len number of bytes to read
 *
 */
void hw_uart_read_buffer(HW_UART_ID uart, void *data, uint16_t len);

/**
 * \brief Read number of bytes from UART
 *
 * This function initializes UART for receiving data.
 * When \p len data is received or timeout occurs, user provided callback function will be fired with
 * number of bytes read so far.
 *
 * \param [in] uart identifies UART to use
 * \param [out] data buffer for incoming data
 * \param [in] len max number of bytes to read
 * \param [in] cb function to call from interrupt when data is received
 * \param [in] user_data parameter passed to cb function
 *  \return success or no success of receiving:
 *      HW_UART_CONFIG_ERR_NOERR = successful reception
 *      HW_UART_CONFIG_ERR_RX_SIZE = unsuccessful reception, dma burst mode level incorrect
 *
 */
HW_UART_CONFIG_ERR hw_uart_receive(HW_UART_ID uart, void *data, uint16_t len, hw_uart_rx_callback cb,
                                                                                void *user_data);
/**
 * \brief Read number of bytes from UART with error checking
 *
 * Function initializes UART for receiving data. It is a superset of the \p hw_uart_receive
 * and provides information about the receiving errors.
 * When \p len data is received or timeout occurs user provided callback will be fired with
 * number of bytes read so far.
 * If an error occurrs during reception, the \p err_cb will be called providing info about
 * the error type.
 *
 * \param [in] uart identifies UART to use
 * \param [out] data buffer for incoming data
 * \param [in] len max number of bytes to read
 * \param [in] cb function to call from interrupt when data is received
 * \param [in] user_data parameter passed to cb function
 * \param [in] err_cb function to call from interrupt when an error is detected
 * \param [in] error_data parameter passed to err_cb function
 *
 */
void hw_uart_receive_error_checking(HW_UART_ID uart, void *data, uint16_t len, hw_uart_rx_callback cb,
                                            void *user_data, hw_uart_err_callback err_cb, void *error_data);

#if HW_UART_USE_DMA_SUPPORT

/**
 * \brief Assign DMA channel to UART rx and tx
 *
 * This function specifies the DMA channel to use for specified UART rx and tx FIFO.
 * It will setup the channel only if there is no transmission in progress.
 *
 * \param [in] uart identifies UART to use
 * \param [in] channel id of DMA channel to use for rx,
 *                     tx will use next channel,
 *                     -1 if DMA should not be used
 * \param [in] pri priority DMA_PRIO_0 (lowest),
 *                          .....
 *                          DMA_PRIO_7 (highest)
 *
 */
void hw_uart_set_dma_channels(HW_UART_ID uart, int8_t channel, HW_DMA_PRIO pri);

/**
 * \brief Assign DMA channels to UART rx and tx
 *
 * This function specifies the DMA channel to use for specified UART rx and tx FIFO.
 * It will setup the channel only if there is no transmission in progress.
 *
 * \param [in] uart identifies UART to use
 * \param [in] uart_init pointer to the UART configuration structure;
 *                       only the DMA-related fields are used by this function
 * \param [in] pri priority DMA_PRIO_0 (lowest),
 *                          .....
 *                          DMA_PRIO_7 (highest)
 *
 */
void hw_uart_set_dma_channels_ex(HW_UART_ID uart, const uart_config_ex *uart_init, HW_DMA_PRIO pri);
#endif

//============== Interrupt handling ============================================

/**
 * \brief Set the Received Data Available interrupt
 *
 * \param [in] uart identifies UART to use
 * \param [in] recdataavail should be 0 to disable and 1 to enable
 *
 */
__STATIC_INLINE void hw_uart_rec_data_int_set(HW_UART_ID uart, uint8_t recdataavail)
{
        // Set ERBFI bit in Interrupt Enable Register
        HW_UART_REG_SETF(uart, IER_DLH, ERBFI_DLH0, recdataavail);
}
/**
 * \brief Set the Transmit Holding Register empty interrupt
 *
 * \param [in] uart identifies UART to use
 * \param [in] txempty should be 0 to disable and 1 to enable
 *
 */
__STATIC_INLINE void hw_uart_tx_empty_int_set(HW_UART_ID uart, uint8_t txempty)
{
        // Set ETBEI bit in Interrupt Enable Register
        HW_UART_REG_SETF(uart, IER_DLH, ETBEI_DLH1, txempty);
}

/**
 * \brief Set the Line Status interrupt
 *
 * \param [in] uart identifies UART to use
 * \param [in] linestat should be 0 to disable and 1 to enable
 *
 */
__STATIC_INLINE void hw_uart_linestat_int_set(HW_UART_ID uart, uint8_t linestat)
{
        // Set ELSI bit in Interrupt Enable Register
        HW_UART_REG_SETF(uart, IER_DLH, ELSI_DLH2, linestat);
}

/**
 * \brief Set the Programmable THRE interrupt
 *
 * \param [in] uart identifies UART to use
 * \param [in] pthre should be 0 to disable and 1 to enable
 *
 */
__STATIC_INLINE void hw_uart_pthre_int_set(HW_UART_ID uart, uint8_t pthre)
{
        // Set PTIME bit in Interrupt Enable Register
        HW_UART_REG_SETF(uart, IER_DLH, PTIME_DLH7, pthre);
}

/**
 * \brief Get the Interrupt ID
 *
 * \param [in] uart identifies UART to use
 *
 * \return interrupt type
 *
 */
__STATIC_INLINE HW_UART_INT hw_uart_get_interrupt_id(HW_UART_ID uart)
{
        return (HW_UART_INT) (UBA(uart)->UART2_IIR_FCR_REG & 0xF);
}

/**
 * \brief Write Scratch pad register
 *
 * \param [in] uart identifies UART to use
 * \param [in] value the value to be written
 *
 * \warning Reserved when retarget is used, else it is free to use.
 *
 */
__STATIC_INLINE void hw_uart_write_scr(HW_UART_ID uart, uint8_t value)
{
        if (uart == HW_UART3) {
                REG_SETF(UART3, UART3_CONFIG_REG, ISO7816_SCRATCH_PAD, value);
                return;
        }
        UBA(uart)->UART2_SCR_REG = value;
}

/**
 * \brief Read Scratch pad register
 *
 * \param [in] uart identifies UART to use
 *
 * \return register value
 *
 * \warning Reserved when retarget is used, else it is free to use.
 *
 */
__STATIC_INLINE uint8_t hw_uart_read_scr(HW_UART_ID uart)
{
        if (uart == HW_UART3)
                return REG_GETF(UART3, UART3_CONFIG_REG, ISO7816_SCRATCH_PAD);
        return UBA(uart)->UART2_SCR_REG;
}

//==================== Configuration functions =================================

/**
 * \brief Get the baud rate setting
 *
 * \param [in] uart identifies UART to use
 *
 * \return baud rate of specified uart
 *
 */
HW_UART_BAUDRATE hw_uart_baudrate_get(HW_UART_ID uart);

/**
 * \brief Set the baud rate
 *
 * \param [in] uart identifies UART to use
 * \param [in] baud_rate uart baud rate
 *
 */
void hw_uart_baudrate_set(HW_UART_ID uart, HW_UART_BAUDRATE baud_rate);

//=========================== FIFO control functions ===========================

/**
 * \brief Check if there is data available for read
 *
 * \param [in] uart identifies UART to use
 *
 * \return true if there is data to receive
 *
 */
__STATIC_INLINE bool hw_uart_is_data_ready(HW_UART_ID uart)
{
        return (UBA(uart)->UART2_LSR_REG & 1) != 0;
}

/**
 * \brief Get the FIFO mode setting
 *
 * \param [in] uart identifies UART to use
 *
 * \return true if FIFO is enabled (both transmitter and receiver)
 *
 */
__STATIC_INLINE bool hw_uart_is_fifo_enabled(HW_UART_ID uart)
{

        return UBA(uart)->UART2_SFE_REG != 0;
}

/**
 * \brief Disable both FIFOs
 *
 * \param [in] uart identifies UART to use
 *
 */
__STATIC_INLINE void hw_uart_disable_fifo(HW_UART_ID uart)
{
        uint16_t iir_fcr_reg = UBA(uart)->UART2_IIR_FCR_REG;
        iir_fcr_reg &= 0xfffe;
        UBA(uart)->UART2_IIR_FCR_REG = iir_fcr_reg;
}

/**
 * \brief Enable both FIFOs
 *
 * Thresholds should be set before for predictable results.
 *
 * \param [in] uart identifies UART to use
 *
 */
__STATIC_INLINE void hw_uart_enable_fifo(HW_UART_ID uart)
{

        UBA(uart)->UART2_SFE_REG = 1 << HW_UART_REG_FIELD_POS(2, SFE, UART_SHADOW_FIFO_ENABLE);
}

/**
 * \brief Check if receive FIFO is not empty
 *
 * \param [in] uart identifies UART to use
 *
 * \return true if FIFO is not empty
 *
 */
__STATIC_INLINE bool hw_uart_receive_fifo_not_empty(HW_UART_ID uart)
{

        return HW_UART_REG_GETF(uart, USR, UART_RFNE) != 0;
}

/**
 * \brief Check if transmit FIFO is not full
 *
 * \param [in] uart identifies UART to use
 *
 * \return true if FIFO is full
 *
 */
__STATIC_INLINE bool hw_uart_transmit_fifo_not_full(HW_UART_ID uart)
{

        return HW_UART_REG_GETF(uart, USR, UART_TFNF) != 0;
}

/**
 * \brief Check if transmit FIFO is empty
 *
 * \param [in] uart identifies UART to use
 *
 * \return true if FIFO empty
 *
 */
__STATIC_INLINE bool hw_uart_transmit_fifo_empty(HW_UART_ID uart)
{
        return HW_UART_REG_GETF(uart, USR, UART_TFE) != 0;
}

/**
 * \brief Read number of bytes in receive FIFO
 *
 * \param [in] uart identifies UART to use
 *
 * \return number of bytes in receive FIFO
 *
 */
__STATIC_INLINE uint16_t hw_uart_receive_fifo_count(HW_UART_ID uart)
{

        return UBA(uart)->UART2_RFL_REG;
}

/**
 * \brief Read number of bytes in transmit FIFO
 *
 * \param [in] uart identifies UART to use
 *
 * \return number of bytes in transmit FIFO
 *
 */
__STATIC_INLINE uint16_t hw_uart_transmit_fifo_count(HW_UART_ID uart)
{

        return UBA(uart)->UART2_TFL_REG;
}

/**
 * \brief Enable loopback
 *
 * \param [in] uart identifies UART to use
 *
 */
__STATIC_INLINE void hw_uart_enable_loopback(HW_UART_ID uart)
{
        HW_UART_REG_SETF(uart, MCR, UART_LB, 1);
}

/**
 * \brief Disable loopback
 *
 * \param [in] uart identifies UART to use
 *
 */
__STATIC_INLINE void hw_uart_disable_loopback(HW_UART_ID uart)
{
        HW_UART_REG_SETF(uart, MCR, UART_LB, 0);
}

/**
 * \brief Get the FIFO mode setting
 *
 * \param [in] uart identifies UART to use
 *
 * \return the FIFO mode that has been set in the FIFO Control Register:
 *         0 = FIFO mode disabled,
 *         1 = FIFO mode enabled
 *
 */
uint8_t hw_uart_fifo_en_getf(HW_UART_ID uart);

/**
 * \brief Enable or disable the UART FIFO mode
 *
 * \param [in] uart identifies UART to use
 * \param [in] en should be 0 to disable and 1 to enable FIFO mode
 *
 */
__STATIC_INLINE void hw_uart_fifo_en_setf(HW_UART_ID uart, uint8_t en)
{

        // Write FIFO Enable (FIFOE) bit in FIFO Control Register
        uint16_t iir_fcr_reg = UBA(uart)->UART2_IIR_FCR_REG;
        iir_fcr_reg &= ~0x1;
        iir_fcr_reg |= (en & 0x1);
        UBA(uart)->UART2_IIR_FCR_REG = iir_fcr_reg;
}

/**
 * \brief Get the receive FIFO trigger level at which the
 *        Received Data Available Interrupt is generated
 *
 * \param [in] uart identifies UART to use
 *
 * \return the receive FIFO trigger level:
 *         0 = 1 character in the FIFO,
 *         1 = FIFO 1/4 full,
 *         2 = FIFO 1/2 full,
 *         3 = FIFO 2 less than full
 *
 */
__STATIC_INLINE uint8_t hw_uart_rx_fifo_tr_lvl_getf(HW_UART_ID uart)
{

        return UBA(uart)->UART2_SRT_REG & UART2_UART2_SRT_REG_UART_SHADOW_RCVR_TRIGGER_Msk;
}

/**
 * \brief Set the receive FIFO trigger level at which the
 *        Received Data Available Interrupt is generated
 *
 * \param [in] uart identifies UART to use
 * \param [in] tr_lvl the receive FIFO trigger level:
 *                    0 = 1 character in the FIFO,
 *                    1 = FIFO 1/4 full,
 *                    2 = FIFO 1/2 full,
 *                    3 = FIFO 2 less than full
 *
 */
__STATIC_INLINE void hw_uart_rx_fifo_tr_lvl_setf(HW_UART_ID uart, uint8_t tr_lvl)
{

        UBA(uart)->UART2_SRT_REG = tr_lvl;
}

/**
 * \brief Get the transmit FIFO trigger level at which the
 *        Transmit Holding Register Empty (THRE) Interrupt is generated
 *
 * \param [in] uart identifies UART to use
 *
 * \return the transmit FIFO trigger level:
 *         0 = FIFO empty,
 *         1 = 2 characters in the FIFO,
 *         2 = FIFO 1/4 full,
 *         3 = FIFO 1/2 full
 *
 */
uint8_t hw_uart_tx_fifo_tr_lvl_getf(HW_UART_ID uart);

/**
 * \brief Set the transmit FIFO trigger level at which the
 *        Transmit Holding Register Empty (THRE) Interrupt is generated
 *
 * \param [in] uart identifies UART to use
 * \param [in] tr_lvl the transmit FIFO trigger level:
 *                    0 = FIFO empty,
 *                    1 = 2 characters in the FIFO,
 *                    2 = FIFO 1/4 full,
 *                    3 = FIFO 1/2 full
 *
 */
__STATIC_INLINE void hw_uart_tx_fifo_tr_lvl_setf(HW_UART_ID uart, uint8_t tr_lvl)
{

        UBA(uart)->UART2_STET_REG = tr_lvl;
}

/**
 * \brief Reset UART transmit FIFO
 *
 * \param [in] uart identifies UART to use
 *
 */
__STATIC_INLINE void hw_uart_tx_fifo_flush(HW_UART_ID uart)
{
        HW_UART_REG_SETF(uart, SRR, UART_XFR, 1);
}

/**
 * \brief Reset UART receive FIFO
 *
 * \param [in] uart identifies UART to use
 *
 */
__STATIC_INLINE void hw_uart_rx_fifo_flush(HW_UART_ID uart)
{
        HW_UART_REG_SETF(uart, SRR, UART_RFR, 1);
}

/**
 * \brief Check whether reading buffer is empty
 *
 * Works for both when Rx FIFO is enabled or not.
 *
 * \param [in] uart identifies UART to use
 *
 * \return true if there is no data available for reading
 *
 */
__STATIC_INLINE bool hw_uart_read_buf_empty(HW_UART_ID uart)
{
        return !HW_UART_REG_GETF(uart, LSR, UART_DR);
}

/**
 * \brief Check whether writing buffer is full
 *
 * Works for both when Tx FIFO is enabled or not.
 *
 * \param [in] uart identifies UART to use
 *
 * \return true if transmit buffer is full
 *
 */
__STATIC_INLINE bool hw_uart_write_buf_full(HW_UART_ID uart)
{
        return !HW_UART_REG_GETF(uart, LSR, UART_THRE);
}

/**
 * \brief Check whether transmitter is empty.
 *
 * If FIFOs are enabled (FCR[0] set to one), this bit is set whenever
 * the Transmitter Shift Register  and the FIFO are both empty.
 * If FIFOs are disabled, this bit is set whenever the Transmitter
 * Holding Register and the Transmitter Shift Register are both empty.
 *
 * \param [in] uart identifies UART to use
 *
 * \return true if transmitter is empty
 *
 */
__STATIC_INLINE bool hw_uart_transmit_empty(HW_UART_ID uart)
{
        return HW_UART_REG_GETF(uart, LSR, UART_TEMT);
}

#if dg_configUART_SOFTWARE_FIFO
/**
 * \brief Set buffer that will be used for incoming data when there is no read in progress
 *
 * \param [in] uart identifies UART to use
 * \param [in] buf buffer to use as software FIFO
 * \param [in] size size of \p buf
 *
 */
void hw_uart_set_soft_fifo(HW_UART_ID uart, uint8_t *buf, uint8_t size);
#endif

//=========================== DMA control functions ============================

/**
 * \brief Set UART DMA mode
 *
 * \param [in] uart identifies UART to use
 * \param [in] dma_mode DMA mode 0 or 1
 *
 */
__STATIC_INLINE void hw_uart_dma_mode_setf(HW_UART_ID uart, uint8_t dma_mode)
{

        UBA(uart)->UART2_SDMAM_REG = ((dma_mode & 1) << UART2_UART2_SDMAM_REG_UART_SHADOW_DMA_MODE_Pos)
                                        << UART2_UART2_SDMAM_REG_UART_SHADOW_DMA_MODE_Msk;
}

/**
 * \brief Clear DMA request
 *
 * \param [in] uart identifies UART to use
 *
 */
__STATIC_INLINE void hw_uart_clear_dma_request(HW_UART_ID uart)
{
        UBA(uart)->UART2_DMASA_REG = 1;
}

//=========================== Line control functions ============================

/**
 * \brief Set UART line settings
 *
 * This function initializes UART registers with given configuration.
 * It also initializes all internal software variables for buffered transmissions.
 *
 * \param [in] uart identifies UART to use
 * \param [in] cfg pointer to the UART configuration structure
 *
 */
void hw_uart_init(HW_UART_ID uart, const uart_config *cfg);

/**
 * \brief Set UART line settings (extended version)
 *
 * This function initializes UART registers with given configuration,
 * including tx and rx FIFO trigger levels.
 * It also initializes all internal software variables for buffered transmissions.
 * Extra care is needed when configuring DMA burst mode and FIFO trigger level:
 * If DMA burst mode is 4x, then the TX or RX trigger level should be 1/4 full.
 * If DMA burst mode is 8x, then the TX or RX trigger level should be 1/2 full.
 *
 * \param [in] uart identifies UART to use
 * \param [in] cfg pointer to the UART configuration structure
 * \return FIFO and burst mode configuration error:
 *         HW_UART_CONFIG_ERR_RX_FIFO = rx FIFO trigger level is different than rx dma burst
 *         HW_UART_CONFIG_ERR_TX_FIFO = tx FIFO trigger level is different than tx dma burst
 * \warning Extra care is needed when configuring DMA burst mode and FIFO trigger level:
 * If DMA burst mode is 4x, then the TX or RX trigger level should be 1/4 full.
 * If DMA burst mode is 8x, then the TX or RX trigger level should be 1/2 full.
 *
 */
HW_UART_CONFIG_ERR hw_uart_init_ex(HW_UART_ID uart, const uart_config_ex *cfg);

/**
 * \brief Re-initialize UART registers
 *
 * Call this function with the configuration that should be re-applied. It should be
 * called after platform exits power sleep mode.
 * This function is similar to uart_init, but it does not initialize software variables
 * used for transmission control; it just re-applies hardware configuration.
 * It will turn on interrupts if transmission was in progress.
 *
 * \param [in] uart identifies UART to use
 * \param [in] cfg pointer to the UART configuration structure
 *
 */
void hw_uart_reinit(HW_UART_ID uart, const uart_config *cfg);


/**
 * \brief  Disables UART controller
 *
 * \param [in] uart UART controller instance
 */
void hw_uart_deinit(HW_UART_ID uart);

/**
 * \brief Re-initialize UART registers (extended version)
 *
 * Call this function with the configuration that should be re-applied. It should be
 * called after platform exits power sleep mode.
 * This function is similar to uart_init_ex, but it does not initialize software variables
 * used for transmission control; it just re-applies hardware configuration.
 * It will turn on interrupts if transmission is in progress.
 *
 * \param [in] uart identifies UART to use
 * \param [in] cfg pointer to the UART configuration structure
 *
 */
void hw_uart_reinit_ex(HW_UART_ID uart, const uart_config_ex *cfg);

/**
 * \brief Get UART line settings
 *
 * \param [in] uart identifies UART to use
 * \param [in] cfg pointer to the UART configuration structure
 */
void hw_uart_cfg_get(HW_UART_ID uart, uart_config *cfg);

//=========================== Modem control functions ==========================
/**
 * \brief Get the Auto Flow Control Enable (AFCE) setting
 *
 * \param [in] uart identifies UART to use
 *
 * \return Auto Flow Control Enable (AFCE):
 *         0 = Auto Flow Control Mode disabled,
 *         1 = Auto Flow Control Mode enabled
 *
 */
uint8_t hw_uart_afce_getf(HW_UART_ID uart);

/**
 * \brief Enable or disable Auto Flow Control
 *
 * \param [in] uart identifies UART to use
 * \param [in] afce Auto Flow Control Enable (AFCE):
 *                  0 = disable Auto Flow Control Mode,
 *                  1 = enable Auto Flow Control Mode
 *
 */
void hw_uart_afce_setf(HW_UART_ID uart, uint8_t afce);

/**
 * \brief Get UART diagnostic mode status
 *
 * \param [in] uart identifies UART to use
 *
 * \return the value of the loop back bit
 *
 */
uint8_t hw_uart_loopback_getf(HW_UART_ID uart);

/**
 * \brief Set UART in diagnostic mode
 *
 * \param [in] uart identifies UART to use
 * \param [in] lb loop back bit value
 *
 */
void hw_uart_loopback_setf(HW_UART_ID uart, uint8_t lb);

/**
 * \brief Get RTS output value
 *
 * \param [in] uart identifies UART to use
 *
 * \return RTS output value
 *
 */
uint8_t hw_uart_rts_getf(HW_UART_ID uart);

/**
 * \brief Set RTS output value
 *
 * \param [in] uart identifies UART to use
 * \param [in] rtsn Value for the RTS output (asserted low)
 *
 */
void hw_uart_rts_setf(HW_UART_ID uart, uint8_t rtsn);

//=========================== Line status functions ============================

/**
 * \brief Get the line status register error
 *
 * \param [in] uart identifies UART to use
 *
 * \return the line status error
 *
 *
 */
HW_UART_ERROR hw_uart_error_getf(HW_UART_ID uart);

/**
 * \brief Get the value of the Receiver FIFO Error bit
 *
 * \param [in] uart identifies UART to use
 *
 * \return Receiver FIFO Error bit value:
 *         0 = no error in RX FIFO,
 *         1 = error in RX FIFO
 *
 */
uint8_t hw_uart_rx_fifo_err_getf(HW_UART_ID uart);

/**
 * \brief Get the value of the Transmitter Empty bit
 *
 * \param [in] uart identifies UART to use
 *
 * \return 1 if transmitter FIFO is empty
 *
 */
uint8_t hw_uart_is_tx_fifo_empty(HW_UART_ID uart);

/**
 * \brief Get the value of the Transmit Holding Register Empty bit
 *
 * \param [in] uart identifies UART to use
 *
 * \return Transmit Holding Register Empty bit value
 *
 */
uint8_t hw_uart_thr_empty_getf(HW_UART_ID uart);

/**
 * \brief Get the value of the Break Interrupt bit
 *
 * \param [in] uart identifies UART to use
 *
 * \return Break Interrupt bit value: This is used to indicate the detection of a
 *         break sequence on the serial input data.
 *
 */
uint8_t hw_uart_break_int_getf(HW_UART_ID uart);

/**
 * \brief Get the value of the Framing Error bit
 *
 * \param [in] uart identifies UART to use
 *
 * \return Framing Error bit value:
 *         0 = no framing error,
 *         1 = framing error
 *
 *
 */
uint8_t hw_uart_frame_err_getf(HW_UART_ID uart);

/**
 * \brief Get the value of the Parity Error bit
 *
 * \param [in] uart identifies UART to use
 *
 * \return Parity Error bit value:
 *         0 = no parity error,
 *         1 = parity error
 *
 */
uint8_t hw_uart_parity_err_getf(HW_UART_ID uart);

/**
 * \brief Get the value of the Overrun Error bit
 *
 * \param [in] uart identifies UART to use
 *
 * \return Overrun Error bit value:
 *         0 = no overrun error,
 *         1 = overrun error
 *
 */
uint8_t hw_uart_overrun_err_getf(HW_UART_ID uart);

//=========================== Modem status functions ===========================

/**
 * \brief Get CTS input status
 *
 * \param [in] uart identifies UART to use
 *
 * \return status of CTS input:
 *         0 = CTSn input is de-asserted (logic 1),
 *         1 = CTSn input is asserted (logic 0),
 *         In loopback mode, CTS is the same as RTS.
 *
 */
uint8_t hw_uart_cts_getf(HW_UART_ID uart);

/**
 * \brief Get Delta CTS
 *
 * \param [in] uart identifies UART to use
 *
 * \return DCTS:
 *         0 = No change on CTS since last read of Modem Control Register,
 *         1 = Change on CTS since last read of Modem Control Register,
 *         Note that calling this function will clear the DCTS bit,
 *         In loopback mode, DCTS reflects changes on RTS.
 *
 */
uint8_t hw_uart_delta_cts_getf(HW_UART_ID uart);

/**
 * \brief Check if buffered write is in progress
 *
 * \param [in] uart identifies UART to use
 *
 * \return true if there is TX transmission in progress
 *
 */
bool hw_uart_tx_in_progress(HW_UART_ID uart);

/**
 * \brief Check if buffered read is in progress
 *
 * \param [in] uart identifies UART to use
 *
 * \return true if there is RX transmission in progress
 *
 */
bool hw_uart_rx_in_progress(HW_UART_ID uart);

#if dg_configUART_RX_CIRCULAR_DMA

/**
 * \brief Enable circular RX DMA for UART
 *
 * This function reconfigures RX DMA channel to use circular buffer for incoming data and enables it
 * immediately. Incoming data will be read by DMA to intermediate buffer. Subsequent calls to
 * hw_uart_receive() will fire callback function as soon as there are enough data available in buffer to be read,
 * but the actual read from intermediate buffer has to be done using hw_uart_copy_rx_circular_dma_buffer().
 *
 * The recommended way to move between DMA buffer and user buffer is to call hw_uart_abort_receive(),
 * which will copy the amount of bytes specified on read (or the actual number of received bytes, if
 * hw_uart_abort_receive() is called before the requested amount of bytes are received).
 *
 * \note
 * This function can be only called for a UART that is configured using \p dg_configUARTx_RX_CIRCULAR_DMA_BUF_SIZE.
 *
 * \param [in] uart identifies UART to use
 *
 */
void hw_uart_enable_rx_circular_dma(HW_UART_ID uart);

/**
 * \brief Copy data from circular RX DMA buffer to user buffer
 *
 * \param [in] uart identifies UART to use
 * \param [in] buf buffer to copy data to
 * \param [in] len length of data to copy to buffer
 *
 */
void hw_uart_copy_rx_circular_dma_buffer(HW_UART_ID uart, uint8_t *buf, uint16_t len);

#endif /* dg_configUART_RX_CIRCULAR_DMA */

#endif /* dg_configUSE_HW_UART */


#endif /* HW_UART_H_ */

/**
 * \}
 * \}
 */
