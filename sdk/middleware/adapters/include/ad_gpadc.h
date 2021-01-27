/**
 * \addtogroup MID_SYS_ADAPTERS
 * \{
 * \addtogroup GPADC_ADAPTER GPADC Adapter
 *
 * \brief General Purpose Analog-Digital Converter adapter
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_gpadc.h
 *
 * @brief GPADC adapter API
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if (dg_configGPADC_ADAPTER == 1)

#ifndef AD_GPADC_H_
#define AD_GPADC_H_

#include "ad.h"
#include "hw_gpadc.h"
#include "hw_gpio.h"
#include "osal.h"
#include "resmgmt.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief GPADC I/O configuration
 *
 * GPADC I/O configuration
 */

typedef struct ad_gpadc_io_conf {
        ad_io_conf_t input0;
        ad_io_conf_t input1;
        HW_GPIO_POWER voltage_level;
} ad_gpadc_io_conf_t;

/**
 * \brief GPADC driver configuration
 *
 * Configuration of GPADC low level driver(s)
 *
 * \note There may be more than one driver configurations needed (e.g DMA)
 *
 */
typedef gpadc_config ad_gpadc_driver_conf_t;

/**
 * \brief GPADC controller instance
 */
#define HW_GPADC_1                      ((void *)GPADC_BASE)

/**
 * \brief GPADC controller configuration
 *
 * Configuration of GPADC controller
 *
 */
typedef struct ad_gpadc_controller_conf {
        const HW_GPADC_ID               id;        /**< Controller instance*/
        const ad_gpadc_io_conf_t       *io;        /**< IO configuration*/
        const ad_gpadc_driver_conf_t   *drv;       /**< Driver configuration*/
} ad_gpadc_controller_conf_t;

/**
 * \brief GPADC Handle returned by ad_gpadc_open()
 */
typedef void *ad_gpadc_handle_t;

/**
 * \brief enum with return values of API calls
 */
typedef enum {
        AD_GPADC_ERROR_NONE                    =  0,
        AD_GPADC_ERROR_HANDLE_INVALID          = -1,
        AD_GPADC_ERROR_CHANGE_NOT_ALLOWED      = -2,
        AD_GPADC_ERROR_ADAPTER_NOT_OPEN        = -3,
        AD_GPADC_ERROR_CONFIG_INVALID          = -4,
        AD_GPADC_ERROR_ASYNC_READ_IN_PROGRESS  = -5,
        AD_GPADC_ERROR_TIMEOUT                 = -6,
        AD_GPADC_ERROR_OTHER                   = -7,
} AD_GPADC_ERROR;

/**
 * \brief GPADC oversampling
 *
 * In this mode multiple successive conversions will be executed and the results are added together
 * to increase the effective number of bits
 *
 */
typedef enum {
        HW_GPADC_OVERSAMPLING_1_SAMPLE          = 0,    /**< 1 sample is taken or 2 in case chopping is enabled */
        HW_GPADC_OVERSAMPLING_2_SAMPLES         = 1,    /**< 2 samples are taken */
        HW_GPADC_OVERSAMPLING_4_SAMPLES         = 2,    /**< 4 samples are taken */
        HW_GPADC_OVERSAMPLING_8_SAMPLES         = 3,    /**< 8 samples are taken */
        HW_GPADC_OVERSAMPLING_16_SAMPLES        = 4,    /**< 16 samples are taken */
        HW_GPADC_OVERSAMPLING_32_SAMPLES        = 5,    /**< 32 samples are taken */
        HW_GPADC_OVERSAMPLING_64_SAMPLES        = 6,    /**< 64 samples are taken */
        HW_GPADC_OVERSAMPLING_128_SAMPLES       = 7     /**< 128 samples are taken */
} HW_GPADC_OVERSAMPLING;

/**
 * \brief Asynchronous callback function
 *
 */
typedef void (*ad_gpadc_user_cb)(void *user_data, int value);

/**
 * \brief Initialize GPADC adapter and some required variables
 *
 * \warning     Do not call this function directly. It is called
 *              automatically during power manager initialization.
 *
 */
void ad_gpadc_init(void);

/**
 * \brief Read value of the measurement from the selected source
 *
 * This function reads measurement value synchronously.
 *
 * \param [in] handle   handle to GPADC source
 * \param [out] value   pointer to data to be read from selected source
 *
 * \sa ad_gpadc_open()
 * \sa ad_gpadc_close()
 *
 */
int ad_gpadc_read(ad_gpadc_handle_t handle, uint16_t *value);


/**
 * \brief Read the raw value of the measurement from the selected source
 *
 * This function reads measurement value synchronously.
 *
 * \param [in] handle   handle to GPADC source
 * \param [out] value   pointer to data to be read from selected source
 *
 * \sa ad_gpadc_open()
 * \sa ad_gpadc_close()
 *
 */
int ad_gpadc_read_raw(ad_gpadc_handle_t handle, uint16_t *value);

/**
 * \brief Attempt to read the measurement from the selected source within a timeout period.
 *
 * This function attempts to read measurement value synchronously
 *
 * \param [in]  handle handle to GPADC source
 * \param [out] value pointer to data to be read from selected source
 * \param [in]  timeout number of ticks to wait
 *              0 - no wait take GPADC if it is available
 *              RES_WAIT_FOREVER - wait until GPADC becomes available
 *              Other value specifies how many ticks to wait until GPADC becomes available
 *
 * \return 0 on success, <0: error
 *
 */
int ad_gpadc_read_to(ad_gpadc_handle_t handle, uint16_t *value, uint32_t timeout);

