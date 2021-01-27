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
from api.script_base import run_script, ScriptArgumentsParser, prepare_cli_programmer_ini, ProductId


def prepare_local_ini_file(cfg=None, prod_id=None, device_id=None, port=None, log=None,
                           target_reset_cmd=None, jlink_path=None):
    prepare_cli_programmer_ini(cfg=cfg, prod_id=prod_id, device_id=device_id, port=port, log=log,
                               target_reset_cmd=target_reset_cmd, jlink_path=jlink_path)


def parse_args():
    parser = ScriptArgumentsParser()
    parser.add_argument('--cfg', type=str, help='config path')
    parser.add_argument('--id', type=str, help='device serial number')
    parser.add_revision_args()
    parser.add_argument('--port', type=int, help='GDB port')
    parser.add_argument('--log', type=str, help='JLink log')
    parser.add_argument('--trc', type=str,
                        help='target reset command')
    parser.add_argument('--jlink_path', type=str, help='JLink path')
    args = parser.parse_args()

    return args.cfg, args.prod_id, args.id, args.port, args.log, args.trc, args.jlink_path


if __name__ == '__main__':
    run_script(prepare_local_ini_file, parse_args)
