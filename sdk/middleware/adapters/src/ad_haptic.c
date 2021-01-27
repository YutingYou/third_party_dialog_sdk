/**
 * \addtogroup MID_SYS_ADAPTERS
 * \{
 * \addtogroup HAPTIC_ADAPTER Haptic Adapter
 *
 * \brief Adapter for the HAPTIC Feedback (ERM/LRA) Driver/Controller
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_haptic.c
 *
 * @brief Haptic Adapter implementation
 *
 * Copyright (C) 2019-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if ((DEVICE_VARIANT == DA14697) || (DEVICE_VARIANT == DA14699))

#if dg_configHAPTIC_ADAPTER

#include "ad_haptic.h"
#include "hw_timer.h"
#include "hw_sys.h"
#include "sys_timer.h"
#include "sys_clock_mgr.h"
#include "resmgmt.h"


/****************** INTERNAL MACRO DEFINITIONS ******************/


#define AD_HAPTIC_HANDLE_IS_VALID(handle)   (handle == &ad_haptic_data)
#define AD_HAPTIC_ENTER_UNINTERRUPTIBLE_SECTION   NVIC_DisableIRQ(LRA_IRQn)
#define AD_HAPTIC_LEAVE_UNINTERRUPTIBLE_SECTION   NVIC_EnableIRQ(LRA_IRQn)

/**
 * \brief Pre-scale timer ticks for the Waveform decoder
 *
 * This function-like macro scales the number of ticks (\b _timerticks_) read from a timer of a
 * specific counting frequency (\b _timerfreq_) appropriately so that they can be used as the time
 * input parameter by the wm_decoder_update() function. (Waveform Decoder already scales down the
 * time that is given to it, by 2^TIMER_DIV_SHIFT in order to convert it into
 * AD_HAPTIC_WM_SEQUENCE_TIMEBASE units. This conversion however only works if the given time is in
 * AD_HAPTIC_WM_SEQUENCE_TIMEBASE / 2^TIMER_DIV_SHIFT units.
 * Since the timer being used may have a different counting period, its ticks have to be scaled up
 * before they can be fed to the Waveform Decoder.)
 */
#define AD_HAPTIC_WM_DECODER_TIME(_timerticks_, _timerfreq_) \
        ((((uint64_t) 1000000 << TIMER_DIV_SHIFT) / AD_HAPTIC_WM_SEQUENCE_TIMEBASE) * (_timerticks_) / (_timerfreq_))


/****************** INTERNAL DATA TYPE DEFINITIONS ******************/


/**
 * Haptic adapter (internal) dynamic data
 */
typedef struct {
        /**< Haptic Driver/Controller current configuration */
        const ad_haptic_controller_conf_t *conf;
        OS_TASK  owner; /**< The task which opened the haptic operating session */
        uint8_t drive_mode;
        uint16_t target_level;
        wm_decoder waveform_decoder;
        uint8_t wm_sequence_id; /**< Id of the sequence that is currently played. If there is no
                                     sequence being played, it is set to WM_SEQUENCE_ID_NONE. */
        bool urgency; /**< True if an urgent sequence if being played. Otherwise (in case of no
                           sequence or a non-urgent sequence), false. This is used by
                           ad_haptic_wm_sequence_ended() so that it knows which resource to release. */
        bool async_play; /**< True, if an asynchronous sequence playback is in progress. */
        bool disable_on_complete; /**< True, if the haptic drive should be automatically disabled
                                       as soon as the sequence playback is ended. */
        ad_haptic_user_async_cb_t user_async_cb;
        ad_haptic_user_async_cb_data_t user_async_cb_data;
} ad_haptic_data_t;


/****************** INTERNAL GLOBAL VARIABLE DECLARATIONS ******************/


static ad_haptic_data_t ad_haptic_data;
__RETAINED static OS_EVENT ad_haptic_event; /* Semaphore used by async playbacks */


/****************** INTERNAL FUNCTION DEFINITIONS ******************/


/**
 * @brief Release a Haptic resource
 */
