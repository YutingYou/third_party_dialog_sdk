/**
 * \addtogroup MID_SNC
 * \{
 * \addtogroup SNC_HW_UART
 *
 * \brief UART LLD for SNC context
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file snc_hw_uart.h
 *
 * @brief SNC-Definition of UART Low Level Driver API
 *
 * Copyright (C) 2017-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SNC_HW_UART_H_
#define SNC_HW_UART_H_


#if dg_configUSE_HW_SENSOR_NODE

#if dg_configUSE_SNC_HW_UART

#include "snc_defs.h"
#include "ad.h"
#include "hw_uart.h"

/*
 * ENUMERATION, DATA TYPE AND STRUCTURE DEFINITIONS
 *****************************************************************************************
 */

/**
 * \brief UART errors (bit positions)
 */
typedef enum {
        SNC_HW_UART_ERROR_RX    = 0,                    /**< Receiver error */
        SNC_HW_UART_ERROR_OE    = HW_UART_ERR_OE,       /**< Overrun error */
        SNC_HW_UART_ERROR_PE    = HW_UART_ERR_PE,       /**< Parity error */
        SNC_HW_UART_ERROR_FE    = HW_UART_ERR_FE,       /**< Framing error */
        SNC_HW_UART_ERROR_BI    = HW_UART_ERR_BI,       /**< Break interrupt indication*/
} SNC_HW_UART_ERROR;

#if (dg_configUART_ADAPTER == 0)
/**
 * \brief UART I/O configuration
 *
 * UART I/O configuration
 */
typedef struct {
        ad_io_conf_t rx;                /**< Configuration of RX signal */
        ad_io_conf_t tx;                /**< Configuration of TX signal */
        ad_io_conf_t rtsn;              /**< Configuration of RTS signal */
        ad_io_conf_t ctsn;              /**< Configuration of CTS signal */
        HW_GPIO_POWER voltage_level;    /**< Signals' voltage level */
} snc_uart_io_conf_t;

/**
 * \brief UART driver configuration
 *
 * Configuration of UART low level driver
 */
typedef struct {
        uart_config_ex hw_conf;         /**< Configuration of UART low level driver */
} snc_uart_driver_conf_t;

/**
 * \brief UART controller configuration
 *
 * Configuration of UART controller
 */
typedef struct {
        const HW_UART_ID id;                    /**< Controller instance */
        const snc_uart_io_conf_t *io;           /**< IO configuration */
        const snc_uart_driver_conf_t *drv;      /**< Driver configuration */
} snc_uart_controller_conf_t;

#else
#include "ad_uart.h"

/**
 * \brief UART I/O configuration
 *
 * UART I/O configuration
 */
typedef ad_uart_io_conf_t snc_uart_io_conf_t;

/**
 * \brief UART driver configuration
 *
 * Configuration of UART low level driver
 */
typedef ad_uart_driver_conf_t snc_uart_driver_conf_t;

/**
 * \brief UART controller configuration
 *
 * Configuration of UART controller
 */
typedef ad_uart_controller_conf_t snc_uart_controller_conf_t;

#endif /* dg_configUART_ADAPTER */

/*
 * MACRO DEPENDENCIES
 *****************************************************************************************
 */

#include "snc_hw_uart_macros.h"

/*
 * FUNCTION DECLARATIONS
 *****************************************************************************************
 */

//==================== Controller Acquisition functions ========================

/**
 * \brief Function used in SNC context to UART controller
 *
 * \param [in] conf             (const snc_uart_controller_conf_t*: build-time-only value)
 *                              UART controller configuration
 *
 * \sa SNC_uart_close
 */
#define SNC_uart_open(conf)                                                                     \
        _SNC_uart_open(conf)

/**
 * \brief Function used in SNC context to close UART controller
 *
 * \param [in] conf             (const snc_uart_controller_conf_t*: build-time-only value)
 *                              UART controller configuration
 *
 * \sa SNC_uart_open
 */
#define SNC_uart_close(conf)                                                                    \
        _SNC_uart_close(conf)

//==================== Controller Configuration functions ======================

/**
 * \brief Function used in SNC context to reconfigure UART controller
 *
 * This function will apply a new UART driver configuration implied by the given UART controller
 * configuration.
 *
 * \param [in] conf             (const snc_uart_controller_conf_t*: build-time-only value)
 *                              UART controller configuration
 *
 * \sa SNC_uart_open
 * \sa SNC_uart_close
 */
