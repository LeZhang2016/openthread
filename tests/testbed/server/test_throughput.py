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

import os
import time
import unittest

import node

LEADER = 1
NODE_NUMBER = 4

# PACKET_SIZE_ONE_HOP = [95, 568, 1144]
# PACKET_SIZE = [90, 560, 1136]
PACKET_SIZE_ONE_HOP = [95]
PACKET_SIZE = [90]
PACKET_COUNT = [200]
PERIOD = [100, 50, 20]
WORK_TIME = [50, 25, 10]

class throughput_Test(unittest.TestCase):

    def setUp(self):

        work_mode = 'rsdn'
        if "DC_MODE_1" in os.environ.keys():
            period = PERIOD[0]
            work_time = WORK_TIME[0]
            work_mode = 'sdn'
        if "DC_MODE_2" in os.environ.keys():
            period = PERIOD[1]
            work_time = WORK_TIME[1]
            work_mode = 'sdn'
        if "DC_MODE_3" in os.environ.keys():
            period = PERIOD[2]
            work_time = WORK_TIME[2]
            work_mode = 'sdn'

        self.nodes = {}
        for i in range(1, NODE_NUMBER + 1):
            self.nodes[i] = node.Node(i)

            self.nodes[i].set_panid(0xface)
            self.nodes[i].set_addr64(str(i).zfill(16))
            self.nodes[i].set_mode(work_mode)
            if work_mode == 'sdn':
                self.nodes[i].duty_cycle(period, work_time)
            self.nodes[i].set_channel(23)

        for i in range(1, NODE_NUMBER + 1):
            if i is not 1:
                self.nodes[i].add_whitelist(self.nodes[i - 1].get_addr64())
            if i is not NODE_NUMBER:
                self.nodes[i].add_whitelist(self.nodes[i + 1].get_addr64())
            self.nodes[i].enable_whitelist()
            if i is not 1:
                self.nodes[i].set_router_selection_jitter(1)

    def tearDown(self):
        for node in list(self.nodes.values()):
            node.stop()
        del self.nodes

    def test(self):
        test_result = []
        self.nodes[LEADER].start()
        self.nodes[LEADER].set_state('leader')
        self.assertEqual(self.nodes[LEADER].get_state(), 'leader')

        count = 0
        for i in range(2, NODE_NUMBER + 1):
            self.nodes[i].start()
            is_final_state = False
            while not is_final_state:
                is_final_state = self.nodes[i].get_state() is 'router' or count >= 1000
                time.sleep(0.1)
                count = count + 1
        print(time.time())

        self.nodes[LEADER].udp_open()
        for size_index in range(0, len(PACKET_SIZE)):
            for pkt_cnt in PACKET_COUNT:
                for i in range(NODE_NUMBER, 1, -1):
                    time.sleep(0.5)
                    self.nodes[i].udp_open()
                    self.nodes[i].udp_bind('::', '1234')

                    interval = 1

                    self.nodes[LEADER].udp_close()
                    self.nodes[LEADER].udp_open()
                    self.nodes[LEADER].udp_bind('::', '1234')
                    time.sleep(1)
                    if (i == LEADER + 1):
                        pkt_size = PACKET_SIZE_ONE_HOP[size_index]
                    else:
                        pkt_size = PACKET_SIZE[size_index]
                    self.nodes[i].udp_test('fdde:ad00:beef:0:0:ff:fe00:fc00', '1234', pkt_size, pkt_cnt, interval)

                    stats_every_packet = self.nodes[LEADER].udp_monitor()
                    print(stats_every_packet)

                    result = self.nodes[LEADER].udp_result()
                    result_count = int(result.split(',')[0])
                    result_elapse = int(result.split(',')[1])
                    print ({'Packet size' : pkt_size,
                                        'Packet number' : pkt_cnt,
                                        'Hop' : i-1,
                                        'Elapse' : result_elapse/result_count if result_count is not 0 else 0,
                                        'Throughput' : pkt_size * 8 * (result_count-1) * 1000/result_elapse if result_elapse is not 0 else 0,
                                        'Loss number' : pkt_cnt - result_count
                                        })
                    test_result.append({'Packet size' : pkt_size,
                                        'Packet number' : pkt_cnt,
                                        'Hop' : i-1,
                                        'Elapse' : result_elapse/(result_count-1) if result_count is not 1 else 0,
                                        'Throughput' : pkt_size * 8 * (result_count-1) * 1000/result_elapse if result_elapse is not 0 else 0,
                                        'Loss number' : pkt_cnt - result_count
                                        })

                    self.nodes[i].udp_close()
        for obj in test_result:
            print (obj)

if __name__ == '__main__':
    unittest.main()
