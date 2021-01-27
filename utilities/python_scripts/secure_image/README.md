Secure image programming procedure {#secure_image}
==================================================


> _Note_: This procedure is valid only for **DA1469x** devices. All scripts require *Python 3*.
> If Python supports GUI mode (*Tkinter* package is required), the whole configuration will be done using popup-windows.
> Otherwise, scripts will use the simple command line interpreter.


### 1. Check tools

The tools required for secure image programming are the same as the tools
required for typical flash memory programming, i.e. `cli_programmer` and `mkimage`.
Both tools must be located in the `binaries/` directory.

### 2. Import `python_scripts` project to Smart Snippets Studio

This project contains a set of launchers/external tools that can be used from Smart Snippets Studio.

This step is not required if scripts are executed externally (from a terminal/command line).

### 3. Create QSPI programming configuration file

Run the `utilities/python_scripts/qspi/program_qspi_config.py` script (either from Smart Snippets Studio or externally) and choose/provide the following:
- product ID
- flash memory
- active FW image address
- update FW image address

`program_qspi.xml` file will be created in `utilities/python_scripts/qspi` location.

The configuration file can be created also using external tool (`program_qspi_config`) from Smart Snippets Studio.

> _Note_: The same configuration file can be used for programming many devices of the same type
> and with the same flash memory.

### 4. Create product keys and secure image configuration files

Run the `utilities/python_scripts/secure_image/secure_keys_cfg.py` script and choose/provide the following:

| Parameter | Description |
| -- | -- |
| product ID | *DA1469x* |
| secure boot support | *enabled/disabled* |
| public key index | *Used for image's signature verification on the device.* |
| nonce | *Used for application binary encryption/decryption in AES CTR mode. If not provided, it will be generated randomly during image generation.* |
| FW decryption key index | *Used for the application binary decryption on the device.* |
| key revocations (*optional*) | *Select key indices that will be revoked on the device when the secure image will be booted. A revoked key cannot be used for image's signature verification or decryption.* |

It is highly recommended to run the script from its default location (`utilities/python_scripts/secure_image/`).

`product_keys.xml` and `secure_cfg.xml` files will be created in the script's location.

Configuration files can be created also using external tool (`secure_config`) from Smart Snippets Studio.
In that case, files will be created in the script's default location.


> _Note_: Do not lose product keys file, because without it changing the software on the device with
> programmed keys and enabled secure boot will be impossible! The same configuration/product
> keys file can be used for programming many devices of the same type.

### 5. Program product keys and enable secure booting

Run the `utilities/python_scripts/secure_image/secure_keys_prog.py` script (with specifying
paths to the `product_keys.xml` and `secure_cfg.xml` files, if they are in different than the
default location). The script can be executed using JLink or Serial interface (check printed help
message). The script should write the following keys to the OTP memory:
 - 8 public keys - *used for image signature verification (at least one is required)*
 - 8 symmetric keys - *used for image on-the-fly decryption (at least one is required)*
 - 8 symmetric keys (*optional*) - *used for user data encryption/decryption*

If the keys have already been written, then no action will be performed.

This process consists of four steps.
Status of each step is printed in the short report.
Here are some examples of the report fragments:

Writing keys without errors:

    ...
    . Product keys verification... PASS
    . Checking OTP memory emptiness (keys area)... PASS
    . Matching the programmed OTP keys with the ones in product keys file... NOT RUN
    . Writing keys to the OTP memory... PASS
    ...

Keys are already written:

    ...
    . Product keys verification... PASS
    . Checking OTP memory emptiness (keys area)... FAIL
    . Matching the programmed OTP keys with the ones in product keys file... PASS
    . Writing keys to the OTP memory... NOT RUN
    ...

Keys are already written, but do not match to the ones in product keys file:

    ...
    . Product keys verification... PASS
    . Checking OTP memory emptiness (keys area)... FAIL
    . Matching the programmed OTP keys with the ones in product keys file... FAIL
    . Writing keys to the OTP memory... NOT RUN
    ...

Product keys file is invalid:

    ...
    . Product keys verification... FAIL
    . Checking OTP memory emptiness (keys area)... NOT RUN
    . Matching the programmed OTP keys with the ones in product keys file... NOT RUN
    . Writing keys to the OTP memory... NOT RUN
    ...

If secure boot support is enabled (step 4), then the script will try to enable it in
the configuration script (CS), which is placed also in the OTP memory.
If secure boot is already enabled in the CS, then no action will be performed.

Programming can be executed also using external tool (`secure_keys_prog_jtag` or `secure_keys_prog_serial`) from Smart Snippets Studio.

Execution status of each step is shown in the short report.
Here is an example of the report:

    ............................................................................................
    ..
    .. Script execution report
    ..
    ............................................................................................
    .
    . Checking configuration and product keys files... PASS
    . Checking product ID... PASS
    . Product keys verification... PASS
    . Checking OTP memory emptiness (keys area)... PASS
    . Matching the programmed OTP keys with the ones in product keys file... NOT RUN
    . Writing keys to the OTP memory... PASS
    . Reading revocation info from OTP... PASS
    . 	Revoked signature keys: []
    . 	Revoked user data keys: []
    . 	Revoked FW decryption keys: []
    . Enabling secure boot feature in configuration script... PASS

> _Note_: One Time Programmable memory (OTP) is programmed in this step! Since secure-booting
> is enabled in the CS, the non-secure application images cannot be started on the device.

### 6. Build application binary

Build the application that you want to program into device's flash memory. This must be a
Release/Debug QSPI build configuration for DA1469x device (e.g. `DA1469x-00-Release_QSPI_SUOTA`
in the pxp_reporter application).

### 7. Program application image

Run the `utilities/python_scripts/secure_image/secure_img_prog.py` script (with specifying paths to the `product_keys.xml` and `secure_cfg.xml` files, if they are in a different than the default location).
The path to the application binary file must be passed to the script as an argument.
The script can be executed using JLink or Serial interface (check printed help message).
The script should create a signed and encrypted image from the application binary file
and write it to the QSPI flash memory.
If the image has been written correctly, then the product header is written to the QSPI flash memory.

Programming can be executed also using external tool (`secure_keys_img_jtag` or `secure_keys_img_serial`) from Smart Snippets Studio.
In that case, the application project must be selected *before* the tool is started (binary file from current build configuration will be used).

### 8. Reboot device

Press `Reset` button on the device or use the `utilities/python_scripts/qspi/reboot_device.py` script.
If all previous steps have been properly completed (without any errors), then the application should start after this step.



### Increase device security

For increasing device security a few more actions can be done:
- disable debugger
- disable reading (by CPU) and/or writing (any) user data encryption/decryption keys area
- disable reading (by CPU) and/or writing (any) image decryption keys area
- disable writing (any) image signature verification keys area
- disable Development Mode in CS (this feature disables receiving application code from UART and debugger)

The first four can be done by setting a proper value in `SECURE_BOOT_REG` register (in CS).
The last one can be done by writing a special value in the CS (the `cli_programmer` application can be used for this).

> _Note_: Keep in mind that if the Development Mode is disabled, then Software Update over the Air
> (SUOTA) is the only option for changing the software on the device. Therefore, the application that is programmed in flash must support this procedure.

### Known restrictions and limitations

- Configuration script generates always 8 keys for every 3 categories.
If fewer keys should be  used, then the `product_keys.xml` file must be edited manually.
The rest of the keys cannot be written using the script - they can be written manually using `cli_programmer`.

- The keys file cannot be overwritten by the configuration script - if you want to change keys (e.g. for new device family), then you must remove the keys file manually.
In that case, do not forget to keep previous keys file in other location.

- Scripts do not check which keys are already revoked (this information is printed only as log).
If a key was revoked and its revocation is requested, then no action is performed (for the specified key).

### Troubleshooting

When the programmed application image cannot be booted on the device, check the following:

| Possible Cause | Recommended Action |
| -- | -- |
| Secure boot support is not enabled in the configuration script. | Enable it and perform step 5 again. |
| The signature and/or encryption key used for image generation does not match to the key that is stored in the OTP memory. | Make sure that there is no other product keys file that might have been programmed to the OTP memory earlier. Perform step 5 and check status of the relevant step in the short report ('Matching the programmed OTP keys with the ones in product keys file..'). |
| Used signature and/or encryption key has been revoked by the previous image. | At least one key in each category must be valid. Try use another key index. Perform step 5 and check which keys are already revoked on the device. |
