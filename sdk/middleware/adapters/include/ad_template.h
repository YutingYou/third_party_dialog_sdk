/**
 * \addtogroup MID_SYS_ADAPTERS
 * \{
 * \addtogroup YYY_ADAPTER YYY Adapter
 *
 * \brief Adapter for YYY controller
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_template.h (name should be changed here with the actual name)
 *
 * @brief YYY Controller access API
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/*
 * This is a template header file for a controller adapter.
 *
 * Terminology:
 *      Adapter:    A middleware abstraction layer for using a controller. It provides the following services:
 *                  - Arbitration in accessing the controller (between multiple masters, multiple tasks)
 *                  - Configuration of the controller
 *                  - Block sleep while the controller is in use
 *      Controller: Top level view of a peripheral (e.g SPI, I2C), including the complete configuration needed for being fully functional (IOs, DMAs, Driver configuration)
 *      Driver:     The Low Level Driver associated with the controller
 *      YYY:        The controller name (e.g SPI, I2C)
 *
 * Api example usage:
 *      ad_yyy_init():           Called by the system on power up for initializing the adapter. It should NOT be called by the application
 *      ad_yyy_io_config():      Called by the application for configuring the controller interface before powering up the external connected devices (e.g sensors)
 *
 *      ad_yyy_open();           Called by the application for starting a transaction. The sleep is blocked until ad_yyy_close() is called. No other task or master can start a new transaction using the same controller instance
 *        ad_yyy_write();        Blocking write - caller task will block until the resource becomes available
 *        ad_yyy_read();         Blocking read  - caller task will block until the resource becomes available
 *      ad_yyy_reconfig();
 *        ad_yyy_write_async();  Non blocking write - caller task should retry until function returns no error. It will then be notified when transaction is completed
 *        ad_yyy_read_async();   Non blocking read  - caller task should retry until function returns no error. It will then be notified when transaction is completed
 *      ad_yyy_close();          Called by the application for releasing the resource. System is allowed to go to sleep (if no other module blocks sleep)
 *
 *
 *
 * */
#ifndef AD_YYY_H_
#define AD_YYY_H_

#if dg_configYYY_ADAPTER
//#include <hw_yyy.h> //Replace with the header file of the respective Driver (which includes definitions of yyy_config, HW_YYY_ID)
#include <osal.h>
#include <hw_gpio.h>
#include <ad.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Mocks for compiling template project - begin */
/* Remove whole section on real adapter implementation */
typedef void* HW_YYY_ID;
#define YYY_BASE
char YYY1_BASE;
char YYY2_BASE;
#define HW_YYY1                 ((void *)&YYY1_BASE)
#define HW_YYY2                 ((void *)&YYY2_BASE)
#define BSR_PERIPH_ID_YYY1      0
#define BSR_PERIPH_ID_YYY2      1
#define RES_ID_YYY1             0
#define RES_ID_YYY2             1

typedef struct {
        void     *x;
        uint8_t   dma;
} yyy_config;
typedef struct {
        void     *y;
} yyy_config;
/* Mocks for compiling template project - end   */

/*
 * Data types definitions section
 */

/**
 * \brief YYY Handle returned by ad_yyy_open()
 */
typedef void *ad_yyy_handle_t;

/**
 * \brief YYY I/O configuration
 *
 * YYY I/O configuration
 */

typedef struct {
        ad_io_conf_t yyy; /**< configuration of yyy signal */
        ad_io_conf_t zzz; /**< configuration of zzz signal */
        HW_GPIO_POWER voltage_level;
} ad_yyy_io_conf_t;

/**
 * \brief YYY driver configuration
 *
 * Configuration of YYY low level driver(s)
 *
 * \note There may be more than one driver configurations needed (e.g DMA)
 *
 */
typedef struct {
        yyy_config *yyy;         /**< configuration of YYY Low level driver */
        yyy_config *yyy;         /**< configuration of YYY Low level driver */
} ad_yyy_driver_conf_t;

/**
 * \brief YYY controller configuration
 *
 * Configuration of YYY controller
 *
 */
typedef struct {
        HW_YYY_ID               id;        /**< Controller instance*/
        ad_yyy_io_conf_t       *io;        /**< IO configuration*/
        ad_yyy_driver_conf_t   *drv;       /**< Driver configuration*/
} ad_yyy_controller_conf_t;


/*
 * Adapters mandatory function prototypes section
 *
 * This section includes the prototypes of the functions
 * which are mandatory for all adapters
 */



/**
 * \brief Initialize adapter
 *
 * \note: It should ONLY be called by the system.
 *
 */
void ad_yyy_init(void);


