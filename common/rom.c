#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "rom.h"
#include "rom_data.h"

#define ROM_LOCATION  0xC000
#define ROM_END 0xFFFF
#define ROM_SIZE (ROM_END - ROM_LOCATION)
#define ROM_RESET_VECTOR_LB_LOCATION  (0xFFFC - ROM_LOCATION)
#define ROM_RESET_VECTOR_HB_LOCATION  (0xFFFD - ROM_LOCATION)

static uint8_t rom_in_ram[ROM_SIZE];

void rom_init(void)
{
    memcpy(rom_in_ram, iie_enhanced_rom, ROM_SIZE);
}

void rom_update(uint8_t read, uint16_t address, uint8_t *byte)
{
  if ((read) && (address >= ROM_LOCATION))
  {
    *byte = rom_in_ram[address - ROM_LOCATION];
  }
}

void rom_reset_vector_write(uint16_t address)
{
    rom_in_ram[ROM_RESET_VECTOR_LB_LOCATION] = (uint8_t)((address) & 0xFF);
    rom_in_ram[ROM_RESET_VECTOR_HB_LOCATION] = (uint8_t)((address >> 8) & 0xFF);
}
