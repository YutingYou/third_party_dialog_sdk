#########################################################################################
# Copyright (C) 2016-2020 Dialog Semiconductor.
# This computer program includes Confidential, Proprietary Information
# of Dialog Semiconductor. All Rights Reserved.
#########################################################################################

import argparse
import os
import sys
import re
from collections import OrderedDict


def section_contents(lines):
    name = None
    for l in lines:
        if re.match(r"^ \*fill\*\s+[0-9A-Fa-fx]+\s+[0-9A-F-a-fx]+\s*$", l):
            yield l.split() + ["fills"]
        # Full object file info is within the line
        elif re.match(r"^ [\.\w]+\s+[0-9A-Fa-fx]+\s+[0-9A-Fa-fx]+\s+[:\\\s\(\)\-\./\w]+.o(?:bj)?[\)]*\s*$", l):
            m = re.match(r"^ ([\.\w]+)\s+([0-9A-Fa-fx]+)\s+([0-9A-Fa-fx]+)\s+([:\\\s\(\)\-\./\w]+.o(?:bj)?[\)]*)\s*$", l)
            yield list(m.groups())
        # Name is too big so it appears in a single line by its own
        elif re.match(r"^ [\.\w]+\s*$", l):
            name = l.strip()
        # The remainder of the above name
        elif re.match(r"^ {16}[0-9A-Fa-fx]+\s+[0-9A-Fa-fx]+\s+[:\\\s\(\)\-\./\w]+.o(?:bj)?[\)]*\s*$", l):
            m = re.match(r"^ {16}([0-9A-Fa-fx]+)\s+([0-9A-Fa-fx]+)\s+([:\\\s\(\)\-\./\w]+.o(?:bj)?[\)]*)\s*$", l)
            to_yield = [name]
            name = None
            yield to_yield + list(m.groups())
        # Linker stubs
        elif re.match(r"^ ([\.\w]+)\s+[0-9A-Fa-fx]+\s+[0-9A-Fa-fx]+\s+linker\s+stubs\s*$", l):
            m = re.match(r"^ ([\.\w]+)\s+([0-9A-Fa-fx]+)\s+([0-9A-Fa-fx]+)\s+(linker\s+stubs)\s*$", l)
            yield list(m.groups())
        # Remaining part of linker stubs
        elif re.match(r"^ {16}[0-9A-Fa-fx]+\s+[0-9A-Fa-fx]+\s+linker\s+stubs\s*$", l):
            m = re.match(r"^ {16}([0-9A-Fa-fx]+)\s+([0-9A-Fa-fx]+)\s+(linker\s+stubs)\s*$", l)
            to_yield = [name]
            name = None
            yield to_yield + list(m.groups())


def add_section(section_data, name, address, size, lines):
    if name in section_data:
        print("Error: Duplicate section detected. Check the map file and if it's fine, please report a bug!")
        sys.exit(1)
    section_data[name] = {"address": address,
                          "size": size,
                          "contents": []}
    for cont in section_contents(lines):
        section_data[name]["contents"].append(cont)


def section_groups(lines):
    section_group = []
    for l in lines:
        if re.match(r"^\.[a-zA-Z]", l):
            if section_group:
                yield section_group
            section_group = [l]
        elif section_group:
            section_group.append(l)
    yield section_group


def read_section_details(section_group):
    title = section_group[0].split()
    start_of_contents = 2
    if len(title) == 1:
        if len(title[0]) > 14:
            title += section_group[1].split()
            start_of_contents = 2
        else:
            title = (None, None, None)
    return title[0], title[1], title[2], section_group[start_of_contents:]


def generate_report(map_file_name):
    section_data = OrderedDict()
    lines = read_lines(map_file_name)
    for section_group in section_groups(lines):
        name, address, size, remaining_lines = read_section_details(section_group)
        if name is not None:
            add_section(section_data, name, address, size, remaining_lines)
    return section_data

def find_regions(map_file_name):
    regions = {}
    with open(map_file_name, 'r') as f:
        lines = iter(f.readlines())

        for line in lines:
            # seek to section
            if ("Memory Configuration" in line):
                break

        for line in lines:
            if '*default*' in line: # Last region
                break
            check = re.match(r'(.+?)\s+(0[xX][0-9a-fA-F]+)\s+(0[xX][0-9a-fA-F]+)\s', line)
            if check:
                name, address, size = check.groups()
                start_address = int(address, 16)
                end_address = start_address + int(size, 16)
                regions[name] = (start_address, end_address)

    return regions