__STATIC_INLINE void ad_haptic_res_release(resource_mask_t res)
{
        resource_release(res);
}

/**
 * @brief Acquire a Haptic resource
 */
static AD_HAPTIC_ERROR ad_haptic_res_acquire(resource_mask_t res, uint32_t timeout)
{
        uint32_t actual_timeout;
        if (in_interrupt()) {
                /*
                 * When in interrupt context, if the resources are not available, there is no point
                 * in waiting for them, as they might never be freed (we have a deadlock situation,
                 * if all interrupts have equal priority). So in this case, the function should
                 * simply return AD_HAPTIC_ERROR_CONTROLLER_BUSY.
                 */
                actual_timeout = 0;
        } else {
                actual_timeout = timeout;
        }
        if (resource_acquire(res, actual_timeout)) {
                if (ad_haptic_data.owner != NULL) {
                        return AD_HAPTIC_ERROR_NONE;
                }
                ad_haptic_res_release(res);
                return AD_HAPTIC_ERROR_DEVICE_CLOSED;
        }
        return AD_HAPTIC_ERROR_CONTROLLER_BUSY;
}

/**
 * Perform necessary updates on the internal data and release resources, after a waveform sequence
 * has just ended.
 */
static void ad_haptic_wm_sequence_ended(AD_HAPTIC_PLAYBACK_END_TYPE end_type)
{
        ad_haptic_data.target_level = 0;
        ad_haptic_data.wm_sequence_id = WM_SEQUENCE_ID_NONE;
        if (ad_haptic_data.async_play) {
                if (ad_haptic_data.user_async_cb != NULL) {
                        ad_haptic_data.user_async_cb(ad_haptic_data.user_async_cb_data, end_type);
                        ad_haptic_data.async_play = false;
                        ad_haptic_data.user_async_cb = NULL;
                        ad_haptic_data.user_async_cb_data = NULL;
                }
                resource_mask_t res;
                if (ad_haptic_data.urgency) {
                        res = RES_MASK(RES_ID_HAPTIC_DRIVE);
                } else {
                        res = RES_MASK(RES_ID_HAPTIC_DRIVE_SEC);
                }
                ad_haptic_res_release(res);
        }
        else {
                if (in_interrupt()) {
                        OS_EVENT_SIGNAL_FROM_ISR(ad_haptic_event);
                } else {
                        OS_EVENT_SIGNAL(ad_haptic_event);
                }
        }
        if (ad_haptic_data.disable_on_complete) {
                hw_haptic_set_state(HW_HAPTIC_STATE_IDLE);
                ad_haptic_data.disable_on_complete = false;
        }
        ad_haptic_data.urgency = false;
        if (!ad_haptic_data.conf->wm->accel_en) {
                ad_haptic_data.conf->lib->flags.smart_drive_enabled = (ad_haptic_data.drive_mode && 1);
        }
}

static void ad_haptic_stop_current_wm_sequence_internal(void)
{
        if (ad_haptic_data.wm_sequence_id != WM_SEQUENCE_ID_NONE) {
                /* Trigger the same sequence again so that it stops. */
                ASSERT_WARNING(wm_decoder_trigger_sequence(&ad_haptic_data.waveform_decoder, 0, ad_haptic_data.wm_sequence_id) == 0);
                ad_haptic_wm_sequence_ended(AD_HAPTIC_PLAYBACK_END_TYPE_STOP);
                if (ad_haptic_data.conf->drv->half_period) {
                        hw_haptic_set_half_period(ad_haptic_data.conf->drv->half_period);
                }
        }
}

/**
 * @brief Get current time from ISR (for the waveform decoding)
 */
__STATIC_INLINE uint32_t ad_haptic_get_time_from_ISR(void)
{
        return AD_HAPTIC_WM_DECODER_TIME(sys_timer_get_uptime_ticks_fromISR(), configSYSTICK_CLOCK_HZ);
}

/**
 * @brief Get current time (for initiating the waveform decoding)
 */
