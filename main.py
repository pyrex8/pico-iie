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
banks = "0123456789ABCDEFGHIJKLMN"
bank = 0
store = False
file_name = "                        "

SERIAL_BIN_DATA = 0x82
SERIAL_SIZE_LSB = 0x83
SERIAL_SIZE_MSB = 0x84
SERIAL_ADDR_LSB = 0x85
SERIAL_ADDR_MSB = 0x86
SERIAL_REBOOT = 0x87
SERIAL_BANK = 0x88
SERIAL_NAME = 0x89
SERIAL_BIN_START = 0x8A
SERIAL_BIN_STORE = 0x8B

FILE_NAME_LENGTH = 27

BAUDRATE = 115200
COM_PORT = '/dev/ttyUSB0'
ser = serial.Serial(COM_PORT, baudrate=BAUDRATE, rtscts=False)

# wait for byte returned from HC-06
while ser.inWaiting() == 0:
    pass
ser.flush()

try:
    arg_name = str(sys.argv[1])
    file_name = arg_name[:-9] + file_name
    file_ext = arg_name[-3:]

    if len(sys.argv) > 2:
        arg_bank = str(sys.argv[2])[0:1]
        bank = banks.find(arg_bank)
        if bank < 0:
            bank = 0
        store = True
        print("bank:", bank)
    else:
        print("no bank specified")
    if  file_ext == 'bin':
        bin_name = arg_name
    print ("file name:", file_name)
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
    bin_cmd.append(SERIAL_BANK)
    bin_cmd.append(bank)
    ser.write(bin_cmd)

    bin_cmd = bytearray()
    bin_cmd.append(SERIAL_NAME)
    for i in range(FILE_NAME_LENGTH + 1):
        bin_cmd.append(ord(file_name[i]))
    ser.write(bin_cmd)

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

    # add one byte at end
    bin_file.append(0)

    if store == True:
        bin_file.append(SERIAL_BIN_STORE)
    else:
        bin_file.append(SERIAL_BIN_START)

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
