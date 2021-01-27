/**
 * \addtogroup MID_SYS_ADAPTERS
 * \{
 * \addtogroup HAPTIC_ADAPTER HAPTIC Adapter
 *
 * @brief Haptic Adapter
 *
 * This Adapter is a middleware abstraction layer for using the Haptic Driver and Controller
 * hardware module (or simply Haptic Driver) and the haptic related software modules in order
 * to drive haptic actuators (LRA/ERM).
 *
 * It provides the following services:
 * - Arbitration in accessing the Haptic Driver by multiple tasks
 * - Power stability (blocks sleep while the Haptic Driver is in use and ensures the appropriate
 * power domains are on)
 * - Easy and simple configuration and usage of the Haptic Driver via the Haptic Low
 * Level Software Driver and the haptic processing SW modules of the SDK (Haptics Algorithm
 * Library and Waveform Memory Decoder)
 *
 * Functions provided:
 * - ad_haptic_init():                     Called by the system on power up for initializing the
 *                                         Haptic Adapter. It should NOT be called by the application.
 * - ad_haptic_open():                     Called by the application for initializing the Haptic
 *                                         Driver and the haptic SW modules, i.e. for
 *                                         opening a *haptic operating session*. The sleep is
 *                                         blocked until ad_haptic_close() is called.
 * - ad_haptic_reconfig():                 Apply a new haptic configuration
 * - ad_haptic_set_drive_level():          Set the drive intensity level (directly or indirectly,
 *                                         depending on the current drive mode)
 * - ad_haptic_set_polarity():             Set the drive polarity
 * - ad_haptic_set_half_period():          Set the half period
 * - ad_haptic_set_state():                Set the Haptic Driver state (Active or Idle)
 * - ad_haptic_set_drive_mode():           Set the current drive mode (enable/disable the
 *                                         Overdrive and Frequency Control components of the Haptic
 *                                         Algorithm Library)
 * - ad_haptic_play_wm_sequence():         Play a waveform sequence from the waveform memory (with
 *                                         a specific two-level priority)
 * - ad_haptic_stop_wm_sequence():         Stop the currently playing waveform sequence
 * - ad_haptic_update_drive_parameters():  Update the drive parameters (to be used as the interrupt
 *                                         callback function of the Haptic Low Level SW Driver)
 * - ad_haptic_close();                    Called by the application for terminating the haptic
 *                                         operating session. System is allowed to go to sleep.
 *
 * @note Only applicable to DA14697/9 devices of the DA1469x chip family.
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_haptic.h
 *
 * @brief Haptic Driver/Controller access API
 *
 * Copyright (C) 2019-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef AD_HAPTIC_H_
#define AD_HAPTIC_H_

#if ((DEVICE_VARIANT == DA14697) || (DEVICE_VARIANT == DA14699))

#if dg_configHAPTIC_ADAPTER

#include "hw_haptic.h"
#include "haptics_lib.h"
#include "wm_decoder.h"
#include "osal.h"
#include "sys_power_mgr.h"

#ifdef __cplusplus
extern "C" {
#endif


/****************** MACRO DEFINITIONS ******************/


/**
 * @brief Waveform sequence timebase (in us)
 */
#ifndef AD_HAPTIC_WM_SEQUENCE_TIMEBASE
#define AD_HAPTIC_WM_SEQUENCE_TIMEBASE 1365
#endif


/****************** DATA TYPE DEFINITIONS ******************/


/********* Basic types *********/

/**
 * @brief Handle returned by ad_haptic_open()
 *
 * This defines the type of the handle that is returned by ad_haptic_open().
 * The handle is actually a pointer to the Adapter's internal dynamic data.
 * After the haptic operating session is opened, it should be provided on every other haptic
 * operation request (as a parameter to the other functions of the Adapter).
 *
 * @sa ad_haptic_open()
 */
typedef void *ad_haptic_handle_t;

/**
 * @brief User data parameter of the asynchronous user callback function
 *
 * This defines the type of the user data parameter to be passed to the asynchronous user callback
 * function [see ad_haptic_user_async_cb_t].
 *
 * @sa ad_haptic_user_async_cb_t, ad_haptic_play_wm_sequence()
 */
typedef void *ad_haptic_user_async_cb_data_t;


/********* Enumerations *********/

