#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include "keys.h"
#include "pico/stdlib.h"

#define KEYS_DATA_PIN 16
#define KEYS_SCK_PIN 18
#define KEYS_MATRIX_MASK 0x7F
#define KEYS_MATRIX_COMBINATIONS (KEYS_MATRIX_MASK + 1)

uint8_t keys_clk_state = 1;
uint8_t keys_index = 0;
uint8_t keys_zeros = 0;
uint8_t keys_data[KEYS_MATRIX_COMBINATIONS] = {0};

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
    if (gpio_get(KEYS_DATA_PIN) == 0)
    {
        keys_zeros++;
        keys_data[keys_index] = 0;
    }
    else
    {
        keys_zeros = 0;
        keys_data[keys_index] = 1;
    }

    if (keys_zeros == 16)
    {
        keys_index = 0;
    }
    else
    {
        keys_index = (keys_index + 1) & KEYS_MATRIX_MASK;
    }
}

uint8_t keys_data_get(void)
{
    if (keys_data[0x21] == 0)
    {
        keys_data[0x21] = 0;
        return 1;
    }
    return 0;
}
