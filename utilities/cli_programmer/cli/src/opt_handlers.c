/**
 ****************************************************************************************
 *
 * @file opt_handlers.c
 *
 * @brief Handling of CLI options provided on command line
 *
 * Copyright (C) 2015-2020 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cli_common.h"
#include "cli_config_parser.h"
#include <programmer.h>
#ifdef WIN32
#define strdup _strdup
#endif

#define BAUDRATE        115200

struct cli_options main_opts = {
        /* .initial_baudrate = */ BAUDRATE,
        /* .uartboot_config = */ {
                /* .baudrate = */ 1000000,
                /* .baudrate_patch = */ 1,      // always patch (baudrate was increased)
                /* .tx_port = */ 0,
                /* .tx_port_patch = */ 0,
                /* .tx_pin = */ 9,
                /* .tx_pin_patch = */ 0,
                /* .rx_port = */ 0,
                /* .rx_port_patch = */ 0,
                /* .rx_pin = */ 8,
                /* .rx_pin_patch = */ 0,
        },
        /* .timeout = */ 5000,
        /* This field should point on dynamic allocated memory or NULL */
        /* .bootloader_fname = */ NULL,
        /* .gdb_server_config = */ {
                /* .port = */ 2331,
                /* This field should point on dynamic allocated memory or NULL */
                /* .host_name = */ NULL,
                /* This field should point on dynamic allocated memory or NULL */
                /* .gdb_server_path = */ NULL,
                /* .no_kill_gdb_server = */ NO_KILL_MODE_NONE,
                /* .connect_gdb_server = */ true,
                /* .check_bootloader = */ false,
        },
        /*.config_file_path  = */ NULL,
        /* .chip_rev = */ NULL,
        /* .target_reset_cmd  = */ NULL,
};

void set_str_opt(char **opt, const char *val)
{
        if (*opt) {
                free(*opt);
                *opt = NULL;
        }
        if (val) {
                *opt = strdup(val);
        }
}