/**
 * @brief Overdrive state
 *
 * This defines the different enable states of the Overdrive component of the Haptics Algorithm
 * Library (if used).
 *
 * @sa ad_haptic_wm_conf_t, ad_haptic_play_wm_sequence()
 */
typedef enum {
        AD_HAPTIC_OVERDRIVE_OFF = 0, /**< Overdrive inactive */
        AD_HAPTIC_OVERDRIVE_ON  = 1, /**< Overdrive active  */
} AD_HAPTIC_OVERDRIVE;

#if dg_configUSE_HW_LRA
/**
 * \brief Frequency Control state
 *
 * This defines the different enable states of the Frequency Control component of the Haptics
 * Algorithm Library (if used).
 *
 * @note LRA only
 *
 * @sa ad_haptic_wm_conf_t, ad_haptic_play_wm_sequence()
 */
typedef enum {
        AD_HAPTIC_FREQ_CTRL_OFF = 0, /**< Frequency control inactive */
        AD_HAPTIC_FREQ_CTRL_ON  = 1, /**< Frequency control active */
} AD_HAPTIC_FREQ_CTRL;

#endif /* dg_configUSE_HW_LRA */
/**
 * @brief Return status error code
 *
 * This defines the different error codes that are returned by the Adapter's functions.
 *
 * @sa ad_haptic_reconfig(), ad_haptic_set_drive_level(), ad_haptic_set_polarity()
 *     ad_haptic_set_half_period(), ad_haptic_set_mode(), ad_haptic_play_wm_sequence()
 *     ad_haptic_set_state(), ad_haptic_stop_wm_sequence(), ad_haptic_close()
 */
typedef enum {
        AD_HAPTIC_ERROR_DEVICE_DISABLED = -5, /**< The Haptic Driver is in Idle state. Call
                                                   ad_haptic_set_state(HW_HAPTIC_STATE_ACTIVE) to
                                                   enable the drive signal. */
        AD_HAPTIC_ERROR_NOT_APPLICABLE = -4,  /**< Requested Haptic operation not applicable,
                                                   possibly because of the currently selected
                                                   haptic mode. Call ad_haptic_set_mode() to change
                                                   mode. */
        AD_HAPTIC_ERROR_DEVICE_CLOSED = -3,   /**< Haptic operation is no longer active due to a
                                                   ad_haptic_close() call. The Haptic Driver got
                                                   shut down. Call ad_haptic_open() to open a new
                                                   haptic operating session. */
        AD_HAPTIC_ERROR_CONTROLLER_BUSY = -2, /**< Ongoing haptic operation still in progress.
                                                   Resources not released within the specified
                                                   timeout. Call ad_haptic_stop_wm_sequence()
                                                   to stop an ongoing playback or use the urgency
                                                   parameter (if available for the requested haptic
                                                   operation). */
        AD_HAPTIC_ERROR_HANDLE_INVALID = -1,  /**< The specified handle is not valid. */
        AD_HAPTIC_ERROR_NONE = 0,             /**< No error. Requested haptic operation was (or is
                                                   being) executed successfully. */
} AD_HAPTIC_ERROR;

/**
 * @brief Waveform sequence playback end type
 *
 * This defines the different playback end types that will be automatically passed as a parameter
 * to the user callback function [see ad_haptic_user_async_cb_t] in order to provide information of
 * whether the waveform sequence was fully completed or stopped unexpectedly by another haptic
 * operation.
 *
 * @sa ad_haptic_user_async_cb_t, ad_haptic_play_wm_sequence()
 */
typedef enum {
        AD_HAPTIC_PLAYBACK_END_TYPE_NORMAL = 0, /**< The playback ended because the sequence was
                                                     fully completed. */
        AD_HAPTIC_PLAYBACK_END_TYPE_STOP = 1,   /**< The playback ended because it was
                                                     "unexpectedly" stopped by a
                                                     ad_haptic_stop_wm_sequence() or
                                                     ad_haptic_close() call, or because it was
                                                     "outplaced" by a more urgent sequence (i.e. a
                                                     more urgent ad_haptic_play_wm_sequence() call). */
} AD_HAPTIC_PLAYBACK_END_TYPE;


/********* Functions *********/

/**
 * @brief Asynchronous playback callback function
 *
 * This defines the type of function that can be requested to be called as soon as an asynchronous
 * waveform sequence playback ends.
 *
 * @sa ad_haptic_play_wm_sequence()
 */
