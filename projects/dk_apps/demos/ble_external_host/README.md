BLE External host application {#ble_external_host}
========================================================

## Overview

The application purpose is the use of transport layer on DA146xx platform by an
external user. For example a different BLE Host stack may be used with BLE
Controller on DA146xx targets.

Features:
- Any BLE Host stack/application can attach to and start using the DA146xx BLE
  Controller.

## Installation procedure

The project is located in the \b 'projects/dk_apps/demos/ble_external_host' folder.

To install the project follow the [General Installation and Debugging Procedure]
(@ref install_and_debug_procedure).

## Manual testing

Download (from QSPI) the code on the target. Attach to ttyUSBx (Linux host) or
COMx (Windows host) logical communication port. The DA146xx BLE Controller
should be accessible if the attachment was successful.
On a Linux host system for example the 'btattach' tool can be used. The hci[x]
interface is created by the kernel and it is used as an abstraction device
representing DA146xx Controller.
On Windows, 3VT - Waves Validation Tool uses this configuration for testing
and verifying purposes.
