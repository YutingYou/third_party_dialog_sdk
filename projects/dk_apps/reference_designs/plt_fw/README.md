Production Line Tool Firmware {#plt_fw}
=======================================

UART configuration
------------------

The application can be controlled using the serial console exposed over UART2.

GPIO pins configuration is as follows:

| DA1469x GPIO pin | DA1468x GPIO pin | Function |
| ---------------- | ---------------- | -------- |
| P0.9             | P1.3             | UART2 TX |
| P0.8             | P2.3             | UART2 RX |

UART settings are as follows:

| Setting     | Value  |
| ----------- | ------ |
|Baudrate     | 115200 |
|Data bits    | 8      |
|Stop bits    | 1      |
|Parity       | None   |
|Flow control | None   |

This configuration can be changed, by modifying plt_fw.bin binary at specific offsets:

| Offset | Description           | Format       |
| ------ | --------------------- | ------------ |
| 0x0200 | TX port number        | (4 bytes LE) |
| 0x0204 | TX pin number         | (4 bytes LE) |
| 0x0208 | RX port number        | (4 bytes LE) |
| 0x020C | RX pin number         | (4 bytes LE) |
| 0x0210 | UART's baudrate value | (4 bytes LE) |

> _Note_: By default, all values are set to 0xFFFFFFFF in the plt_fw.bin binary.

Bluetooth HCI (Host Controller Interface) protocol background
---------------------------------------
> _Note_: For more details please reference to Bluetooth SIG Core v5.1, Vol 2, Part E, section 5.4 "EXCHANGE OF HCI-SPECIFIC INFORMATION" [ https://www.bluetooth.com/specifications/bluetooth-core-specification/ ]

Host controls the Bluetooth module and monitors its status using HCI commands. The commands are transferred using HCI command packets.
If a command can be executed, it will be and an HCI status event with error code 'no error' will be returned.
If a command can not be executed it will be not, and an HCI status event will be returned with the respective error code.
The format of the HCI Event Packets is similar to the HCI Command Packets. They carry an event code identifying the event.

> _Note_: For details about HCI commands error code please reference to Bluetooth SIG Core v5.1, Vol 2, Part D, section 2 "ERROR CODE DESCRIPTIONS".

- HCI communication flow between Host and Controller

    Module | direction | transport | direction | Module
    :--: | :--: | :--: | :--: | :--:
    [ BT HOST ] | >>>> | [ Transport Layer payload (HCI command packet) ] | >>>> | [ BT CONTROLLER ]
    [ BT HOST ] | <<<< |  [ Transport Layer payload (HCI event packet) ]  | <<<< | [ BT CONTROLLER ]


**HCI command packet**

Each command begins with a 2 byte OpCode which identifies the command type. The OpCode parameter is divided into two fields, called
the OpCode Group Field (OGF) and OpCode Command Field (OCF). The OGF occupies the upper 6 bits of the OpCode, while the OCF occupies the remaining 10 bits.
The OGF of 0x3F is reserved for vendor-specific debug commands.
> _Note_: The OGF composed of all ‘ones’ has been reserved for vendor-specific debug commands. These commands are vendor-specific and are used during manufacturing, for a possible method for updating firmware, and for debugging.

  - HCI command packet as bits map layout

    bits | [0-15] | [16-23] | [24-31] | [32....N]
    :--  |:--    |:--       |:--      |:--
    item | Command OpCode: [ <b>`OCF`</b> (bits: 0-9) <b>`OGF`</b> (bits: 10-15) ] | Parameter Total Length | Command Parameter 0 | ... Command Parameter N

  - Detailed description:

    Packet item          | Description
    :--                  |:--
    <b>`OpCode`</b> (Size: 2 octets) | OGF Range (6bits): 0x00 to 0x3F (0x3F reserved for vendor-specific debug commands; OCF Range (10 bits): 0x0000 to 0x03FF
    <b>`Parameter Total Length`</b> (Size: 1 octet) | Length of all of the parameters contained in this packet measured in octets. (N.B.: total length of parameters, not number of parameters)
    <b>`Command Parameter 0 - N`</b> (Size: Parameter Total Length octets) | Each command has a specific number of parameters associated with it. These parameters and the size of each of the parameters are predefined for each command. Each parameter is an integer number of octets in size.

**HCI event packet**

The HCI Event packets are used by the Controller to notify the Host that an event has occured. If the Controller sends an HCI Event Packet containing an Event Code or an LE subevent code that the Host has not masked out and does not support, the Host shall ignore that packet. The Host shall be able to accept HCI Event packets with up to 255 octets of data excluding the HCI Event packet header.

  - HCI event packet as bits map layout

    bits | [0-7] | [8-15] | [16-31] | [32....N]
    :--  |:--    |:---    |:---     |:---
    item | Event Code | Parameter Total Length | Event Parameter 0 | ... Event Parameter N

  - Detailed description:

    Packet item        | Description
    :---               |:---
    <b>`EventCode`</b> (Size: 1 octet) | Each event is assigned a 1-Octet event code used to uniquely identify different types of events. Range: 0x00 to 0xFF (The event code 0xFF is reserved for the event code used for vendor-specific debug events.)
    <b>`Parameter Total Length`</b> (Size: 1 octet) | Length of all of the parameters contained in this packet, measured in octets
    <b>`Event Parameter 0 - N`</b> (Size: Parameter Total Length octets) | Each event has a specific number of parameters associated with it. These parameters and the size of each of the parameter are predefined for each event. Each parameter is an integer number of octets in size.

Commands overview
-----------------
PLT FW application supports the following commands:
- BLE

| OP code | Name                      | Chip support       | Handled by    | Description                                                                             |
| ------- | ------------------------- | ------------------ | ------------- | --------------------------------------------------------------------------------------- |
| 0x201E  | cont_pkt_tx               | DA1468x \n DA1469x | BLE stack     | Start test - generates BLE test reference packets at a fixed interval                   |
| 0x2034  | TX_TEST_ENH               | DA1469x            | BLE stack     | Starts continuous Tx at 1 or 2Mbps                                                      |
| 0x201D  | start_pkt_rx              | DA1468x \n DA1469x | BLE stack     | Start test - receive BLE test reference packets at a fixed interval                     |
| 0x2033  | RX_TEST_ENH               | DA1469x            | BLE stack     | Starts continuous Rx at 1 or 2Mbps                                                      |
| 0x201F  | stoptest                  | DA1468x \n DA1469x | BLE stack     | Stop any BLE test which is in progress                                                  |
| 0x0C03  | reset                     | DA1468x \n DA1469x | BLE stack     | Reset BLE Link Layer                                                                    |
| 0xFC81  | start_pkt_rx_stats        | DA1468x            | BLE adapter   | Start receive test mode.                                                                |
| 0xFC82  | stop_pkt_rx_stats         | DA1468x            | BLE adapter   | Stop the activity of a RF test mode                                                     |
| 0xFC83  | unmodulated OFF / TX / RX | DA1468x            | BLE adapter   | Start/stop transmitting or start reception a continuous wave (unmodulated transmission) |
| 0xFC84  | start_cont_tx             | DA1468x            | BLE adapter   | Start transmit test mode                                                                |
| 0xFC85  | stop_cont_tx              | DA1468x            | BLE adapter   | Stop the activity of a RF test mode                                                     |
| 0xFC90  | pkt_tx_interval           | DA1468x            | BLE adapter   | Start transmit test mode                                                                |
| 0xFC16  | DBG_RX_TEST_STATS_GET     | DA1469x            | BLE stack     | Gets the extra Rx test results                                                          |
| 0xFC17  | DBG_START_CALIBRATION     | DA1469x            | BLE stack     | Starts calibration                                                                      |
| 0xFC18  | DBG_GET_CAL_RESULT        | DA1469x            | BLE stack     | Gets the result of the calibration via a Command Complete event                         |
| 0xFC14  | DBG_TX_TEST_ENH           | DA1469x            | BLE stack     | Starts continuous Tx at 1 or 2Mbps and sends a burst of N packets                       |
| 0xFC15  | DBG_RX_TEST_ENH           | DA1469x            | BLE stack     | Starts continuous Rx at 1 or 2 Mbps (enhanced)                                          |
| 0xFC3B  | DBG_SET_TX_PW             | DA1469x            | BLE stack     | Sets the Tx power                                                                       |
| 0xFC13  | DBG_TEST_SET_TX_PW        | DA1469x            | BLE stack     | Sets the Tx power in Test mode                                                          |
| 0xFC19  | DBG_SET_EVT_RPRT_STAT     | DA1469x            | BLE stack     | Enables / Disables the reporting after each test 'event'                                |

> _Note_: BLE stack supports also other typical BLE HCI commands.

-  non-BLE

| OP code | Name              | Chip support       | Handled by | Description                                  |
| ------- | ----------------- | ------------------ | ---------- | -------------------------------------------- |
| 0xFE01  | hci_cmd_sleep     | DA1468x \n DA1469x | PLT app    | Set platform sleep mode                      |
| 0xFE02  | xtal_trim         | DA1468x \n DA1469x | PLT app    | Manage XTAL calibration                      |
| 0xFE04  | hci_cmd_otp_read  | DA1468x \n DA1469x | PLT app    | Read from OTP memory                         |
| 0xFE05  | hci_cmd_otp_write | DA1468x \n DA1469x | PLT app    | Write to OTP memory                          |
| 0xFE06  | hci_cmd_rw_reg    | DA1468x \n DA1469x | PLT app    | Read/write register                          |
| 0xFE08  | fw_version_get    | DA1468x \n DA1469x | PLT app    | Get PLT FW version                           |
| 0xFE0A  | hci_custom_action | DA1468x \n DA1469x | PLT app    | Echo back the custom action                  |
| 0xFE0B  | hci_read_adc      | DA1468x \n DA1469x | PLT app    | Read ADC value                               |
| 0xFE0C  | hci_sensor_test   | DA1468x \n DA1469x | PLT app    | Perform sensor test                          |
| 0xFE0D  | hci_gpio_set      | DA1468x \n DA1469x | PLT app    | Set GPIO pin state                           |
| 0xFE0E  | hci_gpio_read     | DA1468x \n DA1469x | PLT app    | Read GPIO pin state                          |
| 0xFE0F  | hci_uart_loop     | DA1468x \n DA1469x | PLT app    | Echo received message                        |
| 0xFE10  | hci_uart_baud     | DA1468x \n DA1469x | PLT app    | Change communication UART's baudrate         |
| 0xFE16  | hci_ext32khz_test | DA1468x \n DA1469x | PLT app    | Check that external 32Khz clock is connected |
| 0xFE17  | hci_gpio_wd       | DA1468x \n DA1469x | PLT app    | Enable external watchdog notifying           |

Commands mapped to PLT FW app's function handler (no-BLE)
---------------------------------------------------------

### hci_cmd_sleep
Set platform sleep mode: active, extended sleep, hibernation (DA1468x) or deep sleep (DA1469x). \n
Command handler: `hci_cmd_sleep()` \n
Platform: DA1468x / DA1469x \n
Handled by: PLT app

#### Command Format
| Byte Description   | Value                                                                                        |
| ------------------ | -------------------------------------------------------------------------------------------- |
| HCI Command Packet | 0x01                                                                                         |
| Command Opcode LSB | 0x01                                                                                         |
| Command Opcode MSB | 0xFE                                                                                         |
| Parameter Length   | 0x03                                                                                         |
| Sleep mode         | 0x00: active mode, 0x01: extended sleep, 0x02: hibernation (DA1468x) or deep sleep (DA1469x) |
| Sleep time (min)   | 0x00 - 0xFF [minutes]                                                                        |
| Sleep time (sec)   | 0x00 - 0xFF [seconds]                                                                        |

#### Return Message
| Byte Description        | Value                                                                                                                                                                    |
| ----------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| HCI Event Packet        | 0x04                                                                                                                                                                     |
| Event Code              | 0x0F                                                                                                                                                                     |
| Parameter Length        | 0x04                                                                                                                                                                     |
| Status                  | 0x00: Command succeeded. \n 0x01 – 0xFF: Command failed. \n See Volume 2, Part D -Error Codes in Bluetooth 4.0 specification for a list of error codes and descriptions. |
| Num_HCI_Command_Packets | 0x01                                                                                                                                                                     |
| Command_Opcode LSB      | 0x01                                                                                                                                                                     |
| Command_Opcode MSB      | 0xFE                                                                                                                                                                     |

> Note: If sleep time is 0 then platform will perform sleep without future wakeup. DA1468x will not wake up after hibernation - platform reboot is required.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### xtrim
Manage XTAL calibration: write/increase/decrese trim value, enable/disable XTAL output on GPIO pin, perform auto calibration test. \n
Command handler: `xtal_trim()` \n
Platform: DA1468x / DA1469x \n

#### Command Format
| Byte Description        | Value                                                                                                                                                                                                                           | Notes                                                                                                                                     |
| ----------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------- |
| HCI Command Packet      | 0x01                                                                                                                                                                                                                            |                                                                                                                                           |
| Command Opcode LSB      | 0x02                                                                                                                                                                                                                            |                                                                                                                                           |
| Command Opcode MSB      | 0xFE                                                                                                                                                                                                                            |                                                                                                                                           |
| Parameter Length        | 0x03                                                                                                                                                                                                                            |                                                                                                                                           |
| Operation               | 0x00: read trim val \n 0x01: write trim val \n 0x02: enable output xtal on P1_2 \n 0x03: increase trim value by delta \n 0x04: decrease trim value by delta \n 0x05: disable XTAL output on P1_2 \n 0x06: auto calibration test |                                                                                                                                           |
| Trim value or delta LSB | 0x00-0xFF                                                                                                                                                                                                                       | trim value LSB when operation=1 \n delta value LSB when operation=3,4 \n GPIO when operation = 6 \n 0x00 otherwise.                       |
| Trim value or delta MSB | 0x00-0xFF                                                                                                                                                                                                                       | trim value MSB when operation=1 \n delta value MSB when operation=3,4 \n 0=16MHz, 1=32MHz when DA1468x and operation=6 \n 0x00 otherwise. |

#### Return Message
| Byte Description        | Value | Notes                                                            |
| ----------------------- | ----- | ---------------------------------------------------------------- |
| HCI Event Packet        | 0x04  |                                                                  |
| Event Code              | 0x0E  |                                                                  |
| Parameter Length        | 0x05  |                                                                  |
| Num_HCI_Command_Packets | 0x01  |                                                                  |
| Command_Opcode LSB      | 0x02  |                                                                  |
| Command_Opcode MSB      | 0xFE  |                                                                  |
| Trim value LSB          | 0xXX  | Trim value for operation=0 status code 2 for operation=6 0x0000. |
| Trim value MSB          | 0xXX  | Trim value for operation=0 status code 2 for operation=6 0x0000. |

> Note: GPIO Px_y is encoded as (`x` << 5) + `y`. E.g. P1_5 is encoded as 37 (0x25 in hex). 5...7 bits store a port number and 0...4 bits store pin number.

> Note: XTAL trim value calibration returns zero on success. A non zero value indicates failure.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### hci_otp_read
Read data from OTP memory. \n
Command handler: `hci_cmd_otp_read()` \n
Platform: DA1468x / DA1469x \n
Handled by: PLT app

#### Command Format
| Byte Description         | Value                                    |
| ------------------------ | ---------------------------------------- |
| HCI Command Packet       | 0x01                                     |
| Command Opcode LSB       | 0x04                                     |
| Command Opcode MSB       | 0xFE                                     |
| Parameter Length         | 0x03                                     |
| Start word address (LSB) | The 32-bits word's address               |
| Start word address (MSB) |                                          |
| Words to read            | Number of words to read [min=1 - max=60] |

#### Return Message
| Byte Description        | Value                                     |
| ----------------------- | ------------------------------------------|
| HCI Event Packet        | 0x04                                      |
| Event Code              | 0x0E                                      |
| Parameter Length        | 0x05 + 4 * (n words returned)             |
| Num_HCI_Command_Packets | 0x01                                      |
| Command_Opcode LSB      | 0x04                                      |
| Command_Opcode MSB      | 0xFE                                      |
| Status                  | 0x00 = Succeeded   0xFF = Not Succeeded   |
| Words Returned          | Number of words returned [min=1 - max=60] |
| Word 1 byte 0 (LSB)     | Word number 1                             |
| Word 1 byte 1           |                                           |
| Word 1 byte 2           |                                           |
| Word 1 byte 3           |                                           |
| ...                     |                                           |
| Word n byte 0 (LSB)     | Word number n                             |
| Word n byte 1           |                                           |
| Word n byte 2           |                                           |
| Word n byte 3           |                                           |

> Note: DA1468x OTP memory contains 0x4000 32-bits words. DA1469x OTP memory contains 0x400 32-bits words.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### hci_otp_write
Write data to OTP memory. \n
Command handler: `hci_cmd_otp_write()` \n
Platform: DA1468x / DA1469x \n
Handled by: PLT app

#### Command Format
| Byte Description         | Value                                                 |
| ------------------------ | ----------------------------------------------------- |
| HCI Command Packet       | 0x01                                                  |
| Command Opcode LSB       | 0x05                                                  |
| Command Opcode MSB       | 0xFE                                                  |
| Parameter Length         | 0x03 + 4 * (n word to write)                          |
| Start word address (LSB) | The 32-bits word's address (must be even for DA1468x) |
| Start word address (MSB) |                                                       |
| Words to write           | Number of words to write [min=1 - max=60]             |
| Word 1 byte 0 (LSB)      | Word number 1                                         |
| Word 1 byte 1            |                                                       |
| Word 1 byte 2            |                                                       |
| Word 1 byte 3            |                                                       |
| ...                      |                                                       |
| Word n byte 0 (LSB)      | Word number n                                         |
| Word n byte 1            |                                                       |
| Word n byte 2            |                                                       |
| Word n byte 3            |                                                       |

#### Return Message
| Byte Description        | Value                                   |
| ----------------------- | ----------------------------------------|
| HCI Event Packet        | 0x04                                    |
| Event Code              | 0x0E                                    |
| Parameter Length        | 0x05                                    |
| Num_HCI_Command_Packets | 0x01                                    |
| Command_Opcode LSB      | 0x05                                    |
| Command_Opcode MSB      | 0xFE                                    |
| Status                  | 0x00 = Succeeded   0xFF = Not Succeeded |
| Words Written           | Number of words written [max=60]        |

> Note: DA1468x OTP memory contains 0x4000 32-bits words. DA1469x OTP memory contains 0x400 32-bits words.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### hci_cmd_read_write_reg
Write/read 16-bits or 32-bits register. \n
Command handler: `hci_cmd_rw_reg()` \n
Platform: DA1468x / DA1469x \n
Handled by: PLT app

#### Command
| Byte Description   | Value                                                                |
| ------------------ | -------------------------------------------------------------------- |
| HCI Command Packet | 0x01                                                                 |
| Command Opcode LSB | 0x06                                                                 |
| Command Opcode MSB | 0xFE                                                                 |
| Parameter Length   | 0x09                                                                 |
| Operation          | 0x00=read_reg32, 0x01=write_reg32, 0x02=read_reg16, 0x03=write_reg16 |
| Address[0] (LSB)   | register address byte 0                                              |
| Address[1]         | register address byte 1                                              |
| Address[2]         | register address byte 2                                              |
| Address[3]         | register address byte 3                                              |
| Data[0] (LSB)      | data byte 0 (16/32 bit mode)                                         |
| Data[1]            | data byte 1 (16/32 bit mode)                                         |
| Data[2]            | data byte 2 (32 bit mode)                                            |
| Data[3]            | data byte 3 (32 bit mode)                                            |

#### Return message
| Byte Description        | Value                                                                |
| ----------------------- | -------------------------------------------------------------------- |
| HCI Event Packet        | 0x04                                                                 |
| Event Code              | 0x0E                                                                 |
| Parameter Length        | 0x09                                                                 |
| Num_HCI_Command_Packets | 0x01                                                                 |
| Command_Opcode LSB      | 0x06                                                                 |
| Command_Opcode MSB      | 0xFE                                                                 |
| Operation               | 0x00=read_reg32, 0x01=write_reg32, 0x02=read_reg16, 0x03=write_reg16 |
| Status                  | 0x00=Succeeded 0xFF=Not Succeeded                                    |
| Data[0] (LSB)           | data byte 0 (16/32 bit mode)                                         |
| Data[1]                 | data byte 1 (16/32 bit mode)                                         |
| Data[2]                 | data byte 2 (32 bit mode)                                            |
| Data[3]                 | data byte 3 (32 bit mode)                                            |
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### hci_firmware_version_get
Get the Bluetooth controller version and PLT FW version strings. \n
Command handler: `fw_version_get()` \n
Platform: DA1468x / DA1469x \n
Handled by: PLT app

#### Command Format
| Byte Description   | Value |
| ------------------ | ----- |
| HCI Command Packet | 0x01  |
| Command Opcode LSB | 0x08  |
| Command Opcode MSB | 0xFE  |
| Parameter Length   | 0x00  |

#### Return Message
| Byte Description                            | Value                                                             |
| ------------------------------------------- | ----------------------------------------------------------------- |
| HCI Event Packet                            | 0x04                                                              |
| Event Code                                  | 0x0E                                                              |
| Parameter Length                            | 0x45                                                              |
| Num_HCI_Command_Packets                     | 0x01                                                              |
| Command_Opcode LSB                          | 0x08                                                              |
| Command_Opcode MSB                          | 0xFE                                                              |
| BLE_version_length                          | 0xXX (Max value 32).                                              |
| Application_version_length  (Max value 32)  | 0xXX (Max value 32).                                              |
| BLE_common firmware_version (32 bytes)      | 32 bytes string  containing the BLE common firmware version.      |
| BLE_application_firmware_version (32 bytes) | 32 bytes string  containing the BLE application firmware version. |
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### hci_custom_action
Perform a custom action - echo one byte from command payload. \n
Command handler: `hci_custom_action()` \n
Platform: DA1468x / DA1469x \n

#### Command Format
| Byte Description   | Value |
| ------------------ | ----- |
| HCI Command Packet | 0x01  |
| Command Opcode LSB | 0x0A  |
| Command Opcode MSB | 0xFE  |
| Parameter Length   | 0x01  |
| Custom action      | 0xXX  |

#### Return Message
| Byte Description        | Value                                             |
| ----------------------- | ------------------------------------------------- |
| HCI Event Packet        | 0x04                                              |
| Event Code              | 0x0E                                              |
| Parameter Length        | 0x04                                              |
| Num_HCI_Command_Packets | 0x01                                              |
| Command_Opcode LSB      | 0x0A                                              |
| Command_Opcode MSB      | 0xFE                                              |
| Return Data             | 0xXX (echos back the Custom action byte received) |
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### hci_read_adc
Read value from analog input (battery voltage). \n
Command handler: `hci_read_adc()` \n
Platform: DA1468x / DA1469x \n

#### Command Format
| Byte Description   | Value |
| ------------------ | ----- |
| HCI Command Packet | 0x01  |
| Command Opcode LSB | 0x0B  |
| Command Opcode MSB | 0xFE  |
| Parameter Length   | 0x00  |

#### Return Message
| Byte Description        | Val                     |
| ----------------------- | ----------------------- |
| HCI Event Packet        | 0x04                    |
| Event Code              | 0x0E                    |
| Parameter Length        | 0x05                    |
| Num_HCI_Command_Packets | 0x01                    |
| Command_Opcode LSB      | 0x0B                    |
| Command_Opcode MSB      | 0xFE                    |
| Result (LSB)            | GP_ADC_RESULT_REG (LSB) |
| Result (MSB)            | GP_ADC_RESULT_REG (MSB) |
- - - - - - - - - - - - - - - - - - - - - - - - - - -

### hci_sensor_test
Perform a write or read using master I2C or SPI interface. \n
Command handler: `hci_sensor_test()` \n
Platform: DA1468x / DA1469x \n

#### Command Format
| Byte Description             | Value                                                                                                           |
| ---------------------------- | --------------------------------------------------------------------------------------------------------------- |
| HCI Command Packet           | 0x01                                                                                                            |
| Command Opcode LSB           | 0x0C                                                                                                            |
| Command Opcode MSB           | 0xFE                                                                                                            |
| Parameter Length             | 0x11                                                                                                            |
| Iface                        | 0=SPI, 1=I2C                                                                                                    |
| Read/Write                   | 0=Read, 1=Write                                                                                                 |
| spi_clk_port or i2c_scl_port | P0=0, P1=1, …                                                                                                   |
| spi_clk_pin or i2c_scl_pin   | Px_0=0, Px_1=1, …                                                                                               |
| spi_di_pin or i2c_sda_pin    | P0=0, P1=1, …                                                                                                   |
| spi_di_pin or i2c_sda_pin    | Px_0=0, Px_1=1, …                                                                                               |
| spi_do_port                  | P0=0, P1=1, …                                                                                                   |
| spi_do_pin                   | Px_0=0, Px_1=1, …                                                                                               |
| spi_cs_port                  | P0=0, P1=1, …                                                                                                   |
| spi_cs_pin                   | Px_0=0, Px_1=1, …                                                                                               |
| Register address             | A sensor register address                                                                                       |
| Register data to write       | Data to write to the sensor register if Read/Write=1                                                            |
| I2C slave address            | The sensor I2C slave address used if Iface=1                                                                    |
| int_gpio_check               | 0=Do nothing. 1=Set the following GPIO to input pull-down after the interface (SPI or I2C) has been initialized |
| int_port                     | P0=0, P1=1, …                                                                                                   |
| int_pin                      | Px_0=0, Px_1=1, …                                                                                               |
| Pins voltage level           | 0=3.3V, 1=1.8V                                                                                                  |

#### Return Message
| Byte Description                       | Value                                                                                                                                                                  |
| -------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| HCI Event Packet                       | 0x04                                                                                                                                                                   |
| Event Code                             | 0x0E                                                                                                                                                                   |
| Parameter Length                       | 0x04                                                                                                                                                                   |
| Num_HCI_Command_Packets                | 0x01                                                                                                                                                                   |
| Command_Opcode LSB                     | 0x0C                                                                                                                                                                   |
| Command_Opcode MSB                     | 0xFE                                                                                                                                                                   |
| Sensor register data or INT GPIO level | 0xXX. Byte read from address specified in byte “Register address” shown in the command format above, or the INT GPIO level (high=0x01 or low=0x00) if int_gpio_check=1 |
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### hci_gpio_set
Configure GPIO pin. \n
Command handler: `hci_gpio_set()` \n
Platform: DA1468x / DA1469x \n

#### Command Format
| Byte Description   | Value                                                                                                                                       |
| ------------------ | ------------------------------------------------------------------------------------------------------------------------------------------- |
| HCI Command Packet | 0x01                                                                                                                                        |
| Command Opcode LSB | 0x0D                                                                                                                                        |
| Command Opcode MSB | 0xFE                                                                                                                                        |
| Parameter Length   | 0x06                                                                                                                                        |
| GPIO               | An enumeration of all available GPIOs. GPIO Px_y is encoded as (`x` << 5) + `y`. 5…7 bits store a port number and 0…4 bits store pin number |
| Mode               | 0=Input, 1=Input pullup, 2=Input pulldown, 3=Output, 4=Output Push Pull, 5= Output Open Drain                                               |
| Voltage level      | 0=3.3V, 1=1.8V                                                                                                                              |
| Reset/Set          | 0=Reset, 1=Set. Valid in output modes. If PWM>0 then Reset=PWM stop. Set=PWM start                                                          |
| PWM                | 0=No PWM, Other=PWM frequency (multiply of 1kHz)                                                                                            |
| PWM duty cycle     | 0x00 - 0x64 (in percentages)                                                                                                                |

#### Return Message
| Byte Description        | Value                   |
| ----------------------- | ----------------------- |
| Byte Description        | Value                   |
| HCI Event Packet        | 0x04                    |
| Event Code              | 0x0E                    |
| Parameter Length        | 0x04                    |
| Num_HCI_Command_Packets | 0x01                    |
| Command_Opcode LSB      | 0x0D                    |
| Command_Opcode MSB      | 0xFE                    |
| Status                  | 0=Succeeded, 0xFF=Error |

> Note: DA1468x support 3 LED drivers which can be used as GPIO: 0xF1->LED1, 0xF2->LED2, 0xF3->LED3. DA1469x support 2 LED drivers which can be used as GPIO: 0xF1->LED1, 0xF2->LED2.

> Note: If the GPIO pin is used by UART then the error is returned.

> Note: If GPIO is a JTAG pin, then debugger is disabled.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### hci_gpio_read
Get GPIO pin state (low/high). \n
Command handler: `hci_gpio_read()` \n
Platform: DA1468x / DA1469x \n

#### Command Format
| Byte Description   | Value                                                                                                                                        |
| ------------------ | -------------------------------------------------------------------------------------------------------------------------------------------- |
| HCI Command Packet | 0x01                                                                                                                                         |
| Command Opcode LSB | 0x0E                                                                                                                                         |
| Command Opcode MSB | 0xFE                                                                                                                                         |
| Parameter Length   | 0x01                                                                                                                                         |
| GPIO               | An enumeration of all available GPIOs. GPIO Px_y is encoded as (`x` << 5) + `y`. 5…7 bits store a port number and 0…4 bits store pin number. |

#### Return Message
| Byte Description        | Value         |
| ----------------------- | ------------- |
| HCI Event Packet        | 0x04          |
| Event Code              | 0x0E          |
| Parameter Length        | 0x04          |
| Num_HCI_Command_Packets | 0x01          |
| Command_Opcode LSB      | 0x0E          |
| Command_Opcode MSB      | 0xFE          |
| Reset/Set               | 0=Low, 1=High |
- - - - - - - - - - - - - - - - - - - - - -

### hci_uart_loop
Send back received data. \n
Command handler: `hci_uart_loop()` \n
Platform: DA1468x / DA1469x \n

#### Command Format
| Byte Description   | Value                                            |
| ------------------ | ------------------------------------------------ |
| HCI Command Packet | 0x01                                             |
| Command Opcode LSB | 0x0F                                             |
| Command Opcode MSB | 0xFE                                             |
| Parameter Length   | Variable                                         |
| Data[XX]           | Data to be echoed back in UART. Variable length. |

#### Return Message
| Byte Description        | Value                                 |
| ----------------------- | ------------------------------------- |
| HCI Event Packet        | 0x04                                  |
| Event Code              | 0x0E                                  |
| Parameter Length        | 0x04                                  |
| Num_HCI_Command_Packets | 0x01                                  |
| Command_Opcode LSB      | 0x0F                                  |
| Command_Opcode MSB      | 0xFE                                  |
| Data[XX]                | Loop back UART data. Variable length. |
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### hci_uart_baud
Change communication UART baudrate. \n
Command handler: `hci_uart_baud()` \n
Platform: DA1468x / DA1469x \n

#### Command Format
| Byte Description   | Value                                                                                   |
| ------------------ | --------------------------------------------------------------------------------------- |
| HCI Command Packet | 0x01                                                                                    |
| Command Opcode LSB | 0x10                                                                                    |
| Command Opcode MSB | 0xFE                                                                                    |
| Parameter Length   | 0x01                                                                                    |
| Data               | Baud Rate: \n 0 ==> 9600 \n 1 ==> 19200 \n 2 ==> 57600 \n 3 ==> 115200 \n 4 ==> 1000000 |

#### Return Message
| Byte Description        | Value                    |
| ----------------------- | ------------------------ |
| HCI Event Packet        | 0x04                     |
| Event Code              | 0x0E                     |
| Parameter Length        | 0x04                     |
| Num_HCI_Command_Packets | 0x01                     |
| Command_Opcode LSB      | 0x10                     |
| Command_Opcode MSB      | 0xFE                     |
| Status                  | 0x0=Succeeded, 0x1=Error |

> Note: Response message is sent using the previous baudrate.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### hci_ext32khz_test
Check that external 32Khz clock is connected by performing clock calibration. \n
Command handler: `hci_ext32khz_test()` \n
Platform: DA1468x / DA1469x \n
Handled by: PLT app

#### Command Format
| Byte Description   | Value |
| ------------------ | ----- |
| HCI Command Packet | 0x01  |
| Command Opcode LSB | 0x16  |
| Command Opcode MSB | 0xFE  |
| Parameter Length   | 0x00  |

#### Return Message
| Byte Description        | Value                              |
| ----------------------- | ---------------------------------- |
| HCI Event Packet        | 0x04                               |
| Event Code              | 0x0E                               |
| Parameter Length        | 0x04                               |
| Num_HCI_Command_Packets | 0x01                               |
| Command_Opcode LSB      | 0x16                               |
| Command_Opcode MSB      | 0xFE                               |
| Status                  | 0x00=Succeeded, 0xFF=Not Succeeded |
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### hci_gpio_wd
Enable external watchdog notifying. Start square wave on QPIO pin: 15ms high state, 2s low state. \n
Command handler: `hci_gpio_wd()` \n
Platform: DA1468x / DA1469x \n
Handled by: PLT app

#### Command Format
| Byte Description   | Value                                                                                                                                        |
| ------------------ | -------------------------------------------------------------------------------------------------------------------------------------------- |
| HCI Command Packet | 0x01                                                                                                                                         |
| Command Opcode LSB | 0x17                                                                                                                                         |
| Command Opcode MSB | 0xFE                                                                                                                                         |
| Parameter Length   | 0x02                                                                                                                                         |
| gpio_pad           | An enumeration of all available GPIOs. GPIO Px_y is encoded as (`x` << 5) + `y`. 5…7 bits store a port number and 0…4 bits store pin number. |
| gpio_lvl           | 0x00=3.3[V] 0x01=1.8[V]                                                                                                                      |

#### Return Message
| Byte Description        | Value                              |
| ----------------------- | ---------------------------------- |
| HCI Event Packet        | 0x04                               |
| Event Code              | 0x0E                               |
| Parameter Length        | 0x04                               |
| Num_HCI_Command_Packets | 0x01                               |
| Command_Opcode LSB      | 0x17                               |
| Command_Opcode MSB      | 0xFE                               |
| Status                  | 0x00=Succeeded, 0xFF=Not Succeeded |

> Note: The state of the pin is not changed during sleep.
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

Commands NOT mapped to a function handler (BLE)
-----------------------------------------------

### cont_pkt_tx
Start test - generates BLE test reference packets at a fixed interval. \n
Platform: DA1468x / DA1469x \n
Handled by: `BLE stack`

#### Command Format
| Byte Description   | Value                                                                                                                                                                                                                                                                                                                                   |
| ------------------ | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| HCI Command Packet | 0x01                                                                                                                                                                                                                                                                                                                                    |
| Command Opcode LSB | 0x1E                                                                                                                                                                                                                                                                                                                                    |
| Command Opcode MSB | 0x20                                                                                                                                                                                                                                                                                                                                    |
| Parameter Length   | 0x03                                                                                                                                                                                                                                                                                                                                    |
| Frequency          | `= (F – 2402) / 2`, where F ranges from 2402 MHz to 2480 MHz. \n Range: 0x00 – 0x27.                                                                                                                                                                                                                                                    |
| Data Length        | 0x01-0x25: Length in bytes of payload data in each packet                                                                                                                                                                                                                                                                               |
| Payload Type       | 0x00: Pseudo-Random bit sequence 9 \n 0x01: Pattern of alternating bits ‘11110000’ \n 0x02: Pattern of alternating bits ‘10101010’ \n 0x03: Pseudo-Random bit sequence 15 \n 0x04: Pattern of All ‘1’ bits \n 0x05: Pattern of All ‘0’ bits \n 0x06: Pattern of alternating bits ‘00001111’ \n 0x07: Pattern of alternating bits ‘0101’ |

#### Return Message
| Byte Description        | Value                                                                                                                                                                    |
| ----------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| HCI Event Packet        | 0x04                                                                                                                                                                     |
| Event Code              | 0x0E                                                                                                                                                                     |
| Parameter Length        | 0x04                                                                                                                                                                     |
| Num_HCI_Command_Packets | 0x01                                                                                                                                                                     |
| Command_Opcode LSB      | 0x1E                                                                                                                                                                     |
| Command_Opcode MSB      | 0x20                                                                                                                                                                     |
| Status                  | 0x00: command succeeded. \n 0x01 – 0xFF: command failed. \n See Volume 2, Part D -Error Codes in Bluetooth 4.0 specification for a list of error codes and descriptions. |
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### TX_TEST_ENH
Starts continuous Tx at 1 or 2Mbps. \n
Platform: DA1469x \n
Handled by: `BLE stack`

#### Command Format
| Byte Description    | Value                                                                                                                                                                                                                                                                                                                                   |
| ------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| HCI Command Packet  | 0x01                                                                                                                                                                                                                                                                                                                                    |
| Command Opcode LSB  | 0x34                                                                                                                                                                                                                                                                                                                                    |
| Command Opcode MSB  | 0x20                                                                                                                                                                                                                                                                                                                                    |
| Parameter Length    | 0x04                                                                                                                                                                                                                                                                                                                                    |
| TX_Channel          | `= (F – 2402) / 2`, where F ranges from 2402 MHz to 2480 MHz. \n Range: 0x00 – 0x27.                                                                                                                                                                                                                                                    |
| Length_Of_Test_Data | Length in bytes of payload data in each packet. \n 0 is not accepted                                                                                                                                                                                                                                                                    |
| Packet_Payload      | 0x00: Pseudo-Random bit sequence 9 \n 0x01: Pattern of alternating bits ‘11110000’ \n 0x02: Pattern of alternating bits ‘10101010’ \n 0x03: Pseudo-Random bit sequence 15 \n 0x04: Pattern of All ‘1’ bits \n 0x05: Pattern of All ‘0’ bits \n 0x06: Pattern of alternating bits ‘00001111’ \n 0x07: Pattern of alternating bits ‘0101’ |
| PHY                 | 0x01: Transmitter set to use the LE 1M PHY. \n 0x02: Transmitter set to use the LE 2M PHY.                                                                                                                                                                                                                                              |

#### Return Message
| Byte Description        | Value                                                                                                                                                                    |
| ----------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| HCI Event Packet        | 0x04                                                                                                                                                                     |
| Event Code              | 0x0E                                                                                                                                                                     |
| Parameter Length        | 0x04                                                                                                                                                                     |
| Num_HCI_Command_Packets | 0x01                                                                                                                                                                     |
| Command_Opcode LSB      | 0x34                                                                                                                                                                     |
| Command_Opcode MSB      | 0x20                                                                                                                                                                     |
| Status                  | 0x00: command succeeded. \n 0x01 – 0xFF: command failed. \n See Volume 2, Part D -Error Codes in Bluetooth 4.0 specification for a list of error codes and descriptions. |
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### start_pkt_rx
Start test - receive BLE test reference packets at a fixed interval. \n
Platform: DA1468x / DA1469x \n
Handled by: `BLE stack`

#### Command Format
| Byte Description   | Value                                                                                |
| ------------------ | ------------------------------------------------------------------------------------ |
| HCI Command Packet | 0x01                                                                                 |
| Command Opcode LSB | 0x1D                                                                                 |
| Command Opcode MSB | 0x20                                                                                 |
| Parameter Length   | 0x01                                                                                 |
| Frequency          | `= (F – 2402) / 2`, where F ranges from 2402 MHz to 2480 MHz. \n Range: 0x00 – 0x27. |

#### Return Message
| Byte Description        | Value                                                                                                                                                                    |
| ----------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| HCI Event Packet        | 0x04                                                                                                                                                                     |
| Event Code              | 0x0E                                                                                                                                                                     |
| Parameter Length        | 0x04                                                                                                                                                                     |
| Num_HCI_Command_Packets | 0x01                                                                                                                                                                     |
| Command_Opcode LSB      | 0x1D                                                                                                                                                                     |
| Command_Opcode MSB      | 0x20                                                                                                                                                                     |
| Status                  | 0x00: Command succeeded. \n 0x01 – 0xFF: Command failed. \n See Volume 2, Part D -Error Codes in Bluetooth 4.0 specification for a list of error codes and descriptions. |
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### RX_TEST_ENH
Starts continuous Rx at 1 or 2Mbps. \n
Platform: DA1469x \n
Handled by: `BLE stack`

#### Command Format
| Byte Description   | Value                                                                                                                                                                                                                                                                                                                                   |
| ------------------ | ------------------------------------------------------------------------------------ |
| HCI Command Packet | 0x01                                                                                 |
| Command Opcode LSB | 0x33                                                                                 |
| Command Opcode MSB | 0x20                                                                                 |
| Parameter Length   | 0x03                                                                                 |
| RX_Channel         | `= (F – 2402) / 2`, where F ranges from 2402 MHz to 2480 MHz. \n Range: 0x00 – 0x27. |
| PHY                | 0x01: Receiver set to use the LE 1M PHY. \n 0x02: Receiver set to use the LE 2M PHY. |
| Modulation_Index   | This parameter is currently ignored.                                                 |

#### Return Message
| Byte Description        | Value                                                                                                                                                                    |
| ----------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| HCI Event Packet        | 0x04                                                                                                                                                                     |
| Event Code              | 0x0E                                                                                                                                                                     |
| Parameter Length        | 0x04                                                                                                                                                                     |
| Num_HCI_Command_Packets | 0x01                                                                                                                                                                     |
| Command_Opcode LSB      | 0x33                                                                                                                                                                     |
| Command_Opcode MSB      | 0x20                                                                                                                                                                     |
| Status                  | 0x00: command succeeded. \n 0x01 – 0xFF: command failed. \n See Volume 2, Part D -Error Codes in Bluetooth 4.0 specification for a list of error codes and descriptions. |
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### stoptest
Stop any BLE test which is in progress. \n
Platform: DA1468x / DA1469x \n
Handled by: `BLE stack`

#### Command Format
| Byte Description   | Value |
| ------------------ | ----- |
| HCI Command Packet | 0x01  |
| Command Opcode LSB | 0x1F  |
| Command Opcode MSB | 0x20  |
| Parameter Length   | 0x00  |

#### Return Message
| Byte Description                             | Value                                                                                                                                                                  |
| -------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| HCI Event Packet                             | 0x04                                                                                                                                                                   |
| Event Code                                   | 0x0E                                                                                                                                                                   |
| Parameter Length                             | 0x06                                                                                                                                                                   |
| Num_HCI_Command_Packets                      | 0x01                                                                                                                                                                   |
| Command_Opcode LSB                           | 0x1F                                                                                                                                                                   |
| Command_Opcode MSB                           | 0x20                                                                                                                                                                   |
| Status                                       | 0x00: Command succeeded. \n 0x01-0xFF: Command failed. \n See Volume 2, Part D -Error Codes in Bluetooth 4.0 specification for a list of error codes and descriptions. |
| Number of packets received / transmitted LSB | 0xXX                                                                                                                                                                   |
| Number of packets received / transmitted MSB | 0xXX                                                                                                                                                                   |
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### reset
Reset BLE Link Layer. \n
After the reset is completed, the current operational state will be lost. \n
The Controller will enter standby mode. \n
The Controller will automatically revert to the default values for the parameters for which default values are defined in the specification. \n
Platform: DA1468x \ DA1469x \n
Handled by: `BLE stack`

#### Command Format
| Byte Description   | Value |
| ------------------ | ----- |
| HCI Command Packet | 0x01  |
| Command Opcode LSB | 0x03  |
| Command Opcode MSB | 0x0C  |
| Parameter Length   | 0x00  |

#### Return Message
| Byte Description        | Value                                                                                                                                                                                                                  |
| ----------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| HCI Event Packet        | 0x04                                                                                                                                                                                                                   |
| Event Code              | 0x0E                                                                                                                                                                                                                   |
| Parameter Length        | 0x04                                                                                                                                                                                                                   |
| Num_HCI_Command_Packets | 0x01                                                                                                                                                                                                                   |
| Command_Opcode LSB      | 0x03                                                                                                                                                                                                                   |
| Command_Opcode MSB      | 0x0c                                                                                                                                                                                                                   |
| Status                  | 0x00: Reset command succeeded, was received and will be executed. \n 0x01-0xFF: Reset command failed. \n See Volume 2, Part D -Error Codes in Bluetooth 4.0 specification for a list of error codes and descriptions." |
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### start_pkt_rx_stats
Start receive test mode: checks the parameters, set the receive mode parameters, turn on the mode and set the LLM state. \n
Platform: DA1468x \n
Handled by: `BLE adapter`

#### Command Format
| Byte Description   | Value                                                                                |
| ------------------ | ------------------------------------------------------------------------------------ |
| HCI Command Packet | 0x01                                                                                 |
| Command Opcode LSB | 0x81                                                                                 |
| Command Opcode MSB | 0xFC                                                                                 |
| Parameter Length   | 0x01                                                                                 |
| Frequency          | `= (F – 2402) / 2`, where F ranges from 2402 MHz to 2480 MHz. \n Range: 0x00 – 0x27. |

#### Return Message
| Byte Description        | Value                                                                                                                                                                    |
| ----------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| HCI Event Packet        | 0x04                                                                                                                                                                     |
| Event Code              | 0x0E                                                                                                                                                                     |
| Parameter Length        | 0x04                                                                                                                                                                     |
| Num_HCI_Command_Packets | 0x01                                                                                                                                                                     |
| Command_Opcode LSB      | 0x81                                                                                                                                                                     |
| Command_Opcode MSB      | 0xFC                                                                                                                                                                     |
| Status                  | 0x00: Command succeeded. \n 0x01 – 0xFF: Command failed. \n See Volume 2, Part D -Error Codes in Bluetooth 4.0 specification for a list of error codes and descriptions. |
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### stop_pkt_rx_stats
Stop the activity of a RF test mode and flushes all the RX data for this event. \n
Platform: DA1468x \n
Handled by: `BLE adapter`

#### Command Format
| Byte Description   | Value |
| ------------------ | ----- |
| HCI Command Packet | 0x01  |
| Command Opcode LSB | 0x82  |
| Command Opcode MSB | 0xFC  |
| Parameter Length   | 0x00  |

#### Return Message
| Byte Description                                | Value                                                                                                                                                                    |
| ----------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| HCI Event Packet                                | 0x04                                                                                                                                                                     |
| Event Code                                      | 0x0E                                                                                                                                                                     |
| Parameter Length                                | 0x0C                                                                                                                                                                     |
| Num_HCI_Command_Packets                         | 0x01                                                                                                                                                                     |
| Command_Opcode LSB                              | 0x82                                                                                                                                                                     |
| Command_Opcode MSB                              | 0xFC                                                                                                                                                                     |
| Status                                          | 0x00: Command succeeded. \n 0x01 – 0xFF: Command failed. \n See Volume 2, Part D -Error Codes in Bluetooth 4.0 specification for a list of error codes and descriptions. |
| Number of received packets LSB                  | 0xXX                                                                                                                                                                     |
| Number of received packets MSB                  | 0xXX                                                                                                                                                                     |
| Number of received packets with sync errors LSB | 0xXX                                                                                                                                                                     |
| Number of received packets with sync errors MSB | 0xXX                                                                                                                                                                     |
| Number of received packets with CRC errors LSB  | 0xXX                                                                                                                                                                     |
| Number of received packets with CRC errors MSB  | 0xXX                                                                                                                                                                     |
| RSSI LSB                                        | 0xXX                                                                                                                                                                     |
| RSSI MSB                                        | 0xXX                                                                                                                                                                     |
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### unmodulated OFF / TX / RX
Start transmitting (TX), stop transmitting (OFF) or start reception (RX) a continuous wave (unmodulated transmission). \n
Platform: DA1468x \n
Handled by: `BLE adapter`

#### Command Format
| Byte Description   | Value                                                                                |
| ------------------ | ------------------------------------------------------------------------------------ |
| HCI Command Packet | 0x01                                                                                 |
| Command Opcode LSB | 0x83                                                                                 |
| Command Opcode MSB | 0xFC                                                                                 |
| Parameter Length   | 0x02                                                                                 |
| Operation          | 0x4F: OFF \n 0x54: unmodulated TX \n 0x52: unmodulated RX                            |
| Frequency          | `= (F – 2402) / 2`, where F ranges from 2402 MHz to 2480 MHz. \n Range: 0x00 – 0x27. |

#### Return Message
| Byte Description        | Value |
| ----------------------- | ----- |
| HCI Event Packet        | 0x04  |
| Event Code              | 0x0E  |
| Parameter Length        | 0x03  |
| Num_HCI_Command_Packets | 0x01  |
| Command_Opcode LSB      | 0x83  |
| Command_Opcode MSB      | 0xFC  |
- - - - - - - - - - - - - - - - - -

### start_cont_tx
Start transmit test mode: checks the parameters, set the transmit mode parameters, turn on the mode and set the LLM state. \n
Platform: DA1468x \n
Handled by: `BLE adapter`

#### Command Format
| Byte Description   | Value                                                                                                                                                                                                                                                                                                                                   |
| ------------------ | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| HCI Command Packet | 0x01                                                                                                                                                                                                                                                                                                                                    |
| Command Opcode LSB | 0x84                                                                                                                                                                                                                                                                                                                                    |
| Command Opcode MSB | 0xFC                                                                                                                                                                                                                                                                                                                                    |
| Parameter Length   | 0x02                                                                                                                                                                                                                                                                                                                                    |
| Frequency          | `= (F – 2402) / 2`, where F ranges from 2402 MHz to 2480 MHz. \n Range: 0x00 – 0x27.                                                                                                                                                                                                                                                    |
| Payload Type       | 0x00: Pseudo-Random bit sequence 9 \n 0x01: Pattern of alternating bits ‘11110000’ \n 0x02: Pattern of alternating bits ‘10101010’ \n 0x03: Pseudo-Random bit sequence 15 \n 0x04: Pattern of All ‘1’ bits \n 0x05: Pattern of All ‘0’ bits \n 0x06: Pattern of alternating bits ‘00001111’ \n 0x07: Pattern of alternating bits ‘0101’ |

#### Return Message
| Byte Description        | Value                                                                                                                                                                    |
| ----------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| HCI Event Packet        | 0x04                                                                                                                                                                     |
| Event Code              | 0x0E                                                                                                                                                                     |
| Parameter Length        | 0x04                                                                                                                                                                     |
| Num_HCI_Command_Packets | 0x01                                                                                                                                                                     |
| Command_Opcode LSB      | 0x84                                                                                                                                                                     |
| Command_Opcode MSB      | 0xFC                                                                                                                                                                     |
| Status                  | 0x00: Command succeeded. \n 0x01 – 0xFF: Command failed. \n See Volume 2, Part D -Error Codes in Bluetooth 4.0 specification for a list of error codes and descriptions. |
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### stop_cont_tx
Stop the activity of a RF test mode  and flushes all the TX data for this event. \n
Platform: DA1468x \n
Handled by: `BLE adapter`

#### Command Format
| Byte Description   | Value |
| ------------------ | ----- |
| HCI Command Packet | 0x01  |
| Command Opcode LSB | 0x85  |
| Command Opcode MSB | 0xFC  |
| Parameter Length   | 0x00  |

#### Return Message
| Byte Description        | Value                                                                                                                                                                    |
| ----------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| HCI Event Packet        | 0x04                                                                                                                                                                     |
| Event Code              | 0x0E                                                                                                                                                                     |
| Length                  | 0x04                                                                                                                                                                     |
| Num_HCI_Command_Packets | 0x01                                                                                                                                                                     |
| Command_Opcode LSB      | 0x85                                                                                                                                                                     |
| Command_Opcode MSB      | 0xFC                                                                                                                                                                     |
| Status                  | 0x00: Command succeeded. \n 0x01 – 0xFF: Command failed. \n See Volume 2, Part D -Error Codes in Bluetooth 4.0 specification for a list of error codes and descriptions. |
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### pkt_tx_interval
Start transmit test mode: checks the parameters, set the transmit mode parameters, turn on the mode and set the LLM state. \n
Platform: DA1468x \n
Handled by: `BLE adapter`

#### Command Format
| Byte Description                  | Value                                                                                                                                                                                                                                                                                                                                   |
| --------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| HCI Command Packet                | 0x01                                                                                                                                                                                                                                                                                                                                    |
| Command Opcode LSB                | 0x90                                                                                                                                                                                                                                                                                                                                    |
| Command Opcode MSB                | 0xFC                                                                                                                                                                                                                                                                                                                                    |
| Parameter Length                  | 0x09                                                                                                                                                                                                                                                                                                                                    |
| Frequency                         | `= (F – 2402) / 2`, where F ranges from 2402 MHz to 2480 MHz. \n Range: 0x00 – 0x27.                                                                                                                                                                                                                                                    |
| Data Length                       | 0x01-0x25 Length in bytes of payload data in each packet                                                                                                                                                                                                                                                                                |
| Payload Type                      | 0x00: Pseudo-Random bit sequence 9 \n 0x01: Pattern of alternating bits ‘11110000’ \n 0x02: Pattern of alternating bits ‘10101010’ \n 0x03: Pseudo-Random bit sequence 15 \n 0x04: Pattern of All ‘1’ bits \n 0x05: Pattern of All ‘0’ bits \n 0x06: Pattern of alternating bits ‘00001111’ \n 0x07: Pattern of alternating bits ‘0101’ |
| Number of packets to transmit LSB | 0xXX                                                                                                                                                                                                                                                                                                                                    |
| Number of packets to transmit MSB | 0xXX                                                                                                                                                                                                                                                                                                                                    |
| Interval in us byte 0 (LSB)       | 0xXX                                                                                                                                                                                                                                                                                                                                    |
| Interval in us byte 1             | 0xXX                                                                                                                                                                                                                                                                                                                                    |
| Interval in us byte 2             | 0xXX                                                                                                                                                                                                                                                                                                                                    |
| Interval in us byte 3 (MSB)       | 0xXX                                                                                                                                                                                                                                                                                                                                    |

#### Return Message
| Byte Description        | Value                                                                                                                                                                    |
| ----------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| HCI Event Packet        | 0x04                                                                                                                                                                     |
| Event Code              | 0x0F                                                                                                                                                                     |
| Parameter Length        | 0x04                                                                                                                                                                     |
| Status                  | 0x00: Command succeeded. \n 0x01 – 0xFF: Command failed. \n See Volume 2, Part D -Error Codes in Bluetooth 4.0 specification for a list of error codes and descriptions. |
| Num_HCI_Command_Packets | 0x01                                                                                                                                                                     |
| Command_Opcode LSB      | 0x90                                                                                                                                                                     |
| Command_Opcode MSB      | 0xFC                                                                                                                                                                     |

#### Message returned when transmission is completed
| Byte Description        | Value                                                        |
| ----------------------- | ------------------------------------------------------------ |
| HCI Event Packet        | 0x04                                                         |
| Event Code              | 0x0E                                                         |
| Parameter Length        | 0x04                                                         |
| Num_HCI_Command_Packets | 0x01                                                         |
| Command_Opcode LSB      | 0x90                                                         |
| Command_Opcode MSB      | 0xFC                                                         |
| Status                  | 0x00: Command successfully completed \n 0x01: Illegal params |
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### DBG_RX_TEST_STATS_GET
Gets the extra Rx test results. \n
Platform: DA1469x \n
Handled by: `BLE stack`

#### Command Format
| Byte Description   | Value |
| ------------------ | ----- |
| HCI Command Packet | 0x01  |
| Command Opcode LSB | 0x16  |
| Command Opcode MSB | 0xFC  |
| Parameter Length   | 0x00  |

#### Return Message
| Byte Description        | Value                                                    |
| ----------------------- | -------------------------------------------------------- |
| HCI Event Packet        | 0x04                                                     |
| Event Code              | 0x0E                                                     |
| Length                  | 0x0E                                                     |
| Num_HCI_Command_Packets | 0x01                                                     |
| Command_Opcode LSB      | 0x16                                                     |
| Command_Opcode MSB      | 0xFC                                                     |
| Status                  | 0x00: Command succeeded. \n 0x01 – 0xFF: Command failed. |
| RX packets total LSB    | 0xXX                                                     |
| RX packets total MSB    | 0xXX                                                     |
| RX packets SYNCERR LSB  | 0xXX                                                     |
| RX packets SYNCERR MSB  | 0xXX                                                     |
| RX packets LENERR LSB   | 0xXX                                                     |
| RX packets LENERR MSB   | 0xXX                                                     |
| RX packets CRCERR LSB   | 0xXX                                                     |
| RX packets CRCERR MSB   | 0xXX                                                     |
| RSSI LSB                | 0xXX                                                     |
| RSSI MSB                | 0xXX                                                     |
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### DBG_START_CALIBRATION
Starts RF calibration. \n
Platform: DA1469x \n
Handled by: `BLE stack`

#### Command Format
| Byte Description   | Value                                                                                            |
| ------------------ | ------------------------------------------------------------------------------------------------ |
| HCI Command Packet | 0x01                                                                                             |
| Command Opcode LSB | 0x17                                                                                             |
| Command Opcode MSB | 0xFC                                                                                             |
| Parameter Length   | 0x01                                                                                             |
| Calibration type   | 0x01: FULL \n 0x02: DCOFF \n 0x03: IFF \n 0x04: KDCO ADPLL \n 0x06: TX DIV2 \n 0x09: TEMPERATURE |

#### Return Message
| Byte Description        | Value                                                    |
| ----------------------- | -------------------------------------------------------- |
| HCI Event Packet        | 0x04                                                     |
| Event Code              | 0x0E                                                     |
| Length                  | 0x04                                                     |
| Num_HCI_Command_Packets | 0x01                                                     |
| Command_Opcode LSB      | 0x17                                                     |
| Command_Opcode MSB      | 0xFC                                                     |
| Status                  | 0x00: Command succeeded. \n 0x01 – 0xFF: Command failed. |
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### DBG_GET_CAL_RESULT
Gets the result of the calibration via a Command Complete event. \n
The event is returned when the calibration completes. \n
Platform: DA1469x \n
Handled by: `BLE stack`

#### Command Format
| Byte Description   | Value |
| ------------------ | ----- |
| HCI Command Packet | 0x01  |
| Command Opcode LSB | 0x18  |
| Command Opcode MSB | 0xFC  |
| Parameter Length   | 0x00  |

#### Return Message
| Byte Description        | Value                                             |
| ----------------------- | ------------------------------------------------- |
| HCI Event Packet        | 0x04                                              |
| Event Code              | 0x0E                                              |
| Length                  | 0x31                                              |
| Num_HCI_Command_Packets | 0x01                                              |
| Command_Opcode LSB      | 0x18                                              |
| Command_Opcode MSB      | 0xFC                                              |
| Status                  | 0x00: Command succeeded. \n 0x1F: Command failed. |
| DC-offset result        | 0xXX                                              |
| IFF result LSB          | 0xXX                                              |
| IFF result              | 0xXX                                              |
| IFF result              | 0xXX                                              |
| IFF result MSB          | 0xXX                                              |
| KDCO result [0]         | 0xXX                                              |
| KDCO result [1]         | 0xXX                                              |
| KDCO result [2]         | 0xXX                                              |
| KDCO result [3]         | 0xXX                                              |
| KDCO result [4]         | 0xXX                                              |
| KDCO result [5]         | 0xXX                                              |
| KDCO result [6]         | 0xXX                                              |
| KDCO result [7]         | 0xXX                                              |
| KDTC result [0] LSB     | 0xXX                                              |
| KDTC result [0] MSB     | 0xXX                                              |
| KDTC result [1] LSB     | 0xXX                                              |
| KDTC result [1] MSB     | 0xXX                                              |
| KDTC result [2] LSB     | 0xXX                                              |
| KDTC result [2] MSB     | 0xXX                                              |
| KDTC result [3] LSB     | 0xXX                                              |
| KDTC result [3] MSB     | 0xXX                                              |
| KDTC result [4] LSB     | 0xXX                                              |
| KDTC result [4] MSB     | 0xXX                                              |
| KDTC result [5] LSB     | 0xXX                                              |
| KDTC result [5] MSB     | 0xXX                                              |
| KDTC result [6] LSB     | 0xXX                                              |
| KDTC result [6] MSB     | 0xXX                                              |
| KDTC result [7] LSB     | 0xXX                                              |
| KDTC result [7] MSB     | 0xXX                                              |
| Tx-Div2 result [0]      | 0xXX                                              |
| Tx-Div2 result [1]      | 0xXX                                              |
| Tx-Div2 result [2]      | 0xXX                                              |
| Tx-Div2 result [3]      | 0xXX                                              |
| Tx-Div2 result [4]      | 0xXX                                              |
| Tx-Div2 result [5]      | 0xXX                                              |
| Tx-Div2 result [6]      | 0xXX                                              |
| Tx-Div2 result [7]      | 0xXX                                              |
| IQ-Div2 result [0]      | 0xXX                                              |
| IQ-Div2 result [1]      | 0xXX                                              |
| IQ-Div2 result [2]      | 0xXX                                              |
| IQ-Div2 result [3]      | 0xXX                                              |
| IQ-Div2 result [4]      | 0xXX                                              |
| IQ-Div2 result [5]      | 0xXX                                              |
| IQ-Div2 result [6]      | 0xXX                                              |
| IQ-Div2 result [7]      | 0xXX                                              |
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### DBG_TX_TEST_ENH
Starts continuous Tx at 1 or 2 Mbps and sends a burst of N packets. \n
Platform: DA1469x \n
Handled by: `BLE stack`

#### Command Format
| Byte Description   | Value                                                                                                                                                                  |
| ------------------ | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| HCI Command Packet | 0x01                                                                                                                                                                   |
| Command Opcode LSB | 0x14                                                                                                                                                                   |
| Command Opcode MSB | 0xFC                                                                                                                                                                   |
| Parameter Length   | 0x0B                                                                                                                                                                   |
| TX Freq            | As defined in 5.0/Vol.2/Part E/7.8.51                                                                                                                                  |
| Test Data Len      | 0xXX, Length in bytes of payload data in each packet. It must hold N > 0.                                                                                              |
| PK Payload type    | As defined in 5.0/Vol.2/Part E/7.8.51                                                                                                                                  |
| TX Phy             | 0x01: transmitter set to use the LE 1M PHY, 0x02: transmitter set to use the LE 2M PHY                                                                                 |
| Test Type          | 0x01: MODULATED_INFINITE_PACK_TEST, \n 0x02: MODULATED_INFINITE_NONPACK_TEST, \n 0x03: MODULATED_FINITE_PACK_TEST, \n 0x04: UNMODULATED_INFINITE_NONPACK_TEST          |
| TX Packet Num LSB  | 0xXX, Number of packets to be transmitted.                                                                                                                             |
| TX Packet Num MSB  |                                                                                                                                                                        |
| TX Interval us LSB | 0: The packet interval is computed as specified in Core Specification 5.0 volume 6, part F, section 4.1.6, 1 to 149: Forbidden, >=150: The packet IFS in microseconds. |
| TX Interval us     |                                                                                                                                                                        |
| TX Interval us     |                                                                                                                                                                        |
| TX Interval us MSB |                                                                                                                                                                        |

> Note: In MODULATED_INFINITE_PACK_TEST (0x01): TX Packet Num value is ignored.

> Note: In MODULATED_INFINITE_NONPACK_TEST (0x02): TX Packet Num and TX Interval us values are ignored. Test Data Len is not applicable but it must be set to a value > 1.

> Note: In MODULATED_FINITE_PACK_TEST (0x03): if TX Packet Num is 0 then an error is returned.

> Note: In UNMODULATED_INFINITE_NONPACK_TEST (0x04): Test Data Len, PK Payload type, TX Phy, TX Packet Num and TX Interval us values are ignored.

> Note: There is no upper bound for the IFS so we accept any value up to 0xFFFFFFFF. However huge values are of no practical value.

> Note: If Tx Phy = 2, then Test Data Length should be > 1.

#### Return Message
| Byte Description        | Value                                                    |
| ----------------------- | -------------------------------------------------------- |
| HCI Event Packet        | 0x04                                                     |
| Event Code              | 0x0E                                                     |
| Length                  | 0x04                                                     |
| Num_HCI_Command_Packets | 0x01                                                     |
| Command_Opcode LSB      | 0x14                                                     |
| Command_Opcode MSB      | 0xFC                                                     |
| Status                  | 0x00: Command succeeded. \n 0x01 – 0xFF: Command failed. |
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### DBG_RX_TEST_ENH
Starts continuous Rx at 1 or 2 Mbps (enhanced). \n
Platform: DA1469x \n
Handled by: `BLE stack`

#### Command Format
| Byte Description   | Value                                                                                               |
| ------------------ | --------------------------------------------------------------------------------------------------- |
| HCI Command Packet | 0x01                                                                                                |
| Command Opcode LSB | 0x15                                                                                                |
| Command Opcode MSB | 0xFC                                                                                                |
| Parameter Length   | 0x0C                                                                                                |
| RX Freq            | Rx channel. As defined in 5.0/Vol.2/Part E/7.8.50                                                   |
| RX Phy             | 0x01: receiver set to use the LE 1M PHY, 0x02: receiver set to use the LE 2M PHY                    |
| RX Mod             | Modulation index. This parameter is currently ignored.                                              |
| Test Data Len      | Transmitter side data length                                                                        |
| RX Interval us LSB | Transmitter side packet interval.                                                                   |
| RX Interval us     |                                                                                                     |
| RX Interval us     |                                                                                                     |
| RX Interval us MSB |                                                                                                     |
| Normal Winsz LSB   | Rx window size applied at "event interval", Units: microseconds                                     |
| Normal Winsz MSB   |                                                                                                     |
| First Winsz LSB    | Rx window size applied to the 1st reception ("scan" mode), Units: slots (1 slot = 625 microseconds) |
| First Winsz MSB    |                                                                                                     |

> Note: If another test is running or if any parameter is wrong, then an error status is returned.

> Note: If RX Interval us > 0, then the following must hold: First Winsz > 0, 0 < Normal Winsz <= 8171, 0 < Test Data Len <= 251, RX Interval us >= 150, RX Interval us >= Normal Winsz + 100.

> Note: The Normal Winsz is in usec or slots and is 14-bits wide counting up to 8171 usec.

> Note: The actual Rx window will open for 2*Normal Winsz + SYNC_WORD_duration (16382 usec maximum).

> Note: The First Winsz is in 625usec slots and is 14-bits wide.

> Note: The actual Rx window in this case opens for First Winsz duration.

> Note: The RX Interval us must be the same as the TX Interval us that has been passed to the Tx side.

> Note: The report counters are reset whenever an Rx test is started.

> Note: The Test Data Len is mandatory and must match the payload length transimitted by the Tx side.

> Note: In case of a SYNC error, the Rx window will not be increased.

#### Return Message
| Byte Description        | Value                                                    |
| ----------------------- | -------------------------------------------------------- |
| HCI Event Packet        | 0x04                                                     |
| Event Code              | 0x0E                                                     |
| Length                  | 0x04                                                     |
| Num_HCI_Command_Packets | 0x01                                                     |
| Command_Opcode LSB      | 0x15                                                     |
| Command_Opcode MSB      | 0xFC                                                     |
| Status                  | 0x00: Command succeeded. \n 0x01 – 0xFF: Command failed. |
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### DBG_SET_TX_PW
Sets the Tx power. \n
Platform: DA1469x \n
Handled by: `BLE stack`

#### Command Format
| Byte Description   | Value                                                                                   |
| ------------------ | --------------------------------------------------------------------------------------- |
| HCI Command Packet | 0x01                                                                                    |
| Command Opcode LSB | 0x3B                                                                                    |
| Command Opcode MSB | 0xFC                                                                                    |
| Parameter Length   | 0x02                                                                                    |
| AIROP              | Bit field, bits description: 0 - Advertising, 1 - Scanning, 2 - Initiation, 3 - RF test |
| PW Lvl             | 0x00 - 0x11, 0x01: the lowest TX power level. \n 0x11: the maximum TX power level.      |

#### Return Message
| Byte Description        | Value                                                    |
| ----------------------- | -------------------------------------------------------- |
| HCI Event Packet        | 0x04                                                     |
| Event Code              | 0x0E                                                     |
| Length                  | 0x04                                                     |
| Num_HCI_Command_Packets | 0x01                                                     |
| Command_Opcode LSB      | 0x3B                                                     |
| Command_Opcode MSB      | 0xFC                                                     |
| Status                  | 0x00: Command succeeded. \n 0x01 – 0xFF: Command failed. |
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### DBG_TEST_SET_TX_PW
Sets the Tx power in Test mode. \n
Platform: DA1469x \n
Handled by: `BLE stack`

#### Command Format
| Byte Description   | Value                                        |
| ------------------ | -------------------------------------------- |
| HCI Command Packet | 0x01                                         |
| Command Opcode LSB | 0x13                                         |
| Command Opcode MSB | 0xFC                                         |
| Parameter Length   | 0x03                                         |
| Pwr Level          | 0x00 - 0x0F                                  |
| Coarse Atten       | 0x00 - 0x02, valid only if Pwr Level = 0x01  |
| Femonly Fine Atten | 0x00 - 0x06                                  |

#### Return Message
| Byte Description        | Value                                                    |
| ----------------------- | -------------------------------------------------------- |
| HCI Event Packet        | 0x04                                                     |
| Event Code              | 0x0E                                                     |
| Length                  | 0x04                                                     |
| Num_HCI_Command_Packets | 0x01                                                     |
| Command_Opcode LSB      | 0x13                                                     |
| Command_Opcode MSB      | 0xFC                                                     |
| Status                  | 0x00: Command succeeded. \n 0x01 – 0xFF: Command failed. |
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### DBG_SET_EVT_RPRT_STAT
Enables / Disables the reporting after each test 'event'. \n
When Enable is a non-zero value after each test 'event', an HCI Event will be sent via the Mailbox containing information about the current status. \n
Platform: DA1469x \n
Handled by: `BLE stack`

#### Command Format
| Byte Description   | Value                              |
| ------------------ | ---------------------------------- |
| HCI Command Packet | 0x01                               |
| Command Opcode LSB | 0x19                               |
| Command Opcode MSB | 0xFC                               |
| Parameter Length   | 0x01                               |
| Enable             | 0x00: Disable, 0x01 - 0xFF: Enable |

#### Return Message
| Byte Description        | Value                                                    |
| ----------------------- | -------------------------------------------------------- |
| HCI Event Packet        | 0x04                                                     |
| Event Code              | 0x0E                                                     |
| Length                  | 0x04                                                     |
| Num_HCI_Command_Packets | 0x01                                                     |
| Command_Opcode LSB      | 0x19                                                     |
| Command_Opcode MSB      | 0xFC                                                     |
| Status                  | 0x00: Command succeeded. \n 0x01 – 0xFF: Command failed. |
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

