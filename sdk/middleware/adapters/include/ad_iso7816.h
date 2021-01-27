/**
 * \addtogroup MID_SYS_ADAPTERS
 * \{
 * \addtogroup ISO7816_ADAPTER ISO7816 Adapter
 *
 * \brief ISO7816 SmartCard adapter
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_iso7816.h
 *
 * @brief ISO7816 adapter API
 *
 * Copyright (C) 2018-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef AD_ISO7816_H_
#define AD_ISO7816_H_

#if dg_configISO7816_ADAPTER

#include <hw_iso7816.h>
#include <osal.h>
#include <hw_gpio.h>
#include <ad.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Data types definitions section
 */

/**
 * \brief ISO7816 Handle returned by ad_iso7816_open()
 */
typedef void *ad_iso7816_handle_t;

/**
 * \brief ISO7816 I/O configuration
 *
 * ISO7816 I/O configuration
 */
typedef struct {
        ad_io_conf_t rst;               /**< Configuration of reset signal */
        ad_io_conf_t data;              /**< Configuration of data signal */
        ad_io_conf_t clk;               /**< Configuration of clock signal */
        HW_GPIO_POWER voltage_level;    /**< Voltage of the device pins */
} ad_iso7816_io_conf_t;

/**
 * \brief ISO7816 driver configuration
 *
 * Configuration of ISO7816 low level driver(s)
 *
 * \note There may be more than one driver configurations needed (e.g DMA)
 */
typedef struct {
        bool manual_control;            /**< Controls if card is automatically activated during
                                             initialization */
        bool opt_speed;                 /**< Controls if a PPS requesting higher speed is
                                             automatically exchanged */
        HW_ISO7816_FIFO_TX_LVL tx_lvl;  /**< Transmit FIFO interrupt trigger level (applicable to
                                             T=1 transactions only) */
        HW_ISO7816_FIFO_RX_LVL rx_lvl;  /**< Receive FIFO interrupt trigger level */
} ad_iso7816_driver_conf_t;

/**
 * \brief ISO7816 controller configuration
 *
 * Configuration of ISO7816 controller
 */
typedef struct {
        const ad_iso7816_io_conf_t *io;       /**< IO configuration*/
        const ad_iso7816_driver_conf_t *drv;  /**< Driver configuration*/
} ad_iso7816_controller_conf_t;

/**
 * \brief ISO7816 adapter error codes
 */
typedef enum {
        AD_ISO7816_ERROR_OTHER               = -4,
        AD_ISO7816_ERROR_CONTROLLER_BUSY     = -3,
        AD_ISO7816_ERROR_DRIVER_CONF_INVALID = -2,
        AD_ISO7816_ERROR_HANDLE_INVALID      = -1,
        AD_ISO7816_ERROR_NONE                = 0
} AD_ISO7816_ERROR;

/**
 * \brief Initialize adapter
 *
 * \note It should ONLY be called by the system.
 */
void ad_iso7816_init(void);

/**
 * \brief Open ISO7816 controller
 *
 * This function:
 * - Acquires the resources needed for using the controller
 * - Configures the controller interface IOs
 * - Initializes the drivers associated with the controller
 * - If device not in manual control (\ref ad_iso7816_driver_conf_t.manual_control) it also performs
 *   an activation of the device, receives and parses ATR, and configures the ISO7816 block
 *   accordingly.
 * - If optimization of speed is permitted (\ref ad_iso7816_driver_conf_t.opt_speed), it issues a
 *   PPS request in case card supports a higher data rate.
 *
 * \param [in] conf             Controller configuration
 *
 * \return !=0: handle that should be used in subsequent API calls, NULL: error
 *
 * \note The function will block until it acquires all controller resources
 */
ad_iso7816_handle_t ad_iso7816_open(const ad_iso7816_controller_conf_t *conf);

/**
 * \brief Reconfigure ISO7816 controller
 *
 * This function will apply a new ISO7816 driver configuration.
 *
 * \param [in] handle           Handle returned from ad_iso7816_open()
 * \param [in] conf             New driver configuration
 *
 * \sa ad_iso7816_open()
 *
 * \return 0: success, <0: error code
 */
