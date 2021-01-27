# BLE CLI application {#ble_cli}

## Overview
This application provides a Command Line Interface handling functions and events from the SDK APIs.

Supported APIs:
 * `ble_gap.h`
 * `ble_gatts.h`
 * `ble_gattc.h`
 * `hw_gpio.h`
 * `hw_timer1.h` and `hw_timer2.h` (DA1468x) or `hw_timer.h`(DA1469x) - PWM related features only
 * `hw_wkup.h`

## Installation procedure
The project is located in the `projects/dk_apps/demos/ble_cli` folder.

To install the project follow the [General Installation and Debugging Procedure](@ref install_and_debug_procedure).

## Development Kit Setup
Configure your Development Kit as described below:

> ### DA1469x Development Kit Pro Mainboard
> - UTX Enabled S1 (1-12) ON
> - URX Enabled S1 (2-11) ON
> - UCTS Enabled S1 (4-9) ON

> For more details refer to [DA1469x Pro Development Kit Mother Board Schematic](https://www.dialog-semiconductor.com/products/da14695-development-kit-pro)
(Page 2 - Dip Switch S1 in the left bottom corner)

> ### DA1468x Development Kit Pro
> - TX  Enabled J15 (1-3) ON
> - RX  Enabled J15 (4-5) ON
> - CTS Enabled J15 (7-8) ON 

> For more details refer to [UM-B-060 User Manual: DA1468x/DA1510x Development Kit - Pro](https://support.dialog-semiconductor.com/resource/um-b-060-user-manual-da1468xda1510x-development-kit-pro)
(Chapter 5.3.5 - Full UART configuration HCI/UART header (J15))

## UART Setup
Use a uart termninal (Putty etc) in order to establish uart communication with the following settings:

- Baudrate:     115.200 bps
- Data Bits:    8
- Stop Bits:    1
- Parity:       None
- Flow Control: None or RTS/CTS
- Local Echo: Enabled (optional) in order for the user to see the given commands

## Usage
To use the application, the device must be connected over a serial port. Once the application has started, the message "ble_cli application started" will be printed out.

### Commands
The commands are split in categories named after the supported APIs.
To list all available commands per category type the category name i.e.:
~~~~
$> gap
    gap commands:
    gap address_get
    gap address_set
    gap dev_name_set
    gap dev_name_get
    gap appearance_set
    gap appearance_get
    gap per_pref_conn_params_set
    gap per_pref_conn_params_get
~~~~
In case category name does not match any of the supported APIs, all available categories will be printed out.

#### Getting Help
To find command arguments type command then `help`. Application will print the helper pattern i.e.:
~~~~
$> gap address_set help
    usage: gap address_set [pub|priv|priv-res|priv-nonres] <bd_addr> [renew_dur]
~~~~
The command helper pattern will also be printed if any of the mandatory arguments is invalid or missing.

#### Running a Command
To run a command type it with all its mandatory arguments. The ommitted optional arguments are set to their default value.

Example:
~~~~
    gap address_set priv 80:EA:AD:00:00:00
~~~~
will set the device address to _private_, _80:EA:AD:00:00:00_ with the default renew duration (see `ble_config.h`).

### Events
Application will print notifications on GAP, GATTS, or GATTC events. Event Notification contains event data and event name i.e.:
~~~~
    BLE_EVT_GAP_ADV_REPORT
        Type of advertising packet: 0
        BD address of advertising device: public, B1:B2:B3:B4:B5:B6
        RSSI: 46
        Length of advertising data: 11
        Data: 0201060432501929308404
~~~~
