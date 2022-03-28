#include "cassette.h"
#include "pico/stdlib.h"

#define CASSETTE_OUT_ADDR 0xC020
#define CASSETTE_ADDR_ALT_MASK 0xFFF0

const uint CASSETTE_PIN = CASSETTE_PIN_NUMBER;

static uint8_t pin_state = 0;

void cassette_init(void)
{
    gpio_init(CASSETTE_PIN);
    gpio_set_dir(CASSETTE_PIN, GPIO_OUT);
}

void cassette_update(uint8_t read, uint16_t address, uint8_t *byte)
{
    // read or write toggles cassette output driver
    if ((address & CASSETTE_ADDR_ALT_MASK) == CASSETTE_OUT_ADDR)
    {
        pin_state ^= 1;
        gpio_put(CASSETTE_PIN, pin_state);
    }
}
