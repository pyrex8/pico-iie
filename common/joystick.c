#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
// #include <stdio.h>
#include "joystick.h"

#define JOYSTICK_BUTTON0_ADDR 0xC061
#define JOYSTICK_BUTTON1_ADDR 0xC062
#define JOYSTICK_PADDLE0_ADDR 0xC064
#define JOYSTICK_PADDLE1_ADDR 0xC065

#define JOYSTICK_ALT_BIT_MASK_ADDR 0xFFF7

#define JOYSTICK_TIMER_RESET_ADDR 0xC070

#define JOYSTICK_TIMER_COUNT_MULT  255
#define JOYSTICK_TIMER_COUNT_DIV  2816

// 2816 us / 31.84us

#define JOYSTICK_TIMER_COUNT_MAX  (JOYSTICK_TIMER_COUNT_DIV)

#define JOYSTICK_DATA_PIN_LOW  0x7F
#define JOYSTICK_DATA_PIN_HIGH 0xFF

static uint8_t button0 = 0;
static uint8_t button1 = 0;
static uint8_t paddle0 = 0;
static uint8_t paddle1 = 0;

static uint8_t button0_byte = 0;
static uint8_t button1_byte = 0;
static uint8_t paddle0_byte = 0;
static uint8_t paddle1_byte = 0;

static uint16_t timer_count = 0;


void joystick_update(uint8_t read, uint16_t address, uint8_t *byte)
{
    if (address == JOYSTICK_TIMER_RESET_ADDR)
    {
        timer_count = 0;
        paddle0_byte = JOYSTICK_DATA_PIN_HIGH;
        paddle1_byte = JOYSTICK_DATA_PIN_HIGH;
    }

    if (read)
    {
        if ((address & JOYSTICK_ALT_BIT_MASK_ADDR) == JOYSTICK_BUTTON0_ADDR)
        {
            *byte = button0_byte;
        }

        else if ((address & JOYSTICK_ALT_BIT_MASK_ADDR) == JOYSTICK_BUTTON1_ADDR)
        {
            *byte = button1_byte;
        }

        else if ((address & JOYSTICK_ALT_BIT_MASK_ADDR) == JOYSTICK_PADDLE0_ADDR)
        {
            *byte = paddle0_byte;
        }

        else if ((address & JOYSTICK_ALT_BIT_MASK_ADDR) == JOYSTICK_PADDLE1_ADDR)
        {
            *byte = paddle1_byte;
        }
    }
}


void joystick_scanline_update(uint8_t cycles)
{
    uint32_t counter_threshold = 0;
    if (timer_count < JOYSTICK_TIMER_COUNT_MAX)
    {
        timer_count += cycles;
    }

    counter_threshold = (timer_count * JOYSTICK_TIMER_COUNT_MULT) / JOYSTICK_TIMER_COUNT_DIV;

    if(button0)
    {
        button0_byte = JOYSTICK_DATA_PIN_HIGH;
    }
    else
    {
        button0_byte = JOYSTICK_DATA_PIN_LOW;
    }

    if(button1)
    {
        button1_byte = JOYSTICK_DATA_PIN_HIGH;
    }
    else
    {
        button1_byte = JOYSTICK_DATA_PIN_LOW;
    }

    if(counter_threshold < paddle0)
    {
        paddle0_byte = JOYSTICK_DATA_PIN_HIGH;
    }
    else
    {
        paddle0_byte = JOYSTICK_DATA_PIN_LOW;
    }

    if(counter_threshold < paddle1)
    {
        paddle1_byte = JOYSTICK_DATA_PIN_HIGH;
    }
    else
    {
        paddle1_byte = JOYSTICK_DATA_PIN_LOW;
    }
}

void joystick_state_set(uint8_t btn0, uint8_t btn1, uint8_t pdl0, uint8_t pdl1)
{
    button0 = btn0;
    button1 = btn1;
    // Only update if timer is not running
    if (timer_count >= JOYSTICK_TIMER_COUNT_MAX)
    {
        paddle0 = pdl0;
        paddle1 = pdl1;
    }

}
