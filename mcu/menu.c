#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include "menu.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/stdlib.h"

#define MENU_NAME_LENGTH 24
#define MENU_BIN_LENGTH 0xC000
#define MENU_PAGE_LENGTH (MENU_BIN_LENGTH + MENU_NAME_LENGTH + 5)
#define FLASH_TARGET_OFFSET (512 * 1024) // choosing to start at 512K

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

const uint8_t* flash_target_contents = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);
static uint8_t flash_data[40];
static uint8_t menu_bank = 0;
static uint8_t menu_file_name;

union menu_page
{
   uint8_t flash_data[MENU_PAGE_LENGTH];
   struct __attribute__((__packed__)) menu
   {
        uint8_t bank;
        uint8_t name[MENU_NAME_LENGTH];
        uint8_t file_size_lsb;
        uint8_t file_size_msb;
        uint8_t bin_address_lsb;
        uint8_t bin_address_msb;
        uint8_t bin_data[MENU_BIN_LENGTH];
   } menu;
} menu_page;


void flash_data_save()
{
    int flash_dataSize = sizeof(flash_data);

    int writeSize = (flash_dataSize / FLASH_PAGE_SIZE) + 1;
    int sectorCount = ((writeSize * FLASH_PAGE_SIZE) / FLASH_SECTOR_SIZE) + 1;
    uint32_t interrupts = save_and_disable_interrupts();

    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_SECTOR_SIZE * sectorCount);
    flash_range_program(FLASH_TARGET_OFFSET, flash_data, FLASH_PAGE_SIZE * writeSize);
    restore_interrupts(interrupts);
}

void menu_str_print(uint8_t *str, uint8_t len, uint8_t x, uint8_t y)
{
    for (int i = 0; i < len; i++)
    {
        if (str[i] != '\0')
        {
            menu[line[y] + x + i] = str[i] + 128;
        }
    }
}

void menu_hex_print(uint16_t value, uint8_t x, uint8_t y)
{
    menu[line[y] + x] = menu_select[(value >> 12) & 0x0F] + 128;
    menu[line[y] + x + 1] = menu_select[(value >>  8) & 0x0F] + 128;
    menu[line[y] + x + 2] = menu_select[(value >>  4) & 0x0F] + 128;
    menu[line[y] + x + 3] = menu_select[(value) & 0x0F] + 128;
}

void flash_data_read()
{
    memcpy(&flash_data, flash_target_contents, sizeof(flash_data));
}

void menu_init(void)
{
    memset(menu, '.' + 128, MENU_CHARACTERS_SIZE);
    for (int i = 0; i < sizeof(menu_select); i++)
    {
        menu[line[i]] = ' ' + 128;
        menu[line[i] + 1] = menu_select[i] + 128;
        menu[line[i] + 2] = ' ' + 128;
    }

    flash_data[0] = 'b' + 128;

    // flash_data_save();
   flash_data_read();

    menu[line[0] + 3] = flash_data[0];
    menu[line[0] + 4] = flash_data[1];
    menu[line[0] + 5] = flash_data[2];

    menu[line[23] + 3] = menu_select[menu_bank] + 128;

    uint16_t value = sizeof(menu_page);
    menu_hex_print(value, 10, 10);

    uint8_t name[] = "FILE";
    menu_str_print(name, sizeof(name), 5, 5);
}

void menu_update(void)
{

}

void menu_data_get(uint8_t *data)
{
    memcpy(data, &menu[0], MENU_CHARACTERS_SIZE);
}

void menu_bank_set(uint8_t data)
{
    menu_page.menu.bank = data;
}

void menu_name_set(uint8_t data)
{
    menu_file_name = data;
}