static void opth_help(void)
{
        printf("usage: cli_programmer [-h] [--cfg <config_file>]\n"
                "                      [-s <baudrate>] [-i <baudrate>] \n"
                "                      [-p <port_num>] [-r <host>]\n"
                "                      [--tx-port <port_num>] [--tx-pin <pin_num>] \n"
                "                      [--rx-port <port_num>] [--rx-pin <pin_num] [-w timeout] \n"
                "                      [--no-kill [mode]] [--gdb-cmd <cmd>] \n"
                "                      [--trc <cmd>] \n"
                "                      [--save-ini] \n"
                "                      [--save <config_file>]\n"
                "                      [--prod-id <id>]\n"
                "                      [-b file] \n"
                "                      [--check-booter-load]\n");
        printf("                       <interface> <command> [<args>]\n");
        printf("\n");
        printf("interface can be gdbserver or serial port name\n");
        printf("\n");
        printf("options:\n");
        printf("    -h                     Print this message\n");
        printf("    --cfg <config-file>    Use options from user specified file.\n"
                "                           Default file can be created with --save option.\n"
                "                           Options from this file override any options that\n"
                "                           were specified on command line before --cfg option\n"
                "                           it is best to put this option as the first option.\n");
        printf("    -i <baudrate>          Initial baud rate used for uploading uartboot or a \n"
                "                           user supplied binary. This depends on the rate used \n"
                "                           by the boot loader of the device. The default \n"
                "                           behavior is to use the value passed by '-s' \n"
                "                           (see below) or its default, if the parameter is not \n"
                "                           given. This option is ignored by the \"boot\" \n"
                "                           command. '-s' option should be used in this case. \n");
        printf("    -s <baudrate>          Baud rate used for UART by uartboot. This parameter \n"
                "                           is patched to the uploaded uartboot binary (in that \n"
                "                           way passed as a parameter). The default value is %d.\n",
                main_opts.uartboot_config.baudrate);
        printf("    --tx-port <port_num>   GPIO port used for UART Tx by uartboot. This \n"
                "                           parameter is patched to the uploaded uartboot binary \n"
                "                           (in that way passed as a parameter). Default value \n"
                "                           is %d. This argument is ignored when the 'boot' \n"
                "                           command is given (see below).\n",
                main_opts.uartboot_config.tx_port);
        printf("    --tx-pin <pin_num>     GPIO pin used for UART Tx by uartboot. This \n"
                "                           parameter is patched to the uploaded uartboot binary \n"
                "                           (in that way passed as a parameter). Default value \n"
                "                           is %d. This argument is ignored when the 'boot' \n"
                "                           command is given (see below).\n",
                main_opts.uartboot_config.tx_pin);
        printf("    --rx-port <port_num>   GPIO port used for UART Rx by uartboot. This \n"
                "                           parameter is patched to the uploaded uartboot binary \n"
                "                           (in that way passed as a parameter). Default value \n"
                "                           is %d. This argument is ignored when the 'boot' \n"
                "                           command is given (see below).\n",
                main_opts.uartboot_config.rx_port);
        printf("    --rx-pin <pin_num>     GPIO pin used for UART Rx by uartboot. This \n"
                "                           parameter is patched to the uploaded uartboot binary \n"
                "                           (in that way passed as a parameter). Default value \n"
                "                           is %d. This argument is ignored when the 'boot' \n"
                "                           command is given (see below).\n",
                main_opts.uartboot_config.rx_pin);
        printf("    --prod-id <id>         Chip product id (in the form of DAxxxxx-yy). \n");
        printf("    -w <timeout>           Serial port communication timeout.\n");
        printf("    -r <host>              Gdb server host (default: localhost).\n");
        printf("    -p <port>              Gdb server port (default: 2331).\n");
        printf("    --gdb-cmd <cmd>        Gdb server start command. Must be used if there is \n"
                "                           a need to start the gdb server instance on host. \n"
                "                           Without this parameter any gdb server instance won't \n"
                "                           be started or stopped.\n");
        printf("    --no-kill [mode]       Don't stop running GDB Server instances. Modes: \n"
                "                           '0' Stop GDB Server instances during init. and closing \n"
                "                           '1': Don't stop GDB Server during initialization \n"
                "                           '2': Don't stop GDB Server during closing \n"
                "                           '3' or none: Don't stop any GDB Server instance \n");
        printf("    --trc <cmd>            Target reset command. Must be used if there is \n"
                "                           a need to replace the default localhost reset command. \n"
                "                           This option shouldn't be used with '--check-booter-load' \n"
                "                           option.\n");
        printf("    -b <file>              Filename of custom 2nd stage bootloader. \n"
                "                           Filename 'attach' is reserved for attaching to \n"
                "                           target (no reset, no bootloader loading).\n");
        printf("                           If this parameter is not specified built-in version of \n"
                "                           uartboot.bin is used.\n");
        printf("    --save-ini             Save CLI programmer configuration to cli_programmer.ini \n"
                "                           file.\n");
        printf("    --save <config_file>   Save CLI programmer configuration to user specified file.\n");
        printf("    --check-booter-load    Don't force bootloader loading if it is running on the \n"
                "                           platform already. This option shouldn't be used with \n"
                "                           '--trc' option.\n");
        printf("\n");
        printf("commands:\n");
        printf("    write <address> <file> [<size>]\n");
        printf("        writes up to <size> bytes of <file> into RAM memory at <address>\n");
        printf("        if <size> is omitted, complete file is written\n");
        printf("    read <address> <file> <size>\n");
        printf("        reads <size> bytes from RAM memory starting at <address> into <file>\n");
        printf("        if <file> is specified as either '-' or '--', data is output to stdout \n"
                "        as hexdump\n");
        printf("        hexdump is either 16-bytes (-) or 32-bytes (--) wide\n");
        printf("    write_qspi <address> <file> [<size>]\n");
        printf("        writes up to <size> bytes of <file> into QSPI flash at <address>\n");
        printf("        if <size> is omitted, complete file is written\n");
        printf("    write_qspi_bytes <address> <data1> [<data2> [...]]\n");
        printf("        writes bytes specified on command line into QSPI flash at <address>\n");
        printf("    read_qspi <address> <file> <size>\n");
        printf("        reads <size> bytes from QSPI memory starting at <address> into <file>\n");
        printf("        if <file> is specified as either '-' or '--', data is output to stdout \n"
                "        as hexdump\n");
        printf("        hexdump is either 16-bytes (-) or 32-bytes (--) wide\n");
        printf("    erase_qspi <address> <size>\n");
        printf("        erases <size> bytes of QSPI flash starting at <address>\n");
        printf("        note: actual area erased may be different due to size of erase block\n");
        printf("    chip_erase_qspi\n");
        printf("        erases whole QSPI flash\n");
        printf("    copy_qspi <address_ram> <address_qspi> <size>\n");
        printf("        copies <size> bytes from RAM memory starting at <address_ram> to QSPI \n"
                "       flash\n");
        printf("        at <address_flash>\n");
        printf("    is_empty_qspi [start_address size]\n");
        printf("        checks that <size> bytes of QSPI flash contain only 0xFF values, starting\n"
                "       from <start_address>. Default value of <size> is 1 MB and default value of\n"
                "       <start_address> is 0.\n");
        printf("    read_partition_table\n");
        printf("        reads the partition table (if any) and prints its contents\n");
        printf("    read_partition <part_name|part_id> <address> <file> <size>\n");
        printf("        reads <size> bytes from partition <part_name> or <part_id> starting\n"
                "        at <address> into <file>\n");
        printf("        If `file` is specified as either '-' or '--', data is output to stdout\n"
                "        as hexdump\n");
        printf("        hexdump is either 16-bytes (-) or 32-bytes (--) wide\n");
        printf("        .___________________________._________.\n");
        printf("        |         part_name         | part_id |\n");
        printf("        |---------------------------|---------|\n");
        printf("        |NVMS_FIRMWARE_PART         |    1    |\n");
        printf("        |NVMS_PARAM_PART            |    2    |\n");
        printf("        |NVMS_BIN_PART              |    3    |\n");
        printf("        |NVMS_LOG_PART              |    4    |\n");
        printf("        |NVMS_GENERIC_PART          |    5    |\n");
        printf("        |NVMS_PLATFORM_PARAMS_PART  |    15   |\n");
        printf("        |NVMS_PARTITION_TABLE       |    16   |\n");
        printf("        |NVMS_FW_EXEC_PART          |    17   |\n");
        printf("        |NVMS_FW_UPDATE_PART        |    18   |\n");
        printf("        |NVMS_PRODUCT_HEADER_PART   |    19   |\n");
        printf("        |NVMS_IMAGE_HEADER_PART     |    20   |\n");
        printf("        '---------------------------'---------'\n");
        printf("    write_partition <part_name|part_id> <address> <file> [<size>]\n");
        printf("        writes up to <size> bytes of <file> into NVMS partition <part_name>\n"
                "        or <part_id>, according to the above table, at <address>\n");
        printf("        if <size> is omitted, complete file is written\n");
        printf("        if <file> is specified as either '-' or '--', data is output to stdout \n"
                "        as hexdump\n");
        printf("        hexdump is either 16-bytes (-) or 32-bytes (--) wide\n");
        printf("    write_partition_bytes <part_name|part_id> <address> <data1> [<data2> [...]]\n");
        printf("        writes bytes specified on command line into NVMS partition <part_name>\n"
                "        or <part_id>, according to the above table, at <address>\n");
        printf("    write_otp <address> <length> [<data> [<data> [...]]]\n");
        printf("        writes <length> words to OTP at <address>\n");
        printf("        <data> are 32-bit words to be written, if less than <length> words are \n"
                "        specified,\n");
        printf("        remaining words are assumed to be 0x00\n");
        printf("    write_otp_raw_file <address> <file> [<size>]\n");
        printf("        writes up to <size> bytes of <file> into OTP at <address>\n");
        printf("        if <size> is omitted, the complete file is written\n");
        printf("        remaining bytes in the last word are set to 0x00\n");
        printf("    read_otp <address> <length>\n");
        printf("        reads <length> 32-bit words from OTP address <address>\n");
        printf("    write_otp_file <file>\n");
        printf("        writes data to OTP as defined in <file> (default values specified are \n"
                "        written)\n");
        printf("    read_otp_file <file>\n");
        printf("        reads data from OTP as defined in <file> (cells with default value \n"
                "        provided are read)\n");
        printf("        contents of each cell is printed to stdout\n");
        printf("    write_tcs <length> [<reg_addr> <reg_data> [<reg_addr> <reg_data>  [...]]]\n");
        printf("        writes <length> 64-bit words to OTP TCS section at first available (filled with 0) "
                        "section of size <length>\n");
        printf("        <reg_address>: the register address. It will be written as a 64bit word "
                        "[<reg_address>, ~<reg_address>] \n");
        printf("        <reg_data>: the register data. It will be written as a 64bit word "
                        "[<reg_data>, ~<reg_data>] \n");
        printf("    boot <binary_file>\n");
        printf("        boot application binary using 1st stage bootloader (ROM booter) then exit.\n"
               "        If the application will be too big (more than 64kB) and serial interface is\n"
               "        used then \"run\" command should be executed instead.\n");
        printf("    run <binary_file>\n");
        printf("        run application binary using 2nd stage bootloader (uartboot) then exit.\n");
        printf("    read_chip_info [simple]\n");
        printf("        reads chip information. When 'simple' mode is used, only the chip info \n"
                "        stored in registers is returned (no OTP header read).. 'simple' mode is \n"
                "        available with GDB Server interface only. 'simple' mode should be used with \n"
                "        '-b attach' option for best performance.\n");
        printf("\n");
        printf("examples:\n");
        printf("Upload \"test_api.bin\" program to RAM and run it, using UART Tx/Rx \n "
                "P1_0/P1_5 (uses boot rom booter baud rate at 57600):\n");
        printf("> cli_programmer COM40 boot test_api.bin\n\n");

        printf("Upload \"test_api.bin\" program to RAM and run it, using UART Tx/Rx \n "
                "P1_3/P2_3 (uses boot rom booter baud rate at 9600):\n");
        printf("> cli_programmer -s 9600 COM40 boot test_api.bin\n\n");

        printf("Run a few commands with uartboot, using UART Tx/Rx P1_0/P1_5 at baud rate \n"
                "115200 (initial rate for uartboot uploading must be 57600):\n");
        printf("> cli_programmer -i 57600 -s 115200 COM40 write_qspi 0x0 data_i\n");
        printf("> cli_programmer -i 57600 -s 115200 COM40 read_qspi 0x0 data_o 0x100\n\n");

        printf("Run a few commands with uartboot, using UART Tx/Rx P1_3/P2_3 at baud rate \n"
                "115200 (initial rate for uartboot uploading must be 9600):\n");
        printf("> cli_programmer -i 9600 -s 115200 --tx-port 1 --tx-pin 3 --rx-port 2 --rx-pin 3 \n"
                "                 COM40 write_qspi 0x0 data_i\n");
        printf("> cli_programmer -i 9600 -s 115200 --tx-port 1 --tx-pin 3 --rx-port 2 --rx-pin 3 \n"
                "                 COM40 read_qspi 0x0 data_o 0x100\n\n");

        printf("Read qspi flash contents (10 bytes at address 0x0) \n ");
        printf("Start gdbserver manually !\n");
        printf("> cli_programmer gdbserver read_qspi 0 -- 10 \n\n");
        printf("Write register 0x50003000 with value 0x00FF and register 0x50003002 with value 0x00AA \n ");
        printf("> cli_programmer gdbserver write_tcs 4 0x50003000 0x00FF 0x50003002 0x00AA \n\n");

        printf("Write settings to the cli_programmer.ini file.\n");
        printf("> cli_programmer -b c:\\users\\jon\\sdk\\bsp\\system\\loaders\\uartboot\\Release\\uartboot.bin "
                "--save-ini --gdb-cmd \"\\\"C:\\Program Files\\SEGGER\\JLink_V510d\\JLinkGDBServerCL.exe\\\" "
                "-if SWD -device Cortex-M0 -singlerun -silent -speed auto\"\n\n");
        printf("Write 6 bytes specified in command line to flash at address 0x80000\n");
        printf("> cli_programmer gdbserver write_qspi_bytes 0x80000 0x11 0x22 0x33 0x44 0x55 0x66\n\n");
}

