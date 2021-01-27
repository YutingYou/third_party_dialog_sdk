/**
 * \addtogroup MID_SYS_ADAPTERS
 * \{
 * \addtogroup SDADC_ADAPTER SDADC Adapter
 *
 * \brief Sigma Delta Analog-Digital Converter adapter
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_sdadc.h
 *
 * @brief SDADC adapter API
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef AD_SDADC_H_
#define AD_SDADC_H_

#if dg_configSDADC_ADAPTER

#include "ad.h"
#include "hw_gpio.h"
#include "hw_sdadc.h"
#include "osal.h"
#include "resmgmt.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief SDADC Id
 */
#define HW_SDADC        ((void *)SDADC_BASE)
typedef void *HW_SDADC_ID;

/**
* \brief SDADC I/O configuration
*/
typedef struct ad_sdadc_io_conf {
        ad_io_conf_t input0;
        ad_io_conf_t input1;
        HW_GPIO_POWER voltage_level;
} ad_sdadc_io_conf_t;

/**
 * \brief SDADC driver configuration
 *
 * Configuration of the SDADC low level driver
 *
 * \note There may be more than one driver configurations needed (e.g DMA)
 *
 */
typedef sdadc_config ad_sdadc_driver_conf_t;

/**
 * \brief SDADC controller configuration
 *
 * Configuration of SDADC controller
 *
 * \note There may be more than one controller configurations needed (e.g DMA)
 *
 */
typedef struct ad_sdadc_controller_conf {
        const HW_SDADC_ID                id;    /**< Controller instance*/
        const ad_sdadc_io_conf_t        *io;    /**< IO configuration*/
        const ad_sdadc_driver_conf_t    *drv;   /**< Driver configuration*/
} ad_sdadc_controller_conf_t;

 /**
 * \brief SDADC Handle returned by ad_sdadc_open()
 */
typedef void *ad_sdadc_handle_t;

/**
 * \brief Asynchronous callback function
 *
 */
typedef void (*ad_sdadc_user_cb)(void *user_data, int value);

/**
 * \brief Error Codes
 *
 */
typedef enum {
        AD_SDADC_ERROR_NONE                   =  0,
        AD_SDADC_ERROR_HANDLE_INVALID         = -1,
        AD_SDADC_ERROR_DRIVER_CONF_INVALID    = -2,
        AD_SDADC_ERROR_DRIVER_INPUT_INVALID   = -3,
        AD_SDADC_ERROR_DRIVER_MODE_INVALID    = -4,
        AD_SDADC_ERROR_DRIVER_UNINITIALIZED   = -5,
        AD_SDADC_ERROR_IO_CONF_INVALID        = -6,
        AD_SDADC_ERROR_CB_INVALID             = -7,
        AD_SDADC_ERROR_READ_IN_PROGRESS       = -8,
        AD_SDADC_ERROR_CANNOT_ACQUIRE         = -9,
        AD_SDADC_ERROR_ID_INVALID             = -10,
} AD_SDADC_ERROR;

/**
 * \brief Initialize SDADC adapter and some required variables
 *
 * \note: It should ONLY be called by the system.
 *
 */
void ad_sdadc_init(void);

/**
 * \brief Open SDADC controller
 *
 * This function:
 * - Acquires the resources needed for using the controller
 * - Configures the controller interface IOs
 * - Initializes the drivers associated with the controller
 *
 * \param [in] conf controller configuration
 *
 * \return >0: handle that should be used in subsequent API calls, NULL: error
 *
 * \note The function will block until it acquires all controller resources
 */
ad_sdadc_handle_t ad_sdadc_open(const ad_sdadc_controller_conf_t *conf);

/**
 * \brief Reconfigure SDADC controller
 *
 * This function will apply a new SDADC driver configuration.
 *
 * \param [in] handle handle returned from ad_sdadc_open()
 * \param [in] drv    new driver configuration
 *
 * \sa ad_sdadc_open()
 * \sa AD_SDADC_ERROR
 *
 * \return 0: success, <0: error code
 */
int ad_sdadc_reconfig(ad_sdadc_handle_t handle, const ad_sdadc_driver_conf_t *drv);

/**
 * \brief Close SDADC controller
 *
 * This function:
 * - Aborts ongoing transactions
 * - De-initializes the drivers associated with the controller
 * - Resets controller interface IOs (as specified in ad_xxx_open())
 * - Releases the controller resources
 *
 * \param [in] handle handle returned from ad_sdadc_open()
 * \param [in] forced force adapter closing
 *
 * \sa ad_sdadc_open()
 * \sa AD_SDADC_ERROR
 *
 * \return 0: success, <0: error code
 */
int ad_sdadc_close(ad_sdadc_handle_t handle, bool forced);

/**
 * \brief Initialize controller pins to on / off io configuration
 *
 * This function should be called for setting pins to the correct level before external
 * devices are powered up (e.g on system init). It does not need to be called before every
 * ad_sdadc_open() call.
 *
 * \param [in] id         controller instance
 * \param [in] io         controller io configuration
 * \param [in] state      on/off io configuration
 *
 * \sa AD_SDADC_ERROR
 *
 * \return 0: success, <0: error code
 */
int ad_sdadc_io_config(HW_SDADC_ID id, const ad_sdadc_io_conf_t *io, AD_IO_CONF_STATE state);

/**
 * \brief Perform a non blocking read transaction
 *
 * This function performs an asynchronous read transaction
 * Caller task should retry until function returns no error
 * Callback will be called when transaction is completed
 *
 * \param [in]  handle    handle returned from ad_sdadc_open()
 * \param [in]  cb        callback to call after transaction is over (from ISR context)
 * \param [in]  user_data user data passed to cb callback
 *
 * \sa ad_sdadc_open()
 * \sa AD_SDADC_ERROR
 *
 * \return 0 on success, <0: error
 *
 */
int ad_sdadc_read_async(ad_sdadc_handle_t handle, ad_sdadc_user_cb cb, void *user_data);

/**
 * \brief Perform a blocking read transaction
 *
 * This function performs a synchronous read only transaction
 *
 * \param [in]  handle handle returned from ad_sdadc_open()
 * \param [out] value  value containing the read result
 *
 * \sa ad_sdadc_open()
 * \sa AD_SDADC_ERROR
 *
 * \return 0 on success, <0: error
 *
 */
int ad_sdadc_read(ad_sdadc_handle_t handle, int32_t *value);

#ifdef __cplusplus
}
#endif

#endif /* dg_configSDADC_ADAPTER */

#endif /* AD_SDADC_H_ */

/**
 * \}
 * \}
 */
