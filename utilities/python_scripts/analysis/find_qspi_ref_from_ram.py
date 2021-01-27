#!/usr/bin/env python

#
# Copyright (C) 2019 Dialog Semiconductor.
# This computer program includes Confidential, Proprietary Information
# of Dialog Semiconductor. All Rights Reserved.
#

from __future__ import print_function
import subprocess
import sys
import re


def addr_in_ram(x):
    return (x >= 0x20000000 and x < 0x28000000)


def addr_in_flash(x):
    return (x < 0x10000000 and x >= 0)


def print_report(r):
    for line in r:
        print(line)
    print('')


def find_ref_target(st, ref):
    target = st.get(ref)
    if target is None and (ref & 1 != 0):
        # clear LSB and search again, function pointers have LSB set due to THUMB
        target = st.get(ref & ~1)

    return target


def analyze_symbol(st, addr, symbol, f):
    if not addr_in_ram(addr):
        return False, False

    s = 'Function ' + symbol + ' @ ' + hex(addr)
    report = [s]
    report.append('-' * len(s))

    valid_ref = False
    for line in f:
        line = line.strip()

        if len(line) == 0:
            break

        if '.word' in line:
            tokens = line.split()
            ref = int(tokens[3], 16)
            if addr_in_flash(ref):
                target = find_ref_target(st, ref)
                if target is None:
                    target = '???'
                else:
                    valid_ref = True
                report.append('  [' + tokens[0].rstrip(':').rjust(8) + '] ---> \t'
                        + hex(ref).rjust(10) + ' \t(symbol ' + target + ')')

    if len(report) > 2:
        print_report(report)
        print()
        return True, valid_ref
    else:
        return False, False


def get_symbol_table(st, fname):
    p = subprocess.Popen(['arm-none-eabi-nm', '-nC', fname],
            stdout = subprocess.PIPE, stderr = subprocess.PIPE,
            universal_newlines = True)

    for line in p.stdout:
        if not line:
            break

        line = line.strip()
        tokens = line.split()
        st[int(tokens[0], 16)] = tokens[2]

    p.wait()
    if p.returncode == 0:
        return True
    else:
        for line in p.stderr:
            print(line)

        return False


def usage():
    print('Usage: ' + sys.argv[0] + ' <ELF file>')
    print('Analyse the provided ELF and report potential references of')
    print('QSPI addresses from code located in RAM.')
    print()
    print('The following memory mappings are assumed:')
    print(' 0x00000000 - 0x0FFFFFFF:  QSPI flash adddress space')
    print(' 0x20000000 - 0x27FFFFFF:  RAM adddress space')
    print()
    print('arm-none-eabi-nm and arm-none-eabi-objdump are used by this tool,')
    print('so their directory must be in PATH.')



if len(sys.argv) != 2:
    usage()
    exit(3)

symbol_table = {}
if not get_symbol_table(symbol_table, sys.argv[1]):
    usage()
    exit(1)

p = subprocess.Popen(['arm-none-eabi-objdump', '-dC', sys.argv[1]],
        stdout = subprocess.PIPE, stderr = subprocess.PIPE,
        universal_newlines = True)

count = 0
valid_count = 0
for line in p.stdout:
    if not line:
        break

    line = line.strip()
    if line.endswith('>:'):
        tokens = line.split()
        addr = int(tokens[0], 16)
        symbol = re.search('<(.+?)>', tokens[1]).group(1)
        found_ref, valid_ref = analyze_symbol(symbol_table, addr, symbol, p.stdout)
        if found_ref:
            count = count + 1
            if valid_ref:
                valid_count = valid_count + 1

p.wait()
if p.returncode != 0:
    for line in p.stderr:
        print(line)
    usage()
    exit(2)

print('Detected ' + str(count) + ' suspicious functions.')
print(str(valid_count) + ' functions have identified references to code/data in QSPI.')

exit(0)