typedef void (*ad_haptic_user_async_cb_t)(ad_haptic_user_async_cb_data_t user_data, AD_HAPTIC_PLAYBACK_END_TYPE end_type);


/********* Structures *********/

/**
 * @brief Waveform playback configuration
 *
 * This defines the configuration parameter structure for initializing and using the Waveform
 * Memory Decoder SW module (that is used during waveform sequence playbacks).
 * It is part of the Adapter's configuration parameter structure.
 *
 * @sa ad_haptic_controller_conf_t, ad_haptic_play_wm_sequence()
 */
typedef struct {
        const uint8_t *data;      /**< Pointer to the byte array buffer in which the encoded
                                       waveform sequences are stored (waveform memory). */
        bool accel_en;            /**< Indicates whether or not the waveform sequence amplitude is
                                       in unsigned (true, positive values only) or signed format
                                       (false, positive and negative values). If false (signed
                                       format case), then the acceleration and braking is supposed
                                       to be encoded in the waveform sequence, so, in this case,
                                       Overdrive is not applicable. */
        bool timebase;            /**< If false, then the waveform timebase is
                                       AD_HAPTIC_WM_SEQUENCE_TIMEBASE * 4, otherwise it is
                                       AD_HAPTIC_WM_SEQUENCE_TIMEBASE. */
} ad_haptic_wm_conf_t;

/**
 * @brief Haptic Adapter configuration
 *
 * This defines the collective configuration parameter structure for configuring the Haptic
 * Driver and the haptic SW modules.
 *
 * @sa ad_haptic_open(), ad_haptic_reconfig()
 */
typedef struct {
        const haptic_config_t   *drv;               /**< Haptic Driver/Controller configuration
                                                         (for the Haptic Low Level SW Driver).
                                                         @note drv->interrupt_cb should be set to
                                                         ad_haptic_update_drive_parameters() */
        haptics_lib_params *lib;                    /**< Haptics Algorithm Library configuration.
                                                         Set to NULL, in case of not using the
                                                         Library. */
        const ad_haptic_wm_conf_t *wm;              /**< Waveform Memory Decoder configuration.
                                                         Set to NULL, in case of not using the
                                                         Decoder. */
} ad_haptic_controller_conf_t;

/**
 * \brief Haptic Drive mode
 *
 * This defines the structure holding the overall enable state of the Overdrive and Frequency Control components of the
 * Haptics Algorithm Library (if used).
 *
 * \sa ad_haptic_set_drive_mode()
 */
typedef struct {
        AD_HAPTIC_OVERDRIVE    overdrive;           /** < Overdrive component state */
#if dg_configUSE_HW_LRA
        AD_HAPTIC_FREQ_CTRL    freq_ctrl;           /** < Frequency Control component state */
#endif
} ad_haptic_drive_mode_t;


/****************** FUNCTION DECLARATIONS/DEFINITIONS ******************/


/**
 * @brief Initialize Haptic Adapter
 *
 * @note It should ONLY be called by the system.
 */
void ad_haptic_init(void);

/**
 * @brief Open a haptic operating session
 *
 * This function:
 * - Acquires the resources needed for using the Haptic Driver and the Adapter
 * - Ensures that the related Power Domain is powered up and that the system does not go to sleep
 * until the haptic operating session is closed (by a ad_haptic_close() call)
 * - Initializes the Haptic Driver and the haptic SW modules
 *
 * @param [in] conf: Haptic Adapter configuration
 *
 * @return handle that should be used in subsequent API calls
 *
 * @note If an active haptic operating session is already open, the function will block waiting for
 *       it to finish.
 *
 * @sa ad_haptic_close()
 */
ad_haptic_handle_t ad_haptic_open(const ad_haptic_controller_conf_t *conf);

/**
 * @brief Reconfigure haptic operation
 *
 * This function applies a new haptic configuration to the Haptic Adapter.
 * It may block waiting for any ongoing or pending haptic operation to finish.
 *
 * @param [in] handle: handle returned by ad_haptic_open()
 * @param [in] conf:   [see ad_haptic_open()]
 *
 * @return 0: success, <0: error
 *
 * @sa ad_haptic_open(), AD_HAPTIC_ERROR
 */
