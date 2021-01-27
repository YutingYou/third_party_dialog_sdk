/**
 ****************************************************************************************
 *
 * @file protocol_cmds.c
 *
 * @brief UART bootloader protocol
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "protocol.h"
#include "crc16.h"
#include "serial.h"
#include "protocol_cmds.h"

/* inline keyword is not available when building C code in VC++, but __inline is */
#ifdef _MSC_VER
#define inline __inline
#endif
#ifdef WIN32
#include <windows.h>
#else
#include <time.h>
#endif

/* Typical timeout for command execution */
#define EXECUTION_TIMEOUT       5000

#define BOOTLOADER_UPLOAD_MAX_RETRY     5

struct write_buf {
        const void *buf;
        size_t len;
};

static uint8_t *boot_loader_code;
static size_t boot_loader_size;

void set_boot_loader_code(uint8_t *code, size_t size)
{
        boot_loader_code = code;
        boot_loader_size = size;
}

void get_boot_loader_code(uint8_t **code, size_t * size)
{
        *code = boot_loader_code;
        *size = boot_loader_size;
}

/**
 * \brief Reads with timeout char over serial
 *
 * \param [in] timeout - time to wait for char in ms
 *
 * \return on success character read from serial line
 *         negative value on error
 *         -1 when timeout occurred
 *
 */
static int serial_read_char(size_t timeout)
{
        unsigned char c;
        int err = serial_read(&c, 1, timeout);

        if (err == 0) {
                return -1; // timeout
        } else if (err < 0) {
                return err;
        } else {
                //printf("Received 0x%02x\n", c);
                return c;
        }
}

/**
 * \brief Sends one char over serial
 *
 * \param [in] c character to transmit
 *
 * \return on success 1, negative value otherwise
 *
 */
static int serial_write_char(uint8_t c)
{
        return serial_write(&c, 1);
}

#ifdef WIN32
typedef DWORD time_ms_t;
#define get_current_time_ms GetTickCount
#else
typedef long long time_ms_t;
time_ms_t get_current_time_ms(void)
{
        struct timespec spec;

        clock_gettime(CLOCK_REALTIME, &spec);
        return spec.tv_sec * 1000 + spec.tv_nsec / 1000000;

}
#endif

/**
 * \brief Get current boot stage
 *
 * \return >0: Uartboot running. The returned value is uartboot's version
 *          0: ROM bootloader running
 *         <0: error
 *
 */
static int get_boot_stage(int timeout)
{
        int c;
        int err = ERR_PROT_NO_RESPONSE; // assume it nothing comes
        int ver;
        time_ms_t time_limit = get_current_time_ms() + timeout;

        while (get_current_time_ms() < time_limit) {
                c = serial_read_char(time_limit - get_current_time_ms());
                switch (c) {
                case -1:
                        return err;
                case STX:
                        c = serial_read_char(30);
                        // Just STX, first stage
                        if (c < 0) {
                                return 0;
                        }

                        // Verify that second stage is correct
                        if (c != SOH) {
                                continue;
                        }
                        c = serial_read_char(20);
                        if (c < 0) {
                                continue;
                        }
                        ver = c << 8;
                        c = serial_read_char(20);
                        if (c < 0) {
                                continue;
                        }
                        ver += c;
                        if (ver == 0) {
                                return ERR_PROT_UNSUPPORTED_VERSION;
                        }
                        return ver;
                default:
                        err = ERR_PROT_UNKNOWN_RESPONSE;
                }
        }
        return err;
}

/**
 * \brief Sends code to device using first stage bootloader protocol
 *
 * \param [in] buf data to send to device
 * \param [in] size number of bytes to send
 *
 * \return 0 - on success
 *         negative value on error
 *
 */
static int send_initial_code(const uint8_t *buf, uint32_t size)
{
        int sum = 0;
        int remote_sum;
        size_t i;
        uint32_t max_bootloader_size = 0x1FFFF;

        for (i = 0; i < size; ++i) {
                sum ^= (int)buf[i];
        }

        if (size < max_bootloader_size / 2 ) {
                uint8_t legacy_mode_header[3] = { SOH, (uint8_t) size, (uint8_t) (size >> 8) };
                serial_write(legacy_mode_header, 3);
        } else {
                uint8_t extended_mode_header[6] = { SOH, 0x0, 0x0, (uint8_t) size, (uint8_t) (size >> 8), 0x1 };
                serial_write(extended_mode_header, 6);
        }

        if (serial_read_char(100) != ACK) {
                return ERR_PROT_BOOT_LOADER_REJECTED;
        }

        serial_write(buf, size);
        remote_sum  = serial_read_char(1000);
        if (sum != remote_sum) {
                return ERR_PROT_CHECKSUM_MISMATCH;
        } else {
                serial_write_char(ACK);
                return 0;
        }
}

