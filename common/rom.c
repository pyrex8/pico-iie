#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <stddef.h>
// #include <stdio.h>
// #include <string.h>
#include <stdlib.h>

#include "rom.h"
#include "rom_data.h"

#define ROM_LOCATION  0xC000
#define ROM_RESET_VECTOR_LB_LOCATION  0xFFFC
#define ROM_RESET_VECTOR_HB_LOCATION  0xFFFD

static uint8_t rom_reset_vector_lb = 0;
static uint8_t rom_reset_vector_hb = 0;

void rom_init(void)
{
    rom_reset_vector_lb = apple2e_enhanced_rom[ROM_RESET_VECTOR_LB_LOCATION - ROM_LOCATION];
    rom_reset_vector_hb = apple2e_enhanced_rom[ROM_RESET_VECTOR_HB_LOCATION - ROM_LOCATION];
}

void rom_deinit(void)
{
}

void rom_update(uint8_t read, uint16_t address, uint8_t *byte)
{
  if ((read) && (address >= ROM_LOCATION))
  {
    *byte = apple2e_enhanced_rom[address - ROM_LOCATION];
  }

  if (address == ROM_RESET_VECTOR_LB_LOCATION)
  {
      *byte = rom_reset_vector_lb;
  }

  if (address == ROM_RESET_VECTOR_HB_LOCATION)
  {
      *byte = rom_reset_vector_hb;
  }
}

void rom_reset_vector_write(uint8_t lower_byte, uint8_t upper_byte)
{
    rom_reset_vector_lb = lower_byte;
    rom_reset_vector_hb = upper_byte;
}
