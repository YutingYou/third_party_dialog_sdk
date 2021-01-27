BLE Advertising demo application {#ble_adv}
========================================================

## Overview

The application is an example of BLE featured demo. It implements the peripheral
role of device. After power on, the device starts advertising informing about
its accessibility.

Features:
- By default the application advertises exposing target with name 'Dialog ADV Demo'
- Responds to scan request.
- Accepts incoming connections from central peers.
- Allows one connection to be kept at a time.
- Interacts to bonding from central peer.
- Restarts advertising by default after disconnection from peer.
- Exposes Generic Access and Generic Attribute profiles only.

## Installation procedure

The project is located in the \b 'projects/dk_apps/demos/ble_adv' folder.

To install the project follow the [General Installation and Debugging Procedure]
(@ref install_and_debug_procedure).

## Manual testing

Download (from QSPI) the code on the target. Run serial communication terminal
(cutecom, screen, putty) with rate 115200 8n1 and attach to ttyUSBx or COMx
logical port depend on a host system. After restarting the target, on terminal
there can be seen "BLE Advertising demo ready" diagnostic message.
To get diagnostic messages support on terminal dg_configAUTOTEST_ENABLE must be
set in application configuration file. All basic events happening on target
during interaction with connected peer can be seen on the terminal as well like
for instance "Device connected, Device disconnected, Pairing request, ..."
