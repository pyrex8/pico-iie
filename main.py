#!/usr/bin/env python3
"""
Apple //e emulator in C and pygame
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

DISK_NIB_TRACK_SIZE = 6384
DISK_NIB_SIZE = (DISK_TRACKS * DISK_NIB_TRACK_SIZE)

VOLUME = 0xFE

# 2 and 6 write translate table
diskbyte = [
    0x96,0x97,0x9A,0x9B,0x9D,0x9E,0x9F,0xA6,
    0xA7,0xAB,0xAC,0xAD,0xAE,0xAF,0xB2,0xB3,
    0xB4,0xB5,0xB6,0xB7,0xB9,0xBA,0xBB,0xBC,
    0xBD,0xBE,0xBF,0xCB,0xCD,0xCE,0xCF,0xD3,
    0xD6,0xD7,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,
    0xDF,0xE5,0xE6,0xE7,0xE9,0xEA,0xEB,0xEC,
    0xED,0xEE,0xEF,0xF2,0xF3,0xF4,0xF5,0xF6,
    0xF7,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF]

sectornumber = [
    0x00,0x07,0x0E,0x06,0x0D,0x05,0x0C,0x04,
    0x0B,0x03,0x0A,0x02,0x09,0x01,0x08,0x0F]

sector_start_location = 0
offset = 0
value = 0

def A(x):
    return ((((x) >> 1) & DISK_BITS_ODD) | DISK_BITS_EVEN)

def B(x):
    return (((x) & DISK_BITS_ODD) | DISK_BITS_EVEN)

def C(x):
    return (((x) & 0x01) << 1) | (((x) & 0x02) >> 1)

def nibblized_track(track, disk_data):
    track_index = 0
    disk_data_index = 0
    data_pass_1 = bytearray(DISK_SECTOR_NIB_SIZE)
    data_pass_2 = bytearray(DISK_SECTOR_NIB_SIZE)
    track_image = bytearray(DISK_NIB_TRACK_SIZE)

    track_image[track_index:track_index + 48] = [DISK_SELF_SYNC_BYTE] * 48
    track_index += 48

    # address field
    for sector in range(16):

        track_image[track_index:track_index + 3] = [0xD5, 0xAA, 0x96]
        track_index += 3

        track_image[track_index:track_index + 2] = [A(VOLUME), B(VOLUME)]
        track_index += 2

        track_image[track_index:track_index + 2] = [A(track), B(track)]
        track_index += 2

        track_image[track_index:track_index + 2] = [A(sector), B(sector)]
        track_index += 2

        track_image[track_index] = A(VOLUME ^ track ^ sector)
        track_index += 1
        track_image[track_index] = B(VOLUME ^ track ^ sector)
        track_index += 1

        track_image[track_index:track_index + 3] = [0xDE, 0xAA, 0xEB]
        track_index += 3

        # gap 2
        track_image[track_index:track_index + 6] = [DISK_SELF_SYNC_BYTE] * 6
        track_index += 6

        # data field
        track_image[track_index:track_index + 3] = [0xD5, 0xAA, 0xAD]
        track_index += 3

        sector_start_location = sectornumber[sector] << 8
        disk_data_index = (track << 12) + sector_start_location

        # Convert the 256 8-bit bytes into 342 6-bit bytes
        data_index = 0
        offset = 0xAC

        while offset != 0x02:
            value = 0
            value = (value << 2) | C(disk_data[disk_data_index + offset])
            offset = (0x100 + offset - 0x56) & 0xFF
            value = (value << 2) | C(disk_data[disk_data_index + offset])
            offset = (0x100 + offset - 0x56) & 0xFF
            value = (value << 2) | C(disk_data[disk_data_index + offset])
            offset = (0x100 + offset - 0x53) & 0xFF
            data_pass_1[data_index] = (value << 2)  & 0xFF
            data_index += 1

        for i in range(DISK_SECTOR_SIZE):
            data_pass_1[data_index] = disk_data[disk_data_index + i]
            data_index += 1

        # XOR data block with itself offset by one byte. Checksum is 343rd byte
        value  = 0;
        for i in range(DISK_SECTOR_NIB_SIZE - 1):
            data_pass_2[i] = (value ^ data_pass_1[i]) & 0xFF
            value = data_pass_1[i]

        data_pass_2[DISK_SECTOR_NIB_SIZE - 1] = value

        # Convert 6-bit bytes into disk bytes using lookup table.
        for i in range(DISK_SECTOR_NIB_SIZE):
            track_image[track_index + i] = diskbyte[(data_pass_2[i]) >> 2]

        track_index += DISK_SECTOR_NIB_SIZE

        track_image[track_index:track_index + 3] = [0xDE, 0xAA, 0xEB]
        track_index += 3

        # gap 3
        track_image[track_index:track_index + 27] = [DISK_SELF_SYNC_BYTE] * 27
        track_index += 27

    return track_image



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
joystick_x = 0
joystick_y = 0
button_0 = 0
button_1 = 0
joystick_byte_0 = 0
joystick_byte_1 = 0

pygame.init()
pygame.display.set_caption('pyrex8')
# icon = pygame.image.load('icon.png')
# pygame.display.set_icon(icon)
screen = pygame.display.set_mode([SCREEN_X_TOTAL, SCREEN_Y_TOTAL])
clock = pygame.time.Clock()

running = True

while running:

    if joystick_present:
        joystick_x = int((joystick.get_axis(0) + 1) * 127)
        joystick_y = int((joystick.get_axis(1) + 1) * 127)
        button_0 = int(joystick.get_button(0))
        button_1 = int(joystick.get_button(1))
        joystick_byte_0 = (joystick_x & 0xFE) + (button_0 & 0x01)
        joystick_byte_1 = (joystick_y & 0xFE) + (button_1 & 0x01)
    else:
        if pygame.joystick.get_count() > 0:
            joystick = pygame.joystick.Joystick(0)
            joystick.init()
            joystick_present = 1
        joystick_byte_0 = 128
        joystick_byte_1 = 128

    keycode = 0

    for event in pygame.event.get():

        if event.type == pygame.QUIT:
            running = False

        if event.type == pygame.KEYDOWN:
            if event.key == pygame.K_F12:
                running = False

            if event.key == pygame.K_F1:
                keycode = 128

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
    packet.append(joystick_byte_0)
    packet.append(joystick_byte_1)
    ser.write(packet)

    pygame.draw.rect(screen, BLACK, (0, 0, SCREEN_X_TOTAL, SCREEN_Y_TOTAL))

    pygame.display.flip()

    clock.tick(60)

pygame.quit()

ser.flush()

ser.close()

print("end serial")
