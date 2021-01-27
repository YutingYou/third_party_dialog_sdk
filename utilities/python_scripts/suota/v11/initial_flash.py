#!/usr/bin/env python3
#########################################################################################
# Copyright (C) 2015-2019 Dialog Semiconductor.
# This computer program includes Confidential, Proprietary Information
# of Dialog Semiconductor. All Rights Reserved.
#########################################################################################

import argparse
import os
import sys

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '../..'))
sys.path.append(PROJECT_ROOT)

from api import ui
from api.cli_programmer import CliProgrammer
from api.utils import SDK_ROOT
from api.script_base import run_script, ScriptArgumentsParser, select_jlink_serial
from qspi.prepare_local_ini_file import prepare_local_ini_file
from qspi.program_qspi_config import ProgramQspiConfig, ProductId
from qspi.program_qspi import program_qspi
from qspi.reboot_device import reboot_device
from secure_image.generate_keys import ProductKeys
from suota.v11.mkimage import mkimage


SUOTA_LOADER_PATH = os.path.join(SDK_ROOT, 'sdk/bsp/system/loaders/ble_suota_loader')
QSPI_SCRIPTS_PATH = os.path.join(SDK_ROOT, 'utilities/python_scripts/qspi')
CONFIG_FILE = os.path.join(QSPI_SCRIPTS_PATH, 'program_qspi.ini')
IMAGE_FILE = 'application_image.img'
VERSION_FILE = 'sw_version.h'

NVMS_PARTITION_TABLE = 0x07F000
NVMS_IMAGE_HEADER_PART = 0x01F000
NVMS_FW_EXEC_PART = 0x020000

PRODUCT_READY_ADDR = 0x7F8E9D0
SECURE_DEVICE_ADDR = 0x7F8EA68


def initial_flash(binary, bootloader=None, nobootloader=None, cfg=None, device_id=None,
                  jlink_path=None, secure_config=None, keys=None, prod_id=None):

    config = ProgramQspiConfig()
    if prod_id is None:
        if config.is_valid():
            prod_id = config.product_id
        else:
            raise RuntimeError('Product ID was not passed and the configuration file is not valid.')

    if not device_id:
        device_id = select_jlink_serial(jlink_path)

    cli = CliProgrammer(cfg_path=cfg, prod_id=prod_id)

    if not cfg:
        prepare_local_ini_file(cfg=cli.get_cfg(), prod_id=prod_id,
                               device_id=device_id, jlink_path=jlink_path)

    if not os.path.exists(binary):
        raise RuntimeError('Binary file {} does not exist'.format(binary))

    if secure_config is not None and not os.path.exists(secure_config):
        raise RuntimeError('Configuration file {} does not exist.\n'
                           'Run secure_keys_cfg.py to create one'.format(secure_config))

    if secure_config and not keys:
        raise RuntimeError('Product keys file not passed - secure image cannot be created.')

    ui.print_message('Using SDK from {}'.format(SDK_ROOT))
    ui.print_message('cli_programmer from {}'.format(cli.get_path()))
    ui.print_message('image file {}'.format(binary))

    if prod_id == ProductId.DA1469x_00:
        ui.print_title('Erasing product header area')
        cli.erase_qspi(0, 4096)

        ui.print_title('Erasing partition table')
        cli.erase_qspi(NVMS_PARTITION_TABLE, 4096)

        program_qspi(binary, cli, prog_prod_header=True)

    elif prod_id == ProductId.DA14683_00 or prod_id == ProductId.DA14681_01:
        if not (bootloader or nobootloader):
            bootloader_bin = 'ble_suota_loader.bin'
            suota_build_config = \
                prod_id.value + '-Release_' + ('OTP_Secure' if secure_config else 'QSPI')
            bootloader = os.path.join(SUOTA_LOADER_PATH, suota_build_config, bootloader_bin)
            ui.print_message('boot loader {}'.format(bootloader))

        ui.print_title('Preparing image file {}'.format(IMAGE_FILE))

        if secure_config:
            mkimage(binary, IMAGE_FILE, prod_id, VERSION_FILE, security_config=secure_config,
                    product_keys=keys)
        else:
            mkimage(binary, IMAGE_FILE, prod_id)

        if not nobootloader:
            ui.print_title('Erasing bootloader area')
            cli.erase_qspi(0, 4096)

        ui.print_title('Erasing partition table')
        cli.erase_qspi(NVMS_PARTITION_TABLE, 4096)

        ui.print_title('Writing application image {}'.format(binary))
        cli.write_qspi(NVMS_FW_EXEC_PART, binary)

        ui.print_title('Writing image header {}'.format(IMAGE_FILE))
        cli.write_qspi(NVMS_IMAGE_HEADER_PART, IMAGE_FILE, 1024)
        os.remove(IMAGE_FILE)

        if not nobootloader:
            ui.print_title('Writing bootloader')
            if secure_config:
                cli.write_otp_exec(bootloader)
            else:
                program_qspi(bootloader, cli)

        if secure_config:
            if keys:
                prod_keys = ProductKeys(keys)

                if not prod_keys.is_valid(ProductId.DA14683_00):
                    raise RuntimeError('Product key file is not valid.')

                ui.print_title("Writing symmetric keys")
                for index, key in enumerate(prod_keys.symmetric_keys):
                    cli.write_key_sym(index, key)

                ui.print_title("Writing asymmetric keys")
                for index, key in enumerate(prod_keys.asymmetric_keys):
                    cli.write_key_asym(index, key.public)

            # Write 0xAA to the product ready and the secure device field in OTP header
            cli.write_otp(PRODUCT_READY_ADDR, 1, 0xAA)
            cli.write_otp(SECURE_DEVICE_ADDR, 1, 0xAA)

        reboot_device(jlink_number=device_id, jlink_path=jlink_path)
    else:
        raise RuntimeError('Unsupported device selected ({}).'.format(prod_id))


def parse_args():
    parser = ScriptArgumentsParser()

    bootloader_group = parser.add_argument_group('bootloader options')
    bootloader_group = bootloader_group.add_mutually_exclusive_group()
    bootloader_group.add_argument('-b', '--bootloader', metavar='<bootloader>', type=str,
                                  help='path to bootloader')
    bootloader_group.add_argument('--nobootloader', action='store_true',
                                  help='do not flash bootloader')

    config_group = parser.add_argument_group('configuration options')
    config_group.add_argument('--cfg', metavar='<config_path>', type=str, help='configuration file')
    config_group.add_argument('--id', metavar='<serial_number>', type=str, dest='device_id',
                              help='device serial number')
    config_group.add_argument('--jlink_path', metavar='<jlink_path>', type=str,
                              help='path to jlink')

    security_group = parser.add_argument_group('security options')
    security_group.add_argument('--secure_config', metavar='<security_cfg_path>',
                                dest='secure_config', help='path to secure config')
    security_group.add_argument('--keys', metavar='<keys_path>', dest='keys',
                                help='path to keys to be written to OTP. This file is required '
                                     'also for secure image generation.')

    parser.add_argument('binary', metavar='<app_binary>', type=str, help='path to binary to flash')
    parser.add_revision_args()
    args = parser.parse_args()
    return args.binary, args.bootloader, args.nobootloader, args.cfg, args.device_id, \
           args.jlink_path, args.secure_config, args.keys, args.prod_id


if __name__ == '__main__':
    ui.print_header('INITIAL FLASH')
    run_script(initial_flash, parse_args)
    ui.print_footer("FINISHED")
