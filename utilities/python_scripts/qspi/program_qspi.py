#!/usr/bin/env python3
#########################################################################################
# Copyright (C) 2015-2019 Dialog Semiconductor.
# This computer program includes Confidential, Proprietary Information
# of Dialog Semiconductor. All Rights Reserved.
#########################################################################################

import os
import sys
import tempfile
import struct
import binascii


PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
sys.path.append(PROJECT_ROOT)

from api import ui
from api.mkimage import Mkimage
from api.script_base import run_script, ScriptArgumentsParser, ExecutionCanceled
from suota.v11.mkimage import make_sw_version_file
from qspi.program_qspi_config import ProgramQspiConfig, program_qspi_config, ProductId


PRODUCT_HEADER_SIZE = 4 * 0x400
FLASH_BASE_ADDRESS = 0x0
MIN_FW_ADDRESS = 0x2000


def make_product_header(active_fw_image_address, update_fw_image_address, flash_burstcmda_reg_value,
                        flash_burstcmdb_reg_value, flash_write_config_command):
    buff = b''
    buff += struct.pack(">2c", b'P', b'p')
    buff += struct.pack("<2I", active_fw_image_address, update_fw_image_address)
    buff += struct.pack("<2I", int(flash_burstcmda_reg_value), int(flash_burstcmdb_reg_value))
    buff += struct.pack(">H", 0xAA11)
    buff += struct.pack("H", len(flash_write_config_command))
    buff += flash_write_config_command
    buff += struct.pack("<H", binascii.crc_hqx(buff, 0xFFFF))
    buff += b'\xFF' * (PRODUCT_HEADER_SIZE - len(buff))

    return buff


def program_product_header(qspi_config, cli_programmer, serial):
    try:
        product_header_bin = tempfile.mktemp()

        with open(product_header_bin, 'wb') as f:
            f.write(make_product_header(
                active_fw_image_address=qspi_config.active_image_address,
                update_fw_image_address=qspi_config.update_image_address,
                flash_burstcmda_reg_value=qspi_config.flash_burstcmda_reg_value,
                flash_burstcmdb_reg_value=qspi_config.flash_burstcmdb_reg_value,
                flash_write_config_command=qspi_config.flash_write_config_command))

        # Store product header copy
        cli_programmer.write_qspi(FLASH_BASE_ADDRESS + PRODUCT_HEADER_SIZE, product_header_bin,
                                  PRODUCT_HEADER_SIZE, serial_port=serial)
        cli_programmer.write_qspi(FLASH_BASE_ADDRESS, product_header_bin,
                                  PRODUCT_HEADER_SIZE, serial_port=serial)
    finally:
        os.remove(product_header_bin)


def handle_qspi_config(header_config_path, flash_config_path):
    qspi_config = ProgramQspiConfig(config_path=header_config_path)

    if qspi_config.is_valid():
        if flash_config_path is not None:
            if not qspi_config.check_flash_config(flash_config_path):
                if ui.ask(text="Current flash config isn't listed on supported flash config list "
                               "({})\n Do you want to run configurator?\n"
                               .format(flash_config_path), confirmation='Run', denial='Ignore'):
                    program_qspi_config(header_config_path=header_config_path,
                                        flash_config_path=flash_config_path)

    else:  # invalid qspi config
        if header_config_path is None:
            if ui.ask(text='Invalid configuration file: {}\n Do you want to run configurator?\n'
                      .format(header_config_path),
                      confirmation='Run', denial='Abort'):
                program_qspi_config(header_config_path=header_config_path,
                                    flash_config_path=flash_config_path)
                qspi_config.load()
            else:
                ui.print_message("Aborted.\n")
                raise ExecutionCanceled()

    if not qspi_config.is_valid():
        raise RuntimeError("QSPI config is invalid.\nExecution aborted.\n")

    return qspi_config


def program_qspi(image_file, cli_programmer, serial=None, prog_prod_header=False,
                 header_config_path=None, flash_config_path=None, check_booter_load=True):

    qspi_config = handle_qspi_config(header_config_path, flash_config_path)

    if qspi_config.product_id == ProductId.DA1469x_00:
        if image_file is not None:
            ui.print_title("Programming image")

            if qspi_config.update_image_address < MIN_FW_ADDRESS:
                raise RuntimeError(
                    "FW cannot be written at address lower than 0x{:X} (0x{:X} given)".format(
                        MIN_FW_ADDRESS, qspi_config.update_image_address))

            try:
                version_file = tempfile.mktemp()
                make_sw_version_file(version_file)

                try:
                    output_image = tempfile.mktemp()
                    Mkimage().da1469x(image_file, version_file, output_image)
                    cli_programmer.write_qspi(qspi_config.update_image_address, output_image,
                                              check_booter_load=check_booter_load,
                                              serial_port=serial, silent=False)
                finally:
                    os.remove(output_image)
            finally:
                os.remove(version_file)

        if prog_prod_header:
            ui.print_title("Programming product header")
            ui.print_message("Using configuration:\n" + qspi_config.get_config_info())
            program_product_header(qspi_config, cli_programmer, serial)
    elif qspi_config.product_id == ProductId.DA14681_01 or \
            qspi_config.product_id == ProductId.DA14683_00:
        if prog_prod_header:
            ui.print_message("Product header can be programmed only on DA1469x-00 boards. Skipped.")

        if image_file is not None:
            ui.print_title("Programming image")
            cli_programmer.write_qspi_exec(image_file=image_file, serial_port=serial, silent=False)

    else:
        raise RuntimeError("No supported device selected ({})".format(qspi_config.product_id))

    if image_file is None and prog_prod_header is False:
        ui.print_message("No image, no header selected. Nothing to do.")


def parse_args():
    parser = ScriptArgumentsParser()
    parser.add_interface_args()
    parser.add_argument('--prod_header', '-ph', action='store_true',
                        help='program product header (DA1469x-00 only)')
    parser.add_argument('image_file', type=str, nargs='?', help='path to image')
    parser.add_argument('--no_check_booter_load', dest='check_booter_load', action='store_false', default=True,
                        help='Use this argument in order not to use --check-booter-load argument in writing of qspi')
    parser.add_config_args()
    args = parser.parse_args()

    cli, serial = parser.get_interface_from_args(args)

    return args.image_file, cli, serial, args.prod_header, args.header_config, args.flash_config,\
           args.check_booter_load


if __name__ == '__main__':
    ui.print_header('PROGRAM QSPI')
    run_script(program_qspi, parse_args)
    ui.print_footer('FINISHED')