# Careful when modifying. Tho order of elif matters because each makes assumptions about the previous not being true.
def classify_by_object_file(object_name):
    if "arm-none-eabi" in object_name and re.match(r".*/crt.+.o", object_name) or \
            re.match(r"^\./startup/\w+\.o", object_name):
        classify_by_object_file.classification = "startup"
    elif "arm-none-eabi" in object_name:
        classify_by_object_file.classification = "other"
    elif re.match(r"^\./sdk/(\w+)/[/\w]*\w+\.o", object_name):
        sdk_match = re.match(r"^\./sdk/(\w+)/[/\w]*\w+\.o", object_name)
        classify_by_object_file.classification = sdk_match.groups()[0]
    elif re.search(r"(lib\w+[\.\-/\w]+)\.a", object_name) and re.search(r"[/\\]sdk[/\\]", object_name):
        sdk_lib_match = re.search(r"(lib\w+[\.\-/\w]+)\.a", object_name)
        classify_by_object_file.classification = sdk_lib_match.groups()[0]
    elif object_name == "fills":
        pass
    elif re.match(r"^(\.*/|[A-Za-z]:\\)[\w/\\\.]+", object_name) and not re.search(r"ltrans\.o$", object_name):
        classify_by_object_file.classification = "application"
    elif re.match(r"^(linker\s+stubs)", object_name):
        classify_by_object_file.classification = "linker_stubs"
    else:
        classify_by_object_file.classification = "unknown"
    return classify_by_object_file.classification

classify_by_object_file.classification = None


def analyse_section_with_contents(section, contents, modules, subsections):
    """
    Analyze section with content, add subsections to sw modules and extract subsections details
    :param section: The section name
    :param contents: section content
    :param modules: The current modules dictionary to populate
    :param subsections: dictionary of (subsection_name : size) data
    """
    for [sub_section, address, size, object_name] in contents:
            sw_module = classify_by_object_file(object_name)
            if sw_module not in modules:
                modules[sw_module] = OrderedDict()
            if object_name not in modules[sw_module]:
                modules[sw_module][object_name] = []
            modules[sw_module][object_name].append([section, int(size, 16), sub_section, address])
            if sub_section in subsections:
                subsections[sub_section] += int(size, 16)
            else:
                subsections[sub_section] = int(size, 16)


def analyse_section_without_contents(section, section_data, modules, subsections):
    """
    Sections without contents are assumed to be defined in the liner script (can't be elsewhere, can it...?)
    :param section: The section name
    :param section_data: The data of the section (address, contents, size). Contents is expected to be either empty
     or full of "fills".
    :param modules: The current modules dictionary to populate
    :param subsections: dictionary of (subsection_name : size) data
    """
    if "linker_script" not in modules:
        modules["linker_script"] = OrderedDict()
    if "declarations" not in modules["linker_script"]:
        modules["linker_script"]["declarations"] = []
    fills_size = sum([int(sz, 16) for sub, add, sz, o_n in section_data["contents"]])
    modules["linker_script"]["declarations"].append([section, int(section_data["size"], 16) - fills_size, "N/A",
                                                     section_data["address"]])
    if "N/A" in subsections:
        subsections["N/A"] += int(section_data["size"], 16)
    else:
        subsections["N/A"] = int(section_data["size"], 16)

    if fills_size > 0:
        if "fills" not in modules["linker_script"]:
            modules["linker_script"]["fills"] = []
        for sub_section, address, size, object_name in section_data["contents"]:
            modules["linker_script"]["fills"].append([section, int(size, 16), sub_section, address])
            if sub_section in subsections:
                subsections[sub_section] += int(size, 16)
            else:
                subsections[sub_section] = int(size, 16)


