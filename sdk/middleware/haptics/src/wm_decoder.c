/**
 * Haptic Waveform Memory Decoder
 *
 * Copyright (C) 2018-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 */


#include "wm_decoder.h"


//#define TRACE_ENABLE

#ifdef TRACE_ENABLE
#include <stdio.h>
#define TRACE  printf
#endif


#define WM_N_SNIPPETS_INDEX                              0
#define WM_N_SEQUENCES_INDEX                             1
#define WM_SNIPPET_END_PTRS_INDEX                        2
#define WM_SEQUENCE_END_PTRS_INDEX(wm_data)              (WM_SNIPPET_END_PTRS_INDEX + wm_data[WM_N_SNIPPETS_INDEX])
#define WM_SNIPPETS_INDEX(wm_data)                       (WM_SEQUENCE_END_PTRS_INDEX(wm_data) + wm_data[WM_N_SEQUENCES_INDEX])
#define WM_SEQUENCES_INDEX(wm_data)                      (wm_data[WM_SNIPPET_END_PTRS_INDEX + wm_data[WM_N_SNIPPETS_INDEX] - 1] + 1)

#define WM_SNIPPET_END_PTR_INDEX(snippet_id)             (snippet_id ? WM_SNIPPET_END_PTRS_INDEX + snippet_id - 1 : 0)
#define WM_SEQUENCE_END_PTR_INDEX(wm_data, sequence_id)  (WM_SEQUENCE_END_PTRS_INDEX(wm_data) + sequence_id)

#define WM_SNIPPET_INDEX(wm_data, snippet_id)            (snippet_id > 1 ? wm_data[WM_SNIPPET_END_PTRS_INDEX + snippet_id - 2] + 1 : WM_SNIPPETS_INDEX(wm_data))
#define WM_SEQUENCE_INDEX(wm_data, sequence_id)          (sequence_id ? wm_data[WM_SEQUENCE_END_PTRS_INDEX(wm_data) + sequence_id - 1] + 1 : WM_SEQUENCES_INDEX(wm_data))

#define WM_SNIPPET_END_INDEX(wm_data, snippet_id)        wm_data[WM_SNIPPET_END_PTR_INDEX(snippet_id)]
#define WM_SEQUENCE_END_INDEX(wm_data, sequence_id)      wm_data[WM_SEQUENCE_END_PTR_INDEX(wm_data, sequence_id)]


typedef struct wm_frame_byte_0_str {
    unsigned SNP_ID_L: 3;
    unsigned TIMEBASE: 2;
    unsigned GAIN: 2;
    unsigned COMMAND_TYPE: 1;
} wm_frame_byte_0;


typedef struct wm_frame_byte_1_str {
    unsigned SNP_ID_H: 1;
    unsigned FREQ: 1;
    unsigned FREQ_CMD: 1;
    unsigned SNP_ID_LOOP: 4;
    unsigned COMMAND_TYPE: 1;
} wm_frame_byte_1;


typedef struct wm_pwl_byte_str {
    unsigned AMP: 4;
    unsigned TIME: 3;
    unsigned RMP: 1;
} wm_pwl_byte;


/*
 * Starts decoding of given sequence.
 *
 * @param decoder      Pointer to the waveform memory decoder instance
 *                     structure
 * @param time         Current time
 * @param sequence_id  ID of sequence to start/stop
 *
 * @return             Decoder state information:
 *                       0: Playback of sequence active
 *                       1: Playback of sequence complete
 *                       2: Waveform memory decode error
 */
