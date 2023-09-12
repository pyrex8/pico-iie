#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include "joystick.h"
#include "hardware/adc.h"
#include "pico/stdlib.h"

#define JOYSTICK_FIRE_PIN 19
#define JOYSTICK_UP_PIN 20
#define JOYSTICK_DOWN_PIN 21
#define JOYSTICK_LEFT_PIN 22
#define JOYSTICK_RIGHT_PIN 26
#define JOYSTICK_X_PIN 28
#define JOYSTICK_Y_PIN 27

#define JOYSTICK_PDL_CENTER 127
#define JOYSTICK_PDL_FULL_COUNT 255

#define JOYSTICK_ADC_FULL_COUNT 4096
#define JOYSTICK_ADC_LOW (JOYSTICK_ADC_FULL_COUNT / 3)
#define JOYSTICK_ADC_HIGH (JOYSTICK_ADC_FULL_COUNT * 2 / 3)

static uint8_t joystick_btn0 = 0;
static uint8_t joystick_btn1 = 0;
static uint8_t joystick_pdl0 = 0;
static uint8_t joystick_pdl1 = 0;

void joystick_init(void)
{
    gpio_init(JOYSTICK_FIRE_PIN);
    gpio_init(JOYSTICK_UP_PIN);
    gpio_init(JOYSTICK_DOWN_PIN);
    gpio_init(JOYSTICK_LEFT_PIN);
    gpio_init(JOYSTICK_RIGHT_PIN);

    gpio_set_dir(JOYSTICK_FIRE_PIN, GPIO_IN);
    gpio_set_dir(JOYSTICK_UP_PIN, GPIO_IN);
    gpio_set_dir(JOYSTICK_DOWN_PIN, GPIO_IN);
    gpio_set_dir(JOYSTICK_LEFT_PIN, GPIO_IN);
    gpio_set_dir(JOYSTICK_RIGHT_PIN, GPIO_IN);

    gpio_pull_up(JOYSTICK_FIRE_PIN);
    gpio_pull_up(JOYSTICK_UP_PIN);
    gpio_pull_up(JOYSTICK_DOWN_PIN);
    gpio_pull_up(JOYSTICK_LEFT_PIN);
    gpio_pull_up(JOYSTICK_RIGHT_PIN);

    adc_init();
    gpio_set_dir(JOYSTICK_X_PIN, GPIO_IN);
    gpio_set_dir(JOYSTICK_Y_PIN, GPIO_IN);
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);

}

void joystick_update(void)
{
    joystick_btn0 = gpio_get(JOYSTICK_FIRE_PIN) ? 0 : 1;

    adc_select_input(2);
    uint16_t joy_x = adc_read();
    adc_select_input(1);
    uint16_t joy_y = adc_read();

    joystick_pdl0 = JOYSTICK_PDL_FULL_COUNT - joy_x>>4;
    joystick_pdl1 = JOYSTICK_PDL_FULL_COUNT - joy_y>>4;
}

uint8_t joystick_btn0_get(void)
{
    return joystick_btn0;
}

uint8_t joystick_btn1_get(void)
{
    return joystick_btn1;
}

uint8_t joystick_pdl0_get(void)
{
    return joystick_pdl0;
}

uint8_t joystick_pdl1_get(void)
{
    return joystick_pdl1;
}