int ad_iso7816_reconfig(ad_iso7816_handle_t handle, const ad_iso7816_driver_conf_t *conf);

/**
 * \brief Close ISO7816 controller
 *
 * This function:
 * - If device not in manual control (\ref ad_iso7816_driver_conf_t.manual_control) it de-activates
 *   the device.
 * - De-initializes the drivers associated with the controller
 * - Resets controller interface IOs (as specified in ad_iso7816_open())
 * - Releases the controller resources
 *
 * \param [in] handle           Handle returned from ad_iso7816_open()
 * \param [in] force            Force close even if controller is busy
 *
 * \sa ad_iso7816_open()
 *
 * \return 0: success, <0: error code
 */
int ad_iso7816_close(ad_iso7816_handle_t handle, bool force);

/**
* \brief Initialize controller pins to on / off IO configuration
*
* This function should be called for setting pins to the correct level before external
* devices are powered up (e.g on system init). It does not need to be called before every
* ad_iso7816_open() call.
*
* \param [in] io                Controller IO configuration
* \param [in] state             On/off IO configuration
*
* \return 0: success, <0: error code
*/
int ad_iso7816_io_config (const ad_iso7816_io_conf_t *io, AD_IO_CONF_STATE state);

/**
 * \brief Callback function type to be called when APDU (T=0 or T=1) transaction is complete
 *
 * \param[in] user_data         User data passed at call of transact asynchronous functions
 * \param[in] status            Operation status
 *      \arg \c HW_ISO7816_ERR_CRC          CRC or parity error
 *      \arg \c HW_ISO7816_ERR_CARD_NO_RESP Response was not received, card unresponsive
 *      \arg \c HW_ISO7816_ERR_CARD_ABORTED Operation aborted by card
 *      \arg \c HW_ISO7816_ERR_UNKNOWN      Operation completed unsuccessfully
 *      \arg \c HW_ISO7816_ERR_OK           Operation completed successfully
 * \param[in] sw1sw2            Status code as provided by card. In case of error in communication
 *                              the \c HW_ISO7816_SW1SW2_INVALID value is returned.
 * \param[in] length            Length of the response APDU
 */
typedef void (*ad_iso7816_transact_cb)(void *user_data, HW_ISO7816_ERROR status,
        HW_ISO7816_SW1SW2 sw1sw2, size_t length);

/**
 * \brief Perform a device/card activation
 *
 * This function performs a device / card cold reset and then receives and parses ATR and applies
 * indicated settings to the ISO7816 controller.
 *
 * This function is blocking. It will first wait for device access and then it will wait until
 * the reception of ATR and re-configuration is completed.
 *
 * \param[in]  handle           Handle to the ISO7816 device
 * \param[out] atr              (Optional) The buffer to receive the ATR bytes
 *
 * \return <0: error code, >=0 :size in bytes of the received ATR. Zero size indicates that no ATR
 * was received.
 *
 * \sa ad_iso7816_open()
 * \sa ad_iso7816_close()
 *
 * \note Device / card activation is automatically performed during initial open of the device when
 * \ref ad_iso7816_driver_conf_t.manual_control parameter of the device is set to false. Therefore,
 * it should not be called directly in such case.
 */
int ad_iso7816_activate(ad_iso7816_handle_t handle, uint8_t *atr);

/**
 * \brief Perform a device/card warm reset
 *
 * This function performs a device / card warm reset as described in ISO7816 specification.
 *
 * This function is blocking. It will wait for device access.
 *
 * \param[in]  handle           Handle to the ISO7816 device
 *
 * \sa ad_iso7816_open()
 * \sa ad_iso7816_close()
 *
 * \return 0: success, <0: error code
 */
int ad_iso7816_warm_reset(ad_iso7816_handle_t handle);

/**
 * \brief Perform a device/card de-activation
 *
 * This function performs a device / card de-activation and stops clock generation.
 *
 * This function is blocking. It will wait for device access.
 *
 * \param[in]  handle           Handle to the ISO7816 device
 *
 * \sa ad_iso7816_open()
 * \sa ad_iso7816_close()
 *
 * \note Device / card de-activation is automatically performed during final close of the device
 * when \ref ad_iso7816_driver_conf_t.manual_control parameter of the device is set to false.
 * Therefore, it should not be called directly in such case.
 *
 * \return 0: success, <0: error code
 */