/**
 * \brief Wait for ACK
 *
 * \param [in] timeout time in milliseconds to wait for ACK
 *
 * \return 0 - on success
 *         negative value on error
 *
 */
static int wait_for_ack(size_t timeout)
{
        int c = serial_read_char(timeout);
        if (c < 0) {
                return ERR_PROT_NO_RESPONSE;
        } else if (c == NAK) {
                return ERR_PROT_CMD_REJECTED;
        } else if (c != ACK) {
                return ERR_PROT_INVALID_RESPONSE;
        }
        return 0;
}

/**
 * \brief Send command header to device
 *
 * This will wait for ACK after command is sent.
 *
 * \param [in] type command type
 * \param [in] len length of data to be sent in command
 *
 * \return 0 on success, error code on failure
 *
 */
static int send_cmd_header(uint8_t type, uint16_t len)
{
        uint8_t buf[4];

        buf[0] = SOH;
        buf[1] = type;
        buf[2] = (uint8_t) len;
        buf[3] = (uint8_t) (len >> 8);

        if (serial_write(buf, sizeof(buf)) < 0) {
                return ERR_PROT_TRANSMISSION_ERROR;
        }

        return wait_for_ack(300);
}

/**
 * \brief Send command data to device
 *
 * Data is sent using writev() semantics so multiple buffers can be sent using single call.
 * This will wait for ACK from device after data is sent and will also check perform CRC check.
 *
 * \param [in] wb buffers to be written
 * \param [in] cnt number of buffers
 *
 * \return 0 on success, error code on failure
 *
 */
static int send_cmd_data(const struct write_buf *wb, int cnt)
{
        int i;
        uint16_t crc, crc_r;
        int ret = 0;

        crc16_init(&crc);

        for (i = 0; i < cnt; i++) {
                const struct write_buf *b = &wb[i];

                crc16_update(&crc, b->buf, b->len);

                ret = serial_write(b->buf, b->len);
                if (ret < 0) {
                        goto done;
                }
        }

        ret = wait_for_ack(EXECUTION_TIMEOUT);
        if (ret < 0) {
                goto done;
        }

        /* don't care if read is successfull, CRC won't simply match on error */
        crc_r = serial_read_char(30);
        crc_r |= serial_read_char(30) << 8;

        if (crc_r != crc) {
                (void) serial_write_char(NAK); // return value does not matter here
                ret = ERR_PROT_CRC_MISMATCH;
        } else {
                ret = serial_write_char(ACK);
                if (ret <= 0) {
                        /* overwrite ret since serial_write_char returns value from write() call */
                        ret = ERR_PROT_TRANSMISSION_ERROR;
                } else {
                        ret = 0; // indicate success
                }
        }

done:
        return ret;
}

static int read_cmd_data(uint8_t *buf, size_t size)
{
        uint16_t size_r;
        uint16_t crc;
        size_t offset;
        size_t left;
        int read;
        int ret = 0;

        /* don't care if read is successfull, size won't simply match on error */
        size_r = serial_read_char(250);
        size_r |= serial_read_char(30) << 8;
        if (size_r != size) {
                (void) serial_write_char(NAK); // return value does not matter here
                ret = ERR_PROT_COMMAND_ERROR;
                goto done;
        }

        if (serial_write_char(ACK) < 0) {
                ret = ERR_PROT_TRANSMISSION_ERROR;
                goto done;
        }

        offset = 0;
        left = size;
        do {
                read = serial_read(buf + offset, left, 1000);
                if (read > 0) {
                        offset += read;
                        left -= read;
                }
        } while (read > 0 && left > 0);

        if (left > 0) {
                ret = ERR_PROT_TRANSMISSION_ERROR;
                goto done;
        }

        crc = crc16_calculate(buf, size);

        /* return value does not matter (i.e. we won't receive ACK in case of error anyway) */
        (void) serial_write_char((uint8_t) crc);
        (void) serial_write_char((uint8_t) (crc >> 8));
        ret = wait_for_ack(EXECUTION_TIMEOUT);

done:
        return ret;
}

