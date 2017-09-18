from __future__ import print_function

import grpc
import time

import raspi.imageflash
import raspi.testbed_pb2
import raspi.testbed_pb2_grpc

from raspi.service import Service


def run(ip, source_file_path, dest_file_path, dir, serial_number):
    # image = raspi.imageflash.ImageFlash()
    # image.scp_image(ip, source_file_path, dest_file_path, dir)
    # time.sleep(2)
    service = Service()
    service.boot_service(ip)
    time.sleep(2)
    channel = grpc.insecure_channel(ip + ':50051')
    stub = raspi.testbed_pb2_grpc.TestbedStub(channel)
    dest_file_path = dest_file_path + 'arm-none-eabi-ot-cli-ftd.hex'
    response = stub.FlashImage(raspi.testbed_pb2.ImageFlashRequest(file_path= dest_file_path, serial_number=serial_number))
    print("client received: all nodes flash the image " + response.message)


if __name__ == '__main__':
    # ip = '172.23.253.184'
    # ip = '172.23.253.113'
    ip = '172.23.253.56'
    # ip = '172.23.253.59'
    source_file_path = '/home/lezhan/program/openthread/output/nrf52840/bin/arm-none-eabi-ot-cli-ftd.hex'
    dest_file_path = '/home/pi/'
    dir = "openthread"
    # run('172.23.253.184', '/home/pi/arm-none-eabi-ot-cli-ftd.hex', [12])
    run(ip, source_file_path, dest_file_path, dir, [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13])