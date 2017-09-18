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
#       Implements task assignment and provide interface for test

class NodeMgmt():

    def __init__(self):
        self.raspi_ip = ['172.23.253.56', '172.23.253.59']
        # self.raspi_ip = ['172.23.253.113', '172.23.253.184']
        # self.raspi_ip = ['172.23.253.113', '172.23.253.184']
        print (self.raspi_ip)
        return

    def add_raspi_ip_addr(self, ip_addr):
        self.raspi_ip.append(ip_addr)
        return

    def get_node_ip_port(self, node_id):
        if node_id > 14 * len(self.raspi_ip) or node_id <= 0:
            print('Invalid node id')
            return
        port = (node_id -1) % 14
        ip = self.raspi_ip[int((node_id-1) / 14)]
        return ip, port

    def get_node_id(self, ip, port):
        index = self.raspi_ip.sort().index(ip) * 14 + (port - 2000) + 1
        return index