Eccentric Rotating Mass application {#erm}
==========================================

## Overview
This application demonstrates the use of the Haptic Driver/Controller HW block in order to drive
an *Eccentric Rotating Mass* (ERM) vibration motor.
> _Note_: It is only applicable to *DA14697/9* devices.

## Usage
By pressing the K1 button of the Development Kit (DK) Pro, the ERM drive is switched on and off.

The application also redirects I/O (e.g. printf) to SEGGER's Real Time Terminal.
To access the terminal, open a telnet terminal to locahost:19021.

## Setup
The ERM should be connected to the *HDRVP* and *HDRVM* pins (Haptic Driver output) of the device
(accessible through the J10 header of the DK Pro).
> _Note_: The actual two pins of the header may differ among different DK revisions.

## Configuration
### Actuator characteristics
The characteristics of the ERM motor should be specified in the respective configuration macros:

| Parameter      | Description                                                                 |
| --             | --                                                                          |
| ERM_MAX_ABS_V  | Absolute maximum voltage (steady state operation)  (in mV)                  |
| ERM_MAX_NOM_V  | Nominal maximum voltage (transient while changing vibration levels) (in mV) |
| ERM_R          | Impedance (in Ohms)                                                         |

### Drive level and polarity
The drive level and polarity (when the ERM drive is on) should be specified (according to the
desired vibration intensity level and the desired HDRVP/M output pin):
                                                                                                                     
| Parameter       | Description                                                                 |
| --              | --                                                                          |
| ERM_DRIVE_LEVEL | ERM drive level (valid range: 0 - 1.0)                                      |
| ERM_DRIVE PIN   | ERM drive polarity (Haptic Driver output pin): HW_HAPTIC_ERM_OUTPUT_HDRVP/M |