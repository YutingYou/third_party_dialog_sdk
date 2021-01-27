#!/usr/bin/env python3
#########################################################################################
# Copyright (C) 2015-2019 Dialog Semiconductor.
# This computer program includes Confidential, Proprietary Information
# of Dialog Semiconductor. All Rights Reserved.
#########################################################################################

import os
import re
import struct
import sys
import xml.dom.minidom as xmldom
import xml.etree.ElementTree as ElemTree
from collections import OrderedDict

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
sys.path.append(PROJECT_ROOT)

from api import ui
from api.script_base import run_script, ProductId, ExecutionCanceled, ScriptArgumentsParser

DEFAULT_CONFIG_PATH = os.path.join(os.path.join(os.path.dirname(__file__), 'program_qspi.xml'))
DEFAULT_FLASH_CONFIGS = os.path.join(PROJECT_ROOT, "qspi", "flash_configurations.xml")
MIN_FW_ADDRESS = 0x2000

PRODUCT_IDS_DICT = OrderedDict((
    ('DA1469x-00', ProductId.DA1469x_00),
    ('DA14682/3-00', ProductId.DA14683_00),
    ('DA14680/1-01', ProductId.DA14681_01),
))


class ProgramQspiConfig(object):
    PROGRAM_QSPI_TAG = 'program_qspi'
    PRODUCT_ID_TAG = 'product_id'
    PRODUCT_HEADER_TAG = 'product_header'
    ACTIVE_FW_IMAGE_ADDRESS_TAG = 'active_fw_image_address'
    UPDATE_FW_IMAGE_ADDRESS_TAG = 'update_fw_image_address'
    FLASH_NAME_TAG = 'flash_name'
    FLASH_SIZE_TAG = 'flash_size'
    FLASH_BURSTCMDA_REG_VALUE_TAG = 'flash_burstcmda_reg_value'
    FLASH_BURSTCMDB_REG_VALUE_TAG = 'flash_burstcmdb_reg_value'
    FLASH_WRITE_CONFIG_COMMAND_TAG = 'flash_write_config_command'

    def __init__(self, config_path=None):
        if not config_path:
            config_path = DEFAULT_CONFIG_PATH

        self.config_path = config_path
        self.product_id = None

        self.active_image_address = None
        self.update_image_address = None
        self.flash_name = None
        self.flash_size = None
        self.flash_burstcmda_reg_value = None
        self.flash_burstcmdb_reg_value = None
        self.flash_write_config_command = None

        self.load()

    def load(self):
        try:
            root = ElemTree.parse(self.config_path).getroot()
        except (ElemTree.ParseError, FileNotFoundError):
            root = ElemTree.Element(self.PROGRAM_QSPI_TAG)

        product_id = root.find(self.PRODUCT_ID_TAG)
        self.product_id = product_id.text if product_id is not None else None

        try:
            self.product_id = ProductId(self.product_id)
        except ValueError:
            self.product_id = None

        product_header = root.find(self.PRODUCT_HEADER_TAG)
        if product_header:
            active_image_address = product_header.find(self.ACTIVE_FW_IMAGE_ADDRESS_TAG)
            self.active_image_address = int(active_image_address.text, 16) \
                if active_image_address is not None else None

            update_image_address = product_header.find(self.UPDATE_FW_IMAGE_ADDRESS_TAG)
            self.update_image_address = int(update_image_address.text, 16) \
                if update_image_address is not None else None

            flash_name = product_header.find(self.FLASH_NAME_TAG)
            self.flash_name = str(flash_name.text) if flash_name is not None else None

            flash_size = product_header.find(self.FLASH_SIZE_TAG)
            self.flash_size = int(flash_size.text, 16) if flash_size is not None else None

            flash_burstcmda_reg_value = product_header.find(self.FLASH_BURSTCMDA_REG_VALUE_TAG)
            self.flash_burstcmda_reg_value = int(flash_burstcmda_reg_value.text, 16) \
                if flash_burstcmda_reg_value is not None else None

            flash_burstcmdb_reg_value = product_header.find(self.FLASH_BURSTCMDB_REG_VALUE_TAG)
            self.flash_burstcmdb_reg_value = int(flash_burstcmdb_reg_value.text, 16) \
                if flash_burstcmdb_reg_value is not None else None

            flash_write_config_command = product_header.find(self.FLASH_WRITE_CONFIG_COMMAND_TAG)
            self.flash_write_config_command = \
                self.parse_config_command(flash_write_config_command.text) \
                    if flash_write_config_command is not None else None


    def is_valid(self):
        if not self.product_id:
            return False

        if self.product_id == ProductId.DA1469x_00:
            return self.active_image_address is not None and self.update_image_address is not None \
                   and self.flash_name is not None \
                   and self.flash_size is not None \
                   and self.flash_burstcmda_reg_value is not None \
                   and self.flash_burstcmdb_reg_value is not None \
                   and self.flash_write_config_command is not None

        return True

    def check_flash_config(self, flash_config_path):
        try:
            fconfig = get_flash_configurations(flash_config_path)
        except:
            raise RuntimeError("Invalid flash config file ({}).\nExecution aborted.\n"
                               .format(flash_config_path))

        for item in fconfig:
            try:
                if self.flash_burstcmda_reg_value == \
                    int(fconfig[item][ProgramQspiConfig.FLASH_BURSTCMDA_REG_VALUE_TAG], 16) \
                    and self.flash_name == \
                    str(fconfig[item][ProgramQspiConfig.FLASH_NAME_TAG]) \
                    and self.flash_size == \
                    int(fconfig[item][ProgramQspiConfig.FLASH_SIZE_TAG], 16) \
                    and self.flash_burstcmdb_reg_value == \
                    int(fconfig[item][ProgramQspiConfig.FLASH_BURSTCMDB_REG_VALUE_TAG], 16) \
                    and self.flash_write_config_command == self.parse_config_command(
                        fconfig[item][ProgramQspiConfig.FLASH_WRITE_CONFIG_COMMAND_TAG]):
                        return True
            except:
                raise RuntimeError("Corrupted config file - missing value ({}).\nExecution aborted.\n"
                                 .format(flash_config_path))

        return False

    def save(self):
        if not self.is_valid():
            raise ValueError('Configuration is incomplete')

        root = ElemTree.Element(self.PROGRAM_QSPI_TAG)

        product_id = ElemTree.Element(self.PRODUCT_ID_TAG)
        product_id.text = str(self.product_id.value)
        root.append(product_id)

        if self.product_id == ProductId.DA1469x_00:
            product_header = ElemTree.Element(self.PRODUCT_HEADER_TAG)

            active_image_address = ElemTree.Element(self.ACTIVE_FW_IMAGE_ADDRESS_TAG)
            active_image_address.text = hex(self.active_image_address)
            product_header.append(active_image_address)

            update_image_address = ElemTree.Element(self.UPDATE_FW_IMAGE_ADDRESS_TAG)
            update_image_address.text = hex(self.update_image_address)
            product_header.append(update_image_address)

            flash_name = ElemTree.Element(self.FLASH_NAME_TAG)
            flash_name.text = str(self.flash_name)
            product_header.append(flash_name)

            flash_size = ElemTree.Element(self.FLASH_SIZE_TAG)
            flash_size.text = hex(self.flash_size)
            product_header.append(flash_size)

            flash_burstcmda_reg_value = ElemTree.Element(self.FLASH_BURSTCMDA_REG_VALUE_TAG)
            flash_burstcmda_reg_value.text = hex(self.flash_burstcmda_reg_value)
            product_header.append(flash_burstcmda_reg_value)

            flash_burstcmdb_reg_value = ElemTree.Element(self.FLASH_BURSTCMDB_REG_VALUE_TAG)
            flash_burstcmdb_reg_value.text = hex(self.flash_burstcmdb_reg_value)
            product_header.append(flash_burstcmdb_reg_value)

            flash_write_config_command = ElemTree.Element(self.FLASH_WRITE_CONFIG_COMMAND_TAG)
            flash_write_config_command.text = \
                ' '.join([hex(byte) for byte in self.flash_write_config_command])
            product_header.append(flash_write_config_command)

            root.append(product_header)

        with open(self.config_path, 'wb') as f:
            text = ElemTree.tostring(root)
            f.write(xmldom.parseString(text).toprettyxml(indent=' ' * 4, encoding='UTF-8'))

    def get_config_info(self):
        buf = 'Product ID: ' + str(self.product_id.value if self.product_id else '') + '\n'

        if self.product_id == ProductId.DA1469x_00 and self.product_id is not None:
            buf += 'Product Header:\n'
            buf += '\tactive_image_address: {}\n'.format(
                hex(self.active_image_address) if self.active_image_address is not None else 'N/A')
            buf += '\tupdate_image_address: {}\n'.format(
                hex(self.update_image_address) if self.update_image_address is not None else 'N/A')
            buf += '\tflash_name: {}\n'.format(
                self.flash_name if self.flash_name is not None else 'N/A')
            buf += '\tflash_size: {}M\n'.format(
                int(self.flash_size / (1024 * 1024)) if self.flash_size is not None else 'N/A')
            buf += '\tflash_burstcmda_reg_value: {}\n'.format(
                hex(self.flash_burstcmda_reg_value) if self.flash_burstcmda_reg_value is not None
                else 'N/A')
            buf += '\tflash_burstcmdb_reg_value: {}\n'.format(
                hex(self.flash_burstcmdb_reg_value) if self.flash_burstcmdb_reg_value is not None
                else 'N/A')
            buf += '\tflash_write_config_command: {}\n'.format(
                ' '.join([hex(byte) for byte in self.flash_write_config_command]) if
                self.flash_write_config_command is not None else 'N/A')

        return buf

    @staticmethod
    def parse_config_command(command_string):
        values = command_string.split(' ')
        buff = b''

        try:
            for value in values:
                buff += struct.pack('<B', int(value, 16))
        except:
            raise RuntimeError("Invalid value in flash config file ({}).\nExecution aborted.\n"
                             .format(value))

        return buff


