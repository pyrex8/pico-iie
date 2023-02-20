#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include "keyboard.h"

#define KEYBOARD_MASK 0xFFF0

static bool mem_key_waiting = false;
static uint8_t mem_key_code = 0;

uint8_t keyboard_data_read(void)
{
    int rtn = mem_key_code | (mem_key_waiting ? 0x80 : 0);
    return rtn;
}

uint8_t keyboard_flag_read(void)
{
    int rtn = mem_key_code | (mem_key_waiting ? 0x80 : 0);

    mem_key_waiting = false;
    return rtn;
}

void keyboard_update(uint8_t read, uint16_t address, uint8_t *byte)
{
    if((address & KEYBOARD_MASK) == 0xC000)
    {
        *byte = keyboard_data_read();
    }

    if((address & KEYBOARD_MASK) == 0xC010)
    {
        *byte = keyboard_flag_read();
    }
}

void keyboard_key_code_set(uint8_t key_code)
{
    if (key_code > 0 && key_code < 128)
    {
        mem_key_waiting = true;
        mem_key_code = key_code;
    }
}
