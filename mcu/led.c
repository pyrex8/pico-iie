
#include "led.h"
#include "pico/stdlib.h"

const uint TEST_RED_PIN = LED_RED_PIN_NUMBER;
const uint TEST_GREEN_PIN = LED_GREEN_PIN_NUMBER;

void led_red_init(void)
{
    gpio_init(TEST_RED_PIN);
    gpio_set_dir(TEST_RED_PIN, GPIO_OUT);
}

void led_red_set(bool value)
{
    gpio_put(TEST_RED_PIN, value);
}

void led_green_init(void)
{
    gpio_init(TEST_GREEN_PIN);
    gpio_set_dir(TEST_GREEN_PIN, GPIO_OUT);
}

void led_green_set(bool value)
{
    gpio_put(TEST_GREEN_PIN, value);
}
