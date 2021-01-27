#!/usr/bin/env python3
#########################################################################################
# Copyright (C) 2015-2019 Dialog Semiconductor.
# This computer program includes Confidential, Proprietary Information
# of Dialog Semiconductor. All Rights Reserved.
#########################################################################################

import os
import sys

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
sys.path.append(PROJECT_ROOT)

from api import ui
from api.cli_programmer import CliProgrammer
from api.utils import SDK_ROOT
from api.script_base import run_script, ScriptArgumentsParser
from qspi.prepare_local_ini_file import prepare_local_ini_file
from qspi.program_qspi_config import ProgramQspiConfig, program_qspi_config, ProductId
from qspi.create_nvparam import create_nvparam

NVPARAM_ADDERESS_680 = 0x80000
NVPARAM_ADDERESS_690_4MB = 0x1FF000
NVPARAM_ADDERESS_690_1MB = 0x070000


def program_qspi_nvparam(project_output_path, cfg=None, device_id=None, arm_gcc_path=None):
    config = ProgramQspiConfig()
    while not config.is_valid():
        program_qspi_config()
        config = ProgramQspiConfig()

    if config.product_id == ProductId.DA1469x_00:
        if config.flash_size == 0x400000:
            nvparam_address = NVPARAM_ADDERESS_690_4MB
        else:
            nvparam_address = NVPARAM_ADDERESS_690_1MB
    elif config.product_id == ProductId.DA14681_01 or config.product_id == ProductId.DA14683_00:
        nvparam_address = NVPARAM_ADDERESS_680
    else:
        raise RuntimeError("Unknown product ID")

    if not os.path.exists(project_output_path):
        raise RuntimeError("Build the desired project first")

    cli_programmer = CliProgrammer(cfg_path=cfg, prod_id=config.product_id, jlink_id=device_id)

    if not cfg:
        prepare_local_ini_file(cfg=cli_programmer.get_cfg(), device_id=device_id)

    project_path = os.path.join(project_output_path, '..')

    create_nvparam(project_output_path, [project_path + "/config", SDK_ROOT + "/sdk/middleware/adapters/include"],
                   arm_gcc_path)

    nvparam_bin = os.path.join(project_output_path, 'nvparam.bin')
    cli_programmer.write_qspi(nvparam_address, nvparam_bin)


def parse_args():
    parser = ScriptArgumentsParser()
    parser.add_argument('--cfg', metavar='<cfg_path>', type=str, help='config path')
    parser.add_argument('--id', metavar='<serial_number>', type=str, help='device serial number')
    parser.add_argument('--arm_gcc_path', metavar='<arm_gcc_path>', type=str,
                        help='path to cross compiler')
    parser.add_argument('path', metavar='<project_output_path>', type=str,
                        help='folder which contains the binary you want to program')
    args = parser.parse_args()
    return args.path, args.cfg, args.id, args.arm_gcc_path


if __name__ == '__main__':
    ui.print_header('PROGRAM QSPI NVPARAM')
    run_script(program_qspi_nvparam, parse_args)
    ui.print_footer('FINISHED')
