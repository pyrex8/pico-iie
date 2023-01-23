#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include "keys.h"
#include "pico/stdlib.h"

#define KEYS_DATA_PIN 16
#define KEYS_SCK_PIN 18


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
