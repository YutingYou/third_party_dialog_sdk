/**
 ****************************************************************************************
 *
 * @file cmd_handlers.c
 *
 * @brief Handling of CLI commands provided on command line
 *
 * Copyright (C) 2015-2019 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <programmer.h>
#include "cli_common.h"

#define DEFAULT_SIZE 0x100000

/* Maximum size for an image */
#define MAX_IMAGE_SIZE 0x7F000

/* Pointer will hold memory for executable application code */
static uint8_t *executable_bin;
/* Executable application size */
static size_t executable_size;

static int cmdh_write(int argc, char *argv[]);
static int cmdh_read(int argc, char *argv[]);
static int cmdh_write_qspi(int argc, char *argv[]);
static int cmdh_write_qspi_bytes(int argc, char *argv[]);
static int cmdh_read_qspi(int argc, char *argv[]);
static int cmdh_read_partition_table(int argc, char *argv[]);
static int cmdh_read_partition(int argc, char *argv[]);
static int cmdh_write_partition(int argc, char *argv[]);
static int cmdh_write_partition_bytes(int argc, char *argv[]);
static int cmdh_erase_qspi(int argc, char *argv[]);
static int cmdh_chip_erase_qspi(int argc, char *argv[]);
static int cmdh_copy_qspi(int argc, char *argv[]);
static int cmdh_is_empty_qspi(int argc, char *argv[]);
static int cmdh_write_otp(int argc, char *argv[]);
static int cmdh_read_otp(int argc, char *argv[]);
static int cmdh_write_otp_file(int argc, char *argv[]);
static int cmdh_write_otp_raw_file(int argc, char *argv[]);
static int cmdh_read_otp_file(int argc, char *argv[]);
static int cmdh_write_tcs(int argc, char *argv[]);
static int cmdh_boot(int argc, char *argvp[]);
static int cmdh_run(int argc, char *argvp[]);
static int cmdh_read_chip_info(int argc, char *argvp[]);


/**
 * \brief CLI command handler description
 *
 */
struct cli_command {
        const char *name;                       /**< name of command */
        int min_num_p;                          /**< minimum number of parameters */
        int (* func) (int argc, char *argv[]);  /**< handler function, return non-zero for success */
};

/**
 * \brief CLI command handlers
 *
 */
static struct cli_command cmds[] = {
        { "write",                 2, cmdh_write, },
        { "read",                  3, cmdh_read, },
        { "write_qspi",            2, cmdh_write_qspi, },
        { "write_qspi_bytes",      2, cmdh_write_qspi_bytes, },
        { "read_qspi",             3, cmdh_read_qspi, },
        { "erase_qspi",            2, cmdh_erase_qspi, },
        { "chip_erase_qspi",       0, cmdh_chip_erase_qspi, },
        { "read_partition_table",  0, cmdh_read_partition_table, },
        { "read_partition",        4, cmdh_read_partition, },
        { "write_partition",       3, cmdh_write_partition, },
        { "write_partition_bytes", 3, cmdh_write_partition_bytes, },
        { "copy_qspi",             3, cmdh_copy_qspi, },
        { "is_empty_qspi",         0, cmdh_is_empty_qspi, },
        { "write_otp",             2, cmdh_write_otp, },
        { "read_otp",              2, cmdh_read_otp, },
        { "write_otp_file",        1, cmdh_write_otp_file, },
        { "write_otp_raw_file",    2, cmdh_write_otp_raw_file, },
        { "read_otp_file",         1, cmdh_read_otp_file, },
        { "write_tcs",             3, cmdh_write_tcs },
        { "boot",                  1, cmdh_boot, },
        { "run",                   1, cmdh_run, },
        { "read_chip_info",        0, cmdh_read_chip_info, },
        /* end of table */
        { NULL,             0, NULL, }
};

static int get_filesize(const char *fname)
{
        struct stat st;

        if (stat(fname, &st) < 0) {
                return -1;
        }

        return st.st_size;
}

