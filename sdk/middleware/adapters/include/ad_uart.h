/**
 * \addtogroup MID_SYS_ADAPTERS
 * \{
 * \addtogroup UART_ADAPTER UART Adapter
 *
 * \brief Universal Asynchronous Receiver-Transmitter adapter
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_uart.h
 *
 * @brief UART adapter API
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef AD_UART_H_
#define AD_UART_H_

#if dg_configUART_ADAPTER

#include "ad.h"
#include "hw_uart.h"
#include "hw_dma.h"
#include "osal.h"
#include "resmgmt.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Asynchronous callback function
 *
 */
typedef void (*ad_uart_user_cb)(void *user_data, uint16_t transferred);

/**
 * \brief UART resource types.
 */
typedef enum
{
        /** Configuration resource. Acquiring this resource will block calls to ad_uart_open() for
         * the same UART bus. */
        AD_UART_RES_TYPE_CONFIG,

        /** Tx (write) resource. Acquiring this resource will block write operations on the same
         * bus. */
        AD_UART_RES_TYPE_WRITE,

        /** Rx (read) resource. Acquiring this resource will block read operations on the same
         * bus. */
        AD_UART_RES_TYPE_READ,

        /** Enumeration end. */
        AD_UART_RES_TYPES
} AD_UART_RES_TYPE;


/**
 * \brief UART adapter error codes
 *
 */
typedef enum {
        AD_UART_ERROR_RESOURCE_NOT_AVAILABLE    = -6,   /** The Resource (Rx, Tx, Config) is not available. */
        AD_UART_ERROR_CONTROLLER_BUSY           = -5,   /** The UART controller is not available. */
        AD_UART_ERROR_DEVICE_CLOSED             = -4,   /** The device is closed. */
        AD_UART_ERROR_CONTROLLER_CONF_INVALID   = -3,   /** The controller configuration is invalid. */
        AD_UART_ERROR_GPIO_CONF_INVALID         = -2,   /** The GPIO configuration is invalid. */
        AD_UART_ERROR_HANDLE_INVALID            = -1,   /** The handle returned from ad_uart_open() is not valid.*/
        AD_UART_ERROR_NONE                      = 0     /** No error.*/
} AD_UART_ERROR;


/**
 * \brief UART I/O configuration
 *
 * UART I/O configuration
 */
typedef struct {
        ad_io_conf_t rx;                /** Rx pin configuration */
        ad_io_conf_t tx;                /** Tx pin configuration */
        ad_io_conf_t rtsn;              /** RTS pin configuration */
        ad_io_conf_t ctsn;              /** CTS pin configuration */
        HW_GPIO_POWER voltage_level;    /** Pin voltage level */
} ad_uart_io_conf_t;

/**
 * \brief UART driver configuration
 *
 * UART I/O configuration
 */
typedef struct {
        uart_config_ex  hw_conf;        /** Low lever driver configuration */
} ad_uart_driver_conf_t;

/**
 * \brief UART controller configuration
 *
 * Configuration of UART controller
 *
 */
typedef struct {
        const HW_UART_ID id;                    /** Pointer to the UART id */
        const ad_uart_io_conf_t *io;            /** Pointer to the pin configuration */
        const ad_uart_driver_conf_t *drv;       /** Pointer to the low level driver configuration */
} ad_uart_controller_conf_t;


typedef void *ad_uart_handle_t;


/**
 * \brief Initialize UART adapter
 *
 * \note It should ONLY be called by the system.
 */
void ad_uart_init(void);


/**
 * \brief Open UART controller
 *
 * This function:
 * - Acquires the resources needed for using the controller
 * - Configures the controller interface IOs
 * - Initializes the drivers associated with the controller
 *
 *
 * \param [in] ad_uart_ctrl_conf  controller configuration
 *
 * \return >0: handle that should be used in subsequent API calls, NULL: error
 *
 * \note The function will block until it acquires all controller resources
 */
ad_uart_handle_t ad_uart_open(const ad_uart_controller_conf_t *ad_uart_ctrl_conf);


/**
 * \brief Close UART controller
 *
 * This function:
 * - Aborts ongoing transactions
 * - De-initializes the drivers associated with the controller
 * - Resets controller interface IOs (as specified in ad_uart_open())
 * - Releases the controller resources
 *
 * \param [in] handle handle returned from ad_uart_open()
 * \param [in] force force close even if controller is busy
 *
 * \sa ad_uart_open()
 *
 * \return 0: success, <0: error code
 */
int ad_uart_close(ad_uart_handle_t handle, bool force);


/**
 * \brief Reconfigure UART controller
 *
 * This function will apply a new UART driver configuration.
 *
 * \param [in] handle handle returned from ad_uart_open()
 * \param [in] ad_drv new driver configuration
 *
 * \sa ad_uart_open()
 *
 * \return 0: success, <0: error code
 */
int ad_uart_reconfig(ad_uart_handle_t handle, const ad_uart_driver_conf_t *ad_drv);

