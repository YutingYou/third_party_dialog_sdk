Sensor Node (SNC) SNC-Adapter application {#snc_test}
==================================

## Overview

This sample application utilizes SNC-adapter to configure and use the SNC module.

More specifically, in this startup application, the developer will have the chance to become familiar with the following: 

* Microcode (uCode) block development. Explore the SeNIS language and get started with microcode development 
* Sensor Node Low Level Peripheral driver API. Using the I2C, SPI, Timers through the Sensor Node Controller
* Microcode (uCode) block registration/initialization. Initialize/Create a uCode block. Associate a uCode block with a PDC event as an execution trigger.
* SNC queues mechanism usage. SNC Queue creation/configuration. Using the SNC queues to send data from CM33 to SNC and vice versa
* Sensor Node debugging methods. Demonstration of the Sensor Node Controller Emulator and Sensor Node Controller Breakpoints

Three separate demo applications have been developed to demonstrate the above functionality:

* Ambient light sensor periodic samples collection
        * A simple uCode block that reads ambient light samples from the I2C BH1750 ambient light sensor in a fixed RTC interval and notifies the CM33 every 16 samples
        * Perform all the needed initializations on the CM33 side (uCode registration/creation, sensor initialization etc.)
        * Create a PDC entry with the RTC clock as the trigger source and associate it with the uCode we developed

* Continuous sampling of an accelerometer sensor
        * A simple uCode block that reads samples from an SPI ADXL362 accelerometer whenever the sensor generates a FIFO full interrupt.
        * Perform all the needed initializations on the CM33 side (uCode registration/creation, sensor initialization, interrupt initializations, etc)
        * Create a PDC entry with the sensor's interrupt pin as a trigger source and associate it with the uCode block we developed.
        * Use the SNC queues API. Create an SNC to CM33 queue to be used by the SNC uCode block in order to send accelerometer samples to CM33.

* EEPROM write/read demo application
        * A simple uCode block that writes an I2C EEPROM's pages and notifies CM33 every time it writes an EEPROM page
        * Use the SNC queues API. Create a CM33 to SNC queue in order to send pages to the uCode block that writes the EEPROM.
        * Perform all the needed initializations on the CM33 side(uCode registration/creation, SNC queue configuration
        * Create a PDC entry with a Software Trigger and associate it with the EEPROM Writer uCode
        * Development of a simple uCode block that reads back the written pages in a fixed RTC interval
        * Use the SNC queues API. 
                * Create a CM33 to SNC queue. This queue will be used by CM33 to send page addresses to the EEPROM reader uCode block which in turn will read them.
                * Create an SNC to CM33 queue. This queue will be used by the SNC uCode block in order to send the read page data back to CM33
        * Perform all the needed initializations on the CM33 side (uCode registration/creation/ SNC queues configuration)
        * Create a PDC entry with the RTC clock as the trigger source and associate it with the EEPROM reader uCode block

## Installation procedure

The project is located in the \b `projects\dk_apps\features\snc_test` folder.

To install the project follow the [General Installation and Debugging Procedure](@ref install_and_debug_procedure).