static bool check_otp_cell_address(uint32_t *addr)
{
        uint32_t _addr = *addr;
        uint32_t otp_base_mem = 0x10080000;
        uint32_t max_cell = 0x400;
        uint8_t  addr_to_cel_div = 2; /*4 bytes per cell */


/* convert mapped address to cell address, if possible */
        if ((_addr & otp_base_mem) == otp_base_mem) {
                _addr = (_addr & 0xFFFF) >> addr_to_cel_div;
        }

        /* there are maximum cells depending on the OTP type (64 bits/32 bits per cell) ... */
        if (_addr >= max_cell) {
                return false;
        }

        *addr = _addr;

        return true;
}

static int cmdh_write(int argc, char *argv[])
{
        unsigned int addr;
        const char *fname = argv[1];
        unsigned int size = 0;
        int ret;
        const char *chip_rev;
        const prog_memory_sizes_t *sizes;

        if (!get_number(argv[0], &addr)) {
                prog_print_err("invalid address\n");
                return 0;
        }

        if (argc > 2) {
                if (!get_number(argv[2], &size) || !size) {
                        prog_print_err("invalid size\n");
                        return 0;
                }
        } else {
                int file_size = get_filesize(fname);

                if (file_size < 0) {
                        prog_print_err("could not open file\n");
                        return 0;
                }

                size = file_size;
        }

        prog_get_chip_rev(&chip_rev);
        prog_get_memory_sizes(chip_rev, &sizes);
        if (size > sizes->ram_size) {
                prog_print_err("invalid size exceeding RAM size\n");
                return 0;
        }

        ret = prog_write_file_to_ram(addr, fname, size);
        if (ret) {
                prog_print_err("write to RAM failed: %s (%d)\n", prog_get_err_message(ret), ret);
                return 0;
        }

        return 1;
}

static int cmdh_read(int argc, char *argv[])
{
        unsigned int addr;
        const char *fname = argv[1];
        unsigned int size = 0;
        uint8_t *buf = NULL;
        int ret;
        const char *chip_rev;
        const prog_memory_sizes_t *sizes;

        if (!get_number(argv[0], &addr)) {
                prog_print_err("invalid address\n");
                return 0;
        }

        if (!get_number(argv[2], &size) || !size) {
                prog_print_err("invalid size\n");
                return 0;
        }

        prog_get_chip_rev(&chip_rev);
        prog_get_memory_sizes(chip_rev, &sizes);
        if (size > sizes->ram_size) {
                prog_print_err("invalid size exceeding RAM size\n");
                return 0;
        }

        if (!strcmp(fname, "-") || !strcmp(fname, "--")) {
                buf = malloc(size);
                ret = prog_read_memory(addr, buf, size);
        } else {
                ret = prog_read_memory_to_file(addr, fname, size);
        }
        if (ret) {
                free(buf);
                prog_print_err("read from RAM failed: %s (%d)\n", prog_get_err_message(ret), ret);
                return 0;
        }

        if (buf) {
                dump_hex(addr, buf, size, !strcmp(fname, "--") ? 32 : 16);
                free(buf);
        }

        return 1;
}

static int cmdh_write_qspi(int argc, char *argv[])
{
        unsigned int addr;
        const char *fname = argv[1];
        unsigned int size = 0;
        int ret;
        const char *chip_rev;
        const prog_memory_sizes_t *sizes;

        if (!get_number(argv[0], &addr)) {
                prog_print_err("invalid address\n");
                return 0;
        }

        if (argc > 2) {
                if (!get_number(argv[2], &size) || !size) {
                        prog_print_err("invalid size\n");
                        return 0;
                }
        } else {
                int file_size = get_filesize(fname);

                if (file_size < 0) {
                        prog_print_err("could not open file\n");
                        return 0;
                }

                size = file_size;
        }

        prog_get_chip_rev(&chip_rev);
        prog_get_memory_sizes(chip_rev, &sizes);
        if (size > sizes->qspi_size) {
                prog_print_err("invalid size exceeding QSPI size\n");
                return 0;
        }

        ret = prog_write_file_to_qspi(addr, fname, size);
        if (ret) {
                prog_print_err("write to QSPI failed: %s (%d)\n", prog_get_err_message(ret), ret);
                return 0;
        }

        return 1;
}

