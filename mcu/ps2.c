#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include "ps2.h"
#include "pico/stdlib.h"

#define PS2_DATA_PIN 16
#define PS2_SCK_PIN 18

#define PS2_ASCII_MAP_SIZE 0x77
#define PS2_SHIFT_KEY 0x12
#define PS2_CTRL_KEY 0x14
#define PS2_CAPLOCK_KEY 0x58
#define ASCII_LOWER_CASE_OFFSET 0x20

#define PS2_UP_KEY 0x75
#define PS2_DOWN_KEY 0x72
#define PS2_LEFT_KEY 0x6B
#define PS2_RIGHT_KEY 0x74
#define PS2_F1_KEY 0x05
#define PS2_F2_KEY 0x06
#define PS2_F3_KEY 0x04
#define PS2_F10_KEY 0x09

#define A2E_UP_KEY 0x0B
#define A2E_DOWN_KEY 0x0A
#define A2E_LEFT_KEY 0x08
#define A2E_RIGHT_KEY 0x15

static uint8_t bit_position = 0;
static uint8_t data_raw = 0;
static uint8_t clk_state = 1;
static uint8_t clk_state_last = 1;

static uint8_t ps2_data = 0;
static uint8_t ps2_key_up = 0;
static uint8_t ps2_shift = 0;
static uint8_t ps2_ctrl = 0;
static uint8_t ps2_caplock = 0;

static uint8_t key_code = 0;

static Ps2Operation ps2_operation;

static const uint8_t keymap_ps2_2_iie[PS2_ASCII_MAP_SIZE] =
{
//0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x60, 0x00, //0x00
  0x00, 0x00, 0x00, 0x00, 0x00, 0x51, 0x31, 0x00, 0x00, 0x00, 0x5A, 0x53, 0x41, 0x57, 0x32, 0x00, //0x10
  0x00, 0x43, 0x58, 0x44, 0x45, 0x34, 0x33, 0x00, 0x00, 0x20, 0x56, 0x46, 0x54, 0x52, 0x35, 0x00, //0x20
  0x00, 0x4E, 0x42, 0x48, 0x47, 0x59, 0x36, 0x00, 0x00, 0x00, 0x4D, 0x4A, 0x55, 0x37, 0x38, 0x00, //0x30
  0x00, 0x2C, 0x4B, 0x49, 0x4F, 0x30, 0x39, 0x00, 0x00, 0x2E, 0x2F, 0x4C, 0x3B, 0x50, 0x2D, 0x00, //0x40
  0x00, 0x00, 0x27, 0x0,  0x5B, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x5D, 0x00, 0x5C, 0x00, 0x00, //0x50
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, //0x60
  0x00, 0x00, 0x0A, 0x00, 0x15, 0x0B, 0x1B,                                                       //0x70
};

static const uint8_t keymap_iie_shift[PS2_ASCII_MAP_SIZE] =
{
//0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x7E, 0x00, //0x00
  0x00, 0x00, 0x00, 0x00, 0x00, 0x51, 0x21, 0x00, 0x00, 0x00, 0x5A, 0x53, 0x41, 0x57, 0x40, 0x00, //0x10
  0x00, 0x43, 0x58, 0x44, 0x45, 0x24, 0x23, 0x00, 0x00, 0x20, 0x56, 0x46, 0x54, 0x52, 0x25, 0x00, //0x20
  0x00, 0x4E, 0x42, 0x48, 0x47, 0x59, 0x5E, 0x00, 0x00, 0x00, 0x4D, 0x4A, 0x55, 0x26, 0x2A, 0x00, //0x30
  0x00, 0x3C, 0x4B, 0x49, 0x4F, 0x29, 0x28, 0x00, 0x00, 0x3E, 0x3F, 0x4C, 0x3A, 0x50, 0x5F, 0x00, //0x40
  0x00, 0x00, 0x22, 0x0,  0x7B, 0x2B, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x7D, 0x00, 0x7C, 0x00, 0x00, //0x50
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, //0x60
  0x00, 0x00, 0x0A, 0x00, 0x15, 0x0B, 0x1B,                                                       //0x70
};

void ps2_init(void)
{
    gpio_set_dir(PS2_DATA_PIN, GPIO_IN);
    gpio_set_dir(PS2_SCK_PIN, GPIO_IN);
}

void ps2_update(void)
{
    clk_state = gpio_get(PS2_SCK_PIN);
    // wait for falling edge of clock
    if (clk_state_last)
    {
        if (clk_state == 0)
        {
            if (bit_position < 9)
            {
                data_raw = data_raw >> 1;
                if (gpio_get(PS2_DATA_PIN))
                {
                    data_raw |= 0x80;
                }
                if (bit_position == 8)
                {
                    ps2_data = data_raw;
                }
            }
            bit_position++;
            // PS/2 Keyboard data is 11 bits long
            // start (0) + bytes (LSB first) + parity bit (odd) + stop (1)
            if (bit_position > 10)
            {
                bit_position = 0;

                if(ps2_data == 0xF0)
                {
                  ps2_key_up = 1;
                }
                else
                {
                    if(ps2_key_up == 0)
                    {
                        if(ps2_data == PS2_SHIFT_KEY)
                        {
                            ps2_shift = 1;
                        }
                        if(ps2_data == PS2_CTRL_KEY)
                        {
                            ps2_ctrl = 1;
                        }

                        if(ps2_data < PS2_ASCII_MAP_SIZE)
                        {
                            key_code = keymap_ps2_2_iie[ps2_data];
                            if(ps2_data == PS2_CAPLOCK_KEY)
                            {
                                ps2_caplock = 1 - ps2_caplock;
                            }
                            // shift alphabet only NOTE:still not working
                            if(ps2_caplock && key_code > 0x40 && key_code < 0x5B)
                            {
                                key_code += 0x20;
                            }

                            if(ps2_data == PS2_F1_KEY)
                            {
                                ps2_operation = PS2_MAIN_PAUSE;
                            }

                            if(ps2_data == PS2_F2_KEY)
                            {
                                ps2_operation = PS2_MAIN_RESET;
                            }

                            if(ps2_shift)
                            {
                                key_code = keymap_iie_shift[ps2_data];
                            }

                            if(ps2_ctrl)
                            {
                                if(ps2_data == PS2_F10_KEY)
                                {
                                    ps2_operation = PS2_MAIN_REBOOT;
                                }
                            }

                        }
                    }
                    else
                    {
                        if(ps2_data == PS2_SHIFT_KEY)
                        {
                            ps2_shift = 0;
                        }
                        if(ps2_data == PS2_CTRL_KEY)
                        {
                            ps2_ctrl = 0;
                        }
                        ps2_key_up = 0;
                    }
                }
            }
        }
    }
    clk_state_last = clk_state;
}

void ps2_command(Ps2Operation *operation, uint8_t *data)
{
    *operation = ps2_operation;
    ps2_operation = PS2_MAIN_NULL;
    *data = 0;
}

bool ps2_data_ready(void)
{
    if (key_code > 0)
    {
        return true;
    }
    return false;
}

uint8_t ps2_data_get(void)
{
    uint8_t rtn_keycode = key_code;
    key_code = 0;
    return rtn_keycode;
}
