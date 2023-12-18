#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include "joystick.h"
#include "hardware/adc.h"
#include "pico/stdlib.h"

#define JOYSTICK_SW0_PIN 20
#define JOYSTICK_SW1_PIN 22
#define JOYSTICK_SW2_PIN 21
#define JOYSTICK_SW3_PIN 19
#define JOYSTICK_X_PIN 27
#define JOYSTICK_Y_PIN 26
#define JOYSTICK_X_ADC 1
#define JOYSTICK_Y_ADC 0

#define JOYSTICK_ADC_FULL_COUNT 4095
#define JOYSTICK_ADC_HALF_COUNT (JOYSTICK_ADC_FULL_COUNT >> 1)

#define JOYSTICK_PDL_FULL_COUNT 255

#define JOYSTICK_FULL_COUNT (JOYSTICK_ADC_FULL_COUNT * JOYSTICK_PDL_FULL_COUNT)

static uint8_t joystick_btn0 = 0;
static uint8_t joystick_btn1 = 0;
static uint8_t joystick_btn0n = 0;
static uint8_t joystick_btn1n = 0;
static uint8_t joystick_pdl0 = 0;
static uint8_t joystick_pdl1 = 0;

void joystick_init(void)
{
    gpio_init(JOYSTICK_SW0_PIN);
    gpio_init(JOYSTICK_SW1_PIN);
    gpio_init(JOYSTICK_SW2_PIN);
    gpio_init(JOYSTICK_SW3_PIN);

    gpio_set_dir(JOYSTICK_SW0_PIN, GPIO_IN);
    gpio_set_dir(JOYSTICK_SW1_PIN, GPIO_IN);
    gpio_set_dir(JOYSTICK_SW2_PIN, GPIO_IN);
    gpio_set_dir(JOYSTICK_SW3_PIN, GPIO_IN);
 
    gpio_pull_down(JOYSTICK_SW0_PIN);
    gpio_pull_down(JOYSTICK_SW1_PIN);
    gpio_pull_up(JOYSTICK_SW2_PIN);
    gpio_pull_up(JOYSTICK_SW3_PIN);

    adc_init();
    gpio_set_dir(JOYSTICK_X_PIN, GPIO_IN);
    gpio_set_dir(JOYSTICK_Y_PIN, GPIO_IN);
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);
}

void joystick_update(void)
{
    joystick_btn0 = gpio_get(JOYSTICK_SW0_PIN) ? 1 : 0;
    joystick_btn1 = gpio_get(JOYSTICK_SW1_PIN) ? 1 : 0;
    joystick_btn0n = gpio_get(JOYSTICK_SW2_PIN) ? 0 : 1;
    joystick_btn1n = gpio_get(JOYSTICK_SW3_PIN) ? 0 : 1;

    adc_select_input(JOYSTICK_X_ADC);
    uint16_t joy_x = adc_read();
    adc_select_input(JOYSTICK_Y_ADC);
    uint16_t joy_y = adc_read();

    joystick_pdl0 = JOYSTICK_PDL_FULL_COUNT;
    joystick_pdl1 = JOYSTICK_PDL_FULL_COUNT;

    if (joy_x > JOYSTICK_ADC_HALF_COUNT)
    {
        joystick_pdl0 = JOYSTICK_FULL_COUNT / joy_x - JOYSTICK_PDL_FULL_COUNT;
    }

    if (joy_y > JOYSTICK_ADC_HALF_COUNT)
    {
        joystick_pdl1 = JOYSTICK_FULL_COUNT / joy_y - JOYSTICK_PDL_FULL_COUNT;
    }
}

uint8_t joystick_btn0_get(void)
{
    return joystick_btn0 | joystick_btn0n;
}

uint8_t joystick_btn1_get(void)
{
    return joystick_btn1 | joystick_btn1n;
}

uint8_t joystick_pdl0_get(void)
{
    return joystick_pdl0;
}

uint8_t joystick_pdl1_get(void)
{
    return joystick_pdl1;
}