static int cmdh_write_qspi_bytes(int argc, char *argv[])
{
        unsigned int addr;
        int ret;
        int i;
        unsigned int b;
        uint8_t *buf = (uint8_t *) malloc(argc);

        if (buf == NULL) {
                return 0;
        }

        if (!get_number(argv[0], &addr)) {
                prog_print_err("invalid address %s\n", argv[0]);
                ret = 0;
                goto end;
        }

        for (i = 1; i < argc; ++i) {
                if (!get_number(argv[i], &b)) {
                        prog_print_err("invalid byte '%s'\n", argv[i]);
                        ret = 0;
                        goto end;
                }
                buf[i - 1] = (uint8_t) b;
        }

        ret = prog_write_to_qspi(addr, buf, argc - 1);
        if (ret) {
                prog_print_err("write to QSPI failed: %s (%d)\n", prog_get_err_message(ret), ret);
                ret = 0;
                goto end;
        }
end:
        if (buf) {
                free(buf);
        }

        return 1;
}


static int cmdh_read_qspi(int argc, char *argv[])
{
        unsigned int addr;
        const char *fname = argv[1];
        unsigned int size = 0;
        uint8_t *buf = NULL;
        int ret;
        const char *chip_rev;
        const prog_memory_sizes_t *sizes;

        if (!get_number(argv[0], &addr)) {
                prog_print_err("invalid address\n");
                return 0;
        }

        if (!get_number(argv[2], &size) || !size) {
                prog_print_err("invalid size\n");
                return 0;
        }

        prog_get_chip_rev(&chip_rev);
        prog_get_memory_sizes(chip_rev, &sizes);
        if (size > sizes->qspi_size) {
                prog_print_err("invalid size exceeding QSPI size\n");
                return 0;
        }

        if (!strcmp(fname, "-") || !strcmp(fname, "--")) {

                buf = malloc(size);
                if (!buf) {
                        ret = ERR_ALLOC_FAILED;
                } else {
                        ret = prog_read_qspi(addr, buf, size);
                }
        } else {
                ret = prog_read_qspi_to_file(addr, fname, size);
        }
        if (ret) {
                free(buf);
                prog_print_err("read from QSPI failed: %s (%d)\n", prog_get_err_message(ret), ret);
                return 0;
        }

        if (buf) {
                dump_hex(addr, buf, size, !strcmp(fname, "--") ? 32 : 16);
                free(buf);
        }

        return 1;
}

static int cmdh_erase_qspi(int argc, char *argv[])
{
        unsigned int addr;
        unsigned int size;
        int ret;

        if (!get_number(argv[0], &addr)) {
                prog_print_err("invalid address\n");
                return 0;
        }

        if (!get_number(argv[1], &size)) {
                prog_print_err("invalid size\n");
                return 0;
        }

        ret = prog_erase_qspi(addr, size);
        if (ret) {
                prog_print_err("erase QSPI failed: %s (%d)\n", prog_get_err_message(ret), ret);
                return 0;
        }

        return 1;
}

static int cmdh_read_partition_table(int argc, char *argv[])
{
        int ret = 0;
        unsigned int size = 0;
        uint8_t *buf = NULL;

        ret = prog_read_partition_table(&buf, &size);
        if (ret) {
                prog_print_err("read partition table failed: %s (%d)\n", prog_get_err_message(ret),
                                                                                                ret);
                goto done;
        }

        ret = dump_partition_table(buf, size);

done:
        free(buf);
        return (ret == 0);
}

