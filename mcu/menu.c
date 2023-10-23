#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include "menu.h"
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/stdlib.h"

#define MENU_BANKS_TOTAL 24
#define MENU_BANK_NUMBER_LENGTH 1
#define MENU_NAME_LENGTH 27
#define MENU_HEX_LENGTH 4
#define MENU_ITEM_SPACING 1
#define MENU_BANK_NUMBER_COLUMN (MENU_ITEM_SPACING)
#define MENU_FILE_NAME_COLUMN (MENU_BANK_NUMBER_COLUMN +MENU_ITEM_SPACING +  MENU_BANK_NUMBER_LENGTH)
#define MENU_FILE_SIZE_COLUMN (MENU_FILE_NAME_COLUMN + MENU_ITEM_SPACING + MENU_NAME_LENGTH)
#define MENU_BIN_ADDRESS_COLUMN (MENU_FILE_SIZE_COLUMN + MENU_ITEM_SPACING + MENU_HEX_LENGTH)
#define MENU_CHARACTER_OFFSET 128
#define MENU_BIN_LENGTH 0xC000
#define MENU_PAGE_LENGTH (MENU_BIN_LENGTH + MENU_NAME_LENGTH + 5)

// FLASH_SECTOR_SIZE 0x1000
#define FLASH_WRITE_SIZE ((MENU_PAGE_LENGTH / FLASH_PAGE_SIZE) + 1) // 0xC1
#define FLASH_SECTOR_COUNT (((FLASH_WRITE_SIZE * FLASH_PAGE_SIZE) / FLASH_SECTOR_SIZE) + 1) // 0x0D
#define FLASH_ERASE_SIZE (FLASH_SECTOR_SIZE * FLASH_SECTOR_COUNT) // 0xD000
#define FLASH_PROGRAM_SIZE (FLASH_PAGE_SIZE * FLASH_WRITE_SIZE) // 0xC100

#define FLASH_TARGET_OFFSET (512 * 1024) // choosing to start at 512K




static uint8_t menu[MENU_CHARACTERS_SIZE];
const static uint8_t menu_select[MENU_BANKS_TOTAL] = 
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
        uint16_t file_size;
        uint16_t bin_address;
        uint8_t bin_data[MENU_BIN_LENGTH];
   } menu;
} menu_page;


void flash_data_save()
{
    const uint8_t *data = (const uint8_t *) &menu_page.flash_data;
    uint32_t interrupts = save_and_disable_interrupts();
    flash_range_erase(FLASH_TARGET_OFFSET, FLASH_ERASE_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, data, FLASH_PROGRAM_SIZE);
    restore_interrupts(interrupts);
}

void flash_data_read()
{
    const uint8_t* flash_target_contents = (const uint8_t *) (XIP_BASE + FLASH_TARGET_OFFSET);
    memcpy(&menu_page.flash_data, flash_target_contents, MENU_PAGE_LENGTH);
}

void menu_str_print(uint8_t *str, uint8_t x, uint8_t y)
{
    for (int i = 0; i < MENU_NAME_LENGTH; i++)
    {
        menu[line[y] + x + i] = str[i] + MENU_CHARACTER_OFFSET;
    }
}

void menu_hex_print(uint16_t value, uint8_t x, uint8_t y)
{
    menu[line[y] + x] = menu_select[(value >> 12) & 0x0F] + MENU_CHARACTER_OFFSET;
    menu[line[y] + x + 1] = menu_select[(value >>  8) & 0x0F] + MENU_CHARACTER_OFFSET;
    menu[line[y] + x + 2] = menu_select[(value >>  4) & 0x0F] + MENU_CHARACTER_OFFSET;
    menu[line[y] + x + 3] = menu_select[(value) & 0x0F] + MENU_CHARACTER_OFFSET;
}

void menu_init(void)
{
    memset(menu, ' ' + MENU_CHARACTER_OFFSET, MENU_CHARACTERS_SIZE);
    for (int i = 0; i < sizeof(menu_select); i++)
    {
        menu[line[i] + MENU_BANK_NUMBER_COLUMN] = menu_select[i] + MENU_CHARACTER_OFFSET;
    }

    flash_data[0] = 'b' + MENU_CHARACTER_OFFSET;

    // flash_data_save();
   flash_data_read();

    menu[line[0] + 3] = flash_data[0];
    menu[line[0] + 4] = flash_data[1];
    menu[line[0] + 5] = flash_data[2];

    menu[line[23] + 3] = menu_select[menu_bank] + MENU_CHARACTER_OFFSET;

    uint16_t value = FLASH_SECTOR_SIZE;
    menu_hex_print(value, 10, 10);

    uint8_t name[] = "FILE_NAME                    ";
    memcpy(menu_page.menu.name, name, MENU_NAME_LENGTH);
    menu_str_print(menu_page.menu.name, MENU_FILE_NAME_COLUMN, 1);
    menu_hex_print(menu_page.menu.file_size, MENU_FILE_SIZE_COLUMN, 1);
    menu_hex_print(menu_page.menu.bin_address, MENU_BIN_ADDRESS_COLUMN, 1);
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