__STATIC_INLINE uint32_t ad_haptic_get_time(void)
{
        return AD_HAPTIC_WM_DECODER_TIME(sys_timer_get_uptime_ticks(), configSYSTICK_CLOCK_HZ);
}

static AD_HAPTIC_ERROR ad_haptic_config(const ad_haptic_controller_conf_t *conf)
{
        OS_ASSERT(conf && conf->drv);
        hw_haptic_set_state(HW_HAPTIC_STATE_IDLE);

        /*
         * Configure Low level driver, Haptic library and Waveform Memory Decoder using the given
         * configuration structure.
         */
        hw_haptic_init(conf->drv);
        if (conf->lib) {
                haptics_lib_init(conf->lib);
                ad_haptic_data.drive_mode = 0;
                if (conf->lib->flags.curve_fitter_enabled) {
                        if (conf->lib->flags.smart_drive_enabled) {
                                ad_haptic_data.drive_mode = 1;
                        }
#if dg_configUSE_HW_LRA
                        if (conf->lib->flags.lra_afc_enabled) {
                                ad_haptic_data.drive_mode |= 2;
                        }
#endif
                }
        }
        if (conf->wm) {
                wm_decoder_init(&ad_haptic_data.waveform_decoder, (uint8_t *) conf->wm->data, conf->wm->accel_en, conf->wm->timebase);
        }
        return AD_HAPTIC_ERROR_NONE;

}


/****************** PUBLIC FUNCTION DEFINITIONS ******************/


ad_haptic_handle_t ad_haptic_open(const ad_haptic_controller_conf_t *conf)
{
        /* Acquire resources. */
        resource_mask_t res = (RES_MASK(RES_ID_HAPTIC) | RES_MASK(RES_ID_HAPTIC_CONFIG));
        resource_acquire(res, RES_WAIT_FOREVER);

        /* Prevent sleep. */
        pm_sleep_mode_request(pm_mode_idle);

        /* Power up related Power Domains (if not already up). */
        hw_sys_pd_com_enable();
        hw_sys_pd_periph_enable();

        /* Initialize internal data. */
        ad_haptic_data.conf  = conf;
        ad_haptic_data.owner = OS_GET_CURRENT_TASK(); /* It is actually more used as a flag,
                                                         indicating an active haptic operating
                                                         session. */
        ad_haptic_data.target_level = 0;
        ad_haptic_data.async_play = false;
        ad_haptic_data.wm_sequence_id = WM_SEQUENCE_ID_NONE;
        ad_haptic_data.user_async_cb = NULL;
        ad_haptic_data.user_async_cb_data = NULL;
        ad_haptic_data.urgency = false;
        ad_haptic_data.disable_on_complete = false;

        if (ad_haptic_config(conf) == AD_HAPTIC_ERROR_NONE) {
                resource_release(RES_MASK(RES_ID_HAPTIC_CONFIG));
                return &ad_haptic_data;
        }

        ad_haptic_data.owner = NULL;

        /* Power down related Power Domains (if not used by someone else). */
        hw_sys_pd_periph_disable();
        hw_sys_pd_com_disable();

        /* Allow sleep. */
        pm_sleep_mode_release(pm_mode_idle);

        /* Release resource. */
        resource_release(res);

        return NULL;
}

AD_HAPTIC_ERROR ad_haptic_reconfig(ad_haptic_handle_t handle, const ad_haptic_controller_conf_t *conf)
{
        /* Check input validity. */
        if (!(AD_HAPTIC_HANDLE_IS_VALID(handle))) {
                return AD_HAPTIC_ERROR_HANDLE_INVALID;
        }

        resource_mask_t res = RES_MASK(RES_ID_HAPTIC_CONFIG) |
                              RES_MASK(RES_ID_HAPTIC_DRIVE)  |
                              RES_MASK(RES_ID_HAPTIC_DRIVE_SEC);
        AD_HAPTIC_ERROR ret = ad_haptic_res_acquire(res, RES_WAIT_FOREVER);
        if (ret != AD_HAPTIC_ERROR_NONE) {
                return ret;
        }

        ret = ad_haptic_config(conf);
        ad_haptic_res_release(res);
        return ret;

}

