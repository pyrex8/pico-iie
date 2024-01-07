// Includes -------------------------------------------------------------------
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include "key.h"
#include "pico/stdlib.h"

// Definitions ----------------------------------------------------------------
#define KEY_DATA_PIN 14
#define KEY_SCK_PIN 15
#define KEY_MATRIX_MASK 0x7F
#define KEY_MATRIX_TOTAL (KEY_MATRIX_MASK + 1)
#define KEY_MATRIX_VALID (KEY_MATRIX_TOTAL - 0x10)
#define KEY_PRESSES_RESET 0
#define KEY_PRESSES_MAX 16

// Apple IIe keyboard codes
#define UP   0x0B
#define DOWN 0x0A
#define LEFT 0x08
#define RGHT 0x15
#define RTN  0x0D
#define TAB  0x09
#define ESC  0x1B
#define DEL  0x7F

// Matrix scan codes index
#define KEY_SHIFT 0x60
#define KEY_CTRL 0x38
#define KEY_CAPS_LOCK 0x28
#define KEY_RESET 0x48
#define KEY_ESC 0x07
#define KEY_TAB 0x04
#define KEY_DEL 0x5D
#define KEY_RTN 0x58

#define KEY_OFFSET_CAPS_LOCK_OFF  0x000
#define KEY_OFFSET_CAPS_LOCK_ON   0x080
#define KEY_OFFSET_CAPS_SHIFT     0x100

#define KEY_DATA_EMPTY 0
#define KEY_DATA_READY 1
#define KEY_USED_FALSE 0
#define KEY_USED_TRUE 1

#define KEY_SWITCH_CLOSED 0

// Consts ---------------------------------------------------------------------
static const uint8_t key_iie[KEY_MATRIX_TOTAL * 4] =
{
// caps lock off

//0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x00, 0x00,  'z', 0x00,  TAB, 0x00,  'a',  ESC, //0x0
  0x00, 0x00,  'x', 0x00,  'q', 0x00,  'd',  '1', //0x1
  0x00, 0x00,  'c', 0x00,  'w', 0x00,  's',  '2', //0x2
  0x00, 0x00,  'v', 0x00,  'e', 0x00,  'h',  '3', //0x3
  0x00, 0x00,  'b', 0x00,  'r', 0x00,  'f',  '4', //0x4
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x5
  0x00, 0x00,  'n', 0x00,  'y', 0x00,  'g',  '6', //0x6
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x7
   ' ',  '0',  '.',  '[',  'i', LEFT,  ';',  '8', //0x8
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x9
  '\'',  '-',  '/',  ']',  'o', RGHT,  'l',  '9', //0xA
   RTN, '\\',  'm',  '`',  't',  DEL,  'j',  '5', //0xB
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0xC
    UP,  '=',  ',',  'p',  'u', DOWN,  'k',  '7', //0xD
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0xE
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0xF

// caps lock on

//0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x00, 0x00,  'Z', 0x00,  TAB, 0x00,  'A',  ESC, //0x0
  0x00, 0x00,  'X', 0x00,  'Q', 0x00,  'D',  '1', //0x1
  0x00, 0x00,  'C', 0x00,  'W', 0x00,  'S',  '2', //0x2
  0x00, 0x00,  'V', 0x00,  'E', 0x00,  'H',  '3', //0x3
  0x00, 0x00,  'B', 0x00,  'R', 0x00,  'F',  '4', //0x4
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x5
  0x00, 0x00,  'N', 0x00,  'Y', 0x00,  'G',  '6', //0x6
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x7
   ' ',  '0',  '.',  '[',  'I', LEFT,  ';',  '8', //0x8
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x9
  '\'',  '-',  '/',  ']',  'O', RGHT,  'L',  '9', //0xA
   RTN, '\\',  'M',  '`',  'T',  DEL,  'J',  '5', //0xB
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0xC
    UP,  '=',  ',',  'P',  'U', DOWN,  'K',  '7', //0xD
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0xE
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0xF

// shift + caps lock off

//0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x00, 0x00,  'Z', 0x00,  TAB, 0x00,  'A',  ESC, //0x0
  0x00, 0x00,  'X', 0x00,  'Q', 0x00,  'D',  '!', //0x1
  0x00, 0x00,  'C', 0x00,  'W', 0x00,  'S',  '@', //0x2
  0x00, 0x00,  'V', 0x00,  'E', 0x00,  'H',  '#', //0x3
  0x00, 0x00,  'B', 0x00,  'R', 0x00,  'F',  '$', //0x4
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x5
  0x00, 0x00,  'N', 0x00,  'Y', 0x00,  'G',  '^', //0x6
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x7
   ' ',  ')',  '>',  '{',  'I', LEFT,  ':',  '*', //0x8
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x9
  '\"',  '_',  '?',  '}',  'O', RGHT,  'L',  '(', //0xA
   RTN,  '|',  'M',  '~',  'T',  DEL,  'J',  '%', //0xB
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0xC
    UP,  '+',  ',',  'P',  'U', DOWN,  'K',  '&', //0xD
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0xE
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0xF

// shift + caps lock on

//0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x00, 0x00,  'Z', 0x00,  TAB, 0x00,  'A',  ESC, //0x0
  0x00, 0x00,  'X', 0x00,  'Q', 0x00,  'D',  '!', //0x1
  0x00, 0x00,  'C', 0x00,  'W', 0x00,  'S',  '@', //0x2
  0x00, 0x00,  'V', 0x00,  'E', 0x00,  'H',  '#', //0x3
  0x00, 0x00,  'B', 0x00,  'R', 0x00,  'F',  '$', //0x4
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x5
  0x00, 0x00,  'N', 0x00,  'Y', 0x00,  'G',  '^', //0x6
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x7
   ' ',  ')',  '>',  '{',  'I', LEFT,  ':',  '*', //0x8
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x9
  '\"',  '_',  '?',  '}',  'O', RGHT,  'L',  '(', //0xA
   RTN,  '|',  'M',  '~',  'T',  DEL,  'J',  '%', //0xB
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0xC
    UP,  '+',  ',',  'P',  'U', DOWN,  'K',  '&', //0xD
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0xE
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0xF
};

