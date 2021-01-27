NV Parameters image area creation script {#nvparam_script}
===================================================

## Overview

This script creates an image of NV-Parameters, which can be then written directly to the proper partition on the QSPI flash using `cli_programmer`.
This process is automated in the `program_qspi_nvparam` launcher script, a member of the `python_scripts` project in Eclipse.
The reader can refer to the `pxp_reporter` demo application as a walkthrough example.

## Project and Parameters Setup

### Platform parameters

The generic platform paramenters are located in the file 
- `${SDKROOT}/sdk/middleware/adapters/include/platform_nvparam.h`

while their values can be edited directly in the file

- `${SDKROOT}/sdk/middleware/adapters/include/platform_nvparam_values.h`

where the NV-parameter area `ble_platform`, its contents, their exact format and their default values (if enabled) are defined.

The user is free to change the NV-parameters, respecting the offset and size values of the former file.
Please note that a parameter remains inactive as long as its validity flag is unset (valid=`0x00`).

### Application-specific parameters

Additional to the platform parameters, common in every target application, this feature allows the definition of application-specific parameters.
This is achieved by creating two files similar to the previous paragraph.
The location of the new files can be in the application's config directory (for details see the manual script execution paragraph).
A typical directory is: `${PROJECT_location}/config`.

And the filenames are fixed (constant) as follows:
- `app_nvparam.h`
- `app_nvparam_values.h`

New NV-parameter logical blocks (areas) can defined, using the same macros and the structure of the `ble_platform` area.
Sample application parameters can be found in the pxp_reporter demo.

## Execution Procedure

### Implementation Details

The directory `{SDKROOT}/utilities/nvparam` holds two source files necessary for building the application named `nvparam.elf`.

- `symbols.c`

holding the symbols for the NV-parameters definitions and values.

- `sections.ld.h`

is preprocessed to produce a linker script which places symbols from `symbols.o` at proper locations (as defined by nvparam offsets) relative to the `nvparam` section.

The `nvparam.elf` and `sections.ld` files are placed in the application's build directory (i.e. `${PROJECT_location}/DA1469x-00-Debug_QSPI/`).
Then, the nvparam section data is extracted form the elf file using `objcopy`.
The outcome is the nvparam.bin file, which can be directly written to the QSPI flash of the target device, at an address that depends on the flash size and the device family (i.e. DA1469x/4MB flash, offset=`0x1FF000`).

Important: The two above mentioned files, their location and the location of the various python scripts are not expected to change. Doing so requires adapting the source code.

### Generic Procedures

- Select and build the desired build configuration of the target application.
- Write the image to the QSPI flash of the target device.

### Automatically, via Eclipse

- Open the project `python_scripts` in Eclipse under `${SDKROOT}/utilities/python_scripts`
- Select the target application in the Project Explorer
- Execute the program_qspi_nvparam launcher

### Manually, via the python scripts

The manual execution involves the user running the scripts
- `${SDKROOT}/utilities/python_scripts/qspi/program_qspi_nvparam.py`

and/or

- `${SDKROOT}/utilities/python_scripts/qspi/create_nvparam.py`


providing the necessary arguments, automatically constructed in the Eclipse execution.

Executing the `program_qspi_nvparam.py` script sets the default arguments and executes automatically the latter `create_nvparam.py` script.

A manual execution of the `create_nvparam.py` script, involves the user to manually provide all the arguments.
More specifically, the first of the directories passed via the argument `--inc_path` MUST be the application's configuration directory (i.e. `${PROJECT_location}/config`).
Most importantly, this is where the application-specific NV-parameters must be placed (if any), as described in the previous paragraphs.

Hint: Executing any of the scripts with no arguments prints a help message.

## Using the NV-Parameters

After uploading both the application and the nvparam images to the target QSPI flash, one can read, change and pass to the application any pre-defined NV-parameter, using the respective tool of SmartSnippets Toolbox.
The offset giving the location of the `nvparam` image to the target flash is necessary.
In the default case it is automatically provided by the Toolbox.
