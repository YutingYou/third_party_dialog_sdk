#!/usr/bin/env python
#########################################################################################
# Copyright (C) 2019 Dialog Semiconductor.
# This computer program includes Confidential, Proprietary Information
# of Dialog Semiconductor. All Rights Reserved.
#########################################################################################

import argparse
import os
from collections import OrderedDict

import map_reader

ROW_WIDTH = 24


def diff_process(diff_dict, module_data, method):
    for sw_module in module_data:
        for object_file in module_data[sw_module]:
            for entry in module_data[sw_module][object_file]:
                if entry[0] not in diff_dict:
                    diff_dict[entry[0]] = OrderedDict()

                if entry[2] not in diff_dict[entry[0]]:
                    diff_dict[entry[0]][entry[2]] = 0

                diff_dict[entry[0]][entry[2]] = method(diff_dict[entry[0]][entry[2]], entry[1])


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Compare map files')
    parser.add_argument('map_file_a')
    parser.add_argument('map_file_b')
    parser.add_argument("--verbose", "-v", help="increase output verbosity", action="store_true")
    args = parser.parse_args()

    if not os.path.isfile(args.map_file_a):
        raise Exception('File \'{}\' does not exist.'.format(args.map_file_a))

    if not os.path.isfile(args.map_file_b):
        raise Exception('File \'{}\' does not exist.'.format(args.map_file_b))

    section_data_a = map_reader.generate_report(args.map_file_a)
    section_data_b = map_reader.generate_report(args.map_file_b)

    _, module_data_a = map_reader.analyse_modules(section_data_a)
    _, module_data_b = map_reader.analyse_modules(section_data_b)

    diff = OrderedDict()
    diff_process(diff, module_data_b, lambda a, b: a + b)
    diff_process(diff, module_data_a, lambda a, b: a - b)

    if args.verbose:
        fmt = "{{:<{width}.{width}}} {{:<{width}.{width}}} {{:<{width}.{width}}}".format(
            width=ROW_WIDTH)

        print(fmt.format("SECTION", "SUBSECTION", "DIFF"))

        for section in diff:
            for subsection in diff[section]:
                if diff[section][subsection] != 0:
                    print(fmt.format(section, subsection, "{:+}".format(diff[section][subsection])))
    else:
        fmt = "{{:<{width}.{width}}} {{:<{width}.{width}}}".format(width=ROW_WIDTH)

        print(fmt.format("SECTION", "DIFF"))

        for section in diff:
            section_diff = sum([diff[section][subsection] for subsection in diff[section]])
            if section_diff is not 0:
                print(fmt.format(section, "{:+}".format(section_diff)))
