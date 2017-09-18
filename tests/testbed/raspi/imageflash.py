#!/usr/bin/python
#
#       Copyright (c) 2017  Nest Labs, Inc.
#       All rights reserved.
#
#       This document is the property of Nest. It is considered
#       confidential and proprietary information.
#
#       This document may not be reproduced or transmitted in any form,
#       in whole or in part, without the express written permission of
#       Nest.
#

##
#    @file
#       Implements image upgrade
#

import psutil
import subprocess
import os.path
import multiprocessing

class ImageFlash():

    def __init__(self):
        return

    def scp_image(self, ip, src_file, dest_file, dir):
        cmd = './shell_scp.sh ' + ip + ' ' \
            + src_file + ' '               \
            + dest_file + ' '              \
            + dir
        print (cmd)

        res = self.process(cmd)

    def check_image_file(self, file_name):
        return os.path.isfile(file_name)

    def flash_image(self, file_name, serial_number):
        # cmd = '/home/lezhan/tool/nrfjprog/nrfjprog -f nrf52 --chiperase --program ' + file_name + ' -s ' + serial_number
        cmd = '~/nrfjprog.sh --erase-all ' + file_name + ' ' + serial_number
        # print(cmd)
        res = self.process(cmd)
        # print (res)

        if 'processing completed' in res[0]:
            print('erased successfully!')
        else:
            print('erased failed!')
            return

        cmd = '~/nrfjprog.sh --flash ' + file_name + ' ' + serial_number
        res = self.process(cmd)

        if 'processing completed' in res[0]:
            print('flashed successfully!')
        else:
            print('flashed failed!')

    def process(self, cmd):
        try:
            process_result = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
            return process_result.communicate()
        except Exception as e:
            print(e)
