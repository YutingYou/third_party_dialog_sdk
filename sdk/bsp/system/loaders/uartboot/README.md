UART/SWD bootloader application {#uartboot}
===========================================

## Overview

Application features:
- booting to another application and on-demand code execution
- reading and writing
  - RAM
  - QSPI
  - OTP
  - Registers
- QSPI FLASH memory management
- custom protocol for UART or SWD communication

It is used as a default target application by:
- libprogrammer
- cli_programmer
- SmartSnippets Toolbox

## Vocabulary and notes

uartboot protocols use few terms:
- *SOH* - Start of Heading (0x01 value)
- *STX* - Start of Text (0x02 value)
- *ACK* - Acknowledge (0x06 value)
- *NAK* - Negative Acknowledge (0x15 value)
- *CRC* - 16-bits value calculated from data using CRC16-CCITT algorithm

uartboot uses 32-bits addressing.

### Byte order

The data length, CRC, address, bytes in OTP words should be sent using little-endian (LSB-first).

## UART configuration

The application can be controlled using the serial console exposed over UART2.

GPIO pins configuration is as follows:

| DA1469x GPIO pin | DA1468x GPIO pin | Function |
| ---------------- | ---------------- | -------- |
| P0.9             | P1.3             | UART2 TX |
| P0.8             | P2.3             | UART2 RX |

UART settings are as follows:

| Setting     | DA1469x | DA1468x |
| ----------- | ------- | ------- |
|Baudrate     | 115200  | 57600   |
|Data bits    | 8       | 8       |
|Stop bits    | 1       | 1       |
|Parity       | None    | None    |
|Flow control | None    | None    |

 This configuration can be changed, by modifying uartboot.bin binary at specific offsets:

| Offset | Description           | Format       |
| ------ | --------------------- | ------------ |
| 0x0200 | TX port number        | (4 bytes LE) |
| 0x0204 | TX pin number         | (4 bytes LE) |
| 0x0208 | RX port number        | (4 bytes LE) |
| 0x020C | RX pin number         | (4 bytes LE) |
| 0x0210 | UART's baudrate value | (4 bytes LE) |

By default, all values are set to 0xFFFFFFFF.

### UART transmisison flow

A complete flow for command transmission handling (including in/out data) is as follows:

    <= <STX> <SOH> (ver1) (ver2)
    => <SOH>
    => (type) (len1) (len2)
    call HOP_INIT
    <= <ACK> / <NAK>
    if len > 0
         call HOP_HEADER
         => (data...)
         call HOP_DATA
         <= <ACK> / <NAK>
         <= (crc1) (crc2)
         => <ACK> / <NAK>
    call HOP_EXEC
    call HOP_SEND_LEN
    if len > 0
         <= (len1) (len2)
         => <ACK> / <NAK>
         call HOP_SEND_DATA
         <= (data...)
         => (crc1) (crc2)
         <= <ACK> / <NAK>

      '<=' - data sent by uartboot.
      '=>' - data sent to uartboot by host application.

In case, error occures at any step (HOP_*), uartboot will send 'NAK', which shall terminate the transmission.

## SWD (debugger) interface

The application can be also controlled using the SWD interface (debugger commands), this can
be done by performing the following actions:

- find *DBGP* string marker in uartboot binary (it is aligned to 4 bytes) and mark the fields as follows:

| Offset | Field             | Description                                                            | Format                 |
| ------ | ----------------- | ---------------------------------------------------------------------- | ---------------------- |
| 0x00   | *DBGP*            | `DBGP` bytes                                                           | (4 bytes)              |
| 0x04   | *RUN_SWD*         | SWD interface usage marker (0: SWD is not used, other: SWD if is used) | (4 bytes LE)           |
| 0x08   | *NUM_ADDRESS*     | Current index of the command                                           | (4 bytes LE)           |
| 0x0C   | *CMD_BUF_ADDRESS* | Current command buffer address                                         | (4 bytes LE / pointer) |
| 0x10   | *BUF_ADDRESS*     | Input/output payload buffer address                                    | (4 bytes LE / pointer) |
| 0x14   | *ACK_NAK_ADDRESS* | Input/output *ACK/NAK* value                                           | (4 bytes LE)           |

- write '0x01' to *RUN_SWD* field in the application binary
- load application binary to the device RAM memory and execute it - application should start in SWD mode and stop at the breakpoint
- write input data at *BUF_ADDRESS* (if needed)
- write command header at *CMD_BUF_ADDRESS*
- increase value at *NUM_ADDRESS*
- set field at *ACK_NAK_ADDRESS* to NAK value
- continue the application (leave the breakpoint)
- uartboot executes given command then stops at the breakpoint again.
- read field at *ACK_NAK_ADDRESS* - if command executed correctly it will contain ACK value
- read output data from buffer at *CMD_BUF_ADDRESS* (if needed)

Note: Detaching the debugger from board will cause a hardfault and it will be not possible to communicate with uartboot anymore until application restart.

## Commands overwiev

uartboot application supports the following commands:

| Command | Name                 | Description                               |
| ------- | -------------------- | ----------------------------------------- |
| 0x01    | Write                | Write data to RAM                         |
| 0x02    | Read                 | Read data from RAM                        |
| 0x03    | Copy to QSPI         | Copy data from RAM to QSPI memory         |
| 0x04    | Erase QSPI           | Erase a part of the QSPI memory           |
| 0x05    | Run                  | Execute a part of code                    |
| 0x06    | Write OTP            | Write data to OTP memory                  |
| 0x07    | Read OTP             | Read data from OTP memory                 |
| 0x08    | Read QSPI            | Read data from QSPI memory                |
| 0x09    | Customer specific    | Perform customer specific action          |
| 0x0A    | Read partition table | Read partition table from QSPI memory     |
| 0x0B    | Get version          | Get uartboot's version                    |
| 0x0C    | Chip erase QSPI      | Erase whole QSPI FLASH memory             |
| 0x0D    | Is empty QSPI        | Check that a part of QSPI memory is empty |
| 0x0E    | Read partition       | Read data from QSPI FLASH partion         |
| 0x0F    | Write partition      | Write data to QSPI FLASH partion          |
| 0x10    | Get QSPI state       | Get QSPI controller state                 |
| 0x11    | GPIO WD              | Enable external watchdog notifying        |
| 0x12    | QSPI direct write    | Write data directly to QSPI memory        |
| 0x30    | Change baudrate      | Change communication UART's baudrate      |
| 0xFF    | Dummy                | Write 'Live' marker to data buffer        |

## Supported commands

Supported commands' description.

### Write
Write data to RAM memory.

#### Command Format

| Byte Description        | Value           |
| ----------------------- | --------------- |
| SOH                     | 0x01            |
| Command Opcode          | 0x01            |
| Length LSB              | (0x04 + N)      |
| Length MSB              | (0x04 + N) >> 8 |
| Destination address LSB | 0xXX            |
| Destination address     | 0xXX            |
| Destination address     | 0xXX            |
| Destination address MSB | 0xXX            |
| Data 0                  | Data byte 0     |
| …                       | …               |
| Data n                  | Data byte N     |

Note: SWD interface is able to write directly into RAM memory, which is
      suggested way for performing this operation.
      This command can be also used for writing values to registers.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### Read
Read data from RAM memory.

#### Command Format

| Byte Description   | Value |
| ------------------ | ----- |
| SOH                | 0x01  |
| Command Opcode     | 0x02  |
| Length LSB         | 0x06  |
| Length MSB         | 0x00  |
| Source address LSB | 0xXX  |
| Source address     | 0xXX  |
| Source address     | 0xXX  |
| Source address MSB | 0xXX  |
| Data length LSB    | 0xXX  |
| Data length MSB    | 0xXX  |

#### Return Message

| Byte Description | Value       |
| ---------------- | ----------- |
| Data 0           | Data byte 0 |
| …                | …           |
| Data n           | Data byte N |

Note: SWD interface is able to read directly from RAM memory, which is
      suggested way for performing this operation.
      This command can be also used for reading values from registers.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### Copy to QSPI
Copy data from RAM to QSPI FLASH memory.

#### Command Format

| Byte Description        | Value |
| ----------------------- | ----- |
| SOH                     | 0x01  |
| Command Opcode          | 0x03  |
| Length LSB              | 0x0A  |
| Length MSB              | 0x00  |
| Source address LSB      | 0xXX  |
| Source address          | 0xXX  |
| Source address          | 0xXX  |
| Source address MSB      | 0xXX  |
| Data length LSB         | 0xXX  |
| Data length MSB         | 0xXX  |
| Destination address LSB | 0xXX  |
| Destination address     | 0xXX  |
| Destination address     | 0xXX  |
| Destination address MSB | 0xXX  |

Note: Data stored in the same FLASH sector will be erased if the write
      cannot be performed directly.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### Erase QSPI
Erase a part of the QSPI FLASH memory.

#### Command Format

| Byte Description | Value |
| ---------------- | ----- |
| SOH              | 0x01  |
| Command Opcode   | 0x04  |
| Length LSB       | 0x08  |
| Length MSB       | 0x00  |
| QSPI address LSB | 0xXX  |
| QSPI address     | 0xXX  |
| QSPI address     | 0xXX  |
| QSPI address MSB | 0xXX  |
| Size LSB         | 0xXX  |
| Size             | 0xXX  |
| Size             | 0xXX  |
| Size MSB         | 0xXX  |

Note: All QSPI FLASH sectors which include given area (specified by
      address + size) will be erased (erase operation is sector aligned).

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### Run
Execute a part of the code.

#### Command Format

| Byte Description   | Value |
| ------------------ | ----- |
| SOH                | 0x01  |
| Command Opcode     | 0x05  |
| Length LSB         | 0x04  |
| Length MSB         | 0x00  |
| Memory address LSB | 0xXX  |
| Memory address     | 0xXX  |
| Memory address     | 0xXX  |
| Memory address MSB | 0xXX  |

Note: Code can be executed from different memories e.g. RAM, QSPI, OTP, ROM.
      If the memory address is the same as the input buffer then uartboot
      will copy all data from input buffer at RAM 0 address and perform
      the SW Reset. This feature can be used for booting application from
      RAM which are too big for handling by ROM bootlaoder.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### Write OTP
Write data to the OTP memory.

#### Command Format

| Byte Description    | Value               |
| ------------------- | ------------------- |
| SOH                 | 0x01                |
| Command Opcode      | 0x06                |
| Length LSB          | (0x04 + N * 4)      |
| Length MSB          | (0x04 + N * 4) >> 8 |
| OTP cell offset LSB | 0xXX                |
| OTP cell offset     | 0xXX                |
| OTP cell offset     | 0xXX                |
| OTP cell offset MSB | 0xXX                |
| Word 0 LSB          | 0xXX                |
| Word 0              | 0xXX                |
| Word 0              | 0xXX                |
| Word 0 MSB          | 0xXX                |
| …                   | …                   |
| Word n LSB          | 0xXX                |
| Word n              | 0xXX                |
| Word n              | 0xXX                |
| Word n MSB          | 0xXX                |

Note: Cell size is 64-bits for DA1468x and 32-bits for DA1469x.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### Read OTP
Read data from the OTP memory.

#### Command Format

| Byte Description    | Value |
| ------------------- | ----- |
| SOH                 | 0x01  |
| Command Opcode      | 0x07  |
| Length LSB          | 0x06  |
| Length MSB          | 0x00  |
| OTP cell offset LSB | 0xXX  |
| OTP cell offset     | 0xXX  |
| OTP cell offset     | 0xXX  |
| OTP cell offset MSB | 0xXX  |
| Data length LSB     | 0xXX  |
| Data length MSB     | 0xXX  |

#### Return Message

| Byte Description | Value |
| ---------------- | ----- |
| Word 0 LSB       | 0xXX  |
| Word 0           | 0xXX  |
| Word 0           | 0xXX  |
| Word 0 MSB       | 0xXX  |
| …                | …     |
| Word n LSB       | 0xXX  |
| Word n           | 0xXX  |
| Word n           | 0xXX  |
| Word n MSB       | 0xXX  |

Note: Cell size is 64-bits for DA1468x and 32-bits for DA1469x.
      Data length is a number of 32-bits words.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### Read QSPI
Read data from QSPI memory.

#### Command Format

| Byte Description   | Value |
| ------------------ | ----- |
| SOH                | 0x01  |
| Command Opcode     | 0x08  |
| Length LSB         | 0x06  |
| Length MSB         | 0x00  |
| Source address LSB | 0xXX  |
| Source address     | 0xXX  |
| Source address     | 0xXX  |
| Source address MSB | 0xXX  |
| Data length LSB    | 0xXX  |
| Data length MSB    | 0xXX  |

#### Return Message

| Byte Description | Value       |
| ---------------- | ----------- |
| Data 0           | Data byte 0 |
| …                | …           |
| Data n           | Data byte N |

- - - - - - - - - - - - - - - - - -

### Customer specific
Perform customer specific action.

#### Command Format

| Byte Description | Value |
| ---------------- | ----- |
| SOH              | 0x01  |
| Command Opcode   | 0x09  |
| Length LSB       | 0x00  |
| Length MSB       | 0x00  |

Note: Behavior and input/output messages are implemented by the customer.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### Read partition table
Read partition table from QSPI memory.

#### Command Format

| Byte Description | Value |
| ---------------- | ----- |
| SOH              | 0x01  |
| Command Opcode   | 0x0A  |
| Length LSB       | 0x00  |
| Length MSB       | 0x00  |

#### Return Message

| Byte Description            | Value |
| --------------------------- | ----- |
| Length LSB                  | 0xXX  |
| Length MSB                  | 0xXX  |
| Sector size LSB             | 0xXX  |
| Sector size MSB             | 0xXX  |
| Start sector 0 LSB          | 0xXX  |
| Start sector 0 MSB          | 0xXX  |
| Start count 0 LSB           | 0xXX  |
| Start count 0 MSB           | 0xXX  |
| Partition ID 0              | 0xXX  |
| Partition name length 0 LSB | 0xXX  |
| Partition name length 0 MSB | 0xXX  |
| Partition name 0 START      | 0xXX  |
| …                           | …     |
| Partition name 0 END        | 0x00  |
| …                           | …     |
| Start sector N LSB          | 0xXX  |
| Start sector N MSB          | 0xXX  |
| Start count N LSB           | 0xXX  |
| Start count N MSB           | 0xXX  |
| Partition ID N              | 0xXX  |
| Partition name length N LSB | 0xXX  |
| Partition name length N MSB | 0xXX  |
| Partition name N START      | 0xXX  |
| …                           | …     |
| Partition name N END        | 0x00  |

Note: Check uartboot_types.h file for more details.

- - - - - - - - - - - - - - - - - - - - - - - - - -

### Get version
Get uartboot's version.

#### Command Format

| Byte Description | Value |
| ---------------- | ----- |
| SOH              | 0x01  |
| Command Opcode   | 0x0B  |
| Length LSB       | 0x00  |
| Length MSB       | 0x00  |

#### Return Message

| Byte Description    | Value |
| ------------------- | ----- |
| Version character 0 | '0'   |
| Version character 1 | '.'   |
| Version character 2 | '0'   |
| Version character 3 | '.'   |
| Version character 4 | '0'   |
| Version character 5 | '.'   |
| Version character 6 | '3'   |

Note: Response string is sent without the '\0' character at the end.
      Its length is sent before the payload.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### Chip erase QSPI
Erase whole QSPI FLASH memory.

#### Command Format

| Byte Description | Value |
| ---------------- | ----- |
| SOH              | 0x01  |
| Command Opcode   | 0x0C  |
| Length LSB       | 0x00  |
| Length MSB       | 0x00  |

Note: Execution time for this command is slightly longer than for
      the other commands.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### Is empty QSPI
Check that the part of the QSPI FLASH memory is empty.

#### Command Format

| Byte Description  | Value |
| ----------------- | ----- |
| SOH               | 0x01  |
| Command Opcode    | 0x0D  |
| Length LSB        | 0x08  |
| Length MSB        | 0x00  |
| Size LSB          | 0xXX  |
| Size              | 0xXX  |
| Size              | 0xXX  |
| Size MSB          | 0xXX  |
| Start address LSB | 0xXX  |
| Start address     | 0xXX  |
| Start address     | 0xXX  |
| Start address MSB | 0xXX  |

#### Return Message

| Byte Description | Value |
| ---------------- | ----- |
| Number LSB       | 0xXX  |
| Number           | 0xXX  |
| Number           | 0xXX  |
| Number MSB       | 0xXX  |

Note: If Number represents positive value, than it is a number of bytes that
      were checked to be erased. Otherwise, it represents position of first
      non-empty byte multiplied by -1.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### Read partition
Read data from QSPI FLASH partition.

#### Command Format

| Byte Description   | Value |
| ------------------ | ----- |
| SOH                | 0x01  |
| Command Opcode     | 0x0E  |
| Length LSB         | 0x07  |
| Length MSB         | 0x00  |
| Source address LSB | 0xXX  |
| Source address     | 0xXX  |
| Source address     | 0xXX  |
| Source address MSB | 0xXX  |
| Data length LSB    | 0xXX  |
| Data length MSB    | 0xXX  |
| Partition ID       | 0xXX  |

#### Return Message

| Byte Description | Value       |
| ---------------- | ----------- |
| Data 0           | Data byte 0 |
| …                | …           |
| Data n           | Data byte N |

Note: Source address should be an offset from the partition beginning.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### Write partition
Copy data from RAM to QSPI FLASH partition.

#### Command Format

| Byte Description        | Value |
| ----------------------- | ----- |
| SOH                     | 0x01  |
| Command Opcode          | 0x0F  |
| Length LSB              | 0x0B  |
| Length MSB              | 0x00  |
| Source address LSB      | 0xXX  |
| Source address          | 0xXX  |
| Source address          | 0xXX  |
| Source address MSB      | 0xXX  |
| Data length LSB         | 0xXX  |
| Data length MSB         | 0xXX  |
| Destination address LSB | 0xXX  |
| Destination address     | 0xXX  |
| Destination address     | 0xXX  |
| Destination address MSB | 0xXX  |
| Partition ID            | 0xXX  |

Note: Destination address should be an offset from the partition beginning.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### Get QPSI state
Get QSPI controller state

#### Command Format

| Byte Description   | Value |
| ------------------ | ----- |
| SOH                | 0x01  |
| Command Opcode     | 0x10  |
| Length LSB         | 0x01  |
| Length MSB         | 0x00  |
| QSPI Controller ID | 0xXX  |

#### Return Message

| Byte Description  | Value       |
| ----------------- | ----------- |
| Driver configured | 0x01 / 0x00 |
| Manufacture ID    | 0xXX        |
| Device type       | 0xXX        |
| Density           | 0xXX        |

Note: DA1468x supports only one QSPI controller, DA1469x can support 2.
      Manufacture ID, Device type and Density depends on connected QSPI
      memory.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### GPIO WD
Enable external watchdog notifying.
Start square wave on QPIO pin: 15ms high state, 2s low state.

#### Command Format

| Byte Description |   Value                |
| ---------------- | ---------------------- |
| SOH              | 0x01                   |
| Command Opcode   | 0x11                   |
| Length LSB       | 0x02                   |
| Length MSB       | 0x00                   |
| GPIO pad         | (PORT << 5) + PIN      |
| GPIO level       | 0x00: 3.3V, 0x01: 1.8V |

- - - - - - - - - - - - - - - - - - - - - - -

### QSPI direct write
Write data directly to QSPI memory.

#### Command Format

| Byte Description        | Value           |
| ----------------------- | --------------- |
| SOH                     | 0x01            |
| Command Opcode          | 0x12            |
| Length LSB              | (0x05 + N)      |
| Length MSB              | (0x05 + N) >> 8 |
| Verify write            | 0x00 / 0x01     |
| Destination address LSB | 0xXX            |
| Destination address     | 0xXX            |
| Destination address     | 0xXX            |
| Destination address MSB | 0xXX            |
| Data 0                  | Data byte 0     |
| …                       | …               |
| Data n                  | Data byte N     |

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### Change baudrate
Change communication UART's baudrate.

#### Command Format

| Byte Description | Value |
| ---------------- | ----- |
| SOH              | 0x01  |
| Command Opcode   | 0x30  |
| Length LSB       | 0x04  |
| Length MSB       | 0x00  |
| Baudrate LSB     | 0xXX  |
| Baudrate         | 0xXX  |
| Baudrate         | 0xXX  |
| Baudrate MSB     | 0xXX  |

Note: 'ACK' after command execution is sent using the new baudrate.
      This command has no effect in SWD mode.
      Allowed baudrates: 4800, 9600, 14400, 19200, 28800, 38400, 57600, 115200, 230400, 500000, 1000000.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

### Dummy
Write Live marker to the data buffer.

#### Command Format

| Byte Description | Value |
| ---------------- | ----- |
| SOH              | 0x01  |
| Command Opcode   | 0xFF  |
| Length LSB       | 0x00  |
| Length MSB       | 0x00  |

Note: This command writes 'Live' at the beginning of the data buffer.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
