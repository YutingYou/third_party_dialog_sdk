#!/usr/bin/env python3
#########################################################################################
# Copyright (C) 2015-2019 Dialog Semiconductor.
# This computer program includes Confidential, Proprietary Information
# of Dialog Semiconductor. All Rights Reserved.
#########################################################################################

import os
from subprocess import call
import subprocess
import sys

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
sys.path.append(PROJECT_ROOT)

from api.script_base import run_script, ScriptArgumentsParser
from api.utils import SDK_ROOT

DIR_OUT = ''
DIR_NVPARAM = os.path.join(SDK_ROOT, 'utilities', 'nvparam')
NVPARAM_BIN = ''
CUSTOM_CFG_PATH = 0

def find_toolchain(cc_path):
    arm_none_eabi = "arm-none-eabi-gcc"

    if cc_path:
        return os.path.join(cc_path, arm_none_eabi)

    if sys.platform.startswith('linux'):
        arm_toolchain = subprocess.check_output("${ARM_TOOLCHAIN}", shell=True).decode('utf-8')
        cross = subprocess.check_output(
            "which arm-none-eabi-gcc", shell=True).strip().decode('utf-8')
    else:
        arm_none_eabi += ".exe"
        arm_toolchain = "%ARM_TOOLCHAIN%"
        cross = subprocess.check_output(
            "where arm-none-eabi-gcc", shell=True).strip().decode('utf-8').split('\r\n')[0]

    arm_toolchain_paths = [
        os.path.join(arm_toolchain, "bin", arm_none_eabi),
        os.path.join(arm_toolchain, arm_none_eabi),
        cross
    ]

    for atp in arm_toolchain_paths:
        if os.path.isfile(atp):
            return atp

    raise RuntimeError("Cannot find arm-none-eabi-gcc, check your PATH or ARM_TOOLCHAIN settings!")


def create_nvparam(project_path, include_paths, cc_path=None):
    includes = []
    DIR_OUT = project_path
    NVPARAM_BIN = os.path.join(project_path, "nvparam.bin")

    cross = find_toolchain(cc_path)

    APP_NVPARAM_H = include_paths[CUSTOM_CFG_PATH] + "/app_nvparam.h"
    APP_NVPARAM_VAL_H = include_paths[CUSTOM_CFG_PATH] + "/app_nvparam_values.h"

    if os.path.exists(APP_NVPARAM_H) and os.path.exists(APP_NVPARAM_VAL_H):
        NVPARAM_DEF = "-Ddg_configNVPARAM_APP_AREA=1"
    else:
        NVPARAM_DEF = "-Ddg_configNVPARAM_APP_AREA=0"

    for inc_path in include_paths:
        includes.append("-I")
        includes.append(inc_path)

    if call([cross, "-c", NVPARAM_DEF] + includes + ["-o", os.path.join(DIR_OUT, "nvparam-symbols.o"),
                                        os.path.join(DIR_NVPARAM, "symbols.c")]):
        raise RuntimeError("Failed to create nvparam-symbols.o")

    if call([cross, "-E", "-P", "-c", NVPARAM_DEF] + includes + ["-o",
                                                    os.path.join(DIR_OUT, "nvparam-sections.ld"),
                                                    os.path.join(DIR_NVPARAM, "sections.ld.h")]):
        raise RuntimeError("Failed to create nvparam-sections.ld")

    if call([cross, "--specs=nano.specs", "--specs=nosys.specs", "-T",
             os.path.join(DIR_OUT, "nvparam-sections.ld"), "-o",
             os.path.join(DIR_OUT, "nvparam.elf"),
             os.path.join(DIR_OUT, "nvparam-symbols.o")]):
        raise RuntimeError("Failed to create nvparam.elf")

    cross = cross.replace("-gcc", "-objcopy")

    if call([cross, "-O", "binary", "-j", ".nvparam",
             os.path.join(DIR_OUT, "nvparam.elf"), NVPARAM_BIN]):
        raise RuntimeError("Failed to create nvparam.bin")


def prepare_args():
    parser = ScriptArgumentsParser()
    parser.add_argument('out_path', metavar='<output_path>', type=str, help='output path')
    parser.add_argument('inc_paths', metavar='<include_paths>', type=str, nargs='+',
                        help='paths to look for nvparam config files')
    parser.add_argument('--arm_gcc_path', type=str, help='path to cross compiler')
    args = parser.parse_args()
    return args.out_path, args.inc_paths, args.arm_gcc_path


if __name__ == '__main__':
    print("Creating nvparam.bin")
    run_script(create_nvparam, prepare_args)
    print("Successfully created")