// Variables ------------------------------------------------------------------
static uint8_t key_clk_state = 1;
static uint8_t key_test = 0;
static uint8_t key_index = 0;
static uint8_t key_index_waiting = 0;
static uint8_t key_presses_consecutive = KEY_PRESSES_RESET;
static uint8_t key_data[KEY_MATRIX_TOTAL] = {KEY_DATA_EMPTY};
static uint8_t key_used[KEY_MATRIX_TOTAL] = {KEY_USED_FALSE};

static uint8_t key_shift = 0;
static uint8_t key_ctrl = 0;
static uint8_t key_caplock = 0;
static uint8_t menu_enabled = 0;

static KeyOperation key_operation;

// Helper functions -----------------------------------------------------------

void key_clk_low(void)
{
    gpio_put(KEY_SCK_PIN, 0);
}

void key_clk_high(void)
{
    gpio_put(KEY_SCK_PIN, 1);
}

void index_update(void)
{
    bool end_of_scan = (key_presses_consecutive == KEY_PRESSES_MAX? true : false);

    if (end_of_scan)
    {
        key_index = 0;
    }
    else
    {
        key_index = (key_index + 1) & KEY_MATRIX_MASK;
    }
}

void operation_test(void)
{
    if (menu_enabled == 1 && key_index == KEY_RTN)
    {
        menu_enabled = 0;
        key_operation = KEY_MAIN_MENU_SELECT;
    }

    if (key_ctrl == 1)
    {
        if (key_index == KEY_ESC)
        {
            key_operation = KEY_MAIN_PAUSE;
        }
        if (key_index == KEY_TAB)
        {
            key_operation = KEY_MAIN_RESUME;
        }

        if (key_index == KEY_DEL)
        {
            menu_enabled = 1;
            key_operation = KEY_MAIN_MENU;
        }
    }

    if (key_index == KEY_RESET)
    {
        key_operation = KEY_MAIN_RESET;
    }
}

void key_not_pressed(void)
{
    key_used[key_index] = KEY_USED_FALSE;
    key_data[key_index] = KEY_DATA_EMPTY;
}

void shift_key_test(void)
{
    if (key_index == KEY_SHIFT)
    {
        key_shift = key_test;
    }
}

void ctrl_key_test(void)
{
    if (key_index == KEY_CTRL)
    {
        key_ctrl = key_test;
    }
}

void caps_lock_key_test(void)
{
    if (key_index == KEY_CAPS_LOCK)
    {
        key_caplock = key_test;
    }
}

// Functions ------------------------------------------------------------------
void key_init(void)
{
    gpio_init(KEY_DATA_PIN);
    gpio_init(KEY_SCK_PIN);
    gpio_set_dir(KEY_DATA_PIN, GPIO_IN);
    gpio_set_dir(KEY_SCK_PIN, GPIO_OUT);
}

void key_update(void)
{
    key_clk_state ^= 1;
    if (key_clk_state)
    {
        key_clk_high();
        return;
    }

    key_clk_low();

    key_test = gpio_get(KEY_DATA_PIN) == KEY_SWITCH_CLOSED? 1 : 0;
    if (key_test)
    {
        key_presses_consecutive++;

        operation_test();

        if (key_data[key_index] == KEY_DATA_EMPTY)
        {
            if (key_index < KEY_MATRIX_VALID)
            {
                key_index_waiting = key_index;
            }
        }
        key_data[key_index] = key_test;
    }
    else
    {
        key_presses_consecutive = KEY_PRESSES_RESET;
        key_not_pressed();
    }

    shift_key_test();
    ctrl_key_test();
    caps_lock_key_test();

    index_update();

}

void key_command(KeyOperation *operation, uint8_t *data)
{
    *operation = key_operation;
    key_operation = KEY_MAIN_NULL;
    *data = 0;
}

uint8_t key_data_waiting(void)
{
    if (key_used[key_index_waiting] == KEY_USED_FALSE)
    {
        return key_index_waiting;
    }
    return 0;
}

uint8_t key_data_get(void)
{
    uint8_t key = 0;
    uint16_t key_iie_offset = KEY_OFFSET_CAPS_LOCK_OFF;

    key_iie_offset |= key_caplock * KEY_OFFSET_CAPS_LOCK_ON;
    key_iie_offset |= key_shift * KEY_OFFSET_CAPS_SHIFT;

    key = key_iie[(uint16_t)key_index_waiting + key_iie_offset];
    key_used[key_index_waiting] = KEY_USED_TRUE;
    key_index_waiting = 0;
    return key;
}
