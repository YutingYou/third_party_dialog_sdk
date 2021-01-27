HOGP Host {#hogp_host}
===================================

## Overview

The application is a sample implementation of the HOGP Host of the HID Over GATT Profile,
as defined by the Bluetooth Special Interest Group.

The application supports both the Boot Host and the Report Host modes.

It supports clients for:
- the GATT Service
- the HID Service
- the Scan Parameters Service
- the Device Information Service
- the Battery Service

Features:

- Browsing for supported services (the services are listed above)
- Both Boot Host and Report Host roles are supported (configurable in build time in hogp_host_config.h)
- The Report Host role supports reading and writing to Input, Output and Feature reports and notifications
  for Input reports
- The Report Host role reads Report Map, HID Info and External Report References
- The Boot Host role supports notifications for the Boot Mouse Input, the Boot Keyboard Input reports and
  reading/writing operations for all Boot Report characteristics
- Most user interactions such as reports writing are exposed via console interface
- Incoming report's values are printed out to a terminal

## Installation procedure

The project is located in the \b `projects/dk_apps/ble_profiles/hogp_host` folder.

To install the project follow the [General Installation and Debugging Procedure](@ref install_and_debug_procedure).

## Suggested configurable parameters

- the Host role:
  Set the CFG_REPORT_MODE macro in the hogp_host_config.h

- the Connection parameters:
  Set the CFG_CONN_PARAMS macro in the hogp_host_config.h

## PTS testing

The application can be easily used for executing HOGP and SCPP PTS test cases.
User interaction might be triggered by using serial terminal.

## Manual testing

- Build the demo for an execution from the flash.
- Download the hogp_host.bin binary to flash and execute.
- Connect with the serial port terminal.
- Debug logs are printed out to the console
- In order to trigger commands, connect serial terminal

### Available commands

#### `get_protocol <hid_client_id>`

Get Protocol Mode command.

#### `cp_command <hid_client_id> <command>`

Control Point Command.

#### `boot_read <hid_client_id> <boot_report_type>`

Read Boot Report.

#### `boot_write <hid_client_id> <boot_report_type> <data>`

Write Boot Report.

#### `boot_notif <hid_client_id> <boot_report_type> <enable flag>`

Write Boot Report CCC descriptor.

#### `boot_read_ccc <hid_client_id> <boot_report_type>`

Read Boot Report CCC descriptor.

#### `report_read <hid_client_id> <report_type> <report_id>`

Read Report.

#### `report_write <hid_client_id> <report_type> <report_id> <confirm_flag> <data>`

Write Report.

#### `report_notif <hid_client_id> <report_id> <enable>`

Write Input Report CCC descriptor.

#### `report_read_ccc <hid_client_id> <report_id>`

Read Input Report CCC descriptor.

#### `scan <start|stop> [any]`

Start and stop scanning procedure.

Type <b>`scan start`</b> to start scanning for devices with the HIDS service. Only devices which
include UUID_SERVICE_HIDS (0x1812) in their advertising data will be listed.

To list any device found type <b>`scan start any`</b> instead.

The returned list of devices includes an index, a device address and a device name (if available).

Once the desired device is found, type <b>`scan stop`</b> to stop scanning.

#### `connect <address> [public|private]`

Connect to peripheral device.

The address type is searched on the found devices list (if present) or assumed to be public if not
found. It can be however specified manually by typing <b>`connect <address> public`</b> or
<b>`connect <address> private`</b> for public and private address type respectively.

#### `connect <index> [public|private]`

Connect to peripheral device which is matched to the index.

#### `connect cancel`

Cancel ongoing connection attempt.

#### `disconnect`

Disconnect first peripheral on list of connected devices.
