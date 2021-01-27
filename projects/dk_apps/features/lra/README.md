Linear Resonance Actuator application {#lra}
============================================

## Overview
This application demonstrates the use of the *Haptic Driver and Controller* HW block together with the
overall Haptic Support of the SDK in order to drive a *Linear Resonant Actuator* (LRA) for providing
haptic (vibration) effects.
Each haptic effect is produced either by manually setting the amplitude and frequency drive
parameters or by playing a haptic waveform pattern (encoded sequences of amplitude & frequency
pairs) and, in each case, by potentially applying the *Overdrive* & *Frequency Control* efficient
drive principles using the *Haptics Algorithm Library* of the SDK.
> _Note_: It is only applicable to *DA14697/9* devices.

## Usage

The K1 button of the Development Kit (DK) Pro is used in order to execute various haptic operations
in a cyclical order. The effect of each press of the button is described below.

1) System wakes up (if asleep),
   opens a haptic operating session and
   plays a *heartbeat-like* haptic pattern, on the LRA's *nominal* resonant frequency,
   with *Frequency Control and Overdrive off*.

2) Plays the same *heartbeat-like* haptic pattern as before on the LRA's *actual* resonant frequency,
   (*Frequency Control on*) with *Overdrive off*.
   If the button is pressed too quickly and the playback of the previous pattern is still in
   progress, then the previous pattern is waited to finish before playing the new one.

3) Plays a *simple rhythmic* haptic pattern on the LRA's *actual* resonant frequency,
   in the same drive mode as before (*Frequency Control on* and *Overdrive off*).
   If the button is pressed too quickly and the playback of the previous pattern is still in
   progress, then the previous playback is stopped, and the new pattern is played right away.

4) Plays a *phone-ringing-like* haptic pattern on the LRA's *actual* resonant frequency, with
   *Overdrive On*.
   If the button is pressed too quickly and the playback of the previous pattern is still in
   progress, then the previous playback is stopped, and the new pattern is played right away.

5) Drives the LRA constantly at *170Hz frequency* with *75% drive (intensity) level*,
   (*Frequency Control and Overdrive off*).
   If the button is pressed too quickly and the playback of the previous pattern is still in
   progress, then the previous playback is stopped.

6) Drives the LRA constantly at its actual resonant frequency with *60% drive level*
   (*Frequency Control on and Overdrive off*).

7) Closes the haptic operating session.
   System enters sleep mode (as long as there are no other tasks in running state and no Debugger 
   is attached).

The application also redirects I/O (e.g. printf) to SEGGER's Real Time Terminal. To access the
terminal, open a telnet terminal to localhost:19021.

### Efficient drive principles

#### Frequency Control
The drive frequency is adjusted automatically so that it constantly matches the LRA's resonant frequency.

#### Overdrive
The drive level is adjusted automatically in order to reach a specified "target" (desired)
vibration intensity level in minimum time (by actually "accelerating" and "braking" the LRA and thus leading
to *sharper* haptic effects).

## Setup
The LRA should be connected to the *HDRVP* and *HDRVM* pins (Haptic Driver output) of the device
(accessible through the J10 header of the DK Pro).
> _Note_: The actual two pins of the header may differ among different DK revisions.

## Configuration
### Actuator characteristics
The characteristics of the LRA should be specified in the respective configuration macros, in
`platform_devices.h`

| Parameter      | Description                                                                 |
| --             | --                                                                          |
| LRA_FREQUENCY  | Nominal resonant frequency (in Hz)                                          |
| LRA_MIN_FREQ   | Lower limit for the resonant frequency (in Hz)                              |
| LRA_MAX_FREQ   | Maximum limit for the resonant frequency (in Hz)                            |
| LRA_MAX_ABS_V  | Absolute maximum voltage (steady state operation)  (in mV)                  |
| LRA_MAX_NOM_V  | Nominal maximum voltage (transient while changing vibration levels) (in mV) |
| LRA_R          | Impedance (in Ohms)                                                         |

They are needed for calculating the appropriate configuration parameters for initializing 
the Haptic Driver/Controller.

The characteristics of the four following example LRA motors are already provided:

- JHV10L5L00SB
- LVM61530B
- LVM61930B
- G0825001

In case one of these four is used, then only the `LRA_MOTOR` macro needs to be set to the
respective value.

### Haptics Algorithm Library parameters
The Haptics Algorithm Library configuration parameters, should also be specified:

| Parameter                  | Description                                 |
| --                         | --                                          |
| HL_I_DATA_THRESHOLD        | Threshold for i-data validity               |
| HL_CURVE_FITTER_AMP_THRESH | I-data amplitude threshold for Curve fitter |
| HL_LRA_AFC_ZETA            | AFC damping factor                          |
| HL_SMART_DRIVE_APPLY       | Apply SmartDrive algorithm                  |
| HL_SMART_DRIVE_UPDATE      | Update SmartDrive algorithm                 |
| HL_SMART_DRIVE_PEAK_AMP    | Peak expected back EMF amplitude*           |
| HL_SMART_DRIVE_TAU         | Mechanical time constant*                   |
| HL_SMART_DRIVE_LAMBDA      | RLS forgetting factor                       |
| HL_SMART_DRIVE_DELTA       | RLS matrix initialization parameter         |

*For optimum Overdrive operation, it should be set according to the LRA in use.

More details can be found in the Haptics Algorithm Library [documentation](@ref haptics_lib).

### Waveform Memory parameters
The waveform timebase should be specified, in `platform_devices.h`:

| Parameter    | Description                                             |
| --           | --                                                      |
| WM_TIMEBASE  | Waveform sequence timebase: false: 5.44ms, true: 1.36ms |

The waveform memory data (`wm_data`), containing the haptic waveform patterns should be specified
in `waveform_memory.c`.

More details can be found in the Waveform Memory Decoder [documentation](@ref wm_decoder).