def analyse_modules(section_data_dict, subsections_nb=1, hide_empty_sections=False):
    """
    This method finds all object files mentioned in each section. It classifies every object file into a specific sw
    module. It then creates a dictionary of modules. The data of each module key is another dictionary that has object
    files as keys and a list of [section, size, sub_section, address] lists

    :param section_data_dict: The section data structure created by the first read of the map file
    :param subsections_nb: If section contains less than subsections_nb, tuples (section, subsect1) (section, subsect2)
    will be added to return list
    :param hide_empty_sections: Hide empty sections flag
    :return: A list of tuples containing sections that were found and a dictionary of sw modules and their data.
    If there were subsections filtered with subsections_nb param, typles (section, subsect1), (section, subsect2)...
    will be present on a list. If no subsection was filteres, tuple (section, None) will be appended to list
    """
    modules = {}
    seen_sections = OrderedDict()
    for section in section_data_dict:
        if section not in seen_sections:
            seen_sections[section] = OrderedDict()
        if [sub for sub, add, sz, o_n in section_data_dict[section]["contents"] if sub != "*fill*"]:
            analyse_section_with_contents(section, section_data_dict[section]["contents"], modules,
                                                                                                seen_sections[section])
        else:
            analyse_section_without_contents(section, section_data_dict[section], modules, seen_sections[section])

    seen_sections_filtered = []
    for section in seen_sections:
        if hide_empty_sections:
            section_size = 0
            for subsection in seen_sections[section]:
                section_size += seen_sections[section][subsection]

            if section_size is 0:
                # ignore this section
                continue

        if len(seen_sections[section]) >= subsections_nb or len(seen_sections[section]) in [0, 1] or\
            len(seen_sections[section]) == 1:
            seen_sections_filtered.append((section, None))
        else:
            seen_sections_filtered = seen_sections_filtered + [(section, subsection) for subsection in\
                                                               seen_sections[section]]

    return seen_sections_filtered, modules

def format_module_data(sections, module_data):
    object_file_data = OrderedDict()
    for object_file in module_data:
        if object_file not in object_file_data:
            object_file_data[object_file] = [0] * len(sections)
        for entry in module_data[object_file]:
            # Try append size to (section, subsection). Otherwise, put it to (section, None)
            if (entry[0], entry[2]) in sections:
                object_file_data[object_file][sections.index((entry[0], entry[2]))] += entry[1]
            elif (entry[0], None) in sections:
                object_file_data[object_file][sections.index((entry[0], None))] += entry[1]
            else:
                # This subsection was skipped for some reason. most likely, it's size is zero
                if entry[1] is not 0:
                    raise Exception('No section {} to assign the {} subsection'.format(entry[0],
                                                                                       entry[2]))
    return object_file_data


def get_non_zero_sections(sections):
    return list(filter(lambda x: int(sections[x]["size"], 16) != 0, sections))


def read_lines(file_name):
    with open(file_name, 'r') as f:
        lines = f.readlines()
    start_line = 0
    end_line = 0
    while re.match(r"^Linker script and memory map", lines[start_line]) is None:
        start_line += 1
    start_line += 1
    while not lines[start_line].split():
        start_line += 1
    start_line += 1
    while lines[start_line].split():
        start_line += 1
    while re.match(r"^OUTPUT.*elf\B", lines[end_line]) is None:
        end_line += 1
    return lines[start_line:end_line]


def map_file_exists(file_path):
    # Check if file exists
    if not os.path.isfile(file_path):
        print("%s does not exist" % file_path)
        return False
    return True


def parse_args():
    parser = argparse.ArgumentParser(description='Memory utilization report generator for SDK applications.')
    parser.add_argument('map_file')
    parser.add_argument('-sub_nb', metavar='subsection_number', type=int, default=0,
                        help='Max number of subsections in report')
    parser.add_argument('-hide_empty_sections', action='store_true', help='Hide empty sections')

    return parser.parse_args()


if __name__ == '__main__':
    args = parse_args()
    if map_file_exists(args.map_file) is False:
        print("Error while parsing arguments")
        sys.exit(1)
    SECTION_DATA = generate_report(args.map_file)
    for sec in SECTION_DATA:
        if int(SECTION_DATA[sec]["size"], 16) != 0:
            print("%s at %s with size %d (%s)" % (sec, SECTION_DATA[sec]["address"], int(SECTION_DATA[sec]["size"], 16),
                                                  SECTION_DATA[sec]["size"]))
