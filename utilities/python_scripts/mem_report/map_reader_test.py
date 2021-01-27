#
# Copyright (C) 2016-2020 Dialog Semiconductor.
# This computer program includes Confidential, Proprietary Information
# of Dialog Semiconductor. All Rights Reserved.
#

import pytest
from map_reader import classify_by_object_file


def test_sdk_module_classification():
    assert 'moduleA' == classify_by_object_file('./sdk/moduleA/foo.o')
    assert 'moduleB' == classify_by_object_file('./sdk/moduleB/foo/bar.o')
    assert 'application' == classify_by_object_file('./bar/moduleC/foo.o')


def test_sdk_startup_classification():
    assert 'startup' == classify_by_object_file('./startup/foo_bar.o')
    assert 'startup' == classify_by_object_file('./sdk/startup/foo_bar.o')
    assert 'startup' == classify_by_object_file('/home/user/DiaSemi/SmartSnippetsStudio/GCC/4_9-2015q3/bin/../lib/'
                                                'gcc/arm-none-eabi/4.9.3/armv6-m/crti.o')
    assert 'application' == classify_by_object_file('./foo_bar/startup/foo_bar.o')


def test_sdk_application_classification():
    assert 'application' == classify_by_object_file('./foo/bar/blah.o')
    assert 'application' == classify_by_object_file('./foo/bar/blah.a')
    assert 'application' == classify_by_object_file('C:\\projects\\680\\_WRBL\\wrbl_68x_ref\\projects\\target_apps\\'
                                                    'wrbl\\health_toolbox\\src\\lib_sf\\'
                                                    'libSF_Library.a(smartfusionahrs.o)')
    assert 'application' == classify_by_object_file('/home/user/projects/680/_WRBL/wrbl_68x_ref/projects/target_apps/'
                                                    'wrbl/health_toolbox/src/lib_sf/libSF_Library.a(smartfusionahrs.o)')


def test_lto_classification():
    assert 'unknown' == classify_by_object_file('/tmp/ccgbAMkv.ltrans2.ltrans.o')
    assert 'unknown' == classify_by_object_file('C:\Users\user\AppData\Local\Temp\ccl0fEDC.ltrans4.ltrans.o')


def test_sdk_other_classification():
    assert 'other' == classify_by_object_file('/home/user/DiaSemi/SmartSnippetsStudio/GCC/4_9-2015q3/bin/../lib/'
                                              'gcc/arm-none-eabi/4.9.3/armv6-m/libgcc.a(_thumb1_case_sqi.o)')
    assert 'other' == classify_by_object_file('/home/user/DiaSemi/SmartSnippetsStudio/GCC/4_9-2015q3/bin/../lib/'
                                              'gcc/arm-none-eabi/4.9.3/../../../../arm-none-eabi/lib/armv6-m/'
                                              'libg_nano.a(lib_a-memcpy.o)')
    assert 'other' == classify_by_object_file('/home/user/DiaSemi/SmartSnippetsStudio/GCC/4_9-2015q3/bin/../lib/gcc'
                                              '/arm-none-eabi/4.9.3/../../../../arm-none-eabi/lib/armv6-m/'
                                              'libnosys.a(_exit.o)')
    assert 'other' == classify_by_object_file('c:/diasemi/smartsnippetsstudio/gcc/4_9-2015q1/bin/../lib/gcc/'
                                              'arm-none-eabi/4.9.3/../../../../arm-none-eabi/lib/'
                                              'armv6-m\libnosys.a(sbrk.o)')
    assert 'startup' == classify_by_object_file('/home/user/DiaSemi/SmartSnippetsStudio/GCC/4_9-2015q3/bin/../lib/'
                                                'gcc/arm-none-eabi/4.9.3/armv6-m/crtbegin.o')


