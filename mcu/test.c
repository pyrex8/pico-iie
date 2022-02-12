#include "test.h"
#include "pico/stdlib.h"

const uint TEST_PIN = TEST_PIN_NUMBER;

void test_pin_init(void)
{
    gpio_init(TEST_PIN);
    gpio_set_dir(TEST_PIN, GPIO_OUT);
}

void test_pin_low(void)
{
    gpio_put(TEST_PIN, 0);
}

void test_pin_high(void)
{
    gpio_put(TEST_PIN, 1);
}
