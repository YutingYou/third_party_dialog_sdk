Temperature drift monitoring demo application {#snc_gpadc_demo}
==========================

## Overview

The application measures and monitors the temperature drift using the Sensor Node Controller (SNC).
The SNC is configured using the relevant adapter via the registration of the measure-callback and the temperature-parameters.
It then monitors the temperature drift on the on-chip radio sensor. The measurement is performed by the GPADC.

> Note:
> This project redirects I/O (e.g. printf) to SEGGER's Real Time Terminal.
> To access the terminal, open a telnet terminal to locahost:19021.

## Installation procedure

The project is located in the \b `projects\dk_apps\features\snc_gpadc_demo` folder.

To install the project follow the [General Installation and Debugging Procedure](@ref install_and_debug_procedure).

## Usage

The acceptable drift in the Celsius scale are set by the following macro (default value in parenthesis):

~~~{.c}
#define RADIO_TEMP_DRIFT        ( 2 )
~~~

In case the temperature exceeds the acceptable drift deviation, a message is printed on the terminal.

Also, K1 button can be used to set a temperature reference value that exceeds the max temperature range (i.e. 80 degrees) in order to validate the SNC's ucode.