#define SNC_uart_reconfig(conf)                                                                 \
        _SNC_uart_reconfig(conf)

//==================== Status Acquisition functions ============================

/**
 * \brief Function used in SNC context to check if a data transmission, initiated by a UART write
 *        transaction, is still pending
 *
 * \param [in] conf             (const snc_uart_controller_conf_t*: build-time-only value)
 *                              UART controller configuration
 * \param [out] pending         (uint32_t*: use da() or ia())
 *                              pointer to variable indicating pending data transmission
 *                              (1 = pending; 0 = completed data transmission)
 *
 * \sa SNC_uart_write
 */
#define SNC_uart_is_write_pending(conf, pending)                                                \
        _SNC_uart_is_write_pending(conf, pending)

/**
 * \brief Function used in SNC context to wait for a data transmission, initiated by a UART write
 *        transaction, to be completed
 *
 * \param [in] conf             (const snc_uart_controller_conf_t*: build-time-only value)
 *                              UART controller configuration
 *
 * \sa SNC_uart_write
 */
#define SNC_uart_wait_write_pending(conf)                                                       \
        _SNC_uart_wait_write_pending(conf)

//==================== Control functions =======================================

/**
 * \brief Function used in SNC context to abort UART read transaction
 *
 * After a read transaction, the application can decide that if less than requested characters were
 * received (e.g. an error has occurred), read request should be aborted. In such case this function
 * can be used to finish the transaction and reset the corresponding resources (e.g. resetting RX FIFO).
 *
 * \param [in] conf             (const snc_uart_controller_conf_t*: build-time-only value)
 *                              UART controller configuration
 *
 * \sa SNC_uart_read
 * \sa SNC_uart_read_check_error
 */
#define SNC_uart_abort_read(conf)                                                               \
        _SNC_uart_abort_read(conf)

/**
 * \brief Function used in SNC context to (re)start the flow over the UART bus
 *
 * \param [in] conf             (const snc_uart_controller_conf_t*: build-time-only value)
 *                              UART controller configuration
 *
 * \sa SNC_uart_set_flow_off
 * \sa SNC_uart_try_flow_off
 */
#define SNC_uart_set_flow_on(conf)                                                              \
        _SNC_uart_set_flow_on(conf)

/**
 * \brief Function used in SNC context to force-stop the flow over the UART bus
 *
 * \param [in] conf             (const snc_uart_controller_conf_t*: build-time-only value)
 *                              UART controller configuration
 * \param [out] chars_avail     (uint32_t*: use da() or ia())
 *                              indication that there are characters available in the receiver
 *
 * \sa SNC_uart_set_flow_on
 * \sa SNC_uart_try_flow_off
 */
#define SNC_uart_set_flow_off(conf, chars_avail)                                                \
        _SNC_uart_set_flow_off(conf, chars_avail)

/**
 * \brief Function used in SNC context to try to stop the flow over the UART bus
 *
 * This function is used in order to try to stop the flow over the UART bus using the RTS signal.
 * If it is returned an indication of available characters in the receiver, then the RTS signal
 * state has been restored.
 *
 * \param [in] conf             (const snc_uart_controller_conf_t*: build-time-only value)
 *                              UART controller configuration
 * \param [out] chars_avail     (uint32_t*: use da() or ia())
 *                              indication that there are characters available in the receiver
 *
 * \sa SNC_uart_set_flow_on
 * \sa SNC_uart_set_flow_off
 */
#define SNC_uart_try_flow_off(conf, chars_avail)                                                \
        _SNC_uart_try_flow_off(conf, chars_avail)

//==================== Data Read/Write functions ===============================

