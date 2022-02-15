#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include "ram.h"

#define RAM_SIZE  0xC000

static uint8_t memory_ram[RAM_SIZE] = {0};

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


void ram_data_get(uint8_t length, uint16_t address, uint8_t *data)
{
    memcpy(data, &memory_ram[address], length);
}

void ram_all_get(uint8_t *buffer)
{
   memcpy(buffer, memory_ram, RAM_SIZE);
}
