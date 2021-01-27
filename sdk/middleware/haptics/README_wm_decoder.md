Dialog Haptic Waveform Memory Decoder {#wm_decoder}
===================================================


## License Information ##

This software ("Software") is owned by Dialog Semiconductor.

By using this Software you agree that Dialog Semiconductor retains all
intellectual property and proprietary rights in and to this Software and any
use, reproduction, disclosure or distribution of the Software without express
written permission or a license agreement from Dialog Semiconductor is strictly
prohibited. This Software is solely for use on or in conjunction with Dialog
Semiconductor products.

EXCEPT AS OTHERWISE PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES OR AS
REQUIRED BY LAW, THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. EXCEPT
AS OTHERWISE PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES OR BY LAW, IN
NO EVENT SHALL DIALOG SEMICONDUCTOR BE LIABLE FOR ANY DIRECT, SPECIAL, INDIRECT,
INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
PERFORMANCE OF THE SOFTWARE.


## Overview ##

The Haptic Waveform Memory Decoder provides functionality for the storage and
playback of haptic waveform patterns.

It is designed to replicate the Waveform Memory playback functionality available
on the DA7280 Haptic Driver. Please refer to the DA7280 datasheet for further
details.


## API Usage ##

### Initialization ###

An instance of the `wm_decoder` module structure must be instantiated, either
statically or on the heap, and must be maintained for the life-cycle of haptics
processing.

Before Waveform Memory Decoder module processing can be performed, its module
structure must be initialized by calling the `wm_decoder_init()` function.

The parameters of this function are as follows:
* `decoder`: Pointer to the instance of the `wm_decoder` module structure to be
  initialized.
* `wm_data`: Pointer to the waveform memory buffer.
* `ACCELERATION_EN`: Indicates whether or not the waveforms are signed.
* `FREQ_WAVEFORM_TIMEBASE`: Time base of waveforms in `wm_data`.

The `wm_data` parameter references a byte array buffer in which the encoded
waveform patterns are stored. The format of this buffer is as documented in the
Waveform Memory section in the DA7280 datasheet although it is recommended to
use the supplied Waveform Memory Editor Tool to generate this rather than
attempting to encode waveforms manually. This buffer must be instantiated,
either statically or on the heap, and must be maintained for the life-cycle of
haptics processing. The maximum size of this array is denoted by the
`WM_MAX_SIZE` macro.

The `ACCELERATION_EN` parameter replicates the equivalent DA7280 register
behavior (see DA7280 datasheet) and indicates whether the waveforms are to be
treated as unsigned (positive only) or signed (positive and negative). When set
to 0 it is assumed that automatic acceleration/braking is not enabled and that
the overdrive is encoded into the waveform, with braking implemented by means of
negative drive levels. When set to 1, only positive drive levels are encoded and
it is assumed that acceleration/braking, if required, is handled automatically
by an appropriate algorithm.

The `FREQ_WAVEFORM_TIMEBASE` parameter also replicates equivalent DA7280
register behavior, setting the basic ‘TIMEBASE’ used by the waveform decoder
patterns. When 0 the time base is 5.44ms. When set to 1 the time base is 1.36ms.

### Pattern Triggering ###

The `wm_decoder_trigger_sequence()` function is used to trigger the playback of
haptic sequences. This function is designed to be called from the application
layer, either directly or indirectly (i.e. via haptics adapter API).

The parameters of this function are as follows:
* `decoder`: Pointer to the instance of the `wm_decoder` module structure.
* `time`: Current time at which the pattern is triggered.
* `sequence_id`: Sequence ID of pattern to be triggered.

The current time, represented in 32-bit integer form (outlined in more detail in
the section below), should be polled immediately prior to calling this function
and passed to trigger function via the `time` parameter. This represents the
start time of pattern playback.

The sequence to be played is selected by the `sequence_id` parameter. If the
trigger function is called when a sequence with the same sequence ID is already
playing, the current sequence is simply stopped. If called when a sequence with
a different sequence ID is already playing, the current sequence will stop
playing and the new sequence will start.

The trigger function returns the current state of the decoder. Its return value
should therefore be monitored to detect errors. The return values indicate the
following:
  * 0: Pattern triggered (or stopped) successfully.
  * 1: Pattern corresponding to `sequence_id` does not exist in `wm_data`
    buffer.
  * 2: Error decoding the pattern corresponding to `sequence_id` (probably due
    to corrupt `wm_data` buffer).

### Pattern Playback ###

The waveform decoder is designed to be polled asynchronously via the
`wm_decoder_update()` function to update what the current sequence’s drive
parameters should be at the current time. This function is designed to be called
from the low level driver layer (via the registered call-back function).

The parameters of this function are as follows:
* `decoder`: Pointer to the instance of the `wm_decoder` module structure.
* `time`: Current time.

The current time, represented in 32-bit integer form (outlined in more detail in
the section below), should be polled immediately prior to calling this function
and passed to update function via the `time` parameter. The decoder uses this to
calculate how much time has elapsed since the pattern was triggered.

The update function updates the state of decoder according to how much time has
elapsed since the last time the function was called or a pattern triggered. It
iterates through the frames, snippets and piecewise linear (PWL) segments that
make up current sequence until it finds where is should be according to the
elapsed time and calculates the associated drive parameters accordingly.

After calling the `wm_decoder_update()` function, the updated `frequency` and
`amplitude` parameters can be read from the `wm_decoder` structure. These should
be used to configure the driver interface for the next half period.


## Timing ##

The timing of the waveform decoder is designed to be derived more or less
directly from a hardware timer. Ideally this should be free running with a 32
bit field that is incremented every CPU clock cycle (or derivative thereof) and
overflows when it reaches it’s maximum value. This 32 bit timer value should be
polled immediately before being passed (in raw form) to the
`wm_decoder_trigger_sequence()` and `wm_decoder_update()` functions.

The decoder internally scales this 32 bit timer value to the 1.36ms time base
used by DA7280 using the `TIMER_DIV_SHIFT` compile time setting.
`TIMER_DIV_SHIFT` must be set according to the CPU clock speed setting that is
in use while sequences are in operation, such that the CPU clock period is
scaled up to the 1.36ms time base via a power of two multiplier. In other words:

>    2 ^ TIMER_DIV_SHIFT / fc = 1.36ms
