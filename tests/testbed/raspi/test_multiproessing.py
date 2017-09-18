
from concurrent import futures
import time

import grpc

import testbed_pb2
import testbed_pb2_grpc

import serialport
import imageflash

import multiprocessing


_ONE_DAY_IN_SECONDS = 60 * 60 * 24

class Testbed(testbed_pb2_grpc.TestbedServicer):

    def process_image_flash(self, file_path, serial_number):
        image = imageflash.ImageFlash()
        print("flash" + serial_number)
        if image.check_image_file(file_path):
            image.flash_image(file_path, serial_number)

    def process_image_flash1(self, file_path, serial_number):
        image = imageflash.ImageFlash()
        print("flash1")
        if image.check_image_file(file_path):
            image.flash_image(file_path, serial_number)

    def process_image_flash2(self, file_path, serial_number):
        image = imageflash.ImageFlash()
        print("flash2")
        if image.check_image_file(file_path):
            image.flash_image(file_path, serial_number)

    def process_image_flash3(self, file_path, serial_number):
        image = imageflash.ImageFlash()
        print("flash3")
        if image.check_image_file(file_path):
            image.flash_image(file_path, serial_number)

    def FlashImage (self, request, context):
        print(request.file_path)
        print(request.serial_number)
        serial_port = serialport.SerialPort()
        image = imageflash.ImageFlash()
        process1 = None
        process2 = None
        process3 = None
        # for port in request.serial_number:
        #     print(serial_port.get_serial_number('/dev/ttyACM'+str(port)))
        #     #TODO: flash image to boards according to the serial number
        process1 = (multiprocessing.Process(target = self.process_image_flash, args = (request.file_path, serial_port.get_serial_number('/dev/ttyACM'+str(0)),)))
        process2 = (multiprocessing.Process(target = self.process_image_flash, args = (request.file_path, serial_port.get_serial_number('/dev/ttyACM'+str(1)),)))
        process3 = (multiprocessing.Process(target = self.process_image_flash, args = (request.file_path, serial_port.get_serial_number('/dev/ttyACM'+str(2)),)))
            # break;

        process1.start()
        process2.start()
        process3.start()

        return testbed_pb2.ImageFlashReply(message = 'success!')


def serve():
    print("================begin=============")
    serial_port = serialport.SerialPort()
    image = imageflash.ImageFlash()
    testbed = Testbed()
    processes = []
    for port in serial_port.get_all_serial_port():
        # print(serial_port.get_serial_number('/dev/ttyACM'+str(port)))
        processes.append(multiprocessing.Process(target=testbed.process_image_flash, args=(
            "/home/lezhan/program/openthread/output/nrf52840/bin/arm-none-eabi-ot-cli-ftd.hex", serial_port.get_serial_number(port),)))

    for each in processes:
        each.start()

if __name__ == '__main__':
  serve()