static inline unsigned wm_decoder_start(wm_decoder *decoder, uint32_t time, unsigned sequence_id)
{
#ifdef TRACE_ENABLE
    TRACE("Sequence %d started.\n", sequence_id);
#endif

    /* Record start time and sequence ID */
    decoder->start_time = time;
    decoder->sequence_id = sequence_id;

    /* Initialise frame index to start of appropriate sequence */
    decoder->frame_index = WM_SEQUENCE_INDEX(decoder->wm_data, sequence_id);

    /* Reset PWL index (indicates no PWL pair is currently referenced) */
    decoder->pwl_index = 0;

    /* Initialise 'previous' frame parameters */
    decoder->frame_data.snippet_id = 0;
    decoder->frame_data.loop_count = 0;

    /* Initialise 'previous' segment parameters */
    decoder->segment.end_time = 0;
    decoder->segment.end_amplitude = 0;

    /* Update state of decoder to decode first segment of sequence */
    return wm_decoder_update(decoder, time);

}


/*
 * Stops decoding of any sequence currently active.
 *
 * @param decoder      Pointer to the waveform memory decoder instance
 *                     structure
 */
static void wm_decoder_stop(wm_decoder *decoder)
{
#ifdef TRACE_ENABLE
    TRACE("Sequence %d stopped.\n", decoder->sequence_id);
#endif

    /* Reset current sequence ID to none (indicates decoder idle state) */
    decoder->sequence_id = WM_SEQUENCE_ID_NONE;

    /* Reset frequency to 'Default' */
    decoder->frequency = 0;

    /* Reset amplitude to 0 */
    decoder->amplitude = 0;

}


/*
 * Decodes frame at current frame pointer index, extracting the information
 * contained within it.
 *
 * @param decoder      Pointer to the waveform memory decoder instance
 *                     structure
 * @param end_index    Sequence end index
 *
 * @return             Decode state:
 *                       0: Frame decoded
 *                       1: Frame decode error
 */
static inline unsigned wm_decode_frame(wm_decoder *decoder, unsigned end_index)
{
    wm_frame_byte_0 *byte_0 = (wm_frame_byte_0 *) &decoder->wm_data[decoder->frame_index];
    wm_frame_byte_1 *byte_1;

    /* Check frame index is valid and that the value of COMMAND_TYPE of the
       first byte is zero and if not return error */
    if ( decoder->frame_index > end_index || byte_0->COMMAND_TYPE )
        return 1;

#ifdef TRACE_ENABLE
    TRACE("  Decoding frame at index %d.\n", decoder->frame_index);
#endif

    /* Extract fields of byte 0 */
    decoder->frame_data.snippet_id = byte_0->SNP_ID_L;
    decoder->frame_data.timebase = byte_0->TIMEBASE;
    decoder->frame_data.gain = byte_0->GAIN;
    decoder->frame_data.frequency = 0;

    /* Increment frame index and read second byte */
    byte_1 = (wm_frame_byte_1 *) &decoder->wm_data[++decoder->frame_index];

    /* Again check the frame index hasn't moved beyond end of sequence and
       that the value of COMMAND_TYPE of the second byte is not zero. If so,
       this byte belongs to the next frame so stop decoding */
    if ( decoder->frame_index > end_index || !byte_1->COMMAND_TYPE ) {
        decoder->frame_data.loop_count = 0;
        return 0;
    }

    /* Extract fields of byte 1 */
    decoder->frame_data.snippet_id |= byte_1->SNP_ID_H << 3;
    decoder->frame_data.loop_count = byte_1->SNP_ID_LOOP;

    /* Increment frame index */
    decoder->frame_index++;

    /* Check the value of FREQ_CMD is 1. If not, there is no additional FREQ
       byte so stop decoding */
    if ( !byte_1->FREQ_CMD )
        return 0;

    /* Once again make sure the frame index hasn't moved beyond end of
       sequence and if so return error */
    if ( decoder->frame_index > end_index )
        return 1;

    /* Extract frequency */
    decoder->frame_data.frequency = (byte_1->FREQ << 8) | decoder->wm_data[decoder->frame_index++];

    return 0;

}