static int opth_baudrate(const char * param)
{
        return get_number(param, &main_opts.uartboot_config.baudrate);
}

static int opth_initial_baudrate(const char *param)
{
        return get_number(param, &main_opts.initial_baudrate);
}

static int opth_timeout(const char *param)
{
        return get_number(param, &main_opts.timeout);
}

static int opth_gdb_port(const char *param)
{
        return get_number(param, &main_opts.gdb_server_config.port);
}

static int opth_tx_port(const char * param)
{
        return get_number(param, &main_opts.uartboot_config.tx_port);
}

static int opth_tx_pin(const char * param)
{
        return get_number(param, &main_opts.uartboot_config.tx_pin);
}

static int opth_rx_port(const char * param)
{
        return get_number(param, &main_opts.uartboot_config.rx_port);
}

static int opth_rx_pin(const char * param)
{
        return get_number(param, &main_opts.uartboot_config.rx_pin);
}

static int opth_bootloader(const char *param)
{
        set_str_opt(&main_opts.bootloader_fname, param);

        return 1;
}

int handle_option(char opt, const char *param)
{
        int ret = 0;

        switch (opt) {
        case '?':
        case 'h':
                opth_help();
                return 0;
        case 's':
                if (!param || !opth_baudrate(param)) {
                        prog_print_err("invalid baudrate\n");
                        return -1;
                }
                main_opts.uartboot_config.baudrate_patch = 1;
                ret = 1;
                break;
        case 'i':
                if (!param || !opth_initial_baudrate(param)) {
                        prog_print_err("invalid initial baudrate\n");
                        return -1;
                }
                ret = 1;
                break;
        case 'w':
                if (!param || !opth_timeout(param)) {
                        prog_print_err("invalid timeout\n");
                        return -1;
                }
                ret = 1;
                break;
        case 'b':
                if (!param || !opth_bootloader(param)) {
                        prog_print_err("invalid bootloader filename\n");
                        return -1;
                }
                ret = 1;
                break;
        case 'p':
                if (!param || !opth_gdb_port(param)) {
                        prog_print_err("invalid gdbserver port\n");
                        return -1;
                }
                ret = 1;
                break;
        case 'r':
                if (!param) {
                        prog_print_err("invalid gdbserver host name\n");
                        return -1;
                }

                set_str_opt(&main_opts.gdb_server_config.host_name, param);
                ret = 1;
                break;
        default:
                prog_print_err("invalid parameter -%c\n", opt);
                return -1;
        }

        return ret;
}

