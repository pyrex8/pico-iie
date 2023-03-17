// includes -------------------------------------------------------------------
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include "key.h"
#include "pico/stdlib.h"

// definitions ----------------------------------------------------------------
#define KEY_DATA_PIN 18
#define KEY_SCK_PIN 17
#define KEY_MATRIX_MASK 0x7F
#define KEY_MATRIX_TOTAL (KEY_MATRIX_MASK + 1)
#define KEY_MATRIX_VALID (KEY_MATRIX_TOTAL - 0x10)
#define KEY_PRESSES__RESET 0
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
#define KEY_SHIFT_LEFT 0x10
#define KEY_SHIFT_RIGHT 0x1B
#define KEY_CTRL_LEFT 0x41
#define KEY_CTRL_RIGHT 0x09
#define KEY_CAPS_LOCK 0x20
#define KEY_F1 0x00 // f1 = pause
#define KEY_F2 0x42 // f2 = resume
#define KEY_F3 0x43 // shift + f3 = reset
#define KEY_F4 0x44 // shift + f4 = menu
#define KEY_F5 0x45 // shift + f5 = reboot

#define KEY_OFFSET_CAPS_LOCK_ON  0x00
#define KEY_OFFSET_CAPS_LOCK_OFF 0x70
#define KEY_OFFSET_CAPS_SHIFT    0xE0

#define KEY_SHIFT_RIGHT_BIT (1<<0)
#define KEY_SHIFT_LEFT_BIT (1<<1)

#define KEY_DATA_EMPTY 0
#define KEY_DATA_READY 1
#define KEY_PRESSED_FALSE 0
#define KEY_PRESSED_TRUE 1
#define KEY_USED_FALSE 0
#define KEY_USED_TRUE 1

#define KEY_SWITCH_CLOSED 0

// constant data --------------------------------------------------------------
static const uint8_t key_iie[KEY_MATRIX_VALID * 3] =
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

// private variables ----------------------------------------------------------
static uint8_t key_clk_state = 1;
static uint8_t key_index = 0;
static uint8_t key_index_waiting = 0;
static uint8_t consecutive_presses = 0;
static uint8_t key_data[KEY_MATRIX_TOTAL] = {KEY_DATA_EMPTY};
static uint8_t key_pressed[KEY_MATRIX_TOTAL] = {KEY_PRESSED_FALSE};
static uint8_t key_used[KEY_MATRIX_TOTAL] = {KEY_USED_FALSE};
static uint8_t key_iie_offset = 0;

static uint8_t key_shift = 0;
static uint8_t key_ctrl_left = 0;
static uint8_t key_ctrl_right = 0;
static uint8_t key_caplock = 1;

static uint8_t key_pause = 0;
static uint8_t key_resume = 0;
static uint8_t key_reset = 0;
static uint8_t key_reboot = 0;
static uint8_t key_menu = 0;

static uint8_t key_debug = 0;
static const uint8_t key_nibble[] =
{
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
    0xB8, 0xB9, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86
};

// private functions ----------------------------------------------------------
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
    bool end_of_scan = (consecutive_presses == KEY_PRESSES_MAX? true : false);

    if (end_of_scan)
    {
        key_index = 0;
    }
    else
    {
        key_index = (key_index + 1) & KEY_MATRIX_MASK;
    }
}

void either_shift_key_down_test(void)
{
    if (key_index == KEY_SHIFT_LEFT)
    {
        key_shift |= KEY_SHIFT_LEFT_BIT;
    }

    if (key_index == KEY_SHIFT_RIGHT)
    {
        key_shift |= KEY_SHIFT_RIGHT_BIT;
    }
}

void both_shift_key_up_test(void)
{
    if (key_index == KEY_SHIFT_LEFT)
    {
        key_shift &= ~KEY_SHIFT_LEFT_BIT;
    }

    if (key_index == KEY_SHIFT_RIGHT)
    {
        key_shift &= ~KEY_SHIFT_RIGHT_BIT;
    }
}

void key_not_pressed(void)
{
    key_pressed[key_index] = KEY_PRESSED_FALSE;
    key_used[key_index] = KEY_USED_FALSE;
    key_data[key_index] = KEY_DATA_EMPTY;
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

    if (key_caplock)
    {
        key_iie_offset = KEY_OFFSET_CAPS_LOCK_ON;
    }
    else
    {
        key_iie_offset = KEY_OFFSET_CAPS_LOCK_OFF;
    }

    if (key_shift)
    {
        key_iie_offset = KEY_OFFSET_CAPS_SHIFT;
    }

    key = key_iie[key_index_waiting + key_iie_offset];
    key_used[key_index_waiting] = KEY_USED_TRUE;
    key_index_waiting = 0;
    return key;
}

// function definitions -------------------------------------------------------
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

    bool key_press_detected =
        gpio_get(KEY_DATA_PIN) == KEY_SWITCH_CLOSED? true : false;

    if (key_press_detected)
    {
        consecutive_presses++;

        either_shift_key_down_test();

        if (key_data[key_index] == KEY_DATA_EMPTY)
        {
            if (key_index == KEY_CAPS_LOCK)
            {
                key_caplock ^= 1;
            }
            else
            {
                key_debug = key_index;
                key_pressed[key_index] = KEY_PRESSED_TRUE;
                if (key_index < KEY_MATRIX_VALID)
                {
                    key_index_waiting = key_index;
                }
            }
        }
        key_data[key_index] = KEY_DATA_READY;
    }
    else
    {
        consecutive_presses = KEY_PRESSES__RESET;
        both_shift_key_up_test();
        key_not_pressed();
    }

    index_update();
}

void key_operation_get(KeyOperation *operation, uint8_t *data)
{
    *operation = KEY_MAIN_NULL;
    *data = 0;

    if (key_pause)
    {
        key_pause = 0;
        *operation = KEY_MAIN_PAUSE;
        return;
    }

    if (key_resume)
    {
        key_resume = 0;
        *operation = KEY_MAIN_RESUME;
        return;
    }

    if (key_reset)
    {
        key_reset = 0;
        *operation = KEY_MAIN_RESET;
        return;
    }

    if (key_menu)
    {
        key_menu = 0;
        *operation = KEY_MAIN_MENU;
        return;
    }

    if (key_reboot)
    {
        key_reboot = 0;
        *operation = KEY_MAIN_REBOOT;
        return;
    }

    if (key_data_waiting())
    {
        *operation = KEY_KEYBOARD_KEY;
        *data = key_data_get();
    }
}

uint8_t key_ls_nibble_get(void)
{
    uint8_t nibble = key_debug & 0x0F;
    return key_nibble[nibble];
}

uint8_t key_ms_nibble_get(void)
{
    uint8_t nibble = (key_debug >>4) & 0x0F;
    return key_nibble[nibble];
}
