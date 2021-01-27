#!/usr/bin/env python3
#########################################################################################
# Copyright (C) 2015-2019 Dialog Semiconductor.
# This computer program includes Confidential, Proprietary Information
# of Dialog Semiconductor. All Rights Reserved.
#########################################################################################

import os
import re
import sys
import xml.dom.minidom as xmldom
import xml.etree.ElementTree as ElemTree


PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
sys.path.append(PROJECT_ROOT)

from api.mkimage import Mkimage, EllipticCurve, MKIMAGE_BIN
from api.script_base import run_script, ScriptArgumentsParser, ProductId
from api import ui


DEFAULT_PRODUCT_KEYS_FILE = 'product_keys.xml'

SYMMETRIC_KEYS_NUM = {ProductId.DA14683_00: 8, ProductId.DA1469x_00: 8}
SYMMETRIC_KEYS_LEN = {ProductId.DA14683_00: 32, ProductId.DA1469x_00: 32}
ASYMMETRIC_KEYS_NUM = {ProductId.DA14683_00: 4, ProductId.DA1469x_00: 8}
SYMMETRIC_FW_DEC_KEYS_NUM = {ProductId.DA14683_00: 0, ProductId.DA1469x_00: 8}
SYMMETRIC_FW_DEC_KEYS_LEN = {ProductId.DA14683_00: 0, ProductId.DA1469x_00: 32}

ALLOWED_ELLIPTIC_CURVES = [
    EllipticCurve.SECP256R1,
    EllipticCurve.SECP224R1,
    EllipticCurve.SECP192R1,
    EllipticCurve.EDWARDS25519
]

DEFAULT_ELLIPTIC_CURVE_IDX = ALLOWED_ELLIPTIC_CURVES.index(EllipticCurve.SECP256R1)


class ProductKeys:
    KEYS_TAG = 'keys'
    SYMMETRIC_TAG = 'symmetric'
    ASYMMETRIC_TAG = 'asymmetric'
    SYMMETRIC_FW_DEC_TAG = 'symmetric_fw_dec'

    class AsymmetricKey:
        def __init__(self, private, public, elliptic_curve):
            self.private = private
            self.public = public
            self.elliptic_curve = elliptic_curve

    def clear_symmetric_keys(self):
        self.symmetric_keys = []

    def add_symmetric_key(self, key):
        self.symmetric_keys.append(key)

    def clear_asymmetric_keys(self):
        self.asymmetric_keys = []

    def add_asymmetric_key(self, private, public, elliptic_curve):
        self.asymmetric_keys.append(ProductKeys.AsymmetricKey(private, public, elliptic_curve))

    def clear_symmetric_fw_dec_keys(self):
        self.symmetric_fw_dec_keys = []

    def add_symmetric_fw_dec_key(self, key):
        self.symmetric_fw_dec_keys.append(key)

    def __init__(self, config_file):
        self.__file = config_file
        self.symmetric_keys = []
        self.asymmetric_keys = []
        self.symmetric_fw_dec_keys = []

        try:
            root = ElemTree.parse(config_file).getroot()
        except (ElemTree.ParseError, FileNotFoundError):
            root = ElemTree.Element(ProductKeys.KEYS_TAG)

        for key in root.find(ProductKeys.SYMMETRIC_TAG) or []:
            self.add_symmetric_key(key.text)

        for key in root.find(ProductKeys.ASYMMETRIC_TAG) or []:
            self.add_asymmetric_key(key.find('private').text, key.find('public').text,
                                    EllipticCurve(key.find('elliptic_curve').text))

        for key in root.find(ProductKeys.SYMMETRIC_FW_DEC_TAG) or []:
            self.add_symmetric_fw_dec_key(key.text)

    def save(self):
        root = ElemTree.Element(ProductKeys.KEYS_TAG)

        if self.symmetric_keys:
            symmetric = ElemTree.Element(ProductKeys.SYMMETRIC_TAG)
            comment = ElemTree.Comment('User Data Encryption Keys')
            symmetric.append(comment)
            for key in self.symmetric_keys:
                elem = ElemTree.Element('symmetric_key')
                elem.text = key
                symmetric.append(elem)
            root.append(symmetric)

        if self.asymmetric_keys:
            asymmetric = ElemTree.Element(ProductKeys.ASYMMETRIC_TAG)
            comment = ElemTree.Comment('Signature Keys')
            asymmetric.append(comment)
            for key in self.asymmetric_keys:
                elem = ElemTree.Element('asymmetric_keys')

                private = ElemTree.Element('private')
                private.text = key.private
                elem.append(private)

                public = ElemTree.Element('public')
                public.text = key.public
                elem.append(public)

                elliptic_curve = ElemTree.Element('elliptic_curve')
                elliptic_curve.text = key.elliptic_curve.value
                elem.append(elliptic_curve)

                asymmetric.append(elem)
            root.append(asymmetric)

        if self.symmetric_fw_dec_keys:
            symmetric_fw_dec = ElemTree.Element(ProductKeys.SYMMETRIC_FW_DEC_TAG)
            comment = ElemTree.Comment('QSPI FW Decryption Keys')
            symmetric_fw_dec.append(comment)
            for key in self.symmetric_fw_dec_keys:
                elem = ElemTree.Element('symmetric_key')
                elem.text = key
                symmetric_fw_dec.append(elem)
            root.append(symmetric_fw_dec)

        with open(self.__file, 'wb') as f:
            text = ElemTree.tostring(root, short_empty_elements=False)
            f.write(xmldom.parseString(text).toprettyxml(indent=' ' * 4, encoding='UTF-8'))

    def is_valid(self, prod_id):
        if prod_id != ProductId.DA1469x_00 and prod_id != ProductId.DA14683_00:
            return False

        # At least one asymmetric keys pair is required
        if self.asymmetric_keys is None or len(self.asymmetric_keys) < 1 or \
                        len(self.asymmetric_keys) > ASYMMETRIC_KEYS_NUM[prod_id]:
            return False

        # Symmetric keys are optional - could not exist
        if self.symmetric_keys is not None and \
                        len(self.symmetric_keys) > SYMMETRIC_KEYS_NUM[prod_id]:
            return False

        # FW decryption keys should be passed only for DA1469x
        if self.symmetric_fw_dec_keys is not None and \
                        len(self.symmetric_fw_dec_keys) > SYMMETRIC_FW_DEC_KEYS_NUM[prod_id]:
            return False

        if prod_id == ProductId.DA1469x_00:
            # At least one FW decryption key is required for DA1469x
            if self.symmetric_fw_dec_keys is None or len(self.symmetric_fw_dec_keys) < 1:
                return False
            # DA1469x supports only one elliptic curve
            for asym_key in self.asymmetric_keys:
                if asym_key.elliptic_curve != EllipticCurve.EDWARDS25519:
                    return False

        return True