/**
 * \brief Function used in SNC context to perform a blocking write-only UART transaction
 *
 * This function performs a write-only transaction on UART bus using the internal FIFO. In order
 * to wait for the transmission of all data (i.e. write-only transaction) to be completed,
 * SNC_uart_is_write_pending() or SNC_uart_wait_write_pending() can be used. The contents of
 * out_buf address are sent over UART bus.
 *
 * This function is blocking. It waits until all data are finally sent to the UART controller.
 *
 * Two modes are supported based on the addressing mode of out_buf address being passed.
 * Direct addressing is used for passing the address of a buffer (i.e. da(data)), while
 * indirect addressing is used for passing an indirect address to a buffer (i.e. ia(&data_ind))
 * which is internally increased, so that it can point to the next word to access.
 *
 * Example usage:
 * \code{.c}
 * const snc_uart_controller_config_t _UART_CONF1 = {HW_UART1, {\<io_conf\>}, {\<drv_conf\>}};
 * const void* const UART_CONF1 = &_UART_CONF1;
 * uint32_t data[2] = {0x20, 0x10};
 * uint32_t* data_ind = &data[0];
 *
 * {
 *         ...
 *         SNC_uart_open(UART_CONF1);
 *         SNC_uart_write(UART_CONF1, ia(&data_ind), sizeof(data)/sizeof(uint32_t));
 *         SNC_uart_close(UART_CONF1);
 *         ...
 * }
 * \endcode
 *
 * \param [in] conf             (const snc_uart_controller_conf_t*: build-time-only value)
 *                              UART controller configuration
 * \param [in] out_buf          (uint32_t*: use da() or ia())
 *                              buffer containing the data to be sent over the UART bus
 * \param [in] out_len          (uint32_t: use da() or ia() or build-time-only value)
 *                              size of data in bytes to be sent over the UART bus,
 *                              where 1 byte of data is stored to each 32bit word of out_buf
 *
 * \sa SNC_uart_open
 * \sa SNC_uart_close
 * \sa SNC_uart_is_write_pending
 * \sa SNC_uart_wait_write_pending
 */
#define SNC_uart_write(conf, out_buf, out_len)                                                  \
        _SNC_uart_write(conf, out_buf, out_len)

/**
 * \brief Function used in SNC context to perform a blocking read-only UART transaction
 *
 * This function performs a read-only transaction on UART bus using the internal FIFO. The data
 * received over UART bus are stored to in_buf address.
 *
 * This function is blocking. It waits for a given time interval (i.e. wait_time) the first
 * character to be received and then it waits until all data are finally received from the UART
 * controller or a character timeout occurs. If the wait-time interval passes and no character
 * has been received, then the function returns, indicating that no characters have been received
 * (i.e. rcv_len == 0).
 *
 * If an error occurs (e.g. overrun, parity etc.), this function ignores it, thus continuing
 * receiving the requested characters. In order to have error-checking while receiving characters
 * from the UART, SNC_uart_read_check_error() function must be used.
 *
 * Two modes are supported based on the addressing mode of in_buf address being passed.
 * Direct addressing is used for passing the address of a buffer (i.e. da(data)), while
 * indirect addressing is used for passing an indirect address to a buffer (i.e. ia(&data_ind))
 * which is internally increased, so that it can point to the next word to access.
 *
 * Example usage:
 * \code{.c}
 * const snc_uart_controller_config_t _UART_CONF1 = {HW_UART1, {\<io_conf\>}, {\<drv_conf\>}};
 * const void* const UART_CONF1 = &_UART_CONF1;
 * uint32_t data[32] = {0};
 * uint32_t* data_ind = &data[0];
 * uint32_t rcv_data_size;
 *
 * {
 *         ...
 *         SNC_uart_open(UART_CONF1);
 *         SNC_uart_read(UART_CONF1, ia(&data_ind), sizeof(data)/sizeof(uint32_t),
 *                                   -1, da(&rcv_data_size);
 *         SNC_uart_close(UART_CONF1);
 *         ...
 * }
 * \endcode
 *
 * \note For baudrate values > 38400 the RX FIFO must be used, otherwise there will be data loss
 *
 * \param [in] conf             (const snc_uart_controller_conf_t*: build-time-only value)
 *                              UART controller configuration
 * \param [out] in_buf          (uint32_t*: use da() or ia())
 *                              buffer for incoming data
 * \param [in] in_len           (uint32_t: use da() or ia() or build-time-only value)
 *                              size of buffer in 32bit words pointed by in_buf,
 *                              where 1 byte/character of data is stored to each 32bit word
 * \param [in] wait_time        (uint32_t: use da() or ia() or build-time-only value)
 *                              time interval in low power clock ticks waiting for the first
 *                              character to be received
 * \param [out] rcv_len         (uint32_t*: use da() or ia())
 *                              number of characters received
 *
 * \sa SNC_uart_open
 * \sa SNC_uart_close
 * \sa SNC_uart_read_check_error
 */