int ad_iso7816_deactivate(ad_iso7816_handle_t handle);

/**
 * \brief Stop clock of ISO7816
 *
 * This function checks if the clock stop is supported by the card and stops it in such a case. To
 * resume the clock function \ref ad_iso7816_resume_clock() MUST be called
 *
 * \param[in] handle            Handle to the ISO7816 device
 *
 * \sa ad_iso7816_resume_clock
 *
 * \warning Clock MUST be resumed before any ISO7816 operation / interaction in case it was stopped
 *
 * \warning The device must own the bus before calling this function (ad_iso7816_open() must be
 * called previously from task execution context).
 *
 * \return 0: success, <0: error code
 */
int ad_iso7816_stop_clock(ad_iso7816_handle_t handle);

/**
 * \brief Resume clock of ISO7816
 *
 * This function resumes clock that was previously stopped with \ref  ad_iso7816_stop_clock()
 *
 * \param[in] handle            Handle to the ISO7816 device
 *
 * \sa ad_iso7816_stop_clock
 *
 * \warning The device must own the bus before calling this function (ad_iso7816_open() must be
 * called previously from task execution context).
 *
 * \return 0: success, <0: error code
 */
int ad_iso7816_resume_clock(ad_iso7816_handle_t handle);

/**
 * \brief Perform a PPS exchange with the device / card
 *
 * This function performs a PPS exchange with the device / card using the provided parameters.
 *
 * Parameters are grouped into PPS bytes and they are the following:
 *
 *  PPS byte  |  Parameters  |  Mandatory
 * ---------- | ------------ | -------------------
 *  PPS0      |  T           |  Yes
 *  PPS1      |  f, fmax, d  |  No
 *  PPS2      |  SPU         |  No
 *
 * The non mandatory bytes and the corresponding parameters are omitted if the \p pps1 or \p pps2
 * parameters are set to false.
 *
 * This function is blocking. It will first wait for device access and then it will wait until
 * the reception of PPS response is received or a timeout occurs.
 * In a successful PPS exchange, new connection parameters will automatically be applied, otherwise
 * card MUST be deactivated using \ref ad_iso7816_deactivate() in case of manual mode
 * (\ref ad_iso7816_driver_conf_t.manual_control set to 1) or using \ref ad_iso7816_close() function
 * (\ref ad_iso7816_driver_conf_t.manual_control set to 0).
 *
 * \param[in]     handle        Handle to the ISO7816 device
 * \param[in,out] params        PPS parameters as defined in hw_iso7816_pps_params_t.
 *                              Parameters provided are sent to the card and the response is parsed
 *                              and applied to the \p params as well.
 * \param[in]     pps1          True if PPS request will include PPS1 byte (speed parameters)
 * \param[in]     pps2          True if PPS request will include PPS2 byte (SPU parameters)
 *
 * \return 0: success, !0: error code
 * \retval HW_ISO7816_ERR_PPS                    Error in PPS exchange procedure
 * \retval HW_ISO7816_ERR_CRC                    PCK verification failed
 * \retval HW_ISO7816_ERR_CARD_NO_RESP           Response was not received, card unresponsive
 * \retval HW_ISO7816_ERR_PPS_PARTIALLY_ACCEPTED PPS successful, card proposed different parameters
 * \retval HW_ISO7816_ERR_OK                     PPS successful, card accepted parameters
 *
 * \sa ad_iso7816_open()
 * \sa ad_iso7816_close()
 *
 * \warning Device / card PPS exchange is automatically performed during initial open of the device
 * when \ref ad_iso7816_driver_conf_t.opt_speed parameter of the device is set to true and the card
 * supports higher data rate. In such case, this function MUST NOT be called directly, since only
 * one PPS can occur.
 */
int ad_iso7816_exchange_pps(ad_iso7816_handle_t handle, hw_iso7816_pps_params_t *params,
        bool pps1, bool pps2);