def generate_symmetric_keys(key_number, length):
    sym_key_pat = re.compile(r'[ ]+#\d+: ([0-9A-F]+)')
    return sym_key_pat.findall(Mkimage().gen_sym_key(key_number, length))


def generate_asymmetric_keys(key_number, elliptic_curve):
    asym_key_pat = re.compile(r'[ ]+PRIVATE KEY:[ ]+([0-9A-F]+)\r*?\n[ ]+PUBLIC KEY:[ ]+([0-9A-F]+)',
                              re.MULTILINE)
    return asym_key_pat.findall(Mkimage().gen_asym_key(elliptic_curve, key_number))


def generate_keys(output_file=None, elliptic_curve=None, prod_id=None):
    if not output_file:
        output_file = DEFAULT_PRODUCT_KEYS_FILE

    # Use Enum objects instead of string
    try:
        product_id = ProductId(prod_id)
    except ValueError:
        raise RuntimeError('Invalid or no Product ID passed. Execution aborted.')

    if product_id == ProductId.DA14681_01:
        raise RuntimeError('Secure boot feature is not supported by ' + str(product_id.value))

    if not os.path.exists(MKIMAGE_BIN):
        raise RuntimeError('Can not find mkimage. Please install it and run this script again.')

    if os.path.exists(output_file):
        if ui.ask(text='Product keys file already exists.\n'
                       'Would you like to move it to "' + output_file + '.old" and make new file?'):
            os.replace(output_file, output_file + '.old')
        else:
            ui.print_message('Aborting key generation')
            return

    product_keys = ProductKeys(output_file)

    if product_id == ProductId.DA1469x_00:
        # DA1469x devices support only one signature generation algorithm - Ed25519.
        if elliptic_curve and elliptic_curve != EllipticCurve.EDWARDS25519.value:
            raise RuntimeError('Only {} curve could be used for DA1469x ({} passed). Execution '
                               'aborted.'.format(EllipticCurve.EDWARDS25519.value, elliptic_curve))

        elliptic_curve = EllipticCurve.EDWARDS25519.value
    elif not elliptic_curve:
        elliptic_curve = ui.select_item(text='Select elliptic curve used for asymmetric keys',
                                        item_list=[c.value for c in ALLOWED_ELLIPTIC_CURVES])

    # Use Enum objects instead of string
    try:
        elliptic_curve = EllipticCurve(elliptic_curve)
    except ValueError:
        raise RuntimeError('Invalid Elliptic Curve passed. Execution aborted.')

    ui.print_message('Writing keys to {}'.format(output_file))

    if SYMMETRIC_KEYS_NUM[product_id] and SYMMETRIC_KEYS_LEN[product_id]:
        for key in generate_symmetric_keys(SYMMETRIC_KEYS_NUM[product_id],
                                           SYMMETRIC_KEYS_LEN[product_id]):
            product_keys.add_symmetric_key(key=key)

    if ASYMMETRIC_KEYS_NUM[product_id] and elliptic_curve:
        for private, public in generate_asymmetric_keys(ASYMMETRIC_KEYS_NUM[product_id],
                                                        elliptic_curve):
            product_keys.add_asymmetric_key(private=private, public=public,
                                            elliptic_curve=elliptic_curve)

    if SYMMETRIC_FW_DEC_KEYS_NUM[product_id] and SYMMETRIC_FW_DEC_KEYS_LEN[product_id]:
        for key in generate_symmetric_keys(SYMMETRIC_FW_DEC_KEYS_NUM[product_id],
                                           SYMMETRIC_FW_DEC_KEYS_LEN[product_id]):
            product_keys.add_symmetric_fw_dec_key(key=key)

    if not product_keys.is_valid(product_id):
        raise RuntimeError('Generated keys are invalid - cannot save file.')

    product_keys.save()


def parse_args():
    parser = ScriptArgumentsParser()
    parser.add_revision_args()
    parser.add_argument('-o', metavar='<file>', dest='output_file', help='output file')
    parser.add_argument('-ec', '--elliptic_curve', dest='elliptic_curve',
                        help='elliptic curve (omitted for DA1469x devices)',
                        choices=[e.value for e in ALLOWED_ELLIPTIC_CURVES])
    args = parser.parse_args()
    return args.output_file, args.elliptic_curve, args.prod_id


if __name__ == '__main__':
    ui.print_header('GENERATING PRODUCT KEYS')
    run_script(generate_keys, parse_args)
    ui.print_footer('FINISHED')
