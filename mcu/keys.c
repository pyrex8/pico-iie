#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include "keys.h"
#include "pico/stdlib.h"

#define KEYS_DATA_PIN 16
#define KEYS_SCK_PIN 18
#define KEYS_MATRIX_MASK 0x7F
#define KEYS_MATRIX_TOTAL (KEYS_MATRIX_MASK + 1)

uint8_t keys_clk_state = 1;
uint8_t keys_index = 0;
uint8_t keys_zeros = 0;
uint8_t keys_data[KEYS_MATRIX_TOTAL] = {0};
uint8_t keys_pressed[KEYS_MATRIX_TOTAL] = {0};
uint8_t keys_used[KEYS_MATRIX_TOTAL] = {0};

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
        keys_zeros++;
        // debounce with two valid scans in a row
        if (keys_data[keys_index])
        {
            keys_pressed[keys_index] = key_test;
        }
        keys_data[keys_index] = key_test;
    }
    else
    {
        keys_zeros = 0;
        keys_pressed[keys_index] = key_test;
        keys_used[keys_index] = key_test;
        keys_data[keys_index] = key_test;
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
    if (keys_pressed[0x21] && keys_used[0x21] == 0)
    {
        keys_used[0x21] = 1;
        return 1;
    }
    return 0;
}