/*
 * Initialises waveform memory decoder.
 *
 * @param decoder      Pointer to the waveform memory decoder instance
 *                     structure
 * @param wm_data      Pointer to the waveform memory buffer
 * @param ACCELERATION_EN
 * @param FREQ_WAVEFORM_TIMEBASE
 */
void wm_decoder_init(wm_decoder *decoder, uint8_t *wm_data, unsigned ACCELERATION_EN, unsigned FREQ_WAVEFORM_TIMEBASE)
{
    /* Store pointer to waveform memory buffer */
    decoder->wm_data = wm_data;

    /* Store configuration flags */
    decoder->flags.ACCELERATION_EN = ACCELERATION_EN;
    decoder->flags.FREQ_WAVEFORM_TIMEBASE = FREQ_WAVEFORM_TIMEBASE;

    /* Initialise sequence ID */
    decoder->sequence_id = WM_SEQUENCE_ID_NONE;

    /* Initialise decoder to idle state */
    wm_decoder_stop(decoder);

}


/*
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
unsigned wm_decoder_trigger_sequence(wm_decoder *decoder, uint32_t time, unsigned sequence_id)
{
    unsigned curr_sequence_id = decoder->sequence_id;

    /* Ensure sequence exists for given sequence ID. If not return error. */
    if ( sequence_id >= decoder->wm_data[1] )
        return 1;

    /* If a sequence is already being played back, stop it. */
    if ( curr_sequence_id != WM_SEQUENCE_ID_NONE ) {

        /* Stop playback of current sequence */
        wm_decoder_stop(decoder);

        /* If the given sequence ID matches that of the current sequence,
           return without starting new sequence */
        if ( sequence_id == curr_sequence_id )
            return 0;

    }

#ifdef TRACE_ENABLE
    TRACE("Sequence %d triggered.\n", sequence_id);
#endif

    /* Start sequence */
    return wm_decoder_start(decoder, time, sequence_id);

}