#define SNC_uart_read(conf, in_buf, in_len, wait_time, rcv_len)                                 \
        _SNC_uart_read(conf, in_buf, in_len, wait_time, rcv_len)

/**
 * \brief Function used in SNC context to perform a blocking read-only UART transaction
 *        with error-checking enabled
 *
 * This function performs a read-only transaction on UART bus using the internal FIFO and enabling
 * error-checking. The data received over UART bus are stored to in_buf address.
 *
 * This function is blocking. It waits for a given time interval (i.e. wait_time) the first
 * character to be received and then it waits until all data are finally received from the UART
 * controller or a character timeout or an error occurs. If the wait-time interval passes and no
 * character has been received, then the function returns, indicating that no characters have been
 * received (i.e. rcv_len == 0).
 *
 * If an error occurs (e.g. overrun, parity etc.), this function returns, and indicates the error
 * value (i.e. SNC_HW_UART_ERROR flags), the number of received characters and the address
 * of the character which has been stored into the given input buffer (i.e. in_buf) when the error
 * occurred. In order to ignore errors while receiving characters from the UART, SNC_uart_read()
 * function must be used.
 *
 * If NULL (i.e. da(NULL)) is passed as error output return parameter, then error-checking is ignored.
 * If NULL (i.e. da(NULL)) is passed as error character address output return parameter, then
 * no character address is going to be returned.
 *
 * Two modes are supported based on the addressing mode of in_buf address being passed.
 * Direct addressing is used for passing the address of a buffer (i.e. da(data)), while
 * indirect addressing is used for passing an indirect address to a buffer (i.e. ia(&data_ind))
 * which is internally increased, so that it can point to the next word to access.
 *
 * Example usage:
 * \code{.c}
 * const snc_uart_controller_config_t _UART_CONF1 = {HW_UART1, {\<io_conf\>}, {\<drv_conf\>}};
 * const void* const UART_CONF1 = &_UART_CONF1;
 * uint32_t data[32] = {0};
 * uint32_t* data_ind = &data[0];
 * uint32_t rcv_data_size;
 * uint32_t error;
 * uint32_t* p_error_char;
 *
 * {
 *         ...
 *         SNC_uart_open(UART_CONF1);
 *         SNC_uart_read_check_error(UART_CONF1, ia(&data_ind), sizeof(data)/sizeof(uint32_t),
 *                                               -1, da(&rcv_data_size,
 *                                               da(&error), da(&p_error_char));
 *         // Check if an error has occurred
 *         SENIS_if(da(&error), BIT, SNC_HW_UART_ERROR_RX) {
 *                 ... // Code for error handling
 *         }
 *         SNC_uart_close(UART_CONF1);
 *         ...
 * }
 * \endcode
 *
 * \note For baudrate values > 38400 the RX FIFO must be used, otherwise there will be data loss
 *
 * \param [in] conf             (const snc_uart_controller_conf_t*: build-time-only value)
 *                              UART controller configuration
 * \param [out] in_buf          (uint32_t*: use da() or ia())
 *                              buffer for incoming data
 * \param [in] in_len           (uint32_t: use da() or ia() or build-time-only value)
 *                              size of buffer in 32bit words pointed by in_buf,
 *                              where 1 byte/character of data is stored to each 32bit word
 * \param [in] wait_time        (uint32_t: use da() or ia() or build-time-only value)
 *                              time in low power clock ticks waiting for data
 * \param [out] rcv_len         (uint32_t*: use da() or ia())
 *                              number of characters received
 * \param [out] error           (uint32_t*: use da() or ia())
 *                              error produced when reading last character (i.e. SNC_HW_UART_ERROR flags)
 * \param [out] error_char      (uint32_t**: use da() or ia())
 *                              address of the character in the received data which has produced the error
 *
 * \sa SNC_uart_open
 * \sa SNC_uart_close
 * \sa SNC_uart_read
 */
#define SNC_uart_read_check_error(conf, in_buf, in_len, wait_time, rcv_len, error, error_char)  \
        _SNC_uart_read_check_error(conf, in_buf, in_len, wait_time, rcv_len, error, error_char)

#endif /* dg_configUSE_SNC_HW_UART */

#endif /* dg_configUSE_HW_SENSOR_NODE */


#endif /* SNC_HW_UART_H_ */

/**
 * \}
 * \}
 */
