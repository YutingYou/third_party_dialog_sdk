/**
 ****************************************************************************************
 *
 * @file protocol.h
 *
 * @brief Common protocol definitions
 *
 * Copyright (C) 2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H

/**************************************************************************************************
 * Special characters used by protocol
 *************************************************************************************************/

/**
 * \brief Start of Header
 *
 * Sent as the beginning of each command. Second byte of the 'Hello message'.
 */
#define SOH '\x01'

/**
 * \brief Start of Text
 *
 * Sent at the beginning 'Hello message'.
 */
#define STX '\x02'

/**
 * \brief Acknowledge
 *
 * Sent by uartboot or the host application when the received command/data is valid.
 */
#define ACK '\x06'

/**
 * \brief Negative Acknowledge
 *
 * Sent by uartboot or the host application when the received command/data is not valid.
 */
#define NAK '\x15'


/**************************************************************************************************
 * Command for basic functions needed by protocol
 *************************************************************************************************/

/**
 * \brief Write data to RAM
 *
 */
#define CMD_WRITE                  0x01

/**
 * \brief Read data from RAM
 *
 */
#define CMD_READ                   0x02

/**
 * \brief Erase a part of the QSPI FLASH memory
 *
 */
#define CMD_COPY_QSPI              0x03

/**
 * \brief Copy data from RAM to QSPI FLASH memory
 *
 */
#define CMD_ERASE_QSPI             0x04

/**
 * \brief Execute a part of the code
 *
 */
#define CMD_RUN                    0x05

/**
 * \brief Write data to the OTP memory
 *
 */
#define CMD_WRITE_OTP              0x06

/**
 * \brief Read data from the OTP memory
 *
 */
#define CMD_READ_OTP               0x07

/**
 * \brief Read data from QSPI memory
 *
 */
#define CMD_READ_QSPI              0x08

/**
 * \brief Perform a customer specific action
 *
 * Handler for this command should be implemented by customer.
 */
#define CMD_CUSTOMER_SPECIFIC      0x09

/**
 * \brief Read partition table from QSPI memory
 *
 */
#define CMD_READ_PARTITION_TABLE   0x0A

/**
 * \brief Get uartboot's version
 *
 */
#define CMD_GET_VERSION            0x0B

/**
 * \brief Erase whole QSPI FLASH memory
 *
 */
#define CMD_CHIP_ERASE_QSPI        0x0C

/**
 * \brief Check that the part of the QSPI FLASH memory is empty
 *
 */
#define CMD_IS_EMPTY_QSPI          0x0D

/**
 * \brief Read data from QSPI FLASH partition
 *
 * This command is available only when dg_configNVMS_ADAPTER is enabled.
 *
 */
#define CMD_READ_PARTITION         0x0E

/**
 * \brief Copy data from RAM to QSPI FLASH partition
 *
 * This command is available only when dg_configNVMS_ADAPTER is enabled.
 *
 */
#define CMD_WRITE_PARTITION        0x0F

/**
 * \brief Check QSPI memory state
 *
 * If the QSPI memory is connected and recognized then returns device identification.
 *
 */
#define CMD_GET_QSPI_STATE         0x10

/**
 * \brief Enable external watchdog triggering
 *
 * Start square wave on QPIO pin: 15ms high state, 2s low state.
 *
 */
#define CMD_GPIO_WD                0x11

/**
 * \brief Write data directly to the QSPI FLASH memory
 *
 */
#define CMD_DIRECT_WRITE_TO_QSPI   0x12

/**
 * \brief Change communication UART's baudrate
 *
 */
#define CMD_CHANGE_BAUDRATE        0x30

/**
 * \brief Write 'Live' marker to the data buffer
 *
 */
#define CMD_DUMMY                  0xFF

#endif /* PROTOCOL_H */
