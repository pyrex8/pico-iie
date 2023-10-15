#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include "menu.h"
#include "hardware/flash.h"
#include "pico/stdlib.h"

static uint8_t menu[MENU_CHARACTERS_Y][MENU_CHARACTERS_X];


void menu_init(void)
{
    menu[0][0] = '0' + 128;
    menu[1][0] = '8' + 128;
}

void menu_update(void)
{

}

void menu_data_get(uint8_t *data)
{
    memcpy(data, &menu[0][0], MENU_CHARACTERS_SIZE);
}
