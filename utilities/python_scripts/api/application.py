#########################################################################################
# Copyright (C) 2015-2019 Dialog Semiconductor.
# This computer program includes Confidential, Proprietary Information
# of Dialog Semiconductor. All Rights Reserved.
#########################################################################################

import os
import sys
import subprocess

from api.utils import normpath


class ApplicationError(Exception):
    def __init__(self, message, call='', stderr='', return_code=None):
        self.call = call
        self.stderr = stderr
        self.return_code = return_code
        super(ApplicationError, self).__init__(message)


class Application(object):
    def __init__(self, path):
        if not os.path.exists(path):
            raise ApplicationError(str(path) + ' does not exists.')
        if not os.path.isfile(path) or not os.access(path, os.X_OK):
            raise ApplicationError(str(path) + ' is not application file.')

        self.__path = normpath(os.path.abspath(path))

    def get_path(self):
        return self.__path

    def get_basename(self):
        return os.path.basename(self.__path)

    def run(self, args=None, cwd=None, silent=False):
        call = [self.__path] + args if args else [self.__path]
        process_params = {
            'args': ' '.join(call),
            'shell': True,
            'cwd': cwd,
            'stdout': subprocess.PIPE,
            'stderr': subprocess.PIPE,
        }

        sys.stdout.flush()
        pr = subprocess.Popen(**process_params)

        buf = b''
        for line in pr.stdout:
            if not silent:
                sys.stdout.buffer.write(line)
                sys.stdout.flush()
            buf += line

        pr.wait()

        if pr.returncode:
            err_out = pr.stderr.read().decode('utf-8')

            sys.stdout.write(err_out)

            err_msg = '{} has exited with code: {}'.format(os.path.basename(call[0]), pr.returncode)
            raise ApplicationError(err_msg, call=' '.join(call), stderr=err_out,
                                   return_code=pr.returncode)

        return buf.decode('utf-8')
