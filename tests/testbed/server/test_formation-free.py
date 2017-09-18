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

import node

MONITOR = 5
LEADER = 1
NODE_NUMBER = 28

class Formation_Test(unittest.TestCase):

    def setUp(self):
        self.nodes = {}
        for i in range(15, NODE_NUMBER + 1):
            self.nodes[i] = node.Node(i)

            self.nodes[i].set_panid(0xface)
            self.nodes[i].set_addr64(str(i).zfill(16))
            self.nodes[i].set_mode('rsdn')
            self.nodes[i].set_channel(23)

        for i in range(1, NODE_NUMBER + 1):
            # if i is not 1:
            #     self.nodes[i].add_whitelist(self.nodes[i - 1].get_addr64())
            # if i is not NODE_NUMBER:
            #     self.nodes[i].add_whitelist(self.nodes[i + 1].get_addr64())
            # self.nodes[i].enable_whitelist()
            if i is not 1:
                self.nodes[i].set_router_selection_jitter(1)

    def tearDown(self):
        for node in list(self.nodes.values()):
            node.stop()
        del self.nodes

    def test(self):
        self.nodes[LEADER].start()
        self.nodes[LEADER].set_state('leader')
        self.assertEqual(self.nodes[LEADER].get_state(), 'leader')
        timestamp = time.time() * 1000

        for i in range(2, NODE_NUMBER + 1):
            self.nodes[i].start()

        is_final_state = False
        while not is_final_state:
            for i in range(2, NODE_NUMBER + 1):
                is_final_state = True
                print(i, '--------------------', self.nodes[i].get_state())
                is_final_state = is_final_state and self.nodes[i].get_state() is 'router'
        print('The formation time: ', time.time() * 1000 - timestamp)

if __name__ == '__main__':
    unittest.main()
