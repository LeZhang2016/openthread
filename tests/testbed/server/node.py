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
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
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
import sys
import time
if sys.platform != 'win32':
    import node_cli
else:
    import node_api
import unittest

class Node:
    def __init__(self, nodeid):
        if sys.platform != 'win32':
            self.interface = node_cli.otCli(nodeid)
        else:
            self.interface = node_api.otApi(nodeid)
        self.interface.factory_reset()
        self.interface.clear_whitelist()
        self.interface.disable_whitelist()
        self.interface.set_timeout(100)

    def __del__(self):
        del self.interface

    def set_mode(self, mode):
        self.interface.set_mode(mode)

    def debug(self, level):
        self.interface.debug(level)

    def interface_up(self):
        self.interface.interface_up()

    def interface_down(self):
        self.interface.interface_down()

    def thread_start(self):
        self.interface.thread_start()

    def thread_stop(self):
        self.interface.thread_stop()
            
    def commissioner_start(self):
        self.interface.commissioner_start()

    def commissioner_add_joiner(self, addr, psk):
        self.interface.commissioner_add_joiner(addr, psk)

    def joiner_start(self, pskd='', provisioning_url=''):
        self.interface.joiner_start(pskd, provisioning_url)

    def start(self):
        self.interface.interface_up()
        self.interface.thread_start()

    def stop(self):
        self.interface.thread_stop()
        self.interface.interface_down()

    def clear_whitelist(self):
        self.interface.clear_whitelist()

    def enable_whitelist(self):
        self.interface.enable_whitelist()

    def disable_whitelist(self):
        self.interface.disable_whitelist()

    def add_whitelist(self, addr, rssi=None):
        self.interface.add_whitelist(addr, rssi)

    def remove_whitelist(self, addr):
        self.interface.remove_whitelist(addr)

    def get_addr16(self):
        return self.interface.get_addr16()

    def set_addr64(self, extaddr):
        return self.interface.set_addr64(extaddr)

    def get_addr64(self):
        return self.interface.get_addr64()

    def get_hashmacaddr(self):
        return self.interface.get_hashmacaddr()

    def get_channel(self):
        return self.interface.get_channel()

    def set_channel(self, channel):
        self.interface.set_channel(channel)

    def get_masterkey(self):
        return self.interface.get_masterkey()

    def set_masterkey(self, masterkey):
        self.interface.set_masterkey(masterkey)

    def get_key_sequence_counter(self):
        return self.interface.get_key_sequence_counter()

    def set_key_sequence_counter(self, key_sequence_counter):
        self.interface.set_key_sequence_counter(key_sequence_counter)

    def set_key_switch_guardtime(self, key_switch_guardtime):
        self.interface.set_key_switch_guardtime(key_switch_guardtime)

    def set_network_id_timeout(self, network_id_timeout):
        self.interface.set_network_id_timeout(network_id_timeout)

    def get_network_name(self):
        return self.interface.get_network_name()

    def set_network_name(self, network_name):
        self.interface.set_network_name(network_name)

    def get_panid(self):
        return self.interface.get_panid()

    def set_panid(self, panid):
        self.interface.set_panid(panid)

    def get_partition_id(self):
        return self.interface.get_partition_id()

    def set_partition_id(self, partition_id):
        self.interface.set_partition_id(partition_id)

    def set_router_upgrade_threshold(self, threshold):
        self.interface.set_router_upgrade_threshold(threshold)

    def set_router_downgrade_threshold(self, threshold):
        self.interface.set_router_downgrade_threshold(threshold)

    def release_router_id(self, router_id):
        self.interface.release_router_id(router_id)

    def get_state(self):
        return self.interface.get_state()

    def set_state(self, state):
        self.interface.set_state(state)

    def get_timeout(self):
        return self.interface.get_timeout()

    def set_timeout(self, timeout):
        self.interface.set_timeout(timeout)

    def set_max_children(self, number):
        self.interface.set_max_children(number)

    def get_weight(self):
        return self.interface.get_weight()

    def set_weight(self, weight):
        self.interface.set_weight(weight)

    def add_ipaddr(self, ipaddr):
        self.interface.add_ipaddr(ipaddr)

    def get_addrs(self):
        return self.interface.get_addrs()

    def get_context_reuse_delay(self):
        return self.interface.get_context_reuse_delay()

    def set_context_reuse_delay(self, delay):
        self.interface.set_context_reuse_delay(delay)

    def add_prefix(self, prefix, flags, prf = 'med'):
        self.interface.add_prefix(prefix, flags, prf)

    def remove_prefix(self, prefix):
        self.interface.remove_prefix(prefix)

    def add_route(self, prefix, prf = 'med'):
        self.interface.add_route(prefix, prf)

    def remove_route(self, prefix):
        self.interface.remove_route(prefix)

    def register_netdata(self):
        self.interface.register_netdata()

    def energy_scan(self, mask, count, period, scan_duration, ipaddr):
        self.interface.energy_scan(mask, count, period, scan_duration, ipaddr)

    def panid_query(self, panid, mask, ipaddr):
        self.interface.panid_query(panid, mask, ipaddr)

    def udp_close(self):
        self.interface.udp_close()

    def udp_open(self):
        self.interface.udp_open()

    def udp_bind(self, ipaddr, port):
        self.interface.udp_bind(ipaddr, port)

    def udp_test(self, ipaddr, port, pkt_size, count, interval):
        self.interface.udp_test(ipaddr, port, pkt_size, count, interval)

    def udp_result(self):
        return self.interface.udp_result()

    def udp_monitor(self):
        return self.interface.udp_monitor()

    def latency_close(self):
        self.interface.latency_close()

    def latency_open(self, role):
        self.interface.latency_open(role)

    def latency_bind(self, ipaddr, port):
        self.interface.latency_bind(ipaddr, port)

    def latency_test(self, ipaddr, port, pkt_size, count, interval):
        self.interface.latency_test(ipaddr, port, pkt_size, count, interval)

    def latency_monitor(self):
        return self.interface.latency_monitor()

    def monitor_monitor(self, receiver_id, sender_id):
        return self.interface.monitor_monitor(receiver_id, sender_id)

    def monitor_open(self, receive_node_id, send_node_id):
        return self.interface.monitor_open(receive_node_id, send_node_id)

    def monitor_close(self):
        return self.interface.monitor_close()

    def duty_cycle(self, period, work_time):
        return self.interface.duty_cycle(period, work_time)

    def close_telnet(self):
        self.interface.close_telnet()

    def open_telnet(self, nodeid):
        return self.interface.open_telnet(nodeid)

    def scan(self):
        return self.interface.scan()

    def ping(self, ipaddr, num_responses=1, size=None, timeout=5000):
        return self.interface.ping(ipaddr, num_responses, size, timeout)

    def reset(self):
        return self.interface.reset()

    def set_router_selection_jitter(self, jitter):
        self.interface.set_router_selection_jitter(jitter)

    def set_active_dataset(self, timestamp, panid=None, channel=None, channel_mask=None, master_key=None):
        self.interface.set_active_dataset(timestamp, panid, channel, channel_mask, master_key)

    def set_pending_dataset(self, pendingtimestamp, activetimestamp, panid=None, channel=None):
        self.interface.set_pending_dataset(pendingtimestamp, activetimestamp, panid, channel)

    def announce_begin(self, mask, count, period, ipaddr):
        self.interface.announce_begin(mask, count, period, ipaddr)

    def send_mgmt_active_set(self, active_timestamp=None, channel=None, channel_mask=None, extended_panid=None,
                             panid=None, master_key=None, mesh_local=None, network_name=None, binary=None):
        self.interface.send_mgmt_active_set(active_timestamp, channel, channel_mask, extended_panid, panid,
                                            master_key, mesh_local, network_name, binary)

    def send_mgmt_pending_set(self, pending_timestamp=None, active_timestamp=None, delay_timer=None, channel=None,
                              panid=None, master_key=None, mesh_local=None, network_name=None):
        self.interface.send_mgmt_pending_set(pending_timestamp, active_timestamp, delay_timer, channel, panid,
                                             master_key, mesh_local, network_name)

if __name__ == '__main__':
    unittest.main()
