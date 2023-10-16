#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include "menu.h"
#include "hardware/flash.h"
#include "pico/stdlib.h"

static uint8_t menu[MENU_CHARACTERS_SIZE];
const static uint8_t menu_select[] = 
{
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
    'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
};
const static uint16_t line[] =
{
    0x000, 0x080, 0x100, 0x180, 0x200, 0x280, 0x300, 0x380,
    0x028, 0x0A8, 0x128, 0x1A8, 0x228, 0x2A8, 0x328, 0x3A8,
    0x050, 0x0D0, 0x150, 0x1D0, 0x250, 0x2D0, 0x350, 0x3D0,
};

void menu_init(void)
{
    memset(menu, '.' + 128, MENU_CHARACTERS_SIZE);
    for (int i = 0; i < sizeof(menu_select); i++)
    {
        menu[line[i]] = ' ' + 128;
        menu[line[i] + 1] = menu_select[i] + 128;
        menu[line[i] + 2] = ' ' + 128;
    }
}

void menu_update(void)
{

}

void menu_data_get(uint8_t *data)
{
    memcpy(data, &menu[0], MENU_CHARACTERS_SIZE);
}
