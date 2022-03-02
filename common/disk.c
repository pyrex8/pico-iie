#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <stddef.h>
// #include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "disk.h"
#include "disk_rom.h"

#define DISK_CONTROLLER_SLOT   6
#define DISK_ROM_SIZE          0x100
#define DISK_ROM_LOCATION     (0xC000 + (DISK_CONTROLLER_SLOT * DISK_ROM_SIZE))
#define DISK_ROM_LOCATION_END (0xC000 + ((DISK_CONTROLLER_SLOT + 1) * DISK_ROM_SIZE) - 1)
#define DISK_COMMANDS_ADDRESS (0xC080 + (DISK_CONTROLLER_SLOT * 0x10))
#define DISK_COMMANDS_MASK     0xFFF0

#define DISK_PHASE0_OFF (0x00 + DISK_COMMANDS_ADDRESS)
#define DISK_PHASE0_ON  (0x01 + DISK_COMMANDS_ADDRESS)
#define DISK_PHASE1_OFF (0x02 + DISK_COMMANDS_ADDRESS)
#define DISK_PHASE1_ON  (0x03 + DISK_COMMANDS_ADDRESS)
#define DISK_PHASE2_OFF (0x04 + DISK_COMMANDS_ADDRESS)
#define DISK_PHASE2_ON  (0x05 + DISK_COMMANDS_ADDRESS)
#define DISK_PHASE3_OFF (0x06 + DISK_COMMANDS_ADDRESS)
#define DISK_PHASE3_ON  (0x07 + DISK_COMMANDS_ADDRESS)

#define DISK_MOTOR_OFF  (0x08 + DISK_COMMANDS_ADDRESS)
#define DISK_MOTOR_ON   (0x09 + DISK_COMMANDS_ADDRESS)

#define DISK_DRV0EN     (0x0A + DISK_COMMANDS_ADDRESS)
#define DISK_DRV1EN     (0x0B + DISK_COMMANDS_ADDRESS)

#define DISK_Q6L        (0x0C + DISK_COMMANDS_ADDRESS)
#define DISK_Q6H        (0x0D + DISK_COMMANDS_ADDRESS)

#define DISK_Q7L        (0x0E + DISK_COMMANDS_ADDRESS)
#define DISK_Q7H        (0x0F + DISK_COMMANDS_ADDRESS)

#define DISK_TRACKS   35
#define DISK_SECTORS 16
#define DISK_SECTOR_SIZE 256
#define DISK_SECTOR_NIB_SIZE 343

#define DISK_SELF_SYNC_BYTE 0xFF

#define DISK_BITS_EVEN 0xAA
#define DISK_BITS_ODD 0x55

#define DISK_TRACK_SIZE (DISK_SECTORS * DISK_SECTOR_SIZE)
#define DISK_SIZE (DISK_TRACKS * DISK_TRACK_SIZE)

#define DISK_NIB_TRACK_SIZE 6384 // 6.23k
#define DISK_NIB_SIZE (DISK_TRACKS * DISK_NIB_TRACK_SIZE)

#define  MAX(a,b) (((a) > (b)) ? (a) : (b))
#define  MIN(a,b) (((a) < (b)) ? (a) : (b))

static uint8_t memory_disk_rom[DISK_ROM_SIZE] = {0};
static uint8_t disk_data[DISK_SIZE] = {0};   // 143360 bytes 140kb
static uint8_t *disk_nib_track = disk_data;
static uint8_t disk_track = 0;
static uint8_t disk_phase = 0;
static uint8_t disk_track_ready = 0;
static uint8_t disk_track_new_data = 0;
static uint16_t disk_track_nibble_loc = 0;
static uint8_t disk_latch = 0;
static uint8_t disk_motor_on = 0;
static uint8_t disk_write_mode = 0;
static uint8_t disk_stepper_phases = 0;
static uint8_t disk_write = 0;

uint8_t disk_spinning_test(void);

uint8_t disk_control_stepper(uint8_t command);
uint8_t disk_control_motor(uint8_t command);
uint8_t disk_enable(uint8_t command);
uint8_t disk_read_write(uint8_t command);
uint8_t disk_latch_value_set(uint8_t read, uint8_t byte);
uint8_t disk_read_mode_set(uint8_t command);
uint8_t disk_write_mode_set(uint8_t command);

void disk_init(void)
{
    uint8_t *disk_rom_data = (uint8_t*)disk2_rom;
    memcpy(memory_disk_rom, disk_rom_data, DISK_ROM_SIZE);

    disk_track = 0;
    disk_phase = 0;
    disk_track_ready = 0;
    disk_track_new_data = 0;
    disk_track_nibble_loc = 0;
    disk_latch = 0;
    disk_motor_on = 0;
    disk_write_mode = 0;
    disk_stepper_phases = 0;
}

