#!/usr/bin/env python3
#########################################################################################
# Copyright (C) 2015-2019 Dialog Semiconductor.
# This computer program includes Confidential, Proprietary Information
# of Dialog Semiconductor. All Rights Reserved.
#########################################################################################

import os
import sys
import tempfile


PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
sys.path.append(PROJECT_ROOT)

from api import ui
from api.mkimage import Mkimage
from api.script_base import run_script, ScriptArgumentsParser
from suota.v11.mkimage import make_sw_version_file
from qspi.program_qspi_config import ProductId
from qspi.program_qspi import program_product_header, handle_qspi_config, FLASH_BASE_ADDRESS, \
    PRODUCT_HEADER_SIZE
from secure_image.secure_keys_cfg import SecurityConfig, secure_keys_cfg
from secure_image.generate_keys import ProductKeys

DEFAULT_PRODUCT_KEYS_FILE = os.path.abspath(os.path.join(PROJECT_ROOT, 'secure_image',
                                                         'product_keys.xml'))
DEFAULT_SECURE_CFG_FILE = os.path.abspath(os.path.join(PROJECT_ROOT, 'secure_image',
                                                       'secure_cfg.xml'))


def secure_img_prog(image_file, secure_cfg_file, keys_file, cli_programmer, serial=None,
                    header_config_path=None, flash_config_path=None):
    if image_file is None or not os.path.exists(image_file):
        raise RuntimeError("Application binary file not passed or not exist")

    qspi_config = handle_qspi_config(header_config_path, flash_config_path)

    if qspi_config.product_id != ProductId.DA1469x_00:
        raise RuntimeError("This script could be run only for {} ({} selected)".format(
            ProductId.DA1469x_00.value, qspi_config.product_id))

    if secure_cfg_file is None:
        secure_cfg_file = DEFAULT_SECURE_CFG_FILE

    if keys_file is None:
        keys_file = DEFAULT_PRODUCT_KEYS_FILE

    if not os.path.exists(secure_cfg_file) or not os.path.exists(keys_file):
        secure_keys_cfg(configuration_file=secure_cfg_file, keys_file=keys_file,
                        prod_id=qspi_config.product_id)

    secure_cfg = SecurityConfig(secure_cfg_file)
    product_keys = ProductKeys(keys_file)

    if not secure_cfg.is_valid():
        raise RuntimeError("Secure configuration is not valid.")

    if not product_keys.is_valid(secure_cfg.product_id):
        raise RuntimeError('Product keys file is not valid.')

    ui.print_message('Using product keys file: {}'.format(os.path.normpath(keys_file)))
    ui.print_message('Using secure configuration file: {}'.format(os.path.normpath(secure_cfg_file)))

    ui.print_title("Erasing product header area")
    cli_programmer.erase_qspi(FLASH_BASE_ADDRESS, PRODUCT_HEADER_SIZE, serial_port=serial)

    ui.print_title("Programming image")

    try:
        version_file = tempfile.mktemp()
        make_sw_version_file(version_file)

        try:
            output_image = tempfile.mktemp()
            asym_key = product_keys.asymmetric_keys[secure_cfg.security_pub_key_idx]
            sym_key = product_keys.symmetric_fw_dec_keys[secure_cfg.security_sym_key_idx]

            Mkimage().da1469x_secure(image_file, version_file, output_image, asym_key.private,
                                   secure_cfg.security_pub_key_idx, sym_key,
                                   secure_cfg.security_sym_key_idx, secure_cfg.security_nonce,
                                   secure_cfg.make_revocation_string())

            cli_programmer.write_qspi(qspi_config.update_image_address, output_image,
                                      serial_port=serial, silent=False)
        finally:
            os.remove(output_image)
    finally:
        os.remove(version_file)

    ui.print_title("Programming product header")
    ui.print_message("Using configuration:\n" + qspi_config.get_config_info())
    program_product_header(qspi_config, cli_programmer, serial)


def parse_args():
    parser = ScriptArgumentsParser()
    parser.add_interface_args()
    parser.add_argument('image_file', type=str, nargs='?', help='path to image')
    parser.add_argument('--secure_cfg', metavar='<file_path>', dest='secure_cfg_file',
                        help='path to DA1469x-00 secure configuration file')
    parser.add_argument('--keys', metavar='<file_path>', dest='keys_file',
                        help='path to DA1469x-00 keys file')
    parser.add_config_args()
    args = parser.parse_args()

    cli, serial = parser.get_interface_from_args(args)
    return args.image_file, args.secure_cfg_file, args.keys_file, cli, serial, args.header_config, \
           args.flash_config


if __name__ == '__main__':
    ui.print_header('PROGRAM SECURE IMAGE (QSPI)')
    run_script(secure_img_prog, parse_args)
    ui.print_footer('FINISHED')
