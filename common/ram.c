#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include "ram.h"

#define RAM_SIZE  0xC000

static uint8_t memory_ram[RAM_SIZE] = {0};
static uint16_t location = 0;
static uint16_t location_start = 0;

void ram_init(void)
{
    memset(memory_ram, 0, RAM_SIZE);
}

void ram_deinit(void)
{

}

void ram_update(uint8_t read, uint16_t address, uint8_t *byte)
{
    if (address < RAM_SIZE)
    {
        if (read)
        {
            *byte = memory_ram[address];
        }
        else
        {
            memory_ram[address] = *byte;
        }
    }
}


void ram_data_get(uint16_t length, uint16_t address, uint8_t *data)
{
    memcpy(data, &memory_ram[address], length);
}

void ram_data_set(uint16_t length, uint16_t address, uint8_t *data)
{
    memcpy(&memory_ram[address], data, length);
}

void ram_all_get(uint8_t *buffer)
{
   memcpy(buffer, memory_ram, RAM_SIZE);
}

void ram_bin_reset(uint8_t unused)
{
    location = location_start;
}

void ram_bin_addr_lsb(uint8_t data)
{
    location &= 0xFF00;
    location |= data;
    location_start = location;
}

void ram_bin_addr_msb(uint8_t data)
{
    location &= 0x00FF;
    location |= (((uint16_t)data) << 8);
    location_start = location;
}

uint16_t ram_bin_addr_get(void)
{
    return location_start;
}

void ram_bin_data_set(uint8_t data)
{
    ram_update(MEMORY_WRITE, location, &data);
    location++;
}
