/**
 * \addtogroup MID_HAPTICS
 * \{
 * \addtogroup WM_DECODER Haptic Waveform Memory Decoder
 *
 * \brief Module for decoding haptic waveform patterns encoded in the DA728x waveform memory format.
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file wm_decoder.h
 *
 * @brief Haptic Waveform Memory Decoder
 *
 * Copyright (C) 2018-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef WM_DECODER_H
#define WM_DECODER_H


#include <stdint.h>


#define TIMER_DIV_SHIFT      15

#define WM_MAX_SIZE          256

#define WM_SEQUENCE_ID_NONE  0xFF


/**
 * Waveform decoder configuration flags structure
 */
typedef struct wm_decoder_flags_str {
    unsigned ACCELERATION_EN: 1;
    unsigned FREQ_WAVEFORM_TIMEBASE: 1;
    unsigned RSVD: 6;
} wm_decoder_flags;


/**
 * Frame data structure
 */
typedef struct wm_frame_data_str {
    unsigned snippet_id: 4;
    unsigned timebase: 2;
    unsigned gain: 2;
    unsigned loop_count: 4;
    unsigned frequency: 9;
    unsigned rsvd: 11;
} wm_frame_data;


/**
 * Piecewise linear (PWL) segment structure
 */
typedef struct wm_pwl_segment_str {
    uint16_t start_time;
    uint16_t end_time;
    int8_t start_amplitude;
    int8_t end_amplitude;
} wm_pwl_segment;


/**
 * Waveform memory decoder structure
 */
typedef struct wm_decoder_str {

    /* 'Private' parameters (should not be accessed externally) */
    uint8_t *wm_data;
    uint32_t start_time;
    wm_decoder_flags flags;
    uint8_t sequence_id;
    uint8_t frame_index;
    uint8_t pwl_index;
    wm_frame_data frame_data;
    wm_pwl_segment segment;

    /* 'Public' parameters (set by decoder but can be read externally) */
    uint16_t frequency;  /**< Drive frequency */
    int16_t amplitude;   /**< Drive amplitude */

} wm_decoder;


/**
 * Initialises waveform memory decoder.
 *
 * @param decoder      Pointer to the waveform memory decoder instance
 *                     structure
 * @param wm_data      Pointer to the waveform memory buffer
 * @param ACCELERATION_EN
 * @param FREQ_WAVEFORM_TIMEBASE
 */
void wm_decoder_init(wm_decoder *decoder, uint8_t *wm_data, unsigned ACCELERATION_EN, unsigned FREQ_WAVEFORM_TIMEBASE);


/**
 * Triggers start/stop of playback of sequence in waveform memory.
 *
 * @param decoder      Pointer to the waveform memory decoder instance
 *                     structure
 * @param time         Current time
 * @param sequence_id  ID of sequence to start/stop
 *
 * @return             Decoder state information:
 *                       0: Playback of sequence active or current sequence stopped
 *                       1: Sequence ID does not exist
 *                       2: Waveform memory decode error
 */
unsigned wm_decoder_trigger_sequence(wm_decoder *decoder, uint32_t time, unsigned sequence_id);


/**
 * Updates state of waveform decoder according to current time.
 *
 * @param decoder      Pointer to the waveform memory decoder instance
 *                     structure
 * @param time         Current time
 *
 * @return             Decoder state information:
 *                       0: Playback of sequence active
 *                       1: Playback of sequence complete
 *                       2: Waveform memory decode error
 */
unsigned wm_decoder_update(wm_decoder *decoder, uint32_t time);

#endif /* WM_DECODER_H */

/**
 * \}
 * \}
 */
