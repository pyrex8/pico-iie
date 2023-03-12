#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include "keys.h"
#include "pico/stdlib.h"

#define KEYS_DATA_PIN 18
#define KEYS_SCK_PIN 17
#define KEYS_MATRIX_MASK 0x7F
#define KEYS_MATRIX_TOTAL (KEYS_MATRIX_MASK + 1)
#define KEYS_MATRIX_VALID (KEYS_MATRIX_TOTAL - 0x10)

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
#define KEYS_SHIFT_LEFT 0x10
#define KEYS_SHIFT_RIGHT 0x1B
#define KEYS_CTRL_LEFT 0x41
#define KEYS_CTRL_RIGHT 0x09
#define KEYS_CAPS_LOCK 0x20
#define KEYS_F1 0x00 // pause/resume
#define KEYS_F2 0x42 // shift + f2 = reset
#define KEYS_F3 0x43 // ctrl + f3 = reboot

#define KEYS_OFFSET_CAPS_LOCK_ON  0x00
#define KEYS_OFFSET_CAPS_LOCK_OFF 0x70
#define KEYS_OFFSET_CAPS_SHIFT    0xE0

uint8_t keys_clk_state = 1;
uint8_t keys_index = 0;
uint8_t keys_waiting = 0;
uint8_t keys_ones = 0;
uint8_t keys_data[KEYS_MATRIX_TOTAL] = {0};
uint8_t keys_pressed[KEYS_MATRIX_TOTAL] = {0};
uint8_t keys_used[KEYS_MATRIX_TOTAL] = {0};
uint8_t keys_iie_offset = 0;

static uint8_t keys_shift_left = 0;
static uint8_t keys_shift_right = 0;
static uint8_t keys_ctrl_left = 0;
static uint8_t keys_ctrl_right = 0;
static uint8_t keys_caplock = 1;

static const uint8_t keys_iie[KEYS_MATRIX_VALID * 3] =
{
// Caps lock on (default on power up)

//0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  ' ', 0x00, 0x00, 0x00, 0x00, LEFT, DOWN, RGHT, 0x00, 0x00, //0x00
  0x00,  'Z',  'X',  'C',  'V',  'B',  'N',  'M',  ',',  '.',  '/', 0x00, 0x00,   UP, 0x00, 0x00, //0x10
  0x00,  'A',  'S',  'D',  'F',  'G',  'H',  'J',  'K',  'L',  ';', '\'',  RTN, '\\', 0x00, 0x00, //0x20
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x30
   ESC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  DEL, 0x00, 0x00, //0x40
   '`',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',  '-',  '=', 0x00, 0x00, 0x00, //0x50
   TAB,  'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  'O',  'P',  '[',  ']', 0x00, 0x00, 0x00, //0x60

// Caps lock off

//0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  ' ', 0x00, 0x00, 0x00, 0x00, LEFT, DOWN, RGHT, 0x00, 0x00, //0x00
  0x00,  'z',  'x',  'c',  'v',  'b',  'n',  'm',  ',',  '.',  '/', 0x00, 0x00,   UP, 0x00, 0x00, //0x10
  0x00,  'a',  's',  'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';', '\'',  RTN, '\\', 0x00, 0x00, //0x20
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x30
   ESC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  DEL, 0x00, 0x00, //0x40
   '`',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',  '-',  '=', 0x00, 0x00, 0x00, //0x50
   TAB,  'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  'o',  'p',  '[',  ']', 0x00, 0x00, 0x00, //0x60

// shift

//0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  ' ', 0x00, 0x00, 0x00, 0x00, LEFT, DOWN, RGHT, 0x00, 0x00, //0x00
  0x00,  'Z',  'X',  'C',  'V',  'B',  'N',  'M',  '<',  '>',  '?', 0x00, 0x00,   UP, 0x00, 0x00, //0x10
  0x00,  'A',  'S',  'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':', '\"',  RTN,  '|', 0x00, 0x00, //0x20
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x30
   ESC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  DEL, 0x00, 0x00, //0x40
   '~',  '!',  '@',  '#',  '$',  '%',  '^',  '&',  '*',  '(',  ')',  '_',  '+', 0x00, 0x00, 0x00, //0x50
   TAB,  'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',  'O',  'P',  '{',  '}', 0x00, 0x00, 0x00, //0x60
};

void keys_init(void)
{
    gpio_init(KEYS_DATA_PIN);
    gpio_init(KEYS_SCK_PIN);
    gpio_set_dir(KEYS_DATA_PIN, GPIO_IN);
    gpio_set_dir(KEYS_SCK_PIN, GPIO_OUT);
}

void keys_clk_low(void)
{
    gpio_put(KEYS_SCK_PIN, 0);
}

void keys_clk_high(void)
{
    gpio_put(KEYS_SCK_PIN, 1);
}

void keys_update(void)
{
    keys_clk_state ^= 1;
    if (keys_clk_state)
    {
        keys_clk_high();
        return;
    }

    keys_clk_low();

    // 16 zeros in a row marks the end of the keyboard matrix scan
    uint8_t key_test = gpio_get(KEYS_DATA_PIN) == 0? 1 : 0;
    if (key_test)
    {
        keys_ones++;

        if (keys_index == KEYS_SHIFT_LEFT)
        {
            keys_shift_left = 1;
        }

        if (keys_index == KEYS_SHIFT_RIGHT)
        {
            keys_shift_right = 1;
        }

        if (keys_data[keys_index] == 0)
        {
            if (keys_index == KEYS_CAPS_LOCK)
            {
                keys_caplock ^= 1;
            }
            else
            {
                keys_pressed[keys_index] = key_test;
                if (keys_index < KEYS_MATRIX_VALID)
                {
                    keys_waiting = keys_index;
                }
            }
        }
        keys_data[keys_index] = key_test;
    }
    else
    {
        keys_ones = 0;

        if (keys_index == KEYS_SHIFT_LEFT)
        {
            keys_shift_left = 0;
        }

        if (keys_index == KEYS_SHIFT_RIGHT)
        {
            keys_shift_right = 0;
        }

        keys_pressed[keys_index] = key_test;
        keys_used[keys_index] = key_test;
        keys_data[keys_index] = key_test;
    }

    if (keys_ones == 16)
    {
        keys_index = 0;
    }
    else
    {
        keys_index = (keys_index + 1) & KEYS_MATRIX_MASK;
    }
}

uint8_t keys_data_waiting(void)
{
    if (keys_used[keys_waiting] == 0)
    {
        return keys_waiting;
    }
    return 0;
}

uint8_t keys_data_get(void)
{
    uint8_t key = 0;

    if (keys_caplock)
    {
        keys_iie_offset = KEYS_OFFSET_CAPS_LOCK_ON;
    }
    else
    {
        keys_iie_offset = KEYS_OFFSET_CAPS_LOCK_OFF;
    }

    if (keys_shift_left || keys_shift_right)
    {
        keys_iie_offset = KEYS_OFFSET_CAPS_SHIFT;
    }

    key = keys_iie[keys_waiting + keys_iie_offset];
    keys_used[keys_waiting] = 1;
    keys_waiting = 0;
    return key;
}

void keys_operation_update(KeysOperation *operation, uint8_t *data)
{
    *operation = KEYS_MAIN_NULL;
    *data = 0;
    if (keys_data_waiting())
    {
        *operation = KEYS_KEYBOARD_KEY;
        *data = keys_data_get();
    }
}
