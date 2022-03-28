#include "speaker.h"
#include "pico/stdlib.h"

#define SPEAKER_ADDR 0xC030
#define SPEAKER_ADDR_ALT_MASK 0xFFF0

const uint SPEAKER_PIN = SPEAKER_PIN_NUMBER;

static uint8_t pin_state = 0;

void speaker_init(void)
{
    gpio_init(SPEAKER_PIN);
    gpio_set_dir(SPEAKER_PIN, GPIO_OUT);
}

void speaker_update(uint8_t read, uint16_t address, uint8_t *byte)
{
    // read or write toggles speaker driver
    if ((address & SPEAKER_ADDR_ALT_MASK) == SPEAKER_ADDR)
    {
        pin_state ^= 1;
        gpio_put(SPEAKER_PIN, pin_state);
    }
}