static int cmdh_read_partition(int argc, char *argv[])
{
        unsigned int addr;
        const char *fname = argv[2];
        unsigned int size = 0;
        uint8_t *buf = NULL;
        unsigned int id;
        int ret;

        if (!is_valid_partition_name(argv[0], &id)) {
                if (!get_number(argv[0], &id) || !is_valid_partition_id(id)) {
                        prog_print_err("invalid partition name/id or selected partition doesn't "
                                                                                        "exist\n");
                        return 0;
                }
        }

        if (!get_number(argv[1], &addr)) {
                prog_print_err("invalid address\n");
                return 0;
        }

        if (!get_number(argv[3], &size) || !size) {
                prog_print_err("invalid size\n");
                return 0;
        }

        if (size > get_partition_size(id)) {
                prog_print_err("invalid size exceeding partition size\n");
                return 0;
        }

        if (!strcmp(fname, "-") || !strcmp(fname, "--")) {
                buf = malloc(size);
                if (!buf) {
                        ret = ERR_ALLOC_FAILED;
                } else {
                        ret = prog_read_partition(id, addr, buf, size);
                }
        } else {
                ret = prog_read_patrition_to_file(id, addr, fname, size);
        }

        if (ret) {
                free(buf);
                prog_print_err("read from partition failed (%d)\n", ret);
                return 0;
        }

        if (buf) {
                dump_hex(addr, buf, size, !strcmp(fname, "--") ? 32 : 16);
                free(buf);
        }

        return 1;
}

static int cmdh_write_partition(int argc, char *argv[])
{
        unsigned int addr;
        const char *fname = argv[2];
        unsigned int size = 0;
        unsigned int id;
        int ret;

        if (!is_valid_partition_name(argv[0], &id)) {
                if (!get_number(argv[0], &id) || !is_valid_partition_id(id)) {
                        prog_print_err("invalid partition name/id\n");
                        return 0;
                }
        }

        if (!get_number(argv[1], &addr)) {
                prog_print_err("invalid address\n");
                return 0;
        }

        if (argc > 3) {
                if (!get_number(argv[3], &size) || !size) {
                        prog_print_err("invalid size\n");
                        return 0;
                }
        } else {
                int file_size = get_filesize(fname);

                if (file_size < 0) {
                        prog_print_err("could not open file\n");
                        return 0;
                }

                size = file_size;
        }

        if (size > get_partition_size(id)) {
                prog_print_err("invalid size exceeding partition size\n");
                return 0;
        }

        ret = prog_write_file_to_partition(id, addr, fname, size);
        if (ret) {
                prog_print_err("write to partition failed: %s (%d)\n", prog_get_err_message(ret),
                                                                                                ret);
                return 0;
        }

        return 1;
}

static int cmdh_write_partition_bytes(int argc, char *argv[])
{
        unsigned int addr;
        unsigned int id;
        int ret = 0;
        int i;
        unsigned int b;
        uint8_t *buf = (uint8_t *) malloc(argc - 2);

        if (buf == NULL) {
                return ERR_ALLOC_FAILED;
        }

        if (!is_valid_partition_name(argv[0], &id)) {
                if (!get_number(argv[0], &id) || !is_valid_partition_id(id)) {
                        prog_print_err("invalid partition name/id\n");
                        goto end;
                }
        }

        if (!get_number(argv[1], &addr)) {
                prog_print_err("invalid address %s\n", argv[0]);
                goto end;
        }

        for (i = 2; i < argc; ++i) {
                if (!get_number(argv[i], &b)) {
                        prog_print_err("invalid byte '%s'\n", argv[i]);
                        goto end;
                }
                buf[i - 2] = (uint8_t) b;
        }

        ret = prog_write_partition(id, addr, buf, argc - 2);
        if (ret) {
                prog_print_err("write to partition failed: %s (%d)\n", prog_get_err_message(ret),
                                                                                                ret);
                ret = 0;
                goto end;
        }
        ret = 1;

end:
        free(buf);

        return ret;
}

static int cmdh_copy_qspi(int argc, char *argv[])
{
        unsigned int addr_ram;
        unsigned int addr_qspi;
        unsigned int size;
        int ret;

        if (!get_number(argv[0], &addr_ram)) {
                prog_print_err("invalid RAM address\n");
                return 0;
        }

        if (!get_number(argv[1], &addr_qspi)) {
                prog_print_err("invalid QSPI address\n");
                return 0;
        }

        if (!get_number(argv[2], &size)) {
                prog_print_err("invalid size\n");
                return 0;
        }

        ret = prog_copy_to_qspi(addr_ram, addr_qspi, size);
        if (ret) {
                prog_print_err("copy to QSPI failed: %s (%d)\n", prog_get_err_message(ret), ret);
                return 0;
        }

        return 1;
}