/**
 * \brief Open YYY controller
 *
 * This function:
 * - Acquires the resources needed for using the controller
 * - Configures the controller interface IOs
 * - Initializes the drivers associated with the controller
 *
 *
 * \param [in] conf  controller configuration
 *
 * \return Not NULL: handle that should be used in subsequent API calls, NULL: error
 *
 * \note The function will block until it acquires all controller resources
 */
ad_yyy_handle_t ad_yyy_open(const ad_yyy_controller_conf_t *conf);

/**
 * \brief Reconfigure YYY controller
 *
 * This function will apply a new YYY driver configuration.
 *
 * \param [in] handle handle returned from ad_yyy_open()
 * \param [in] conf   new driver configuration
 *
 * \sa ad_yyy_open()
 *
 * \return 0: success, <0: error code
 */
int ad_yyy_reconfig(ad_yyy_handle_t handle, const ad_yyy_driver_conf_t *conf);

/**
 * \brief Close YYY controller
 *
 * This function:
 * - Aborts ongoing transactions
 * - De-initializes the drivers associated with the controller
 * - Resets controller interface IOs (as specified in ad_yyy_open())
 * - Releases the controller resources
 *
 * \param [in] handle handle returned from ad_yyy_open()
 *
 * \sa ad_yyy_open()
 *
 * \return 0: success, <0: error code
 */
int ad_yyy_close(ad_yyy_handle_t handle);

/**
* \brief Initialize controller pins to on / off io configuration
*
* This function should be called for setting pins to the correct level before external
* devices are powered up (e.g on system init). It does not need to be called before every
* ad_yyy_open() call.
*
* \param [in] id         controller instance
* \param [in] io         controller io configuration
* \param [in] state      on/off io configuration
*
* \return 0: success, <0: error code
*/
int ad_yyy_io_config (HW_YYY_ID id, const ad_yyy_io_conf_t *io, AD_IO_CONF_STATE state);



/*
 * Adapter specific function prototypes section
 *
 * This section includes the prototypes of the functions
 * which are specific to each adapter.
 * An example function prototype is shown below
 */

typedef void (*ad_yyy_user_cb)(void *user_data, uint16_t transferred);

/**
 * \brief Perform a blocking write transaction
 *
 * This function performs a synchronous write only transaction
 *
 * \param [in] handle handle returned from ad_yyy_open()
 * \param [in] wbuf   buffer containing the data to be sent to the device
 * \param [in] wlen   size of data to be sent to the device
 *
 * \sa ad_yyy_open()
 *
 * \return 0 on success, <0: error
 *
 */
int ad_yyy_write(ad_yyy_handle_t handle, const uint8_t *wbuf, size_t wlen);

/**
 * \brief Perform a non blocking write transaction
 *
 * This function performs an asynchronous write only transaction
 * Caller task should retry until function returns no error
 * Callback will be called when transaction is completed
 *
 * \param [in] handle handle returned from ad_yyy_open()
 * \param [in] wbuf   buffer containing the data to be sent to the device
 * \param [in] wlen   size of data to be sent to the device
 * \param [in] cb     callback to call after transaction is over (from ISR context)
 * \param [in] user_data user data passed to cb callback
 *
 * \sa ad_yyy_open()
 *
 * \return 0 on success, <0: error
 *
 */
int ad_yyy_write_async(ad_yyy_handle_t handle, const uint8_t *wbuf, size_t wlen, ad_yyy_user_cb cb,
        void *user_data);

/**
 * \brief Perform a blocking read transaction
 *
 * This function performs a synchronous read only transaction
 *
 * \param [in]  handle handle returned from ad_yyy_open()
 * \param [out] rbuf   buffer for incoming data
 * \param [in]  rlen   number of bytes to read
 *
 * \sa ad_yyy_open()
 *
 * \return 0 on success, <0: error
 *
 */
int ad_yyy_read(ad_yyy_handle_t handle, uint8_t *rbuf, size_t rlen);

/**
 * \brief Perform a non blocking read transaction
 *
 * This function performs an asynchronous read only transaction
 * Caller task should retry until function returns no error
 * Callback will be called when transaction is completed
 *
 * \param [in]  handle    handle returned from ad_yyy_open()
 * \param [out] rbuf      buffer for incoming data
 * \param [in]  rlen      number of bytes to read
 * \param [in]  cb        callback to call after transaction is over (from ISR context)
 * \param [in]  user_data user data passed to cb callback
 *
 * \sa ad_yyy_open()
 *
 * \return 0 on success, <0: error
 *
 */
int ad_yyy_read_async(ad_yyy_handle_t handle, uint8_t *rbuf, size_t rlen, ad_yyy_user_cb cb,
        void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* dg_configYYY_ADAPTER */

#endif /* AD_YYY_H_ */

/**
 * \}
 * \}
 */
