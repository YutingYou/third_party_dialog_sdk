/**
 ****************************************************************************************
 *
 * @file platform_devices.c
 *
 * @brief LRA haptic configuration
 *
 * Copyright (C) 2019-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include "platform_devices.h"
#include "ad_haptic.h"

/*
 * Haptic Low Level Driver config
 */
const haptic_config_t lra_cfg = {
        .duty_cycle_nom_max = HW_HAPTIC_CONV_VOLT_TO_DUTYCYCLE(LRA_MAX_NOM_V, V_BAT, LRA_R),
        .duty_cycle_abs_max = HW_HAPTIC_CONV_VOLT_TO_DUTYCYCLE(LRA_MAX_ABS_V, V_BAT, LRA_R),
        .half_period = HW_HAPTIC_CONV_FREQ_TO_HALFPERIOD(LRA_FREQUENCY),
        .interrupt_cb = ad_haptic_update_drive_parameters,
};

/*
 * Haptics Library config
 */
__RETAINED_RW haptics_lib_params haptics_lib = {
        .flags = {
                .start_polarity = false,
        },
        .i_data_threshold = HL_I_DATA_THRESHOLD,
        .curve_fitter = {
                .amplitude_threshold = (uint16_t) (HL_CURVE_FITTER_AMP_THRESH * 32768.0 + 0.5),
        },
        .lra_afc = {
                .half_period = HW_HAPTIC_CONV_FREQ_TO_HALFPERIOD(LRA_FREQUENCY),
                .half_period_min = HW_HAPTIC_CONV_FREQ_TO_HALFPERIOD(LRA_MAX_FREQ),
                .half_period_max = HW_HAPTIC_CONV_FREQ_TO_HALFPERIOD(LRA_MIN_FREQ),
                .zeta = (uint16_t) (HL_LRA_AFC_ZETA * 32768.0 + 0.5),
        },
        .smart_drive = {
                .flags = {
                        .apply = HL_SMART_DRIVE_APPLY,
                        .update = HL_SMART_DRIVE_UPDATE,
                        .amplitude_valid = true,
                },
                .peak_amplitude = (uint16_t) (HL_SMART_DRIVE_PEAK_AMP * 32768.0 + 0.5),
                .tau = (uint16_t) (HL_SMART_DRIVE_TAU * 32768.0 + 0.5),
                .lambda = (uint16_t) (HL_SMART_DRIVE_LAMBDA * 32768.0 + 0.5),
                .delta = HL_SMART_DRIVE_DELTA,
        },
};

/*
 * Waveform Memory Decoder config
 */
extern const uint8_t wm_data[];
const ad_haptic_wm_conf_t wm_cfg = {
        .data = wm_data,
        .accel_en = true,
        .timebase = WM_TIMEBASE,
};

/*
 * Haptic Adapter config
 */
const ad_haptic_controller_conf_t ad_haptic_config = {
        .drv = &lra_cfg,
        .lib = &haptics_lib,
        .wm = &wm_cfg,
};
