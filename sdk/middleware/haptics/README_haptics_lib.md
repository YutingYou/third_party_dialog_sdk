Dialog Haptics Algorithm Library {#haptics_lib}
===============================================


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

The Haptics Algorithms Library provides a set of algorithmic modules for the
open and closed loop monitoring and control of haptic actuators such as
Eccentric Rotating Mass motors (ERMs) and Linear Resonant Actuators (LRAs).

The provided algorithms include functionality for actuator back electro-motive
force (EMF) measurement/analysis, resonant frequency tracking (LRA only) and
acceleration/braking.

The library is designed to work in conjunction with the haptic driver interface
on the Dialog Semiconductor DA1469x Bluetooth Low Energy SoC. This interface can
be configured to drive haptic actuators with either DC levels (in the case of
ERMs) or square wave drive signals (in the case of LRAs) at varying drive levels
and frequencies and can operate in both open and closed loop modes. It is also
able to measure the electrical current flowing through the haptic driver circuit
by using an analog to digital converter (ADC) to sample the voltage drop across
a sense resistor (internal to chip), placed in series with the actuator in the
drive circuit.

The current flowing through an actuator circuit is dependent on both the DC
drive voltage and the back-EMF of the actuator. By calibrating the sense circuit
to account for differences in impedance in both the sense resistor and the
actuator, it is possible to distinguish the back-EMF current from the drive
current and hence monitor the state of the actuator.

In the case of an ERM, the back-EMF is dependent on its speed/loading. By
measuring the back-EMF we can therefore monitor when the ERM reaches equilibrium
speed and how long it takes to get there.

In the case of an LRA, the back-EMF and hence the measured current is induced by
speed of the movement of the magnetic mass through the voice coil. As this
movement is sinusoidal, this results in a sinusoidal signal that is superimposed
on top of the square wave drive signal. By measuring the amplitude of the
back-EMF signal, we can monitor when the LRA reaches equilibrium and how long it
takes to get there. By measuring the phase on the back-EMF signal, we can
monitor whether the LRA is being driven on its resonant frequency.


## Curve Fitter ##

The Curve Fitter module is provided for determining the behavior and state of a
haptic actuator by monitoring and analyzing the electrical current induced by
the back-EMF as it rotates/oscillates.

The haptic driver interface samples the ADC 8 times every half period. Curve
Fitter processing is designed to be triggered just after the seventh sample has
been captured so that the sampled data can be analyzed and new drive parameters
calculated prior to the start of the next half period.

The curve fitter analyzes the shape of the sampled half period data to separate
the sinusoidal back EMF current signal from the DC drive signal to calculate the
amplitude and phase offset of the back-EMF signal (LRA only) as well as the DC
drive level.

### API Usage ###

#### Initialization ####

An instance of the `curve_fitter_params` parameter structure must be
instantiated, either statically or on the heap, and must be maintained for the
life-cycle of haptics processing.

Before processing can be performed on an instance of the Curve Fitter module, a
subset of the parameters within the `curve_fitter_params` structure must be
initialized. These parameters are as follows:
* `amplitude_threshold`: Amplitude threshold below which `phi` is not
  calculated.

Note: All remaining parameters are output parameters and need not be
initialized.

When the amplitude of the back-EMF signal is low, the curve fitter cannot
accurately calculate its phase offset and can erroneously lock on to spurious
peaks generated by noise. This can have a negative impact on the behavior of the
automatic frequency control algorithm. To prevent this, the
`amplitude_threshold` parameter is provided to specify the amplitude threshold
below which the phase offset should not be calculated.

This should be set to an amplitude level at which the back-EMF signal is
comfortably distinguishable from the noise. This parameter uses the same value
representation as the raw `i_data` samples captured by the driver interface (see
below).

Note: When the amplitude of the back-EMF signal is below the specified
threshold, the output phase offset (`phi`) will be set to zero.

#### Processing ####

Curve Fitter module processing is performed by calling the
`curve_fitter_process()` function. This function is designed to be called from
within a call-back function registered with the haptic driver interface's
low-level driver.

The parameters of this function are as follows:
* `params`: Pointer to an instance of the `curve_fitter_params` structure.
* `i_data`: Pointer to an array of ADC current data samples.

The `i_data` pointer should reference an array of the 8 16-bit samples of ADC
data captured by the driver interface over the duration of the half period.

After calling the `curve_fitter_process()` function, the calculated output
parameters can be read from the `curve_fitter_params` structure. The output
parameters are as follows:
* `dc_offset`: DC current component/offset.
* `amplitude`: Amplitude of back-EMF current signal.
* `phi`: Phase offset of back-EMF current signal.
* `phasor`: Phasor representation of back-EMF current signal.
  * `x`: Real component of `phasor`.
  * `y`: Imaginary component of `phasor`.

The representations of the `dc_offset`, `amplitude` parameters and components of
`phasor` parameter are the same as the raw `i_data` samples.

The representation of `phi` is normalized angle in Q1.15 fixed-point format
(i.e. -32768 to 32767 represents -&pi; to +&pi;).


## Automatic Frequency Control (AFC) ##

The Automatic Frequency Control (AFC) module is provided for monitoring the
resonant frequency of an LRA and adjusting the drive frequency accordingly. This
ensures that the LRA is driven on its resonant frequency at all times,
maximizing its efficiency and the amount of accelerative force delivered.

The algorithm monitors the phase offset calculated by the curve fitter to
determine whether or not the LRA is resonating in phase with the drive signal
and hence is being driven at its resonant frequency. When a phase offset is
detected the AFC adjusts the half period of the drive signal accordingly until
the phase offset disappears.

Note: The AFC module is only applicable for LRAs and should be disabled when an
ERM is in use.

### API Usage ###

#### Initialization ####

An instance of the `lra_afc_params` parameter structure must be instantiated,
either statically or on the heap, and must be maintained for the life-cycle of
haptics processing.

Before processing can be performed on an instance of the AFC module, the
parameters within the `lra_afc_params` structure must be initialized. These
parameters are as follows:
* `half_period`: Initial half period.
* `half_period_min`: Minimum half period limit.
* `half_period_max`: Maximum half period limit.
* `zeta`: Damping factor.

The initial half period (`half_period`) should be initialized by setting its
value according to a half period that corresponds to the resonant frequency of
the LRA as specified in its datasheet.

The minimum and maximum half period limits specify the allowable range of half
periods that can be calculated by the algorithm. These prevent the algorithm
from converging on other resonant peaks other than that of the fundamental
resonant frequency, such as harmonic and sub-harmonic frequencies. Typically
this range should be set to encompass the full variability in potential resonant
frequency experienced by all LRAs of a particular type under all operating
conditions. `half_period_min` should always be set to a value which is greater
than half the nominal half period. `half_period_max` should always be set to a
value which is less than double the nominal half period.

The representation of `half_period`, `half_period_min` and `half_period_max` is
the same unsigned 16-bit format as that used to configure the half period in the
driver interface.

In some cases there may be some instability or resonance in the closed control
loop tracking the LRA's resonant frequency. This can result in some oscillation
in the calculated resonant frequency, causing it to take longer than it should
to converge or not to converge at all. To prevent this, a parameter has been
provided (`zeta`) to damp out any oscillations and make the control loop more
stable. This is represented in unsigned Q1.15 fixed-point format with the range
of 0 to 32768 (representing 0.0 to 1.0). The lower this value, the more damping
is applied. Setting this to a value of 32768 (1.0) effectively disables damping.

#### Processing ####

AFC module processing is performed by calling the `lra_afc_process()` function.
This function is designed to be called from within a call-back function
registered with the haptic driver interface's low-level driver.

The parameters of this function are as follows:
* `params`: Pointer to an instance of the `lra_afc_params` structure.
* `half_period`: Half period value for current half period.
* `phase_offset`: Phase offset for current half period.

The `half_period` parameter should be set according to the half period value
used to drive the LRA over the current half period. Ideally this should be read
directly from the driver interface and uses the same representation.

The `phase_offset` parameter should be set according to the value of `phi`
calculated by the curve fitter from the sampled `i_data` for the current half
period and uses the same representation.

After calling the `lra_afc_process()` function, the updated `half_period`
parameter can be read from the `lra_afc_params` structure. Note that this is
also returned by the `lra_afc_process()` function. This updated value should be
used to configure the driver interface for the next half period.


## SmartDrive&trade; ##

The SmartDrive module is provided for overdriving LRAs to accelerate/brake them,
improving their transient responses as well as to monitor and adapt to changes
in their mechanical time constants.

LRAs typically have an inherent mechanical time constant associated with their
mechanics which dictates how long they take to reach their equilibrium state
after a change in the level at which they are driven. This can result in some
lag before the full accelerative force is felt after a pulse is triggered and
for it to die away again after then pulse has ended. This is especially
problematic for the more transient haptic patterns such as short pulses, pulse
trains and more complex dynamic patterns as the energy is spread out over time
resulting in less sharp and distinct haptic sensations.

To alleviate this, the SmartDrive algorithm is able to overdrive the LRA to
accelerate/brake it, shortening the associated start/stop times resulting in an
accelerative amplitude response that more closely matches the drive level
specified in the haptic pattern being driven.

In order to know how hard the LRA should be overdriven, the SmartDrive algorithm
must have knowledge of what its mechanical time constant actually is and track
how it changes according to changes in the operating environment (e.g.
temperature, pressure, mechanical coupling, etc.). It does this by tracking and
adapting to changes in the amplitude response of the LRA's back-EMF signal
according to changes in the drive level, by means of a recursive least squares
(RLS) adaptive filter.

Note: The SmartDrive module is currently only applicable for LRAs and should be
disabled when an ERM is in use. Support for ERMs will be added in a future
release.

### API Usage ###

#### Initialization ####

An instance of the `smart_drive_params` parameter structure must be
instantiated, either statically or on the heap, and must be maintained for the
life-cycle of haptics processing.

Before processing can be performed on an instance of the SmartDrive module, a
subset of the parameters within the `smart_drive_params` structure must be
initialized. These parameters are as follows:
* `flags`: Instance of the `smart_drive_flags` bitfield structure:
  * `apply`: Apply flag.
  * `update`: Update flag.
* `peak_amplitude`: Peak expected back-EMF amplitude.
* `tau`: Mechanical time constant.
* `lambda`: RLS forgetting factor.
* `delta`: RLS matrix initialization parameter.

Note: All remaining parameters are state or output parameters and need not be
initialized.

The `flags` parameter includes a set of control flags for the algorithm. These
can either be set at initialization and left unchanged or modified at runtime if
more dynamic control of the algorithm is required.
* The `apply` flag controls the drive component of the algorithm. When enabled
  the LRA is overdriven to accelerate/brake it.
* The `update` flag controls the adaptive part of the algorithm. When enabled
  the algorithm monitors the measured amplitude response of the LRA's back-EMF
  signal to adapt its internal model coefficients using the RLS filter.

These flags are mutually exclusive and can be used in any configuration. However
when applying the drive component of the algorithm with the adaptive part
disabled, it is necessary to initialize the `tau` parameter with a relatively
accurate estimate of the true mechanical time constant. Note: This configuration
has the advantage of allowing SmartDrive to be used in open loop mode.

`peak_amplitude` should be set according to the expected peak back-EMF amplitude
when the LRA is driven on its resonant frequency at the maximum allowable drive
level. Ideally this value should be obtained by measurement. The representation
of this value is the same as the format used by the raw ADC samples and as
calculated by the curve fitter.

The mechanical time constant (`tau`) is represented in unsigned Q1.15
fixed-point format with a range of 0 to 32768 (representing 0.0 to 1.0). It
should be set according to the start/stop time for the LRA as stated in its
datasheet. If unavailable, this value can also be obtained by measurement.
Alternatively it can be initialized to a value of 32768 (1.0) and the true value
can be calculated by the algorithm at runtime, although it will require some
suitable stimulus before it is able to adapt and accurately accelerate/brake the
LRA.

The RLS forgetting factor (`lambda`) is represented in unsigned Q1.15
fixed-point format with a range of 0 to 32768 (representing 0.0 to 1.0). This
should be initialized to a value of slightly under 32768 (1.0). The default
value of 32440 (0.99) is recommended.

The RLS matrix initialization parameter (`delta`) is represented in unsigned
integer format. The higher this value, the quicker the RLS algorithm will
converge on solution however instability may result if set too high. A value in
the range of 100-1000 is recommended.

After initializing all of the required parameters in the `smart_drive_params`
parameter structure, it is necessary to complete the module initialization by
calling the `smart_drive_init()` function, passing a pointer to the parameter
structure instance via the `params` function parameter.

#### Processing ####

SmartDrive module processing is performed by calling the `smart_drive_process()`
function.  This function is designed to be called from within a call-back
function registered with the haptic driver interface's low-level driver.

The parameters of this function are as follows:
* `params`: Pointer to an instance of the `smart_drive_params` structure.
* `target_level`: Target drive level for next half period.
* `amplitude`: Measured LRA back-EMF amplitude for current half period.
* `drive_level`: Drive level for current half period.

The `target_level` parameter should be set according to the desired target drive
level for the next half period. This parameter is represented as an unsigned
Q1.15 fixed-point format in the range of 0 to 32768 (representing 0 to 100%).

The `amplitude` parameter should be set according to the value of the measured
LRA back-EMF amplitude as calculated by the curve fitter from the sampled
`i_data` for the current half period and uses the same representation.

The `drive_level` parameter should be set according to the *actual* drive level
used to drive the LRA for the current half period, as calculated by the
SmartDrive algorithm in the previous half period and accounting for any other
subsequent external modification (e.g. drive level limiting in the low-level
driver). This parameter is represented as an unsigned Q1.15 fixed-point format
in the range of 0 to 32768 (representing 0 to 100%).

After calling the `smart_drive_process()` function, the updated `drive_level`
parameter can be read from the `smart_drive_params` structure. Note that this is
also returned by the `smart_drive_process()` function. This updated value should
be used to configure the driver interface for the next half period.

Note that when the `apply` flag is disabled, the updated drive level is simply
set to the same value as the given target level. When enabled the given target
level is modified to overdrive the LRA but this is limited by the amount of
dynamic range available. For example, when a target level of 100% is requested,
it is not possible to overdrive the LRA as the target level is already at the
maximum.


## Haptics Algorithm Library Wrapper ##

The Haptics Algorithm Library Wrapper module is provided for integrating all of
the modules within the Haptics Library together so that they can operate
efficiently as a whole and to manage their interactions. Although the library
does allow the flexibility to use each of the constituent modules individually,
it is recommended to use them in the context of the wrapper, especially if more
than a single module is to be used at a time.

In addition to wrapping the modules contained within the library, the wrapper
adds additional functionality such as intelligence for managing the polarity of
the driver interface. This includes a mechanism for making sure that the drive
polarity at the start of all pulses/patterns is consistent.

### API Usage ###

#### Initialization ####

An instance of the `haptics_lib_params` parameter structure must be
instantiated, either statically or on the heap, and must be maintained for the
life-cycle of haptics processing.

Before processing can be performed on an instance of the wrapper module, a
subset of the parameters within the `haptics_lib_params` structure must be
initialized. These parameters are as follows:
* `flags`: Instance of the `haptics_lib_flags` bitfield structure:
  * `curve_fitter_enabled`: Enable flag for curve fitter.
  * `lra_afc_enabled`: Enable flag for AFC.
  * `smart_drive_enabled`: Enable flag for SmartDrive.
  * `start_polarity`: Start polarity of pulses/patterns.
* `i_data_threshold`: Threshold of valid `i_data`.
* `curve_fitter`: Instance of the `curve_fitter_params` parameter structure.
* `lra_afc`: Instance of the `lra_afc_params` parameter structure.
* `smart_drive`: Instance of the `smart_drive_params` parameter structure.

Note: All remaining parameters are output parameters and need not be
initialized.

The `flags` parameter includes a set of control flags for the library. These
either can be set at initialization and left unchanged or modified at runtime if
more dynamic control of the library is required. Most of these flags simply
control the enable state of the individual library modules. The exception is the
`start_polarity` flag which controls the start polarity of all patterns/pulses.

Note: As all of the other modules depend on the curve fitter, the
`curve_fitter_enabled` flag also acts as a global enable/disable control for all
modules.

Due to the way the driver interface's sense circuit is designed, the ADC is
unable to detect voltages across the sense resistor that are below a certain
threshold (including negative voltages). This results in `i_data` that is
sometimes clipped at a certain minimum value. To protect the algorithms from
this erroneous data, the `i_data_threshold` parameter is provided to specify the
threshold below which `i_data` should no longer be considered as valid. If the
wrapper detects that the sampled `i_data` dips below this threshold during a
half period it temporarily disables the adaptation of the AFC and SmartDrive
modules until the data is valid again.

The remaining module parameter structures should be initialized as outlined in
the previous sections.

After initializing all of the required parameters in the `haptics_lib_params`
parameter structure, it is necessary to complete the wrapper initialization by
calling the `haptics_lib_init()` function, passing a pointer to the parameter
structure instance via the `params` function parameter. This in turn calls the
initialization functions for all of the submodules (if they have one).

#### Processing ####

Haptics Algorithm Library Wrapper processing is performed by calling the
`haptics_lib_process()` function. This function is designed to be called from
within a call-back function registered with the haptic driver interface's
low-level driver.

The parameters of this function are as follows:
* `params`: Pointer to an instance of the `haptics_lib_params` structure.
* `i_data`: Pointer to an array of ADC current data samples.
* `half_period`: Pointer to half period value for current half period.
* `drive_level`: Pointer to drive level for current half period.
* `state`: Pointer to an instance of the `haptics_lib_if_state` structure:
  * `drive_polarity`: Phase/polarity of the current half period.
  * `drive_polarity_inverted`: Indicates whether the drive polarity is currently
  inverted.
  * `i_data_valid`: Indicates whether `i_data` is valid.
* `target_level`: Target drive level for next half period.

The `i_data`, `half_period`, `drive_level` and `target_level` values should be
set as has been previously outlined for the submodules where they are
ultimately used. In the case of the `half_period` and `drive_level` however,
these are passed by reference so that they can be updated by the
`haptics_lib_process()` function and passed back to the calling function to be
used to configure the driver interface for the next half period.

The `state` parameter is a bitfield structure of flags which indicate the
current state of the hardware driver interface, primarily with regards to
polarity. These flags should be set by the calling function prior to calling
`haptics_lib_process()`.

The `i_data_valid` flag should be used to indicate whether the data passed via
the `i_data` parameter is valid. When set to 0, the `i_data` is ignored and
adaptation of the algorithms is disabled. In most cases, when operating in
closed loop mode, this should always be set to 1 unless the `i_data` is invalid
for any reason.  Set to 0 when operating in open loop mode.

The `state` parameter is passed by reference so that the
`drive_polarity_inverted` can be modified by the `haptics_lib_process()`
function. This allows the wrapper to indicate to the calling function what the
state of the polarity inversion should be for the next half period. This should
be used to configure the driver interface accordingly.