AD_HAPTIC_ERROR ad_haptic_close(ad_haptic_handle_t handle, bool force)
{
        if (!(AD_HAPTIC_HANDLE_IS_VALID(handle))) {
                return AD_HAPTIC_ERROR_HANDLE_INVALID;
        }

        resource_mask_t res = RES_MASK(RES_ID_HAPTIC_CONFIG);

        if (force) {
                ad_haptic_data.owner = NULL;
        } else {
                res |= RES_MASK(RES_ID_HAPTIC_DRIVE) |
                       RES_MASK(RES_ID_HAPTIC_DRIVE_SEC);
        }

        resource_acquire(res, RES_WAIT_FOREVER);

        ad_haptic_data.owner = NULL;

        if (ad_haptic_data.wm_sequence_id != WM_SEQUENCE_ID_NONE) {
                /*
                 * A waveform sequence is being played.
                 * Since we got here, force is true (otherwise we wouldn't have acquired
                 * the resource). The waveform being played has to be stopped.
                 */
                ad_haptic_stop_current_wm_sequence_internal();
        }

        hw_haptic_shutdown();

        hw_sys_pd_periph_disable();
        hw_sys_pd_com_disable();

        ad_haptic_data.conf = NULL;

        pm_sleep_mode_release(pm_mode_idle);

        /* Release resources. */
        resource_release(res | RES_MASK(RES_ID_HAPTIC));

        return AD_HAPTIC_ERROR_NONE;
}

AD_HAPTIC_ERROR ad_haptic_set_drive_level(ad_haptic_handle_t handle, uint16_t drive_level, HW_HAPTIC_DRIVE_LEVEL_REF ref)
{
        if (!(AD_HAPTIC_HANDLE_IS_VALID(handle))) {
                return AD_HAPTIC_ERROR_HANDLE_INVALID;
        }
        resource_mask_t res = RES_MASK(RES_ID_HAPTIC_CONFIG) |
                              RES_MASK(RES_ID_HAPTIC_DRIVE)  |
                              RES_MASK(RES_ID_HAPTIC_DRIVE_SEC);
        AD_HAPTIC_ERROR ret = ad_haptic_res_acquire(res, RES_WAIT_FOREVER);
        if (ret != AD_HAPTIC_ERROR_NONE) {
                return ret;
        }
        if (!ad_haptic_data.drive_mode) { /* Current drive mode is zero. Set drive level directly. */
                AD_HAPTIC_ENTER_UNINTERRUPTIBLE_SECTION;
                hw_haptic_set_drive_level(drive_level, ref);
                ad_haptic_data.target_level = ref ? drive_level :
                        ((drive_level * ad_haptic_data.conf->drv->duty_cycle_nom_max
                                / ad_haptic_data.conf->drv->duty_cycle_abs_max) + 0.5);
                AD_HAPTIC_LEAVE_UNINTERRUPTIBLE_SECTION;
        }
        else {
                /*
                 * Current drive mode is non-zero. Set only the target drive level.
                 * The actual drive level will be adjusted appropriately by the Haptics Algorithm
                 * Library.
                 */
                ad_haptic_data.target_level = ref ? drive_level :
                        ((drive_level * ad_haptic_data.conf->drv->duty_cycle_nom_max
                                / ad_haptic_data.conf->drv->duty_cycle_abs_max) + 0.5);
        }
        ad_haptic_res_release(res);
        return AD_HAPTIC_ERROR_NONE;
}