static int cmdh_is_empty_qspi(int argc, char *argv[])
{
        unsigned int size = DEFAULT_SIZE;
        unsigned int start_address = 0;
        int ret_number;
        int ret = 0;

        if (argc != 0 && argc != 2) {
                prog_print_err("invalid argument - function is_empty_qspi needs zero or two"
                                                                                "arguments\n");
                return 0;
        }

        if (argc == 2) {
                if (!get_number(argv[0], &start_address)) {
                        prog_print_err("invalid start address\n");
                        return 0;
                }

                if (!get_number(argv[1], &size) || size == 0) {
                        prog_print_err("invalid size\n");
                        return 0;
                }
        }

        ret = prog_is_empty_qspi(size, start_address, &ret_number);

        if (!ret) {
                if (ret_number <= 0) {
                        prog_print_log("QSPI flash region is not empty (byte at 0x%08x + 0x%08x"
                                                "is not 0xFF).\n", start_address, (-1 * ret_number));
                } else {
                        prog_print_log("QSPI flash region is empty (checked %u bytes).\n", ret_number);
                }
                ret = 1;
        } else {
                prog_print_err("check QSPI emptiness failed: %s (%d)\n", prog_get_err_message(ret),
                                                                                        ret);
                ret = 0;
        }

        return ret;
}

static int cmdh_chip_erase_qspi(int argc, char *argv[])
{
        int ret;

        ret = prog_chip_erase_qspi();

        if (ret) {
                prog_print_err("chip erase QSPI failed: %s (%d)\n", prog_get_err_message(ret), ret);
                return 0;
        }

        return 1;
}

static int cmdh_write_otp(int argc, char *argv[])
{
        unsigned int addr;
        unsigned int length = 0;
        uint32_t *buf = NULL;
        int i;
        int ret = 0;
        const char *chip_rev;
        const prog_memory_sizes_t *sizes;

        if (!get_number(argv[0], &addr) || !check_otp_cell_address(&addr)) {
                prog_print_err("invalid address\n");
                goto done;
        }

        if (!get_number(argv[1], &length) || !length) {
                prog_print_err("invalid length\n");
                goto done;
        }

        argc -= 2;
        argv += 2;

        prog_get_chip_rev(&chip_rev);
        prog_get_memory_sizes(chip_rev, &sizes);
        if (length > sizes->otp_size >> 2) {
                prog_print_err("invalid length exceeding OTP size\n");
                goto done;
        }

        if ((buf = (uint32_t *) calloc(length, sizeof(*buf))) == NULL) {
                goto done;
        }

        for (i = 0; i < argc && i < length; i++) {
                if (!get_number(argv[i], &buf[i])) {
                        prog_print_err("invalid data (#%d)\n", i + 1);
                        goto done;
                }
        }

        ret = prog_write_otp(addr, buf, length);
        if (ret) {
                prog_print_err("write to OTP failed: %s (%d)\n", prog_get_err_message(ret), ret);
                ret = 0;
                goto done;
        }

        ret = 1;

done:
        if (buf) {
                free(buf);
        }
        return ret;
}