def confirm_configuration_change(configuration):
    return ui.ask(text="Existing Configuration:\n\n" + configuration.get_config_info() +
                       "\nDo you want to change this configuration?",
                  confirmation='Change', denial='Keep')


def get_flash_configurations(flash_configurations_file=DEFAULT_FLASH_CONFIGS):
    try:
        root = ElemTree.parse(flash_configurations_file).getroot()
    except ElemTree.ParseError:
        raise RuntimeError("Flash configurations file is corrupted")

    def read_configurations():
        for configuration in root.findall('configuration'):
            try:
                yield (configuration.get('name'),
                       {field.tag: field.text for field in configuration})
            except KeyError:
                continue

    return OrderedDict(tuple(read_configurations()))


def _setup_address(value_name, ui_message):
    error_msg = "(Previous value was invalid - not a hex number)"

    while True:
        user_address = ui.ask_value(value_name=value_name, text=ui_message, default='0x2000')
        pattern = re.compile("0x[0-9a-fA-F]+")

        if user_address and pattern.fullmatch(user_address) is None:
            if not error_msg in ui_message:
                ui_message += "\n{}".format(error_msg)
            continue

        try:
            final_address = int(user_address[2:], 16)
        except:
            ui.print_message("Execution aborted.\n")
            raise ExecutionCanceled()

        if final_address >= MIN_FW_ADDRESS:
            break
        else:
            ui.info("FW cannot be written at address lower than 0x{:X} (0x{:X} given)".format(
                MIN_FW_ADDRESS, final_address))

    return final_address


