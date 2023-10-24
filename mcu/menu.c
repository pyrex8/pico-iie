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
#define FLASH_BANK_SIZE (FLASH_SECTOR_SIZE * FLASH_SECTOR_COUNT) // 0xD000
#define FLASH_PROGRAM_SIZE (FLASH_PAGE_SIZE * FLASH_WRITE_SIZE) // 0xC100

// 0xD000 x 24 = 1277952, 1277952 / 1024 = 1248k
#define FLASH_TARGET_OFFSET (512 * 1024) // choosing to start at 512K
#define FLASH_TARGET_BASE (XIP_BASE + FLASH_TARGET_OFFSET)

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

static uint8_t menu_bank = 0;
static uint8_t menu_file_name;

struct __attribute__((__packed__)) storage
{
    struct __attribute__((__packed__)) menu 
    {
        uint8_t bank;
        uint8_t name[MENU_NAME_LENGTH];
        uint16_t file_size;
        uint16_t address;
    } menu;
    uint8_t binary[MENU_BIN_LENGTH];
} storage;


void flash_data_save(uint8_t bank)
{
    const uint8_t *data = (const uint8_t *) &storage;
    uint32_t flash_offset = FLASH_TARGET_OFFSET + bank * FLASH_BANK_SIZE;
    uint32_t interrupts = save_and_disable_interrupts();
    flash_range_erase(flash_offset, FLASH_BANK_SIZE);
    flash_range_program(flash_offset, data, FLASH_PROGRAM_SIZE);
    restore_interrupts(interrupts);
}

void flash_menu_read(uint8_t bank)
{
    const uint8_t* flash_target_contents = (const uint8_t *) (FLASH_TARGET_BASE + bank * FLASH_BANK_SIZE);
    memcpy(&storage.menu, flash_target_contents, MENU_PAGE_LENGTH);
}


void flash_data_read(uint8_t bank)
{
    const uint8_t* flash_target_contents = (const uint8_t *) (FLASH_TARGET_BASE + bank * FLASH_BANK_SIZE);
    memcpy(&storage, flash_target_contents, MENU_PAGE_LENGTH);
}

void menu_str_print(uint8_t *str, uint8_t x, uint8_t y)
{
    for (int i = 0; i < MENU_NAME_LENGTH; i++)
    {
        if (str[i] >= MENU_CHARACTER_OFFSET)
        {
            str[i] = '.';
        }
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
        flash_menu_read(i);
        menu[line[i] + MENU_BANK_NUMBER_COLUMN] = menu_select[i] + MENU_CHARACTER_OFFSET;
        menu_str_print(storage.menu.name, MENU_FILE_NAME_COLUMN, i);
        menu_hex_print(storage.menu.file_size, MENU_FILE_SIZE_COLUMN, i);
        menu_hex_print(storage.menu.address, MENU_BIN_ADDRESS_COLUMN, i);
    }

    uint16_t value = FLASH_SECTOR_SIZE;
    menu_hex_print(value, 10, 10);

    uint8_t name[] = "FILE_NAME                    ";
    memcpy(storage.menu.name, name, MENU_NAME_LENGTH);
    menu_str_print(storage.menu.name, MENU_FILE_NAME_COLUMN, 0);
    menu_hex_print(storage.menu.file_size, MENU_FILE_SIZE_COLUMN, 0);
    menu_hex_print(storage.menu.address, MENU_BIN_ADDRESS_COLUMN, 0);
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
    storage.menu.bank = data;
}

void menu_name_set(uint8_t data)
{
    menu_file_name = data;
}