/**
 * \brief Attempt to read the raw measurement from the selected source within a timeout period.
 *
 * This function attempts to read measurement value synchronously
 *
 * \param [in]  handle handle to GPADC source
 * \param [out] value pointer to data to be read from selected source
 * \param [in]  timeout number of ticks to wait
 *              0 - no wait take GPADC if it is available
 *              RES_WAIT_FOREVER - wait until GPADC becomes available
 *              Other value specifies how many ticks to wait until GPADC becomes available
 *
 * \return 0 on success, <0: error
 *
 */
int ad_gpadc_read_raw_to(ad_gpadc_handle_t handle, uint16_t *value, uint32_t timeout);

/**
 * \brief Read asynchronously value of the measurement from the selected source
 *
 * This function starts asynchronous measurement read.
 *
 * \param [in] handle           handle to GPADC source
 * \param [in] read_async_cb    user callback fired after read operation completes
 * \param [in] user_data        pointer to user data
 *
 * \sa ad_gpadc_open()
 * \sa ad_gpadc_close()
 *
 * \return 0 on success, <0: error
 *
 */
int ad_gpadc_read_async(ad_gpadc_handle_t handle, ad_gpadc_user_cb read_async_cb, void *user_data);

/**
 * \brief Return maximum value that can be read for ADC source
 *
 * Value returned by ad_gpadc_read() can have 10 to 16 bits (right aligned) depending on
 * oversampling specified in source description. This function will return value
 * 0x3FF, 0x7FF, 0xFFF, 0x1FFF, 0x3FFF, 0x7FFF or 0xFFFF depending on oversampling.
 *
 * \param [in] drv   GPADC driver configuration structure
 *
 * \return maximum value that can be returned by ad_gpadc_read
 *
 */
uint16_t ad_gpadc_get_source_max(const ad_gpadc_driver_conf_t *drv);

/**
 * \brief Attempt to acquire exclusive access on GPADC within a timeout period.
 *
 * This function waits for GPADC to become available until the timeout expires.
 * When GPADC is acquired the resource is locked and requires an ad_gpadc_release() call to
 * unlock it.
 *
 * \param [in] timeout number of ticks to wait
 *             0 - no wait take GPADC if it is available
 *             RES_WAIT_FOREVER - wait until GPADC becomes available
 *             Other value specifies how many ticks to wait until GPADC becomes available
 *
 * \return true if GPADC has been acquired, false otherwise
 *
 */
bool ad_gpadc_acquire_to(uint32_t timeout);

/**
 * \brief Open GPADC controller
 *
 * This function:
 * - Acquires the resources needed for using the controller
 * - Configures the controller interface IOs
 * - Initializes the drivers associated with the controller
 *
 *
 * \param [in] conf controller configuration
 *
 * \return >0: pointer to adapter instance - should be used in subsequent API calls, <0: error code
 *
 * \note The function will block until it acquires all controller resources
 */
ad_gpadc_handle_t ad_gpadc_open(const ad_gpadc_controller_conf_t *conf);

/**
 * \brief Reconfigure GPADC controller
 *
 * This function will apply a new GPADC driver configuration.
 *
 * \param [in] p     pointer returned from ad_gpadc_open()
 * \param [in] drv   GPADC driver configuration structure
 *
 * \return 0: success, <0: error code
 */
int ad_gpadc_reconfig(ad_gpadc_handle_t p, const ad_gpadc_driver_conf_t *drv);

/**
* \brief Initialize controller pins to on / off io configuration
*
* This function should be called for setting pins to the correct level before external
* devices are powered up (e.g on system init). It does not need to be called before every
* ad_gpadc_open() call.
*
* \param [in] id         controller instance
* \param [in] io         controller io configuration
* \param [in] state      on/off io configuration
*
* \return 0: success, <0: error code
*/
int ad_gpadc_io_config (const HW_GPADC_ID id, const ad_gpadc_io_conf_t *io, AD_IO_CONF_STATE state);

/**
 * \brief Close GPADC controller
 *
 * This function:
 * - Aborts ongoing transactions
 * - De-initializes the drivers associated with the controller
 * - Resets controller interface IOs (as specified in ad_gpadc_open())
 * - Releases the controller resources
 *
 * \param [in] p        pointer returned from ad_gpadc_open()
 * \param [in] force    force close even if an async read is pending
 *
* \return 0: success, <0: error code
 */
int ad_gpadc_close(ad_gpadc_handle_t p, bool force);

/**
 * \brief Convert raw value read from GPADC to temperature value in degrees Celsius
 *
 * \param [in] drv      GPADC driver configuration structure
 * \param [in] value    value returned from ad_gpadc_read() or ad_gpadc_read_async()
 *
 * \return value of temperature in degrees Celsius
 *
 */
__STATIC_INLINE int ad_gpadc_conv_to_temp(const ad_gpadc_driver_conf_t *drv, uint16_t value)
{
        return hw_gpadc_convert_to_temperature( (gpadc_config *) drv, value);
}

/**
 * \brief Convert raw value read from GPADC to battery voltage in mV
 *
 * \param [in] drv    GPADC driver configuration structure
 * \param [in] value  value returned from ad_gpadc_read() or ad_gpadc_read_async()
 *
 * \return value in mV
 *
 */
__STATIC_INLINE uint16_t ad_gpadc_conv_to_batt_mvolt(const ad_gpadc_driver_conf_t *drv, uint32_t value)
{
        /* Convert to mV, take into account that scaler from 5->1.2 */
        return (((uint32_t) 5000 * value) / ad_gpadc_get_source_max(drv));
}

#ifdef __cplusplus
}
#endif

#endif /* AD_GPADC_H_ */

#endif /* dg_configGPADC_ADAPTER */

/**
 * \}
 * \}
 */