void disk_update(uint8_t read, uint16_t address, uint8_t *byte)
{
    uint8_t command = address & 0x0F;

    if ((read) && (address >= DISK_ROM_LOCATION) && (address <= DISK_ROM_LOCATION_END))
    {
       *byte = memory_disk_rom[address - DISK_ROM_LOCATION];
    }
    if ((address & DISK_COMMANDS_MASK) == DISK_COMMANDS_ADDRESS)
    {
        switch(command)
        {
            case 0x0 ... 0x7:
            {
                *byte = disk_control_stepper(command);
                break;
            }
            case 0x8 ... 0x9:
            {
                *byte = disk_control_motor(command);
                break;
            }
            case 0xA ... 0xB:
            {
                *byte = disk_enable(command);
                break;
            }
            case 0xC:
            {
                *byte = disk_read_write(command);
                break;
            }
            case 0xD:
            {
                *byte = disk_latch_value_set(read, *byte);
                break;
            }
            case 0xE:
            {
                *byte = disk_read_mode_set(command);
                break;
            }
            case 0xF:
            {
                *byte = disk_write_mode_set(command);
                break;
            }
        }
    }
}


uint8_t MemReturnRandomData(uint8_t highbit)
{
  uint8_t retval[16] =
  {
    0x00,0x2D,0x2D,0x30,0x30,0x32,0x32,0x34,
    0x35,0x39,0x43,0x43,0x43,0x60,0x7F,0x7F
  };

  uint8_t r = (uint8_t)(rand() & 0xFF);
  if (r <= 170)
  {
    return 0x20 | (highbit ? 0x80 : 0);
  }
  else
  {
    return retval[r & 15] | (highbit ? 0x80 : 0);
  }
}


void ReadTrack(void)
{
    disk_nib_track = &disk_data[disk_track * DISK_NIB_TRACK_SIZE];

    disk_track_nibble_loc = 0;
    disk_track_ready = 1;
}

void WriteTrack(void)
{
  disk_write = 1;
  disk_track_new_data = 0;
}

uint8_t disk_not_spinning_test(void)
{
    if (disk_motor_on)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

uint8_t disk_control_stepper(uint8_t command)
{
  int phase     = (command >> 1) & 3;
  int phase_bit = (1 << phase);

  // update the magnet states
  if (command & 1)
  {
    // phase on
    disk_stepper_phases |= phase_bit;
  }
  else
  {
    // phase off
    disk_stepper_phases &= ~phase_bit;
  }

  int direction = 0;
  if (disk_stepper_phases & (1 << ((disk_phase + 1) & 3)))
    direction += 1;
  if (disk_stepper_phases & (1 << ((disk_phase + 3) & 3)))
    direction -= 1;

  // apply magnet step, if any
  if (direction)
  {
    disk_phase = MAX(0, MIN(79, disk_phase + direction));
    int newtrack = MIN(DISK_TRACKS - 1, disk_phase >> 1); // (round half tracks down)
    if (newtrack != disk_track)
    {
      if (disk_track_new_data)
      {
        WriteTrack();
      }
      disk_track = newtrack;
      disk_track_ready = 0;
    }
  }
  return (command == 0xE0) ? 0xFF : MemReturnRandomData(1);
}

uint8_t disk_control_motor(uint8_t command)
{
    disk_motor_on = command & 1;
    return MemReturnRandomData(1);
}

uint8_t disk_enable(uint8_t command)
{
    return 0;
}

uint8_t disk_read_write(uint8_t command)
{
 uint8_t result = 0;

  if (!disk_track_ready)
  {
    ReadTrack();
  }

  if (disk_write_mode)
  {
    if (disk_latch & 0x80)
    {
      *(disk_nib_track + disk_track_nibble_loc) = disk_latch;
      disk_track_new_data = 1;
    }
    else
    {
      return 0;
    }
  }
  else
  {
    result = *(disk_nib_track + disk_track_nibble_loc);
  }

  disk_track_nibble_loc++;
  if (disk_track_nibble_loc >= DISK_NIB_TRACK_SIZE)
  {
    disk_track_nibble_loc = 0;
  }

  return result;
}

uint8_t disk_latch_value_set(uint8_t read, uint8_t byte)
{
    uint8_t write = !read;
    if (write)
    {
        disk_latch = byte;
    }
    return disk_latch;
}

uint8_t disk_read_mode_set(uint8_t command)
{
    disk_write_mode = 0;
    return MemReturnRandomData(0);
}


uint8_t disk_write_mode_set(uint8_t command)
{
    disk_write_mode = 1;
    return MemReturnRandomData(1);
}

void disk_file_data_set(uint32_t location, uint8_t data)
{
  disk_data[location] = data;
}