def program_qspi_config(product_id=None, flash_config_path=None, header_config_path=None,
                        flash_configuration=None, default_img_addr=False):
    config = ProgramQspiConfig(header_config_path)

    if not product_id:
        if config.product_id and not confirm_configuration_change(config):
            if header_config_path:
                config.save()
                return
            else:
                raise ExecutionCanceled()

        product_id = ui.select_item(text='Select Product ID',
                                    item_list=list(PRODUCT_IDS_DICT.keys()))
        if not product_id:
            ui.print_message("No valid Product ID selected.\nExecution aborted.\n")
            raise ExecutionCanceled()

        config.product_id = PRODUCT_IDS_DICT[product_id]
    else:
        config.product_id = ProductId(product_id)

    if config.product_id == ProductId.DA1469x_00:
        if not flash_config_path:
            configurations = get_flash_configurations()

        else:
            ui.print_message("\nUsing flash config from {}.\n".format(flash_config_path))
            configurations = get_flash_configurations(flash_config_path)

        if not flash_configuration:
            flash_configuration = ui.select_item(item_list=list(configurations.keys()),
                                             text='Please select flash configuration')
        else:
            ui.print_message("Selected flash: {}\n".format(flash_configuration))

        if not flash_configuration:
            ui.print_message("No valid flash config selected.\nExecution aborted.\n")
            raise ExecutionCanceled()

        try:
            configuration = configurations[flash_configuration]
        except:
            ui.print_message("Unsupported flash config selected.\nExecution aborted.\n")
            raise ExecutionCanceled()

        config.flash_name = flash_configuration
        config.flash_size = int(configuration[ProgramQspiConfig.FLASH_SIZE_TAG], 16)
        config.flash_burstcmda_reg_value = \
            int(configuration[ProgramQspiConfig.FLASH_BURSTCMDA_REG_VALUE_TAG], 16)
        config.flash_burstcmdb_reg_value = \
            int(configuration[ProgramQspiConfig.FLASH_BURSTCMDB_REG_VALUE_TAG], 16)
        config.flash_write_config_command = config.parse_config_command(
            configuration[ProgramQspiConfig.FLASH_WRITE_CONFIG_COMMAND_TAG])

        if default_img_addr:
            config.active_image_address = 0x2000
            config.update_image_address = 0x2000
        else:
            ui.print_title('Active FW Image Address\n')
            config.active_image_address = \
                _setup_address(ProgramQspiConfig.ACTIVE_FW_IMAGE_ADDRESS_TAG,
                               'Insert Active FW image address (hex)')

            ui.print_title('Update FW Image Address\n')
            config.update_image_address = \
                _setup_address(ProgramQspiConfig.UPDATE_FW_IMAGE_ADDRESS_TAG,
                               'Insert Update FW image address (hex)')

    config.save()


def parse_args():
    parser = ScriptArgumentsParser()
    parser.add_revision_args()
    parser.add_config_args()
    parser.add_argument('-sf', '--select_flash', type=str, help='preselect Flash type')
    parser.add_argument('--default_img_addr', action='store_true',
                        help='use default active and update fw image addresses')
    args = parser.parse_args()
    return args.prod_id, args.flash_config, args.header_config, args.select_flash, \
           args.default_img_addr


if __name__ == '__main__':
    ui.print_header('PROGRAM QSPI CONFIGURATOR')
    run_script(program_qspi_config, parse_args)
    ui.print_footer('FINISHED')