/*
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
unsigned wm_decoder_update(wm_decoder *decoder, uint32_t time)
{
    int shift;
    unsigned end_index;
    int32_t start_amplitude, end_amplitude, scaler;
    uint32_t segment_time, segment_duration;
    wm_pwl_byte *pwl;
    uint8_t timebase_shifts[] = {0, 2, 4, 5, 6};
    wm_pwl_byte snippet_0_pwl = {0, 1, 0};
    uint32_t playback_time = time - decoder->start_time;
    uint16_t playback_timebases = playback_time >> TIMER_DIV_SHIFT;

    /* If decoder is idle simply return state */
    if ( decoder->sequence_id == WM_SEQUENCE_ID_NONE )
        return 1;

    /* Check if current time has advanced beyond current segment and if so,
       iterate through sequence frames, snippets and PWL pairs until an
       appropriate PWL pair is found or the sequence ends  */
    while ( !decoder->segment.end_time || playback_timebases >= decoder->segment.end_time ) {

        /* Check if end of snippet has been reached and if so, ... */
        if ( !decoder->frame_data.snippet_id || decoder->pwl_index > WM_SNIPPET_END_INDEX(decoder->wm_data, decoder->frame_data.snippet_id) ) {

            /* Check frame loop counter to see if current frame needs to be
               repeated and if not decode the next frame */
            if ( !decoder->frame_data.loop_count-- ) {

                /* Check if end of sequence has been reached and if so, stop
                   decoder */
                end_index = WM_SEQUENCE_END_INDEX(decoder->wm_data, decoder->sequence_id);
                if ( decoder->frame_index == end_index + 1 ) {
                    wm_decoder_stop(decoder);
                    return 1;
                }

                /* Decode frame at current frame index, checking frame validity
                   and if invalid, stop decoder and return error */
                if ( wm_decode_frame(decoder, end_index) ) {
#ifdef TRACE_ENABLE
                    TRACE("  Error decoding frame at index %d.\n", decoder->frame_index);
#endif
                    wm_decoder_stop(decoder);
                    return 2;
                }

                /* Set drive frequency */
                decoder->frequency = decoder->frame_data.frequency < 12 ? 0 : (decoder->frame_data.frequency + 1) * 2;

#ifdef TRACE_ENABLE
                TRACE("    Snippet ID = %d\n", decoder->frame_data.snippet_id);
                TRACE("    Timebase   = %d\n", decoder->frame_data.timebase);
                TRACE("    Gain       = %d\n", decoder->frame_data.gain);
                TRACE("    Loop Count = %d\n", decoder->frame_data.loop_count);
                TRACE("    Frequency  = %d\n", decoder->frame_data.frequency);
#endif

            }

            /* Initialise PWL index to start of snippet */
            decoder->pwl_index = decoder->frame_data.snippet_id ? WM_SNIPPET_INDEX(decoder->wm_data, decoder->frame_data.snippet_id) : 0;

#ifdef TRACE_ENABLE
            TRACE("    Decoding snippet %d:\n", decoder->frame_data.snippet_id);
#endif

        }

        /* Decode PWL pair at current PWL index */
        pwl = decoder->frame_data.snippet_id ? (wm_pwl_byte *) &decoder->wm_data[decoder->pwl_index] : &snippet_0_pwl;

#ifdef TRACE_ENABLE
        TRACE("      Decoding PWL at index %d:\n", decoder->pwl_index);
        TRACE("        RMP  = %d\n", pwl->RMP);
        TRACE("        TIME = %d\n", pwl->TIME);
        TRACE("        AMP  = %d\n", pwl->AMP);
#endif

        /* Increment PWL index */
        decoder->pwl_index++;

        /* Calculate segment parameters from frame and PWL pair parameters */
        decoder->segment.start_time = decoder->segment.end_time;
        decoder->segment.end_time = decoder->segment.start_time + (pwl->TIME + 1) * (1 << timebase_shifts[decoder->frame_data.timebase + !decoder->flags.FREQ_WAVEFORM_TIMEBASE]);
        end_amplitude = pwl->AMP;
        if ( !decoder->flags.ACCELERATION_EN && end_amplitude > 7) {
            end_amplitude -= 16;
            if ( end_amplitude < -7 )
                end_amplitude = -7;
        }
        end_amplitude <<= 3 - decoder->frame_data.gain;
        decoder->segment.start_amplitude = pwl->RMP ? decoder->segment.end_amplitude : end_amplitude;
        decoder->segment.end_amplitude = end_amplitude;

#ifdef TRACE_ENABLE
        TRACE("        Segment Parameters:\n");
        TRACE("          Start Time      = %d\n", decoder->segment.start_time);
        TRACE("          End Time        = %d\n", decoder->segment.end_time);
        TRACE("          Start Amplitude = %d\n", decoder->segment.start_amplitude);
        TRACE("          End Amplitude   = %d\n", decoder->segment.end_amplitude);
#endif

    }

    /* Convert start/end amplitudes to fixed-point representation */
    scaler = decoder->flags.ACCELERATION_EN ? 4369 : 4681;
    shift = decoder->flags.ACCELERATION_EN ? 4 : 3;
    start_amplitude = (decoder->segment.start_amplitude * scaler) >> shift;
    end_amplitude = (decoder->segment.end_amplitude * scaler) >> shift;

    /* Calculate ratio for current time within current segment */
    segment_time = (playback_time - (decoder->segment.start_time << TIMER_DIV_SHIFT)) >> (TIMER_DIV_SHIFT - 8);
    segment_duration = (decoder->segment.end_time - decoder->segment.start_time) << 8;
    scaler = (segment_time << 15) / segment_duration;

    /* Calculate amplitude by interpolating start/end amplitudes according to
       ratio */
    decoder->amplitude = start_amplitude + ((scaler * (end_amplitude - start_amplitude)) >> 15);

    return 0;

}