int handle_long_option(const char * opt, const char * param) {
        char chip_rev[CHIP_REV_STRLEN] = {0};
        char *param_canonical = NULL;
        uint32_t len = 0;

        if (!strcmp(opt, "tx-port")) {
                if (!param || !opth_tx_port(param)) {
                        prog_print_err("invalid tx port\n");
                        return -1;
                }
                main_opts.uartboot_config.tx_port_patch = 1;
                return 1;
        }
        if (!strcmp(opt, "tx-pin")) {
                if (!param || !opth_tx_pin(param)) {
                        prog_print_err("invalid tx pin\n");
                        return -1;
                }
                main_opts.uartboot_config.tx_pin_patch = 1;
                return 1;
        }
        if (!strcmp(opt, "rx-port")) {
                if (!param || !opth_rx_port(param)) {
                        prog_print_err("invalid rx port\n");
                        return -1;
                }
                main_opts.uartboot_config.rx_port_patch = 1;
                return 1;
        }
        if (!strcmp(opt, "rx-pin")) {
                if (!param || !opth_rx_pin(param)) {
                        prog_print_err("invalid rx pin\n");
                        return -1;
                }
                main_opts.uartboot_config.rx_pin_patch = 1;
                return 1;
        }
        if (!strcmp(opt, "gdb-cmd")) {
                if (!param) {
                        prog_print_err("invalid gdbserver command\n");
                        return -1;
                }

                set_str_opt(&main_opts.gdb_server_config.gdb_server_path, param);
                return 1;
        }
        if (!strcmp(opt, "no-kill")) {
                if (!param) {
                        prog_print_err("invalid no-kill command\n");
                        return -1;
                }
                if (get_number(param, &main_opts.gdb_server_config.no_kill_gdb_server)) {
                        if (main_opts.gdb_server_config.no_kill_gdb_server < NO_KILL_MODE_NONE ||
                                main_opts.gdb_server_config.no_kill_gdb_server > NO_KILL_MODE_ALL) {
                                /* Wrong value */
                                return -1;
                        }

                        return 1;
                }

                main_opts.gdb_server_config.no_kill_gdb_server = NO_KILL_MODE_ALL;
                return 0;
        }
        if (!strcmp(opt, "trc")) {
                if (!param) {
                        prog_print_err("invalid target reset command\n");
                        return -1;
                }

                set_str_opt(&main_opts.target_reset_cmd, param);
                return 1;
        }
        if (!strcmp(opt, "save-ini")) {
                set_str_opt(&main_opts.config_file_path, "cli_programmer.ini");

                return 0;
        }
        if (!strcmp(opt, "save")) {
                if (!param) {
                        prog_print_err("invalid configuration file path\n");
                        return -1;
                }
                len = strlen(param);
                if (len > MAX_CLI_CONFIG_FILE_PATHNAME_LEN) {
                        prog_print_err("too long configuration file path\n");
                        return -1;
                }
                param_canonical = (char *) calloc(len + 1,sizeof(char));
                if (!param_canonical) {
                        prog_print_err("out of memory\n", param);
                        return -1;
                }
                if (!cli_config_canonicalize_file_name(param_canonical, param)) {
                        prog_print_err("non-canonical file given %s\n", param);
                        free(param_canonical);
                        return -1;
                }

                set_str_opt(&main_opts.config_file_path, param_canonical);
                free(param_canonical);
                return 1;
        }
        if (!strcmp(opt, "cfg")) {
                if (!param) {
                        prog_print_err("invalid configuration file path\n");
                        return -1;
                }
                len = strlen(param);
                if (len > MAX_CLI_CONFIG_FILE_PATHNAME_LEN) {
                        prog_print_err("too long configuration file path\n");
                        return -1;
                }
                param_canonical = (char *) calloc(len + 1,sizeof(char));
                if (!param_canonical) {
                        prog_print_err("out of memory\n", param);
                        return -1;
                }
                if (!cli_config_canonicalize_file_name(param_canonical, param)) {
                        prog_print_err("non-canonical or not existing file given %s\n", param);
                        free(param_canonical);
                        return -1;
                }
                if (!cli_config_load_from_ini_file(param_canonical, &main_opts)) {
                        prog_print_err("failed to read configuration from %s (%s)\n",
                                        param, param_canonical);
                        free(param_canonical);
                        return -1;
                }
                free(param_canonical);
                return 1;
        }
        if (!strcmp(opt, "prod-id")) {
                if ((!param) || prog_map_product_id_to_chip_rev((char*)param, chip_rev)) {
                        prog_print_err("invalid chip product id\n");
                        return -1;
                }
                set_str_opt(&main_opts.chip_rev, chip_rev);

                return 1;
        }
        if (!strcmp(opt, "check-booter-load")) {
                main_opts.gdb_server_config.check_bootloader = true;
                return 0;
        }
        prog_print_err("invalid parameter --%s\n", opt);
        return -1;
}
