#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include "ps2.h"
#include "pico/stdlib.h"

#define PS2_DATA_PIN 16
#define PS2_SCK_PIN 18

static bool ps2_keyboard_data_ready = false;
static uint8_t ps2_keyboard_data = 0;

static uint8_t bit_value = 0;
static uint8_t bit_position = 0;
static uint8_t data_value = 0;
static uint8_t clk_state = 1;
static uint8_t clk_state_last = 1;

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
            bit_value = gpio_get(PS2_DATA_PIN);
            bit_position++;
            if (bit_position > 10)
            {
                bit_position = 0;
                ps2_keyboard_data_ready = true;
            }
        }
    }

    // read data

    // PS/2 Keyboard data is 11 bits long
    // start (0) + bytes (LSB first) + parity bit (odd) + stop (1)
    clk_state_last = clk_state;
}

bool ps2_data_ready(void)
{
    if (ps2_keyboard_data_ready)
    {
        ps2_keyboard_data_ready = false;
        return true;
    }
    return false;
}

uint8_t ps2_data_get(void)
{
    return 65;
//    return ps2_keyboard_data;
}