static int read_cmd_dynamic_length(uint8_t **buf, uint32_t *len)
{
        int ret = 0;
        uint16_t size_r;
        size_t offset;
        size_t left;
        int read;
        uint16_t crc;

        size_r = serial_read_char(250);
        size_r |= serial_read_char(30) << 8;
        *len = size_r;

        if (*len > SHRT_MAX) {
                ret = ERR_PROT_TRANSMISSION_ERROR;
                goto done;
        }

        if ((*buf = (uint8_t *) malloc(*len)) == NULL) {
                ret = ERR_ALLOC_FAILED;
                goto done;
        }

        if (serial_write_char(ACK) < 0) {
                free(*buf);
                ret = ERR_PROT_TRANSMISSION_ERROR;
                goto done;
        }

        offset = 0;
        left = *len;
        do {
                read = serial_read(*buf + offset, left, 1000);
                if (read > 0) {
                        offset += read;
                        left -= read;
                }
        } while (read > 0 && left > 0);

        if (left > 0) {
                free(*buf);
                ret = ERR_PROT_TRANSMISSION_ERROR;
                goto done;
        }

        crc = crc16_calculate(*buf, *len);

        /* return value does not matter (i.e. we won't receive ACK in case of error anyway) */
        (void) serial_write_char((uint8_t) crc);
        (void) serial_write_char((uint8_t) (crc >> 8));
        ret = wait_for_ack(150);
done:
        return ret;
}

int protocol_cmd_write(const uint8_t *buf, size_t size, uint32_t addr)
{
        uint8_t header_buf[4];
        struct write_buf wb[2];
        int err;

        err = send_cmd_header(CMD_WRITE, sizeof(header_buf) + size);
        if (err < 0) {
                return err;
        }

        header_buf[0] = (uint8_t) (addr);
        header_buf[1] = (uint8_t) (addr >> 8);
        header_buf[2] = (uint8_t) (addr >> 16);
        header_buf[3] = (uint8_t) (addr >> 24);

        wb[0].buf = header_buf;
        wb[0].len = sizeof(header_buf);
        wb[1].buf = buf;
        wb[1].len = size;
        err = send_cmd_data(wb, 2);
        if (err < 0) {
                return err;
        }

        err = wait_for_ack(EXECUTION_TIMEOUT);

        return err;
}

int protocol_cmd_read(uint8_t *buf, size_t size, uint32_t addr)
{
        uint8_t header_buf[6];
        struct write_buf wb[1];
        int err;

        err = send_cmd_header(CMD_READ, sizeof(header_buf));
        if (err < 0) {
                return err;
        }

        header_buf[0] = (uint8_t) (addr);
        header_buf[1] = (uint8_t) (addr >> 8);
        header_buf[2] = (uint8_t) (addr >> 16);
        header_buf[3] = (uint8_t) (addr >> 24);
        header_buf[4] = (uint8_t) (size);
        header_buf[5] = (uint8_t) (size >> 8);

        wb[0].buf = header_buf;
        wb[0].len = sizeof(header_buf);

        err = send_cmd_data(wb, 1);
        if (err < 0) {
                return err;
        }

        err = wait_for_ack(EXECUTION_TIMEOUT);
        if (err < 0) {
                return err;
        }

        err = read_cmd_data(buf, size);

        return err;
}

int protocol_cmd_copy_to_qspi(uint32_t src_address, size_t size, uint32_t dst_address)
{
        uint8_t header_buf[10];
        struct write_buf wb[1];
        int err;


        err = send_cmd_header(CMD_COPY_QSPI, sizeof(header_buf));
        if (err < 0) {
                return err;
        }

        header_buf[0] = (uint8_t) (src_address);
        header_buf[1] = (uint8_t) (src_address >> 8);
        header_buf[2] = (uint8_t) (src_address >> 16);
        header_buf[3] = (uint8_t) (src_address >> 24);
        header_buf[4] = (uint8_t) (size);
        header_buf[5] = (uint8_t) (size >> 8);
        header_buf[6] = (uint8_t) (dst_address);
        header_buf[7] = (uint8_t) (dst_address >> 8);
        header_buf[8] = (uint8_t) (dst_address >> 16);
        header_buf[9] = (uint8_t) (dst_address >> 24);

        wb[0].buf = header_buf;
        wb[0].len = sizeof(header_buf);

        err = send_cmd_data(wb, 1);
        if (err < 0) {
                return err;
        }

        err = wait_for_ack(EXECUTION_TIMEOUT);

        return err;
}

