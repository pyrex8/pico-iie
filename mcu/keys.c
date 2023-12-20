#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include "keys.h"
#include "pico/stdlib.h"

#define KEYS_DATA_PIN 14
#define KEYS_SCK_PIN 15
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
#define KEYS_SHIFT 0xC0 >> 1
#define KEYS_CTRL 0x70 >> 1
#define KEYS_CAPS_LOCK 0x50 >> 1
#define KEYS_RESET 0x90 >> 1

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

static uint8_t keys_shift = 0;
static uint8_t keys_ctrl = 0;
static uint8_t keys_caplock = 1;

static const uint8_t keys_iie[KEYS_MATRIX_VALID * 3] =
{
// Caps lock on (default on power up)

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


// Caps lock off

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


// shift

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
  '"',  '_',  '?',  '}',  'O', RGHT,  'L',  '(', //0xA
   RTN,  '|',  'M',  '~',  'T',  DEL,  'J',  '%', //0xB
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0xC
    UP,  '+',  ',',  'P',  'U', DOWN,  'K',  '&', //0xD
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

        if (keys_index == KEYS_SHIFT)
        {
            keys_shift = 1;
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

        if (keys_index == KEYS_SHIFT)
        {
            keys_shift = 0;
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

    if (keys_shift)
    {
        keys_iie_offset = KEYS_OFFSET_CAPS_SHIFT;
    }

    key = keys_iie[keys_waiting + keys_iie_offset];
    keys_used[keys_waiting] = 1;
    keys_waiting = 0;
    return key;
}