static int cmdh_read_otp(int argc, char *argv[])
{
        unsigned int addr;
        unsigned int length = 0;
        uint32_t *buf = NULL;
        int ret = 0;
        const char *chip_rev;
        const prog_memory_sizes_t *sizes;

        if (!get_number(argv[0], &addr) || !check_otp_cell_address(&addr)) {
                prog_print_err("invalid address\n");
                goto done;
        }

        if (!get_number(argv[1], &length) || !length) {
                prog_print_err("invalid length\n");
                goto done;
        }

        prog_get_chip_rev(&chip_rev);
        prog_get_memory_sizes(chip_rev, &sizes);
        if (length > sizes->otp_size >> 2) {
                prog_print_err("invalid length exceeding OTP size\n");
                goto done;
        }

        if ((buf = (uint32_t *) calloc(length, sizeof(*buf))) == NULL) {
                goto done;
        }

        ret = prog_read_otp(addr, buf, length);
        if (ret) {
                prog_print_err("read from OTP failed: %s (%d)\n", prog_get_err_message(ret), ret);
                goto done;
        }

        dump_otp(addr, buf, length);
        ret = 1;
done:
        if (buf) {
                free(buf);
        }
        return ret;
}
static bool write_otp_file_value_cb(uint32_t addr, uint32_t size, uint64_t value)
{
        uint8_t *buf;
        unsigned int i;
        int ret;

        buf = calloc(1, size);
        if (!buf) {
                return false;
        }

        memcpy(buf, &value, size < sizeof(value) ? size : sizeof(value));

        prog_print_log("write_otp %04x %d ", addr, size);
        for (i = 0; i < size; i++) {
                prog_print_log("%02X", buf[i]);
        }

        ret = prog_write_otp(addr, (void *) buf, size / 4);
        if (ret < 0) {
                prog_print_log(" (FAILED: %s (%d))\n", prog_get_err_message(ret), ret);
                free(buf);
                return false;
        }

        free(buf);
        prog_print_log(" (OK)\n");

        return true;
}

int cmdh_write_otp_file(int argc, char *argv[])
{
        return parse_otp_file(argv[0], write_otp_file_value_cb);
}

static int cmdh_write_otp_raw_file(int argc, char *argv[])
{
        unsigned int addr;
        const char *fname = argv[1];
        unsigned int size = 0;
        int ret;

        if (!get_number(argv[0], &addr) || !check_otp_cell_address(&addr)) {
                prog_print_err("invalid address\n");
                return 0;
        }

        if (argc > 2) {
                if (!get_number(argv[2], &size)) {
                        prog_print_err("invalid size\n");
                        return 0;
                }
        } else {
                int file_size = get_filesize(fname);

                if (file_size < 0) {
                        prog_print_err("could not open file\n");
                        return 0;
                }

                size = file_size;
        }

        ret = prog_write_file_to_otp(addr, fname, size);
        if (ret) {
                prog_print_err("write to OTP failed: %s (%d)\n", prog_get_err_message(ret), ret);
                return 0;
        }

        return 1;
}

static bool read_otp_file_value_cb(uint32_t addr, uint32_t size, uint64_t value)
{
        uint8_t *buf;
        int ret;

        buf = calloc(1, size);
        if (!buf) {
                return false;
        }

        prog_print_log("read_otp %04x %d ", addr, size);

        ret = prog_read_otp(addr, (void *) buf, size / 4);
        if (ret < 0) {
                prog_print_log(" (FAILED: %s (%d))\n", prog_get_err_message(ret), ret);
                free(buf);
                return false;
        }

        prog_print_log(" (OK)\n");

        dump_otp(addr, (void *) buf, size / 4);

        free(buf);

        return true;
}

int cmdh_read_otp_file(int argc, char *argv[])
{
        return parse_otp_file(argv[0], read_otp_file_value_cb);
}

static int cmdh_write_tcs(int argc, char *argv[])
{
        unsigned int length;
        uint32_t *buf = NULL;
        unsigned int i;
        uint32_t address;
        int ret = 0;

        if (!get_number(argv[0], &length) || !length) {
                prog_print_err("invalid length\n");
                goto done;
        }

        argc -= 1;
        argv += 1;

        if (argc != length) {
                prog_print_err("invalid length. provided data does not match length\n");
                goto done;
        }
        if (length & 0x01) {
                prog_print_err("invalid length. TCS entries need to be in pairs\n");
                goto done;
        }
        if (length > TCS_WORD_SIZE) {
                prog_print_err("invalid length. length is bigger than TCS size\n");
                goto done;
        }

        length <<= 1;

        if ((buf = (uint32_t *) calloc(length, sizeof(*buf))) == NULL) {
                goto done;
        }

        /*Create data + complement data for TCS*/
        for (i = 0; i < (unsigned int)argc; i++) {
                if (!get_number(argv[i], &buf[2*i])) {
                        prog_print_err("invalid data (#%d)\n", i + 1);
                        goto done;
                }
                buf[2*i+1]=~buf[2*i];//calculate the complement
        }

        ret = prog_write_tcs(&address, buf, length);
        if (ret) {
                prog_print_err("write to OTP TCS failed: %s (%d)\n", prog_get_err_message(ret), ret);
                goto done;
        }
        prog_print_log("TCS contents written: \n");
        dump_otp(address, buf, length);
        ret = 1;
done:
        if (buf) {
                free(buf);
        }
        return ret;
}