AD_HAPTIC_ERROR ad_haptic_set_polarity(ad_haptic_handle_t handle, bool polarity)
{
        if (!(AD_HAPTIC_HANDLE_IS_VALID(handle))) {
                return AD_HAPTIC_ERROR_HANDLE_INVALID;
        }
        if (ad_haptic_data.drive_mode) {
                /* Current drive mode is non-zero. There is no sense in setting the polarity directly. */
                return AD_HAPTIC_ERROR_NOT_APPLICABLE;
        }
        resource_mask_t res = RES_MASK(RES_ID_HAPTIC_CONFIG) |
                              RES_MASK(RES_ID_HAPTIC_DRIVE)  |
                              RES_MASK(RES_ID_HAPTIC_DRIVE_SEC);
        AD_HAPTIC_ERROR ret = ad_haptic_res_acquire(res, RES_WAIT_FOREVER);
        if (ret != AD_HAPTIC_ERROR_NONE) {
                return ret;
        }
        hw_haptic_set_polarity(polarity);
        ad_haptic_res_release(res);
        return AD_HAPTIC_ERROR_NONE;
}

AD_HAPTIC_ERROR ad_haptic_set_half_period(ad_haptic_handle_t handle, uint16_t half_period)
{
        if (!(AD_HAPTIC_HANDLE_IS_VALID(handle))) {
                return AD_HAPTIC_ERROR_HANDLE_INVALID;
        }
        resource_mask_t res = RES_MASK(RES_ID_HAPTIC_CONFIG) |
                              RES_MASK(RES_ID_HAPTIC_DRIVE)  |
                              RES_MASK(RES_ID_HAPTIC_DRIVE_SEC);
        AD_HAPTIC_ERROR ret = ad_haptic_res_acquire(res, RES_WAIT_FOREVER);
        if (ret != AD_HAPTIC_ERROR_NONE) {
                return ret;
        }
        hw_haptic_set_half_period(half_period);
        ad_haptic_res_release(res);
        return AD_HAPTIC_ERROR_NONE;
}

AD_HAPTIC_ERROR ad_haptic_set_drive_mode(ad_haptic_handle_t handle, ad_haptic_drive_mode_t *drive_mode)
{

        if (!(AD_HAPTIC_HANDLE_IS_VALID(handle))) {
                return AD_HAPTIC_ERROR_HANDLE_INVALID;
        }
        if (!ad_haptic_data.conf->lib) {
                return AD_HAPTIC_ERROR_NOT_APPLICABLE;
        }

        uint8_t mode = 0;
        haptics_lib_params *haptics_lib = ad_haptic_data.conf->lib;
        resource_mask_t res = RES_MASK(RES_ID_HAPTIC_CONFIG) |
                              RES_MASK(RES_ID_HAPTIC_DRIVE)  |
                              RES_MASK(RES_ID_HAPTIC_DRIVE_SEC);
        AD_HAPTIC_ERROR ret = ad_haptic_res_acquire(res, RES_WAIT_FOREVER);
        if (ret != AD_HAPTIC_ERROR_NONE) {
                return ret;
        }
        AD_HAPTIC_ENTER_UNINTERRUPTIBLE_SECTION;
        if (drive_mode->overdrive) {
                mode = 1;
                haptics_lib->flags.smart_drive_enabled = true;
                haptics_lib->flags.curve_fitter_enabled = true;
        } else {
                haptics_lib->flags.smart_drive_enabled = false;
        }
#if dg_configUSE_HW_LRA
        if (drive_mode->freq_ctrl) {
                mode |= 2;
                haptics_lib->flags.lra_afc_enabled = true;
                haptics_lib->flags.curve_fitter_enabled = true;
        } else {
                haptics_lib->flags.lra_afc_enabled = false;
        }
#endif

        ad_haptic_data.drive_mode = mode;
        AD_HAPTIC_LEAVE_UNINTERRUPTIBLE_SECTION;
        ad_haptic_res_release(res);
        return AD_HAPTIC_ERROR_NONE;
}