int protocol_cmd_direct_write_to_qspi(const uint8_t *buf, size_t size, uint32_t addr, bool verify)
{
        uint8_t header_buf[5];
        struct write_buf wb[2];
        int err;

        err = send_cmd_header(CMD_DIRECT_WRITE_TO_QSPI, sizeof(header_buf) + size);
        if (err < 0) {
                return err;
        }

        header_buf[0] = (uint8_t) (verify);

        header_buf[1] = (uint8_t) (addr);
        header_buf[2] = (uint8_t) (addr >> 8);
        header_buf[3] = (uint8_t) (addr >> 16);
        header_buf[4] = (uint8_t) (addr >> 24);

        wb[0].buf = header_buf;
        wb[0].len = sizeof(header_buf);
        wb[1].buf = buf;
        wb[1].len = size;
        err = send_cmd_data(wb, 2);
        if (err < 0) {
                return err;
        }

        err = wait_for_ack(EXECUTION_TIMEOUT);

        return err;
}

int protocol_cmd_erase_qspi(uint32_t address, size_t size)
{
        uint8_t header_buf[8];
        struct write_buf wb[1];
        int err;

        err = send_cmd_header(CMD_ERASE_QSPI, sizeof(header_buf));
        if (err < 0) {
                return err;
        }

        header_buf[0] = (uint8_t) (address);
        header_buf[1] = (uint8_t) (address >> 8);
        header_buf[2] = (uint8_t) (address >> 16);
        header_buf[3] = (uint8_t) (address >> 24);
        header_buf[4] = (uint8_t) (size);
        header_buf[5] = (uint8_t) (size >> 8);
        header_buf[6] = (uint8_t) (size >> 16);
        header_buf[7] = (uint8_t) (size >> 24);

        wb[0].buf = header_buf;
        wb[0].len = sizeof(header_buf);

        err = send_cmd_data(wb, 1);
        if (err < 0) {
                return err;
        }

        /* it takes about 50ms for sector to be erased, add 200ms to be sure */
        err = wait_for_ack(200 + 50 * size / 0x1000);

        return err;
}

int protocol_cmd_chip_erase_qspi(void)
{
        int err;

        err = send_cmd_header(CMD_CHIP_ERASE_QSPI, 0);
        if (err < 0) {
                return err;
        }

        /* it takes at most 50s for whole flash memory to be erased */
        err = wait_for_ack(50000);

        return err;
}

int protocol_cmd_run(uint32_t address)
{
        uint8_t header_buf[4];
        struct write_buf wb[1];
        int err;

        err = send_cmd_header(CMD_RUN, sizeof(header_buf));
        if (err < 0) {
                return err;
        }

        header_buf[0] = (uint8_t) (address);
        header_buf[1] = (uint8_t) (address >> 8);
        header_buf[2] = (uint8_t) (address >> 16);
        header_buf[3] = (uint8_t) (address >> 24);

        wb[0].buf = header_buf;
        wb[0].len = sizeof(header_buf);

        err = send_cmd_data(wb, 1);
        if (err < 0) {
                return err;
        }

        err = wait_for_ack(150);

        return err;
}

int protocol_cmd_write_otp(uint32_t address, const uint32_t *buf, uint32_t len)
{
        uint8_t header_buf[4];
        struct write_buf wb[2];
        uint32_t size;
        int err;

        size = len * sizeof(*buf);

        err = send_cmd_header(CMD_WRITE_OTP, sizeof(header_buf) + size);
        if (err < 0) {
                return err;
        }

        header_buf[0] = (uint8_t) (address);
        header_buf[1] = (uint8_t) (address >> 8);
        header_buf[2] = (uint8_t) (address >> 16);
        header_buf[3] = (uint8_t) (address >> 24);

        wb[0].buf = header_buf;
        wb[0].len = sizeof(header_buf);
        wb[1].buf = buf;
        wb[1].len = size;

        err = send_cmd_data(wb, 2);
        if (err < 0) {
                return err;
        }

        err = wait_for_ack(150);

        return err;
}

