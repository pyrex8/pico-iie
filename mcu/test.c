#include "test.h"
#include "pico/stdlib.h"

#define TEST_PIN_ADDR_MASK 0xFFF0

const uint TEST0_PIN = TEST0_PIN_NUMBER;
const uint TEST1_PIN = TEST1_PIN_NUMBER;
const uint TEST_PIN = TEST_PIN_NUMBER;

static uint8_t test_pin = 0;

void test0_pin_init(void)
{
    gpio_init(TEST0_PIN);
    gpio_set_dir(TEST0_PIN, GPIO_OUT);
}

void test0_pin_low(void)
{
    gpio_put(TEST0_PIN, 0);
}

void test0_pin_high(void)
{
    gpio_put(TEST0_PIN, 1);
}

void test1_pin_init(void)
{
    gpio_init(TEST1_PIN);
    gpio_set_dir(TEST1_PIN, GPIO_OUT);
}

void test1_pin_low(void)
{
    gpio_put(TEST1_PIN, 0);
}

void test1_pin_high(void)
{
    gpio_put(TEST1_PIN, 1);
}

void test_pin_init(void)
{
    test_pin = 0;
}

void test_pin_update(uint8_t read, uint16_t address, uint8_t *byte)
{
    if ((address & TEST_PIN_ADDR_MASK) == TEST_PIN_ADDR)
    {
        test_pin ^= 1;
        gpio_put(TEST_PIN, test_pin);
    }
}