/**
 * \brief Perform APDU transaction
 *
 * This function performs an APDU transaction on the ISO7816 bus.
 *
 * The buffer containing the command APDU to be transmitted is sent over ISO7816 bus using the
 * already configured protocol (T=0 or T=1). Also, caller must provide an allocated space for the
 * response APDU which will fit the complete response APDU.
 *
 * This function is blocking. It will first wait for device access and then it will wait until the
 * transaction is completed.
 *
 * Example usage:
 * \code{.c}
 * {
 *   ad_iso7816_handle_t handle = ad_iso7816_open(CARD0);
 *   if (handle != NULL) {
 *     ad_iso7816_apdu_transact(handle, ...);
 *     ad_iso7816_apdu_transact(handle, ...);
 *     ...
 *     ad_iso7816_close(handle);
 *   }
 *   else {
 *     //Handle exception
 *   }
 * }
 * \endcode
 *
 * \param[in]  handle           Handle to the ISO7816 device
 * \param[in]  tx_apdu          Pointer to the command APDU
 * \param[in]  tx_len           Size of the command APDU
 * \param[out] rx_apdu          Pointer to buffer for the response APDU
 * \param[out] rx_len           Pointer to variable for the response length
 *
 * \return 0: success, !0: error code
 * \retval HW_ISO7816_ERR_CRC          CRC or parity error
 * \retval HW_ISO7816_ERR_CARD_NO_RESP Response was not received, card unresponsive
 * \retval HW_ISO7816_ERR_CARD_ABORTED Operation aborted by card
 * \retval HW_ISO7816_ERR_UNKNOWN      Operation completed unsuccessfully
 * \retval HW_ISO7816_ERR_OK           Operation completed successfully
 *
 * \sa ad_iso7816_open()
 * \sa ad_iso7816_close()
 */
int ad_iso7816_apdu_transact(ad_iso7816_handle_t handle, const uint8_t *tx_apdu, size_t tx_len,
        uint8_t *rx_apdu, size_t *rx_len);

/**
 * \brief Perform a T=1 supervisory transaction
 *
 * Supervisory blocks are used by the T=1 ISO7816 protocol in order to exchange control information
 * i.e. wait time extension, re-synch request, maximum size of INF field etc.
 *
 * \param[in] handle            Handle to the ISO7816 device
 * \param[in] type              Supervisory PCB type
 * \param[in] value             INF value (if applicable)
 *
 * \return 0: success, !0: error code
 * \retval HW_ISO7816_ERR_CRC          CRC or parity error
 * \retval HW_ISO7816_ERR_CARD_NO_RESP Response was not received, card unresponsive
 * \retval HW_ISO7816_ERR_CARD_ABORTED Operation aborted by card
 * \retval HW_ISO7816_ERR_UNKNOWN      Operation completed unsuccessfully
 * \retval HW_ISO7816_ERR_OK           Operation completed successfully
 *
 * \sa ad_iso7816_open()
 * \sa ad_iso7816_close()
 */
int ad_iso7816_supervisory_transact_t1(ad_iso7816_handle_t handle, HW_ISO7816_S_PCB_VAL type,
        uint8_t value);

/**
 * \brief Perform asynchronous APDU transaction.
 *
 * This function performs an APDU transaction on the ISO7816 bus.
 * The callback function is executed when transaction has been completed. At this point response
 * APDU data are available to be used by the application.
 *
 * This function is not blocking. It returns before the transaction starts and the provided
 * resources must wait the callback function to be called before they can be altered / deleted.
 *
 * \param[in]  handle           Handle to the ISO7816 device
 * \param[in]  tx_apdu          Pointer to the command APDU
 * \param[in]  tx_len           Size of the command APDU
 * \param[out] rx_apdu          Pointer to buffer for the response APDU
 * \param[in]  cb               Callback to call after transaction is over (from ISR context)
 * \param[in]  user_data        User data to pass to \p cb
 *
 * \sa ad_iso7816_open()
 * \sa ad_iso7816_close()
 *
 * \return 0: success, <0: error code
 */
int ad_iso7816_apdu_transact_async(ad_iso7816_handle_t handle, const uint8_t *tx_apdu, size_t tx_len,
        uint8_t *rx_apdu, ad_iso7816_transact_cb cb, void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* dg_configISO7816_ADAPTER */

#endif /* AD_ISO7816_H_ */

/**
 * \}
 * \}
 */