int protocol_cmd_read_otp(uint32_t address, uint32_t *buf, uint32_t len)
{
        uint8_t header_buf[6];
        struct write_buf wb[1];
        uint32_t size;
        int err;

        err = send_cmd_header(CMD_READ_OTP, sizeof(header_buf));
        if (err < 0) {
                return err;
        }

        header_buf[0] = (uint8_t) (address);
        header_buf[1] = (uint8_t) (address >> 8);
        header_buf[2] = (uint8_t) (address >> 16);
        header_buf[3] = (uint8_t) (address >> 24);
        header_buf[4] = (uint8_t) (len);
        header_buf[5] = (uint8_t) (len >> 8);

        wb[0].buf = header_buf;
        wb[0].len = sizeof(header_buf);

        err = send_cmd_data(wb, 1);
        if (err < 0) {
                return err;
        }

        err = wait_for_ack(150);
        if (err < 0) {
                return err;
        }

        size = len * sizeof(*buf);

        err = read_cmd_data((void *) buf, size);

        return err;
}

int protocol_cmd_read_qspi(uint32_t address, uint8_t *buf, uint32_t len)
{
        uint8_t header_buf[6];
        struct write_buf wb[1];
        uint32_t size;
        int err;

        err = send_cmd_header(CMD_READ_QSPI, sizeof(header_buf));
        if (err < 0) {
                return err;
        }

        header_buf[0] = (uint8_t) (address);
        header_buf[1] = (uint8_t) (address >> 8);
        header_buf[2] = (uint8_t) (address >> 16);
        header_buf[3] = (uint8_t) (address >> 24);
        header_buf[4] = (uint8_t) (len);
        header_buf[5] = (uint8_t) (len >> 8);

        wb[0].buf = header_buf;
        wb[0].len = sizeof(header_buf);

        err = send_cmd_data(wb, 1);
        if (err < 0) {
                return err;
        }

        err = wait_for_ack(EXECUTION_TIMEOUT);
        if (err < 0) {
                return err;
        }

        size = len * sizeof(*buf);

        err = read_cmd_data((void *) buf, size);

        return err;
}

int protocol_cmd_is_empty_qspi(unsigned int size, unsigned int start_address, int *ret_number)
{
        uint8_t header_buf[8];
        struct write_buf wb[1];
        int err;

        err =  send_cmd_header(CMD_IS_EMPTY_QSPI, sizeof(header_buf));
        if (err < 0) {
                return err;
        }

        header_buf[0] = (uint8_t) (size);
        header_buf[1] = (uint8_t) (size >> 8);
        header_buf[2] = (uint8_t) (size >> 16);
        header_buf[3] = (uint8_t) (size >> 24);

        header_buf[4] = (uint8_t) (start_address);
        header_buf[5] = (uint8_t) (start_address >> 8);
        header_buf[6] = (uint8_t) (start_address >> 16);
        header_buf[7] = (uint8_t) (start_address >> 24);

        wb[0].buf = header_buf;
        wb[0].len = sizeof(header_buf);

        err = send_cmd_data(wb, 1);

        if (err < 0) {
                return err;
        }

        err = wait_for_ack(EXECUTION_TIMEOUT);

        if (err < 0) {
                return err;
        }

        err = read_cmd_data((void *) ret_number, sizeof(uint32_t));

        return err;
}

int protocol_cmd_read_partition_table(uint8_t **buf, uint32_t *len)
{
        int err;

        err =  send_cmd_header(CMD_READ_PARTITION_TABLE, 0);
        if (err < 0) {
                return err;
        }

        err = wait_for_ack(EXECUTION_TIMEOUT);
        if (err < 0) {
                return err;
        }

        err = read_cmd_dynamic_length(buf, len);

        return err;
}

int protocol_cmd_read_partition(nvms_partition_id_t id, uint32_t address, uint8_t *buf,
                                                                                uint32_t len)
{
        uint8_t header_buf[7];
        struct write_buf wb[1];
        uint32_t size;
        int err;

        err = send_cmd_header(CMD_READ_PARTITION, sizeof(header_buf));
        if (err < 0) {
                return err;
        }

        header_buf[0] = (uint8_t) (address);
        header_buf[1] = (uint8_t) (address >> 8);
        header_buf[2] = (uint8_t) (address >> 16);
        header_buf[3] = (uint8_t) (address >> 24);
        header_buf[4] = (uint8_t) (len);
        header_buf[5] = (uint8_t) (len >> 8);
        header_buf[6] = (uint8_t) (id);

        wb[0].buf = header_buf;
        wb[0].len = sizeof(header_buf);

        err = send_cmd_data(wb, 1);
        if (err < 0) {
                return err;
        }

        err = wait_for_ack(EXECUTION_TIMEOUT);
        if (err < 0) {
                return err;
        }

        size = len * sizeof(*buf);

        err = read_cmd_data((void *) buf, size);

        return err;
}

