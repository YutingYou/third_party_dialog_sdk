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
from api.script_base import run_script, ScriptArgumentsParser, ExecutionCanceled, ProductId


def erase_qspi_jtag(cli_programmer=None, serial=None):
    if ui.ask(text='Are you sure you want to completely erase QSPI?') is False:
        raise ExecutionCanceled()
    cli_programmer.chip_erase_qspi(serial_port=serial, silent=False)


def parse_args():
    parser = ScriptArgumentsParser()
    parser.add_interface_args()
    args = parser.parse_args()

    cli, serial = parser.get_interface_from_args(args)
    return cli, serial


if __name__ == '__main__':
    ui.print_header('ERASE QSPI')
    run_script(erase_qspi_jtag, parse_args)
    ui.print_footer('FINISHED')