static int set_executable_from_file(const char *file_name)
{
        FILE *f;
        struct stat st;
        int err = 0;

        if (file_name == NULL || stat(file_name, &st) < 0) {
                return ERR_FILE_OPEN;
        }

        f = fopen(file_name, "rb");
        if (f == NULL) {
                return ERR_FILE_OPEN;
        }

        executable_bin = realloc(executable_bin, st.st_size);
        if (executable_bin == NULL) {
                err = ERR_ALLOC_FAILED;
                goto end;
        }

        if (st.st_size != fread(executable_bin, 1, st.st_size, f)) {
                err = ERR_FILE_READ;
                goto end;
        }

        if (st.st_size > 0) {
                executable_size = st.st_size;
        } else {
                err = ERR_FILE_EMPTY;
        }
end:
        if (f != NULL) {
                fclose(f);
        }

        return err;
}

int cmdh_boot(int argc, char *argv[])
{
        int ret;

        ret = set_executable_from_file(argv[0]);
        if (ret < 0) {
                prog_print_err("failed to set executable file: %s (%d)\n",
                                                                prog_get_err_message(ret), ret);
                return ret;
        }

        ret = prog_boot(executable_bin, executable_size);
        if (ret < 0) {
                prog_print_err("failed to boot executable: %s (%d)\n",
                                                                prog_get_err_message(ret), ret);
                return ret;
        }

        return 1;
}

int cmdh_run(int argc, char *argv[])
{
        int ret;

        ret = set_executable_from_file(argv[0]);
        if (ret < 0) {
                prog_print_err("failed to set executable file: %s (%d)\n",
                                                                prog_get_err_message(ret), ret);
                return ret;
        }

        ret = prog_run(executable_bin, executable_size);
        if (ret < 0) {
                prog_print_err("failed to run executable: %s (%d)\n",
                                                                prog_get_err_message(ret), ret);
                return ret;
        }

        return 1;
}

int cmdh_read_chip_info(int argc, char *argv[])
{
        chip_info_t chip_info;
        int ret;

        if (argc >= 1 && !strcmp(argv[0], "simple")) {
                char chip_rev[CHIP_REV_STRLEN];
                int ret;

                ret = prog_gdb_read_chip_rev(chip_rev);

                if (ret < 0) {
                        prog_print_err("failed to read chip revision: %s (%d)\n",
                                                                prog_get_err_message(ret), ret);
                        return 0;
                }

                prog_print_log("CHIP REVISION: %s\n", chip_rev);

                return 1;
        }

        ret = prog_read_chip_info(&chip_info);
        if (ret < 0) {
                prog_print_err("failed to read chip info: %s (%d)\n", prog_get_err_message(ret), ret);
                return 0;
        }

        prog_print_log("CHIP INFO:\n"
                "chip_revision = %s\n"
                "chip_id (as read from otp) = %s\n"
                "chip_package = %s\n",
                chip_info.chip_rev, chip_info.chip_otp_id, chip_info.chip_package);
        return 1;
}

int handle_command(char *cmd, int argc, char *argv[])
{
        struct cli_command *cmdh = cmds;

        /* lookup command handler */
        while (cmdh->name && strcmp(cmdh->name, cmd)) {
                cmdh++;
        }

        /* handlers table is terminated by empty entry, so name == NULL means no handler found */
        if (!cmdh->name) {
                prog_print_err("invalid command\n");
                return 1;
        }

        if (argc < cmdh->min_num_p) {
                prog_print_err("not enough parameters\n");
                return 1;
        }

        /*
         * return value from handler (0=failure) will be used as exit code so need to do change it
         * here to have exit code 0 on success
         */
        return !cmdh->func(argc, argv);
}