/**
 * \brief Perform a blocking write transaction
 *
 * This function performs a synchronous write only transaction
 *
 * \param [in] handle handle returned from  ad_uart_open()
 * \param [in] wbuf   buffer containing the data to be sent to the device
 * \param [in] wlen   size of data to be sent to the device
 *
 * \sa ad_uart_open()
 *
 * \return 0 on success, <0: error
 *
 */
int ad_uart_write(ad_uart_handle_t handle, const char *wbuf, size_t wlen);


/**
 * \brief Perform a blocking read transaction
 *
 * This function performs a synchronous read only transaction
 *
 * \param [in]  handle handle returned from ad_uart_open()
 * \param [out] rbuf   buffer for incoming data
 * \param [in]  rlen   number of bytes to read
 * \param [in]  timeout timeout time in ticks to wait for data
 *
 * \sa ad_uart_open()
 *
 * \note  If timeout is OS_EVENT_FOREVER, exactly \p rlen bytes must be received.
 *        If timeout is specified, function can exit after timeout with less bytes
 *        than requested.
 *
 * \return Number of transferred bytes on success, <0: error
 *
 */
int ad_uart_read(ad_uart_handle_t handle, char *rbuf, size_t rlen, OS_TICK_TIME timeout);

/**
 * \brief Perform a non blocking write transaction
 *
 * This function performs an asynchronous write only transaction
 * Caller task should retry until function returns no error
 * Callback will be called when transaction is completed
 *
 * \param [in] handle handle returned from ad_uart_open()
 * \param [in] wbuf   buffer containing the data to be sent to the device
 * \param [in] wlen   size of data to be sent to the device
 * \param [in] cb     callback to call after transaction is over (from ISR context)
 * \param [in] user_data user data passed to cb callback
 *
 * \sa ad_uart_open()
 *
 * \warning Do not call this function consecutively without guaranteeing that the previous
 *          async transaction has been completed.
 *
 * \warning After the callback is called, it is not guaranteed that the scheduler will give
 *          control to the task waiting for this transaction to complete. This is important to
 *          consider if more than one tasks are using this API.
 *
 * \return 0 on success, <0: error
 *
 */
int ad_uart_write_async(ad_uart_handle_t handle, const char *wbuf, size_t wlen,
                          ad_uart_user_cb cb, void *user_data);

/**
 * \brief Perform a non blocking read transaction
 *
 * This function performs an asynchronous read only transaction
 * Caller task should retry until function returns no error
 * Callback will be called when transaction is completed
 *
 * \param [in]  handle    handle returned from ad_uart_open()
 * \param [out] rbuf      buffer for incoming data
 * \param [in]  rlen      number of bytes to read
 * \param [in]  cb        callback to call after transaction is over (from ISR context)
 * \param [in]  user_data user data passed to cb callback
 *
 * \sa ad_uart_open()
 *
 * \warning Do not call this function consecutively without guaranteeing that the previous
 *          async transaction has been completed.
 *
 * \warning After the callback is called, it is not guaranteed that the scheduler will give
 *          control to the task waiting for this transaction to complete. This is important to
 *          consider if more than one tasks are using this API.
 *
 * \return 0 on success, <0: error
 *
 */
int ad_uart_read_async(ad_uart_handle_t handle, char *rbuf, size_t rlen, ad_uart_user_cb cb,
                         void *user_data);

/**
 * \brief Get UART controller id
 *
 * This function returns id that can be used to get UART controller id. This id is argument
 * for lower level functions starting with hw_uart_ prefix.
 *
 * \param [in] handle handle returned from ad_uart_open()
 *
 * \return id that can be used with hw_uart_... functions
 */
HW_UART_ID ad_uart_get_hw_uart_id(ad_uart_handle_t handle);

/**
 * \brief Finish asynchronous read
 *
 * Asynchronous reads allow to create reads with timeouts, after requested number of characters
 * arrive user callback is executed from ISR.
 * At any moment after read is started application can decide that if less than requested
 * characters were received, read request should end.
 * In such case application code should call ad_uart_complete_async_read() to get number of
 * characters read.
 *
 * \param [in] handle handle returned from ad_uart_open()
 *
 * \return number for successfully read characters
 *
 * \sa ad_uart_read_async()
 *
 */
int ad_uart_complete_async_read(ad_uart_handle_t handle);

/**
* \brief Initialize controller pins to on / off io configuration
*
* This function should be called for setting pins to the correct level before external
* devices are powered up (e.g on system init). It does not need to be called before every
* ad_uart_open() call.
*
* \param [in] id         controller instance
* \param [in] io         controller io configuration
* \param [in] state      on/off io configuration
*
* \return 0: success, <0: error code
*/
int ad_uart_io_config (HW_UART_ID id, const ad_uart_io_conf_t *io, AD_IO_CONF_STATE state);

#ifdef __cplusplus
}
#endif

#endif /* dg_configUART_ADAPTER */

#endif /* AD_UART_H_ */

/**
 * \}
 * \}
 */
