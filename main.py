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

DISK_TRACKS = 35
DISK_SECTORS = 16
DISK_SECTOR_SIZE = 256
DISK_SECTOR_NIB_SIZE = 343

DISK_SELF_SYNC_BYTE = 0xFF

DISK_BITS_EVEN = 0xAA
DISK_BITS_ODD = 0x55

DISK_TRACK_SIZE = (DISK_SECTORS * DISK_SECTOR_SIZE)
DISK_SIZE = (DISK_TRACKS * DISK_TRACK_SIZE)

disk_name = ""
bin_name = ""

PIXELS_X = 280
PIXELS_Y = 192
SCREEN_SCALE = 4
SCREEN_X = PIXELS_X * SCREEN_SCALE
SCREEN_Y = PIXELS_Y * SCREEN_SCALE
CHAR_X = 7 * SCREEN_SCALE
CHAR_Y = 8 * SCREEN_SCALE
SCREEN_X_TOTAL = SCREEN_X
SCREEN_Y_TOTAL = SCREEN_Y

VIDEO_MEMORY_SEGMENTS = 3
VIDEO_SEGMENT_OFFSET = 0x28
VIDEO_LINES_PER_SEGMENT = 8
SCREEN_LsdfINE_OFFSET = 0x80
TEXTPAGE1 = 0x400

DSK_FILE_SIZE = 143360

TEXTPAGESIZE = 0x3FF
TEXTCOLUMNS = 40
TEXTLINES = 24

TEXTINVERTED = 128

BLACK = (0, 0, 0)
GREEN = (66, 230, 6)
PURPLE = (186, 23, 245)
ORANGE = (228, 83, 4)
BLUE = (25, 171, 249)
WHITE = (255, 255, 255)

HCOLOR = [BLACK, PURPLE, GREEN, GREEN, PURPLE, BLUE, ORANGE, ORANGE, BLUE, WHITE]

AUDIO_MAGNITUDE = 12000

EXECUTION_PERIOD_CLKS = 17030 # cycles

SOUND_BUFFER_LENGTH = 2000

dest = r'~!@#$%^&*()_+{}|:"<>?'
src =  r"`1234567890-=[]\;',./"

BAUDRATE = 230400
COM_PORT = '/dev/ttyUSB0'
ser = serial.Serial(COM_PORT, baudrate=BAUDRATE, rtscts=False)
ser.flush()

try:
    arg_name = str(sys.argv[1])
    file_ext = arg_name[-3:]
    if  file_ext == 'dsk':
        disk_name = arg_name
    if  file_ext == 'bin':
        bin_name = arg_name
except:
    pass

if disk_name != "":
    # read disk on startup
    disk_image = open(disk_name, 'rb')

    disk_file_len = 0
    disk_file = bytearray()
    disk_file.append(0x83)

    while 1:

        # read by character
        char = disk_image.read(1)

        if not char:
            break

        file_data = ord(char)
        disk_file.append(file_data)
        disk_file_len += 1

    print('disk file length =',disk_file_len)
    disk_image.close()

    ser.flush()
    ser.write(disk_file)

    print('Disk:', disk_name)

if bin_name != "":
    # read bin file on startup
    bin_image = open(bin_name, 'rb')

    bin_file_len = 0
    bin_file = bytearray()
    bin_file.append(0x82)

    while 1:

        # read by character
        char = bin_image.read(1)

        if not char:
            break

        bin_data = ord(char)

        bin_file.append(bin_data)
        bin_file_len += 1

    while bin_file_len < 0x8000:
        bin_file.append(0x00)
        bin_file_len += 1

    print('bin file length =',bin_file_len)
    bin_image.close()

    ser.flush()
    ser.write(bin_file)

    print('Binary:', bin_name)

joystick_present = 0
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
        button_1 = int(joystick.get_button(1))
        paddle_0 = int((joystick.get_axis(0) + 1) * 127)
        paddle_1 = int((joystick.get_axis(1) + 1) * 127)
    else:
        if pygame.joystick.get_count() > 0:
            joystick = pygame.joystick.Joystick(0)
            joystick.init()
            joystick_present = 1
        button_0 = 0
        button_1 = 0
        paddle_0 = 128
        paddle_1 = 128

    keycode = 0

    for event in pygame.event.get():

        if event.type == pygame.QUIT:
            running = False

        if event.type == pygame.KEYDOWN:
            if event.key == pygame.K_F12:
                running = False

            if event.key == pygame.K_F1:
                keycode = 128

            if event.key == pygame.K_F2:
                keycode = 129

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