AD_HAPTIC_ERROR ad_haptic_reconfig(ad_haptic_handle_t handle, const ad_haptic_controller_conf_t *conf);

/**
 * @brief Close active haptic operating session
 *
 * This function stops ongoing haptic activity, deactivates the Haptic Driver/Controller and
 * releases all haptic resources.
 *
 * @param [in] handle: handle returned by ad_haptic_open()
 * @param [in] force:  force close, even if another haptic operation is in progress
 *
 * @return [see ad_haptic_reconfig()]
 *
 * @sa ad_haptic_open()
 */
AD_HAPTIC_ERROR ad_haptic_close(ad_haptic_handle_t handle, bool force);

/**
 * @brief Set Drive Level
 *
 * This function sets the drive level either directly or indirecty, depending on the current
 * drive mode. It may block [see ad_haptic_reconfig()].
 *
 * @param [in] handle:         handle returned by ad_haptic_open()
 * @param [in] drive_level:    [see hw_haptic_set_drive_level()]
 * @param [in] ref:            [see hw_haptic_set_drive_level()]
 *
 * @return [see ad_haptic_reconfig()]
 *
 * @sa hw_haptic_set_drive_level()
 */
AD_HAPTIC_ERROR ad_haptic_set_drive_level(ad_haptic_handle_t handle, uint16_t drive_level, HW_HAPTIC_DRIVE_LEVEL_REF ref);

/**
 * @brief Set Polarity
 *
 * This function sets the polarity, but only in direct drive mode (i.e. if the Haptics Algorithm
 * Library components are deactivated). It may block [see ad_haptic_reconfig()].
 *
 * @param [in] handle:   handle returned by ad_haptic_open()
 * @param [in] polarity: [see ad_haptic_set_polarity()]
 *
 * @return [see ad_haptic_reconfig()]
 *
 * @sa hw_haptic_set_polarity()
 */
AD_HAPTIC_ERROR ad_haptic_set_polarity(ad_haptic_handle_t handle, bool polarity);

/**
 * @brief Set Half Period
 *
 * This function sets the half period. It may block [see ad_haptic_reconfig()].
 *
 * @param [in] handle:      handle returned by ad_haptic_open()
 * @param [in] half_period: [see hw_haptic_set_half_period()]
 *
 * @return [see ad_haptic_reconfig()]
 *
 * @warning The function sets the half period of the haptic drive even if Frequency Control is
 *          activated. In this case, the half period is set to the specified value, but, in the
 *          process, the Frequency Control component of the Haptics Algorithm Library re-adjusts
 *          it appropriately in order to achieve resonance.
 *
 * @sa hw_haptic_set_half_period()
 */
AD_HAPTIC_ERROR ad_haptic_set_half_period(ad_haptic_handle_t handle, uint16_t half_period);

/**
 * @brief Set Drive Mode
 *
 * This function sets the current drive mode of the haptic operating session, which defines the
 * enable status (whether enabled or disabled) of the Overdrive and Frequency Control (in case of
 * LRA) component(s) of the Haptics Algorithm Library (if used). It may block
 * [see ad_haptic_reconfig()].
 *
 * @param [in] handle:     handle returned by ad_haptic_open()
 * @param [in] drive_mode: pointer to the drive mode structure comprising the enable states of the
 *                         different components of the Haptics Algorithm Library
 *
 * @return [see ad_haptic_reconfig()]
 */
AD_HAPTIC_ERROR ad_haptic_set_drive_mode(ad_haptic_handle_t handle, ad_haptic_drive_mode_t *drive_mode);

