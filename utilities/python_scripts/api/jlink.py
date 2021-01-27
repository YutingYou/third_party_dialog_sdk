#########################################################################################
# Copyright (C) 2015-2019 Dialog Semiconductor.
# This computer program includes Confidential, Proprietary Information
# of Dialog Semiconductor. All Rights Reserved.
#########################################################################################

import os
import re
import tempfile

from api import ui
from api.application import Application
from api.utils import is_linux, is_win
from string import Template


if is_win():
    JLINK_EXE = "JLink.exe"
    JLINK_GDB = "JLinkGDBServerCL.exe"
elif is_linux():
    JLINK_EXE = "JLinkExe"
    JLINK_GDB = "JLinkGDBServer"
else:
    JLINK_EXE = ''
    JLINK_GDB = ''


def get_default_jlink_path():
    if is_win():
        from winreg import OpenKey, QueryValueEx, EnumKey, KEY_READ, HKEY_CURRENT_USER

        def enumerate_keys(key):
            i = 0
            while True:
                try:
                    yield EnumKey(key, i)
                    i += 1
                except WindowsError:
                    break

        jlink_root_path = 'Software\SEGGER\J-Link'
        jlink_install_path = ''

        with OpenKey(HKEY_CURRENT_USER, jlink_root_path, 0, KEY_READ) as registry:
            for subdir in enumerate_keys(registry):
                with OpenKey(registry, subdir) as install_path:
                    path, _ = QueryValueEx(install_path, "InstallPath")

                    # check that JLinkExe is actually installed in path
                    binary = os.path.join(path, JLINK_EXE)
                    if os.path.isfile(binary) and os.access(binary, os.X_OK):
                        jlink_install_path = path

        return jlink_install_path

    elif is_linux():
        for path in os.environ["PATH"].split(os.pathsep):
            binary = os.path.join(path, JLINK_EXE)
            if os.path.isfile(binary) and os.access(binary, os.X_OK):
                return path

        return ''

    else:
        return ''


def get_jlink_exe(jlink_path=None):
    return os.path.join(jlink_path if jlink_path else get_default_jlink_path(), JLINK_EXE)


def get_jlink_gdb(jlink_path=None):
    return os.path.join(jlink_path if jlink_path else get_default_jlink_path(), JLINK_GDB)


class JLinkExe(Application):
    def __init__(self, path=None):
        super().__init__(get_jlink_exe(path))

    def __make_showemulist_file(self, showemulist_file):

        showemulist_template = Template(
            'ShowEmulist\n'
            'exit\n'
            '\n'
        )

        with open(showemulist_file, 'w') as f:
            f.write(showemulist_template.substitute())

    def find_jlink_numbers(self, **kwargs):
        try:
            emulist_file = tempfile.mktemp()
            self.__make_showemulist_file(emulist_file)

            cmd = ["-ExitOnError", "-CommandFile", emulist_file]
            kwargs['silent'] = True
            out = self.run(args=cmd, **kwargs)
            return re.findall('Serial number: (\d+)', str(out))
        finally:
            os.remove(emulist_file)


class JLinkGDB(Application):
    def __init__(self, path=None):
        super().__init__(get_jlink_gdb(path))