AD_HAPTIC_ERROR ad_haptic_play_wm_sequence(ad_haptic_handle_t handle,
                                           uint8_t sequence_id,
                                           bool urgency,
                                           bool disable,
                                           bool async,
                                           ad_haptic_user_async_cb_t user_cb,
                                           ad_haptic_user_async_cb_data_t user_data)
{
        if (!(AD_HAPTIC_HANDLE_IS_VALID(handle))) {
                return AD_HAPTIC_ERROR_HANDLE_INVALID;
        }
        if ((!ad_haptic_data.conf->wm) || ((!async) && (in_interrupt()))) {
                return AD_HAPTIC_ERROR_NOT_APPLICABLE;
        }
        resource_mask_t res = RES_MASK(RES_ID_HAPTIC_CONFIG) | RES_MASK(RES_ID_HAPTIC_DRIVE);
        if (!urgency) {
                res |= RES_MASK(RES_ID_HAPTIC_DRIVE_SEC);
        }
        AD_HAPTIC_ERROR ret = ad_haptic_res_acquire(res, RES_WAIT_FOREVER);
        if (ret != AD_HAPTIC_ERROR_NONE) {
                return ret;
        }

        AD_HAPTIC_ENTER_UNINTERRUPTIBLE_SECTION;
        if (ad_haptic_data.wm_sequence_id != WM_SEQUENCE_ID_NONE) {
                /*
                 * A sequence playback is in progress. It has to be stopped, before triggering
                 * the new one. (Since we got in here, urgency should be true.)
                 */
                ad_haptic_stop_current_wm_sequence_internal();
        }

        if (!hw_haptic_get_state()) {
                hw_haptic_set_state(HW_HAPTIC_STATE_ACTIVE);
        }

        if (!ad_haptic_data.conf->wm->accel_en) {
                /*
                 * Acceration/braking is supposed to be specified in the waveform sequence.
                 * Overdrive should be disabled.
                 */
                ad_haptic_data.conf->lib->flags.smart_drive_enabled = false;
        }

        ASSERT_WARNING(!wm_decoder_trigger_sequence(&ad_haptic_data.waveform_decoder,
                in_interrupt() ? ad_haptic_get_time_from_ISR() : ad_haptic_get_time(), sequence_id));
        if (async) {
                ad_haptic_data.user_async_cb  = user_cb;
                ad_haptic_data.user_async_cb_data  = user_data;
                ad_haptic_data.async_play = true;
        } else {
                ad_haptic_data.async_play = false;
        }
        ad_haptic_data.urgency = urgency;
        ad_haptic_data.disable_on_complete = disable;
        ad_haptic_data.wm_sequence_id = sequence_id;
        AD_HAPTIC_LEAVE_UNINTERRUPTIBLE_SECTION;
        if (urgency) {
                res = RES_MASK(RES_ID_HAPTIC_CONFIG);
        } else {
                res = RES_MASK(RES_ID_HAPTIC_CONFIG) | RES_MASK(RES_ID_HAPTIC_DRIVE);
        }
        ad_haptic_res_release(res);
        if (!async) {
                OS_EVENT_WAIT(ad_haptic_event, RES_WAIT_FOREVER);
                if (urgency) {
                        res =  RES_MASK(RES_ID_HAPTIC_DRIVE);
                } else {
                        res =  RES_MASK(RES_ID_HAPTIC_DRIVE_SEC);
                }
                ad_haptic_res_release(res);
        }
        return AD_HAPTIC_ERROR_NONE;
}

AD_HAPTIC_ERROR ad_haptic_stop_wm_sequence(ad_haptic_handle_t handle, bool urgency)
{
        if (!(AD_HAPTIC_HANDLE_IS_VALID(handle))) {
                return AD_HAPTIC_ERROR_HANDLE_INVALID;
        }
        if (!(ad_haptic_data.conf->wm)) {
                return AD_HAPTIC_ERROR_NOT_APPLICABLE;
        }
        resource_mask_t res = RES_MASK(RES_ID_HAPTIC_CONFIG);
        if (!urgency) {
                res |= RES_MASK(RES_ID_HAPTIC_DRIVE);
        }
        AD_HAPTIC_ERROR ret = ad_haptic_res_acquire(res, RES_WAIT_FOREVER);
        if (ret != AD_HAPTIC_ERROR_NONE) {
                return ret;
        }
        AD_HAPTIC_ENTER_UNINTERRUPTIBLE_SECTION;
        ad_haptic_stop_current_wm_sequence_internal();
        AD_HAPTIC_LEAVE_UNINTERRUPTIBLE_SECTION;
        ad_haptic_res_release(res);
        return AD_HAPTIC_ERROR_NONE;
}