/**
 * @brief Play waveform sequence
 *
 * This function drives the haptic actuator with a specific pattern of varying amplitude and
 * frequency (or *waveform sequence*). It may block [see ad_haptic_reconfig()] before starting the
 * "playback" of the waveform sequence (depending on the urgency parameter value, as well as
 * whether the possible ongoing or pending playbacks are urgent or not). As soon as the requested
 * waveform sequence playback is started, then, depending on the async parameter value, the
 * function will either block again, waiting for the playback to end (synchronous operation), or it
 * may return immediately, but after having set a specified callback function to be called as soon
 * as the playback is completed (asynchronous operation).
 *
 * @param [in] handle:      handle returned by ad_haptic_open()
 * @param [in] sequence_id: id of the waveform sequence to be played from the waveform memory
 * @param [in] urgency:     urgency of the waveform sequence playback. If true, the selected
 *                          sequence will be able to preempt a non-urgent waveform sequence that is
 *                          currently being played.
 * @param [in] disable:     automatically disable the haptic drive as soon as the playback ends
 * @param [in] async:       option to specify whether the playback will be synchronous or
 *                          asynchronous, i.e. whether the function will block waiting for the
 *                          playback to be completed, or whether it will return right after
 *                          triggering the waveform (in which case a specified callback function
 *                          will be fired as soon as the playback is completed).
 * @param [in] user_cb:     Callback function that will be fired as soon as the playback is
 *                          completed (only applicable to the asynchronous playback case). Set to
 *                          NULL, if unnecessary.
 * @param [in] user_data:   Parameter to be passed to the callback function (apart from the stop_type
 *                          parameter, which signifies whether the sequence playback was fully
 *                          completed, or it was stopped (either because of a call to
 *                          ad_haptic_stop_wm_sequence() or to ad_haptic_close(), or
 *                          because a more urgent sequence was triggered)).
 *
 * @note If the drive signal is disabled (if Haptic driver is in Idle mode), the function
 *       automatically enables it.
 * @note As soon as the playback ends, the drive parameters are set to their initial default state
 *       (drive_level = zero, half_period = according to the initial resonant frequency of the LRA)
 *       and, if disable = true, the Haptic Driver goes into Idle mode.
 * @note If accel_en (of ad_haptic_wm_conf_t) is false, then the waveform memory is supposed to
 *       contain also overdrive (acceration/braking) information itself. So, in this case, there is
 *       no sense in performing further haptic processing for overdriving the actuator. As a
 *       result, if Overdrive is enabled in this case, it will be temporarily disabled for the
 *       duration of the waveform sequence playback.
 *
 * @return [see ad_haptic_reconfig()]
 *
 */
AD_HAPTIC_ERROR ad_haptic_play_wm_sequence(ad_haptic_handle_t handle,
                                           uint8_t sequence_id,
                                           bool urgency,
                                           bool disable,
                                           bool async,
                                           ad_haptic_user_async_cb_t user_cb,
                                           ad_haptic_user_async_cb_data_t user_data);

/**
 * @brief Stop ongoing waveform sequence playback
 *
 * This function stops any waveform sequence playback that is in progress. If urgency = false, it
 * will stop only non-urgent sequence playbacks and block waiting for possible ongoing or pending
 * urgent playbacks to finish.
 *
 * @param [in] handle:  handle returned by ad_haptic_open()
 * @param [in] urgency: specifies whether urgent waveforms will be stopped as well or not.
 *
 * @return [see ad_haptic_reconfig()]
 */
AD_HAPTIC_ERROR ad_haptic_stop_wm_sequence(ad_haptic_handle_t handle, bool urgency);

/**
 * @brief Set haptic drive state
 *
 * This function enables or disables the haptic drive (puts the Haptic Driver into
 * Active/Idle mode). It may block [see ad_haptic_reconfig()].
 *
 * @param [in] handle: handle returned by ad_haptic_open()
 * @param [in] state:  Haptic Driver state (Active/Idle)
 *
 * @return [see ad_haptic_reconfig()]
 */
AD_HAPTIC_ERROR ad_haptic_set_state(ad_haptic_handle_t handle, HW_HAPTIC_STATE state);

/**
 * @brief Standard Haptic Interrupt Handler's callback function
 *
 * This function is the standard Haptic Interrupt Handler's callback function, for allowing the
 * haptic processing SW modules to interact with the Haptic Low Level SW Driver and operate
 * successfully.
 * It should be used in the Haptic Low Level SW Driver configuration part of
 * ad_haptic_controller_conf_t.
 *
 * @sa hw_haptic_interrupt_cb_t, ad_haptic_controller_conf_t, haptic_config_t
 */
void ad_haptic_update_drive_parameters (int16_t *i_data, uint16_t *half_period, uint16_t *drive_level, uint16_t *hw_state);

#ifdef __cplusplus
}
#endif

#endif /* dg_configHAPTIC_ADAPTER */

#endif /* DEVICE_VARIANT */

#endif /* AD_HAPTIC_H_ */

/**
 * \}
 * \}
 */
