
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
        if image.check_image_file(file_path):
            image.flash_image(file_path, serial_number)

    def FlashImage (self, request, context):
        print(request.file_path)
        print(request.serial_number)
        serial_port = serialport.SerialPort()
        processes = []
        for port in request.serial_number:
            # print(serial_port.get_serial_number('/dev/ttyACM'+str(port)))
            processes.append(multiprocessing.Process(target = self.process_image_flash, args = (request.file_path, serial_port.get_serial_number('/dev/ttyACM'+str(port)),)))

        for each in processes:
            each.start()

        while True:
            is_alive = False
            for each in processes:
                is_alive = is_alive or each.is_alive()
            if not is_alive:
                break

        return testbed_pb2.ImageFlashReply(message = 'success!')


def serve():
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    testbed_pb2_grpc.add_TestbedServicer_to_server(Testbed(), server)
    server.add_insecure_port('[::]:50051')
    server.start()
    try:
        while True:
            time.sleep(_ONE_DAY_IN_SECONDS)
    except KeyboardInterrupt:
        server.stop(0)


if __name__ == '__main__':
  serve()