AD_HAPTIC_ERROR ad_haptic_set_state(ad_haptic_handle_t handle, HW_HAPTIC_STATE state)
{
        if (!(AD_HAPTIC_HANDLE_IS_VALID(handle))) {
                return AD_HAPTIC_ERROR_HANDLE_INVALID;
        }
        resource_mask_t res = RES_MASK(RES_ID_HAPTIC_CONFIG) |
                              RES_MASK(RES_ID_HAPTIC_DRIVE)  |
                              RES_MASK(RES_ID_HAPTIC_DRIVE_SEC);
        AD_HAPTIC_ERROR ret = ad_haptic_res_acquire(res, RES_WAIT_FOREVER);
        if (ret != AD_HAPTIC_ERROR_NONE) {
                return ret;
        }
        hw_haptic_set_state(state);
        ad_haptic_res_release(res);
        return AD_HAPTIC_ERROR_NONE;
}

void ad_haptic_update_drive_parameters (int16_t *i_data, uint16_t *half_period, uint16_t *drive_level, uint16_t *hw_state)
{
        uint16_t fixed_half_period = 0;
        if (ad_haptic_data.wm_sequence_id != WM_SEQUENCE_ID_NONE) {
                /* A wm sequence playback is in progress. */
                int decode_status = wm_decoder_update(&ad_haptic_data.waveform_decoder, ad_haptic_get_time_from_ISR());
                ASSERT_WARNING(decode_status != 2);
                if (decode_status == 1) {
                        /* Playback of sequence complete. */
                        ad_haptic_wm_sequence_ended(AD_HAPTIC_PLAYBACK_END_TYPE_NORMAL);
                        if (ad_haptic_data.conf->drv->half_period) {
                                fixed_half_period = ad_haptic_data.conf->drv->half_period;
                        }
                } else {
                        /*
                         * Playback of sequence not complete. Update the drive_level and frequency
                         * with the decoded values.
                         */
                        int16_t wm_amplitude = ad_haptic_data.waveform_decoder.amplitude;
                        if (ad_haptic_data.conf->wm->accel_en) {
                                ad_haptic_data.target_level = wm_amplitude;
                        } else {
                                if (wm_amplitude < 0) {
                                        *hw_state |= (0x1 << 1);
                                        ad_haptic_data.target_level = (uint16_t) (- wm_amplitude);
                                } else {
                                        *hw_state &= ~(0x1 << 1);
                                        ad_haptic_data.target_level = (uint16_t) wm_amplitude;
                                }
                        }

                        if (ad_haptic_data.waveform_decoder.frequency) {
                                fixed_half_period = HW_HAPTIC_CONV_FREQ_TO_HALFPERIOD(ad_haptic_data.waveform_decoder.frequency);
                        }
                }
        }
        if (ad_haptic_data.drive_mode) {
                haptics_lib_process(ad_haptic_data.conf->lib, i_data, half_period, drive_level,  (haptics_lib_if_state *) hw_state, ad_haptic_data.target_level);
        } else {
                *drive_level = ad_haptic_data.target_level;
        }
        if (fixed_half_period) {
                *half_period = fixed_half_period;
        }
}

void ad_haptic_init(void)
{
        OS_EVENT_CREATE(ad_haptic_event);
}

/**
 * Register ad_haptic_init() function to be called by the system (no dependencies with other
 * adapters, so just use ADAPTER_INIT()).
 */
ADAPTER_INIT(ad_haptic_adapter, ad_haptic_init)

#endif /* dg_configHAPTIC_ADAPTER */

#endif /* DEVICE_VARIANT */

/**
 * \}
 * \}
 */
