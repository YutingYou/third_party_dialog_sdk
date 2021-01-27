Multi Link demo application {#multi_link}
===========================

## Overview

This application allows connecting to many devices by writing their addresses to characteristic.

Features:

- The application uses both Central and Peripheral role.
- It starts up advertising and another central device may connect to it. After a successful
  connection, this central device is considered the main device.
- If the main device disconnects, advertising is started again. The next device that connects will
  become the main device.
- Event and connection information is printed to UART.
- A connected central can use the Multilink service (see below) to use the device's Central role
  and command it to connect to other peripheral devices that advertise.
- The main device can write a peer BD address to the Peripheral Address characteristic and the
  Dialog Multi-link device will initiate a connection procedure to this peer.
- This way it is possible to connect to more than one devices. This application can connect to up to
  eight devices.

## Installation procedure

The project is located in the \b `projects/dk_apps/demos/ble_multi_link` folder.

To install the project follow the [General Installation and Debugging Procedure](@ref install_and_debug_procedure).

### Eclipse method

Using 'SmartSnippets Studio' import the project and build it.
Then press the Run program_qspi_win button
(it is under Run > External Tools > External Tool Configurations) to download the demo
to the QSPI flash.

## PTS testing

Application can be used for executing PTS. It does not require any user
intervention during testing.

## Manual testing

Multilink service UUID:
{3292546e-0a42-4348-aa38-33aab6f9af93}

Peripheral Address characteristic UUID:
{3292546e-0a42-4348-aa38-33aab6f9af94} (properties=Write Without Response)

- Run a serial communication terminal to see application's console output.
  Connection settings: 15200 8n1.
- Run the application.
- Connect to the application. Name of device shall be "Dialog Multi-link".
  After a successful connection, your device will become the main device: the main device controls
  which peers the Dialog Multi-link shall connect to.
- Find the Peripheral Address characteristic and write one or more BD addresses of peripheral
  devices which you would like Dialog Multi-link to connect to.
  The first octet of data written to Peripheral Address characteristic is the address type, either
  public (0x00) or private (0x01). The next 6 octets represent the BD address.
  In order to connect to a device with public address AA:BB:CC:DD:EE:FF, the data written to the
  characteristic should be:
~~~
[0x00 0xff 0xee 0xdd 0xcc 0xbb 0xaa]
~~~

  When the connection is established, the following output will be printed on the console:
~~~
handle_evt_gap_connected: conn_idx=1 peer_address=AA:BB:CC:DD:EE:FF

Nr | Index | Address
 1 |     1 | AA:BB:CC:DD:EE:FF
 2 |     0 | 1C:4D:70:D7:18:0A
~~~
  After every successful connection, active connection information will be printed on the console.
- To disconnect one of the peripheral devices, you can initiate disconnection from that device,
  reset or shut it down.
- To connect to Dialog Multi-link from another central device, you should first disconnect from the
  current main device first.
- After the message "Advertising is on again" is printed on the console, you can try to connect from
  another central device, which will become the new main device after a successful connection.
- You can then write more addresses of peripheral devices to which Dialog Multi-link should connect.
