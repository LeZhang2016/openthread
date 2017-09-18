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

class Service():

    def __init__(self):
        return


    def boot_service(self, ip):
        cmd = './shell_boot_server.sh ' + ip
        # print(cmd)
        res = self.process(cmd)
        # print (res)

    def process(self, cmd):
        try:
            process_result = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
            return process_result.communicate()
        except Exception as e:
            print(e)
