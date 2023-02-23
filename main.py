#!/usr/bin/env python3
"""
pico-iie emulator binary downloader
"""

import os
import time
import numpy
import sys
import serial

bin_name = ""

SERIAL_BIN_DATA = 0x82
SERIAL_SIZE_LSB = 0x83
SERIAL_SIZE_MSB = 0x84
SERIAL_ADDR_LSB = 0x85
SERIAL_ADDR_MSB = 0x86
SERIAL_REBOOT = 0x87

BAUDRATE = 115200
COM_PORT = '/dev/rfcomm0'
ser = serial.Serial(COM_PORT, baudrate=BAUDRATE, rtscts=False)

# wait for byte returned from HC-06
while ser.inWaiting() == 0:
    pass
ser.flush()

try:
    arg_name = str(sys.argv[1])
    file_ext = arg_name[-3:]
    if  file_ext == 'bin':
        bin_name = arg_name
except:
    pass

if bin_name != "":
    # read bin file on startup
    ser.flush()

    bin_cmd = bytearray()
    bin_cmd.append(SERIAL_REBOOT)
    ser.write(bin_cmd)

    time.sleep(0.5)

    bin_image = open(bin_name, 'rb')

    file_size = os.path.getsize(bin_name)
    file_size_hex = hex(file_size)
    file_size_lsb = int(file_size_hex[-2:], 16)
    file_size_msb = int(file_size_hex[-4:-2], 16)

    result = bin_name.index('.')
    bin_address_lsb = int(bin_name[result - 2: result], 16)
    bin_address_msb = int(bin_name[result - 4: result - 2], 16)

    bin_cmd = bytearray()
    bin_cmd.append(SERIAL_SIZE_LSB)
    bin_cmd.append(file_size_lsb)
    ser.write(bin_cmd)

    bin_cmd = bytearray()
    bin_cmd.append(SERIAL_SIZE_MSB)
    bin_cmd.append(file_size_msb)
    ser.write(bin_cmd)

    bin_cmd = bytearray()
    bin_cmd.append(SERIAL_ADDR_LSB)
    bin_cmd.append(bin_address_lsb)
    ser.write(bin_cmd)

    bin_cmd = bytearray()
    bin_cmd.append(SERIAL_ADDR_MSB)
    bin_cmd.append(bin_address_msb)
    ser.write(bin_cmd)

    bin_cmd = bytearray()
    bin_cmd.append(SERIAL_BIN_DATA)
    ser.write(bin_cmd)

    bin_file = bytearray()


    while 1:

        # read by character
        char = bin_image.read(1)

        if not char:
            break

        bin_data = ord(char)

        bin_file.append(bin_data)

    # add one byte at end to start binary
    bin_file.append(0)

    bin_image.close()

    ser.flush()
    ser.write(bin_file)
    time.sleep(2)

    print('bin file name:', bin_name)
    print('bin file size:',file_size, 'bytes')

ser.flush()
ser.close()
print("end serial")
