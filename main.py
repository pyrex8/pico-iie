#!/usr/bin/env python3
"""
pico-iie emulator in C and pygame
"""

import os
import time
import pygame
import pygame.sndarray
import pygame.surfarray
import numpy
import sys
import serial

bin_name = ""

PIXELS_X = 280
PIXELS_Y = 192
SCREEN_SCALE = 4
SCREEN_X = PIXELS_X * SCREEN_SCALE
SCREEN_Y = PIXELS_Y * SCREEN_SCALE
SCREEN_X_TOTAL = SCREEN_X
SCREEN_Y_TOTAL = SCREEN_Y

SERIAL_BIN_DATA = 0x82
SERIAL_SIZE_LSB = 0x83
SERIAL_SIZE_MSB = 0x84
SERIAL_ADDR_LSB = 0x85
SERIAL_ADDR_MSB = 0x86
SERIAL_RESET = 0x87
SERIAL_REBOOT = 0x88

BLACK = (0, 0, 0)

dest = r'~!@#$%^&*()_+{}|:"<>?'
src =  r"`1234567890-=[]\;',./"

BAUDRATE = 230400
COM_PORT = '/dev/ttyUSB0'
ser = serial.Serial(COM_PORT, baudrate=BAUDRATE, rtscts=False)
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
    bin_cmd.append(SERIAL_RESET)
    ser.write(bin_cmd)
    print ("SERIAL_RESET = ", bin_cmd, len(bin_cmd))

    bin_image = open(bin_name, 'rb')

    file_size = hex(os.path.getsize(bin_name))
    file_size_lsb = int(file_size[-2:], 16)
    file_size_msb = int(file_size[-4:-2], 16)
    print("binary file size =", file_size, file_size_msb, file_size_lsb, "bytes")

    result = bin_name.index('.')
    bin_address_lsb = int(bin_name[result - 2: result], 16)
    bin_address_msb = int(bin_name[result - 4: result - 2], 16)
    print("binary start address =", bin_address_msb, bin_address_lsb)

    bin_cmd = bytearray()
    bin_cmd.append(SERIAL_SIZE_LSB)
    bin_cmd.append(file_size_lsb)
    ser.write(bin_cmd)
    print ("SERIAL_SIZE_LSB = ", bin_cmd, len(bin_cmd))

    bin_cmd = bytearray()
    bin_cmd.append(SERIAL_SIZE_MSB)
    bin_cmd.append(file_size_msb)
    ser.write(bin_cmd)
    print ("SERIAL_SIZE_MSB = ", bin_cmd, len(bin_cmd))

    bin_cmd = bytearray()
    bin_cmd.append(SERIAL_ADDR_LSB)
    bin_cmd.append(bin_address_lsb)
    ser.write(bin_cmd)
    print ("SERIAL_ADDR_LSB = ", bin_cmd, len(bin_cmd))

    bin_cmd = bytearray()
    bin_cmd.append(SERIAL_ADDR_MSB)
    bin_cmd.append(bin_address_msb)
    ser.write(bin_cmd)
    print ("SERIAL_ADDR_MSB = ", bin_cmd, len(bin_cmd))

    bin_cmd = bytearray()
    bin_cmd.append(SERIAL_BIN_DATA)
    ser.write(bin_cmd)
    print ("SERIAL_BIN_DATA = ", bin_cmd, len(bin_cmd))

    bin_file_len = 0
    bin_file = bytearray()


    while 1:

        # read by character
        char = bin_image.read(1)

        if not char:
            break

        bin_data = ord(char)

        bin_file.append(bin_data)
        bin_file_len += 1

    print('bin file length =',bin_file_len)
    bin_image.close()

    ser.flush()
    ser.write(bin_file)

    print('Binary:', bin_name)

joystick_present = 0
joystick_buttons = 0
button_0 = 0
button_1 = 0
paddle_0 = 0
paddle_1 = 0

pygame.init()
pygame.display.set_caption('pyrex8')

try:
    icon = pygame.image.load('icon.png')
    pygame.display.set_icon(icon)
except:
    pass

screen = pygame.display.set_mode([SCREEN_X_TOTAL, SCREEN_Y_TOTAL])
clock = pygame.time.Clock()

running = True

while running:

    if joystick_present:
        button_0 = int(joystick.get_button(0))
        if joystick_buttons > 1:
            button_1 = int(joystick.get_button(1))
        paddle_0 = int((joystick.get_axis(0) + 1) * 127)
        paddle_1 = int((joystick.get_axis(1) + 1) * 127)
    else:
        if pygame.joystick.get_count() > 0:
            joystick = pygame.joystick.Joystick(0)
            joystick.init()
            joystick_present = 1
            joystick_buttons = joystick.get_numbuttons()
        button_0 = 0
        button_1 = 0
        paddle_0 = 128
        paddle_1 = 128

    keycode = 0

    for event in pygame.event.get():

        if event.type == pygame.QUIT:
            running = False

        if event.type == pygame.KEYDOWN:
            if event.key == pygame.K_F10:
                running = False

            if event.key == pygame.K_F1:
                keycode = 128

            if event.key == pygame.K_F2:
                keycode = 129

            if event.key == pygame.K_F9:
                keycode = 130

            if event.key < 0x7F:
                keycode = event.key
                mod = pygame.key.get_mods() #KMOD_SHIFT, #KMOD_CAPS

                if keycode >= 0x61 and keycode <= 0x7A and (mod & pygame.KMOD_CAPS) == 0:
                    keycode = keycode - 0x20

                ch = chr(event.key)
                pressed = pygame.key.get_pressed()
                if pressed[pygame.K_RSHIFT] or pressed[pygame.K_LSHIFT] and ch in src:
                    ch = dest[src.index(ch)]
                    keycode = ord(ch)

                keyctrl = event.key
                if (pressed[pygame.K_RCTRL] or pressed[pygame.K_LCTRL]) and keyctrl >= 0x61 and keyctrl <= 0x7A:
                    keycode = keyctrl - 0x60

            if event.key == pygame.K_UP:
                keycode = 0x0B
            if event.key == pygame.K_DOWN:
                keycode = 0x0A
            if event.key == pygame.K_LEFT:
                keycode = 0x08
            if event.key == pygame.K_RIGHT:
                keycode = 0x15

    to_send = bytes([keycode])
    packet = bytearray()
    packet.append(0x81)
    packet.append(keycode)
    packet.append(button_0)
    packet.append(button_1)
    packet.append(paddle_0)
    packet.append(paddle_1)
    ser.write(packet)

    pygame.draw.rect(screen, BLACK, (0, 0, SCREEN_X_TOTAL, SCREEN_Y_TOTAL))
    pygame.display.flip()

    clock.tick(60)

pygame.quit()

ser.flush()
ser.close()
print("end serial")
