General installation and debugging procedure {#install_and_debug_procedure}
===================================================

This guide presents the steps which should be followed in order to build, execute, flash and debug SDK projects.

## Importing a project

-# In `SmartSnippets Studio` select `File->Import` and then `General->Existing projects into Workspace`

-# In the select dialog, choose the root folder of your SDK (`$SDK_ROOT` rather than `$SDK_ROOT\sdk`)

-# Select the project you wish to import from the checked list-box

## Building for RAM

Build your project on the host (user's PC) using a *RAM build configuration (e.g DA1469x-00-Release_RAM)

## Flashing tools

The flashing tools comprise of the following projects:
 - `cli_programmer`: a command line application for issuing programming commands. See also [CLI programmer application] (@ref cli_programmer).

 - `uartboot`: the (secondary) boot loader running on the target which executes libprogrammer commands

 - `mkimage`: an application which constructs image files to be flashed down to your device, by prepending to the elf-binary the appropriate header for the rom boot loader

 - `libprogrammer`: the (host-side) library which implements the programming functionality
 
 - `libmkimage`: the (host-side) library for mkimage

 - `libbo_crypto`: a third-party encryption/decryption library used for [secure image functionality](@ref secure_image).
 
 - `python_scripts`: a collection of scripts which utilize flashing tools and implement the `External tools` functionality

The flashing tools have the following dependencies:
 - `cli_programmer` depends on `libprogrammer` and `uartboot`
 - `mkimage` depends on `libmkimage` and `libbo_crypto`

In order to re-build the flashing tools used by the `External tools`, the flashing tools' projects must be imported and built, using the appropriate build configuration.
The user then may choose between:
* Windows or Linux, depending on the host OS
* static library or dynamic library linking, for the `libprogrammer`, `libbo_crypto` and `libmkimage` mentioned above
* Debug or Release, depending on the use of the target application

## FLASH programming

-# Using the above-described importing procedure, import the `python_scripts` project. Keeping this project open, a set of scripts will be visible under the green arrow menu entitled `External tools`
 
-# Select the chip revision and flash options by running `program_qspi_config` script from `External tools` drop down list (this step needs to be done only once)

-# Build the project on the host (user's PC) using one of the flash build configurations named *QSPI.
  \note Make sure that the build configuration matches your chip revision e.g DA1469x-00-Release_QSPI

-# Select the project to be flashed by clicking on any folder of the project

-# Connect board (USB2 connector) to a PC using a USB cable.

-# Flash the image to the qspi over either the jtag interface or the serial port, using the appropriate script from the `External tools`:
 - Flash over jtag: `program_qspi_jtag`
 - Flash over serial port: `program_qspi_serial` (the scripts prompt for the PC serial port where the board is attached)

## Run and Debug a project

-# Connect the board (USB2 connector) to a PC using a USB cable.
-# Click <b> `'Debug As...'` </b> drop-down list and select:

 - RAM - to launch and debug the project from RAM
 - QSPI - to launch and debug the project from QSPI
 - ATTACH - to attach to the currently running project

\note The tools/launchers that become visible correspond to the selected build configuration of the active project.

## Debug Logs

Open a serial terminal and connect to a proper serial port (e.g /dev/ttyUSB0 for Linux or COMx for Windows environment), baudrate: 115200. It will be needed for printing debug logs for the user. See also [Collect debug information ] (@ref collect_debug_info).
