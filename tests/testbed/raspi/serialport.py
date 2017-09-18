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
#       Implements serial ports management
#

import psutil
import subprocess

class SerialPort():

    def __init__(self):
        self.serial_number_port_map = dict()
        self.serial_port_number_map = dict()
        self.run()
        return

    def __get_all_serial_port(self):
        cmd = 'ls /dev/ttyACM*'
        self.port_list = self.process(cmd).split()
        print(self.port_list)
        return self.port_list

    def __get_all_serial_number(self):
        cmd = '~/nrfjprog.sh --showList'
        # TODO: add try execption  if there is no device in os
        # cmd = '/home/lezhan/tool/nrfjprog/nrfjprog -i'
        self.number_list = self.process(cmd).split()
        print(self.number_list)
        return

    def __generate_number_port_map(self):
        # TODO: add try execption  if there is no device in os
        for port_name in self.port_list:
            cmd = 'udevadm info -q property -n ' + port_name
            for number_name in self.number_list:
                if number_name in self.process(cmd):
                    self.serial_number_port_map[number_name] =  port_name
                    self.serial_port_number_map[port_name] = number_name

    def get_serial_number(self, serial_port):
        return self.serial_port_number_map[serial_port]

    def get_serial_port(self, serial_number):
        return self.serial_number_port_map[serial_number]

    def process(self, cmd):
        try:
            process_result = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
            list = process_result.communicate()[0].decode('UTF-8')
            return list
        except Exception as e:
            print(e)

    def run(self):
        self.__get_all_serial_port()
        self.__get_all_serial_number()
        self.__generate_number_port_map()

if __name__ == '__main__':
    a = SerialPort()
    a.run()
    print (a.get_serial_number('/dev/ttyACM0'))
    print (a.get_serial_port('683174679'))