int protocol_cmd_write_partition(nvms_partition_id_t id, uint32_t dst_address, uint32_t src_address,
                                                                                        size_t size)
{
        uint8_t header_buf[11];
        struct write_buf wb[1];
        int err;

        err = send_cmd_header(CMD_WRITE_PARTITION, sizeof(header_buf));
        if (err < 0) {
                return err;
        }

        header_buf[0] = (uint8_t) (src_address);
        header_buf[1] = (uint8_t) (src_address >> 8);
        header_buf[2] = (uint8_t) (src_address >> 16);
        header_buf[3] = (uint8_t) (src_address >> 24);
        header_buf[4] = (uint8_t) (size);
        header_buf[5] = (uint8_t) (size >> 8);
        header_buf[6] = (uint8_t) (dst_address);
        header_buf[7] = (uint8_t) (dst_address >> 8);
        header_buf[8] = (uint8_t) (dst_address >> 16);
        header_buf[9] = (uint8_t) (dst_address >> 24);
        header_buf[10] = (uint8_t) (id);

        wb[0].buf = header_buf;
        wb[0].len = sizeof(header_buf);

        err = send_cmd_data(wb, 1);
        if (err < 0) {
                return err;
        }

        err = wait_for_ack(EXECUTION_TIMEOUT);

        return err;
}

/* upload executable binary to device and run it */
static int protocol_upload_executable(uint8_t *executable_code, size_t executable_code_size)
{
        int retry_cnt = BOOTLOADER_UPLOAD_MAX_RETRY;
        int prev_baudrate = serial_set_baudrate(prog_get_initial_baudrate());
        int ver_err = -1;
        time_ms_t time_limit = 0;

        prog_print_log("Connecting to device...\n");

        while (retry_cnt--) {
                if (protocol_verify_connection() != CONN_ALLOWED) {
                        prog_print_log("Press RESET.\n");

                        // timeout after 5 s
                        time_limit = get_current_time_ms() + 5000;
                        while (get_current_time_ms() < time_limit) {
                                ver_err = get_boot_stage(get_uart_timeout());
                                // device must be running first stage bootloader
                                if (ver_err == 0) {
                                        break;
                                }
                        }

                        if (ver_err != 0) {
                                prog_print_log("Could not connect to device. \n");
                                // second stage is invalid, return error code
                                if (ver_err > 0) {
                                        ver_err = ERR_PROT_NO_RESPONSE;
                                }
                                goto end;
                        }
                }

                prog_print_log("Uploading boot loader/application executable...\n");
                ver_err = send_initial_code(executable_code, (uint32_t) executable_code_size);

                // if status is not "checksum mismatch" exit loop
                if (ver_err != ERR_PROT_CHECKSUM_MISMATCH) {
                        break;
                }

                prog_print_log("Checksum mismatch, retrying.\n");
        }

        if (ver_err == 0) {
                prog_print_log("Executable uploaded.\n\n");
        }

end:
        serial_set_baudrate(prev_baudrate);
        return ver_err;
}

int protocol_cmd_boot(uint8_t *executable_code, size_t executable_code_size)
{
        return protocol_upload_executable(executable_code, executable_code_size);
}

int protocol_cmd_upload_bootloader(void)
{
        int status;

        status = protocol_upload_executable(boot_loader_code, boot_loader_size);
        if (status != 0) {
                return status;
        }

        status = get_boot_stage(get_uart_timeout());
        if (status < 0) {
                return status;
        } else if (status == 0) {
                return ERR_PROT_UNSUPPORTED_VERSION;
        }

        return 0;
}

connection_status_t protocol_verify_connection(void)
{
        int status;

        // hello message is send with 1s delay. Wait 2s to be sure that it will be received.
        status = get_boot_stage(2000);

        if (status > 0) {
                return CONN_ESTABLISHED;
        } else if (status == 0) {
                return CONN_ALLOWED;
        } else {
                return CONN_ERROR;
        }
}
