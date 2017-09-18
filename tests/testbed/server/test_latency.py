#!/usr/bin/env python
#
#  Copyright (c) 2016, The OpenThread Authors.
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#  3. Neither the name of the copyright holder nor the
#     names of its contributors may be used to endorse or promote products
#     derived from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR pkt_size PARTICULAR PURPOSE
#  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
#  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.
#

import time
import unittest
import multiprocessing
from functools import reduce


import node

MONITOR = 1
LEADER = 2
NODE_NUMBER = 28
PACKET_SIZE = [80]
# PACKET_SIZE = [95 , 184 , 280 , 376 , 472 , 568 , 664 , 760 , 856 , 952 , 1048 , 1144]
# PACKET_SIZE = [90]
PACKET_COUNT = [200]
NODE_PIN = [3, 4, 28, 29]
PERIOD = [20, 100, 50]
WORK_TIME = [10, 50, 25]

class Latency_Test(unittest.TestCase):

    def setUp(self):
        self.nodes = {}
        for i in range(1, NODE_NUMBER + 1):
            self.nodes[i] = node.Node(i)
            self.nodes[i].close_telnet()

    def nodeSettings(self, node_id):
        self.nodes[node_id].set_panid(0xface)
        self.nodes[node_id].set_addr64(str(node_id).zfill(16))
        # self.nodes[node_id].set_mode('rsdn')
        self.nodes[node_id].duty_cycle(PERIOD[1], WORK_TIME[1])
        self.nodes[node_id].set_mode('sdn')
        self.nodes[node_id].set_channel(23)
        self.nodes[node_id].set_router_upgrade_threshold(32)
        self.nodes[node_id].set_router_downgrade_threshold(33)

    def nodeStart(self, node_id):
        count = 0
        self.nodes[node_id].start()
        is_final_state = False
        while not is_final_state:
            is_final_state = self.nodes[node_id].get_state() is 'router' or count >= 1000
            time.sleep(0.1)
            count = count + 1

    def linearTopology(self):
        for i in range(2, NODE_NUMBER + 1):
            self.nodes[i].open_telnet(i)
            self.nodeSettings(i)

            if i is not 1:
                self.nodes[i].add_whitelist(str(i - 1).zfill(16))
            if i is not NODE_NUMBER:
                self.nodes[i].add_whitelist(str(i + 1).zfill(16))
            self.nodes[i].enable_whitelist()
            if i is not 1:
                self.nodes[i].set_router_selection_jitter(1)

            self.nodeSettings(i)
            self.nodes[i].close_telnet()

    def uniformTopology(self):
        for i in range(3, NODE_NUMBER + 1):
            if i % 3 is 0 and i >= 3 and i <= NODE_NUMBER:
                self.nodes[i].open_telnet(i)
                self.nodeSettings(i)
                self.nodes[i].add_whitelist(str(2).zfill(16))
                for j in range(3, NODE_NUMBER + 1):
                    if j % 3 is 1:
                        self.nodes[i].add_whitelist(str(j).zfill(16))
                self.nodes[i].enable_whitelist()
                self.nodes[i].set_router_selection_jitter(1)
                # close ssh and telnet
                self.nodeStart(i)
                self.nodes[i].close_telnet()
        for i in range(2, NODE_NUMBER + 1):
            if i % 3 is 1 and i >= 3 and i <= NODE_NUMBER:
                self.nodes[i].open_telnet(i)
                self.nodeSettings(i)
                for j in range(3, NODE_NUMBER + 1):
                    if j % 3 is 0 or j % 3 is 2:
                        self.nodes[i].add_whitelist(str(j).zfill(16))
                self.nodes[i].enable_whitelist()
                self.nodes[i].set_router_selection_jitter(1)
                self.nodeStart(i)
                # close ssh and telnet
                self.nodes[i].close_telnet()
        for i in range(2, NODE_NUMBER + 1):
            if i % 3 is 2 and i >= 3 and i <= NODE_NUMBER:
                self.nodes[i].open_telnet(i)
                self.nodeSettings(i)
                for j in range(3, NODE_NUMBER + 1):
                    if j % 3 is 1:
                        self.nodes[i].add_whitelist(str(j).zfill(16))
                self.nodes[i].enable_whitelist()
                self.nodes[i].set_router_selection_jitter(1)
                self.nodeStart(i)
                # close ssh and telnet
                self.nodes[i].close_telnet()


    def tearDown(self):
        # for node in list(self.nodes.values()):
        #     node.stop()
        # del self.nodes
        for i in range(1, NODE_NUMBER + 1):
            self.nodes[i].open_telnet(i)
            self.nodes[i].stop()
            self.nodes[i].close_telnet()
        del self.nodes

    def test(self):
        self.nodes[LEADER].open_telnet(LEADER)
        self.nodeSettings(LEADER)
        for i in range(3, NODE_NUMBER + 1):
            if i % 3 is 0:
                self.nodes[LEADER].add_whitelist(str(i).zfill(16))
        self.nodes[LEADER].enable_whitelist()
        self.nodes[LEADER].start()
        self.nodes[LEADER].set_state('leader')
        self.assertEqual(self.nodes[LEADER].get_state(), 'leader')

        self.uniformTopology()

        self.nodes[MONITOR] = node.Node(MONITOR)
        self.nodes[LEADER].udp_open()
        for pkt_size in PACKET_SIZE:
            for pkt_cnt in PACKET_COUNT:
                for i in range(3, 6):
                # for i in range(5, 2, -1):
                    time.sleep(0.5)
                    self.nodes[MONITOR].monitor_open(NODE_PIN[LEADER-2], NODE_PIN[i-2])
                    time.sleep(0.5)
                    self.nodes[i].open_telnet(i)
                    self.nodes[i].udp_open()
                    self.nodes[i].udp_bind('::', '1234')

                    interval = 0

                    self.nodes[LEADER].udp_close()
                    self.nodes[LEADER].udp_open()
                    self.nodes[LEADER].udp_bind('::', '1234')
                    time.sleep(1)

                    self.nodes[i].udp_test('ff03::1', '1234', pkt_size, pkt_cnt, interval)
                    # self.nodes[i].udp_test('fdde:ad00:beef:0:0:ff:fe00:fc00', '1234', pkt_size, pkt_cnt, interval)

                    result = self.nodes[MONITOR].monitor_monitor(NODE_PIN[LEADER-2], NODE_PIN[i-2])

                    print('-------------------------')
                    print(result)
                    print('Recevied packets', len(result), 'latency is ', reduce(lambda x, y: x + y, result) / len(result))
                    print('-------------------------')

                    self.nodes[i].udp_close()
                    self.nodes[i].close_telnet()

        self.nodes[LEADER].udp_close()
        self.nodes[MONITOR].monitor_close()

if __name__ == '__main__':
    unittest.main()