def test_libble_stack_classification_linux():
    assert 'libble_stack_da14681_01' == classify_by_object_file('/home/user/work/gitlab/black_orca_sdk/sdk/interfaces/'
                                                                'ble_stack/DA14681-01-Debug/libble_stack_da14681_01.a'
                                                                '(rom_patch.o)')
    assert 'libble_stack_da14683_00' == classify_by_object_file('/home/user/work/gitlab/black_orca_sdk/sdk/interfaces/'
                                                                'ble_stack/DA14683-00-Debug/libble_stack_da14683_00.a'
                                                                '(rom_patch.o)')


def test_libble_stack_classification_win():
    assert 'libble_stack_da14681_01' == classify_by_object_file('C:\\projects\\680\\_WRBL\\wrbl_68x_ref\\sdk_680\\sdk\\'
                                                                'interfaces\\ble_stack\\DA14681-01-Release\\'
                                                                'libble_stack_da14681_01.a(rom_patch.o)')
    assert 'libble_stack_da14683_00' == classify_by_object_file('C:\\projects\\680\\_WRBL\\wrbl_68x_ref\\sdk_680\\sdk\\'
                                                                'interfaces\\ble_stack\\DA14683-00-Release\\'
                                                                'libble_stack_da14683_00.a(rom_patch.o)')
    assert 'libble_stack_da14681_01' == classify_by_object_file('C:\\My projects\\680\\_WRBL\\wrbl_68x_ref\\sdk_680\\'
                                                                'sdk\\interfaces\\ble_stack\\DA14681-01-Release\\'
                                                                'libble_stack_da14681_01.a(rom_patch.o)')
    assert 'libble_stack_da14683_00' == classify_by_object_file('C:\\My projects\\680\\_WRBL\\wrbl_68x_ref\\sdk_680\\'
                                                                'sdk\\interfaces\\ble_stack\\DA14683-00-Release\\'
                                                                'libble_stack_da14683_00.a(rom_patch.o)')


def test_libusb_lib_classification_linux():
    assert 'libusb_lib_da14681_01' == classify_by_object_file('/home/user/work/gitlab/black_orca_sdk/sdk/interfaces/'
                                                              'usb/DA14681-01-Release/'
                                                              'libusb_lib_da14681_01.a(USB_CDC.o)')
    assert 'libusb_lib_da14683_00' == classify_by_object_file('/home/user/work/gitlab/black_orca_sdk/sdk/interfaces/'
                                                              'usb/DA14683-00-Release/'
                                                              'libusb_lib_da14683_00.a(USB_CDC.o)')

def test_libusb_lib_classification_win():
    assert 'libusb_lib_da14681_01' == classify_by_object_file('C:\\projects\\680\\_WRBL\\wrbl_68x_ref\\sdk_680\\sdk\\'
                                                              'interfaces\\usb\\DA14681-01-Release\\'
                                                              'libusb_lib_da14681_01.a(USB_CDC.o)')
    assert 'libusb_lib_da14681_01' == classify_by_object_file('C:\\projects\\680\\_WRBL\\wrbl_68x_ref\\sdk_680\\sdk\\'
                                                              'interfaces\\usb\\DA14681-01-Release\\'
                                                              'libusb_lib_da14681_01.a(USB_CDC.o)')
    assert 'libusb_lib_da14681_01' == classify_by_object_file('C:\\My projects\\680\\_WRBL\\wrbl_68x_ref\\sdk_680\\sdk'
                                                              '\\interfaces\\usb\\DA14681-01-Release\\'
                                                              'libusb_lib_da14681_01.a(USB_CDC.o)')
    assert 'libusb_lib_da14681_01' == classify_by_object_file('C:\\My projects\\680\\_WRBL\\wrbl_68x_ref\\sdk_680\\'
                                                              'sdk\\interfaces\\usb\\DA14681-01-Release\\'
                                                              'libusb_lib_da14681_01.a(USB_CDC.o)')


def test_new_sdk_lib_classification():
    assert 'libfoo' == classify_by_object_file('/home/user/work/gitlab/black_orca_sdk/sdk/foo/bar/libfoo.a(bar.o)')
    assert 'libfoo' == classify_by_object_file('C:\\My projects\\680\\_WRBL\\wrbl_ref\\sdk\\foo\\bar\\libfoo.a(bar.o)')
