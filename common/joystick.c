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

static uint16_t timer_count = 0;
static uint32_t counter_threshold = 0;

void joystick_update(uint8_t read, uint16_t address, uint8_t *byte)
{
    if (timer_count < JOYSTICK_TIMER_COUNT_MAX)
    {
        timer_count++;
    }

    if (address == JOYSTICK_TIMER_RESET_ADDR)
    {
        timer_count = 0;
    }

    if (read)
    {
        if ((address & JOYSTICK_ALT_BIT_MASK_ADDR) == JOYSTICK_BUTTON0_ADDR)
        {
            *byte = button0;
        }

        else if ((address & JOYSTICK_ALT_BIT_MASK_ADDR) == JOYSTICK_BUTTON1_ADDR)
        {
            *byte = button1;
        }

        else if ((address & JOYSTICK_ALT_BIT_MASK_ADDR) == JOYSTICK_PADDLE0_ADDR)
        {
            counter_threshold = (timer_count * JOYSTICK_TIMER_COUNT_MULT) / JOYSTICK_TIMER_COUNT_DIV;
            if(counter_threshold < paddle0)
            {
                *byte = JOYSTICK_DATA_PIN_HIGH;
            }
            else
            {
                *byte = JOYSTICK_DATA_PIN_LOW;
            }
        }

        else if ((address & JOYSTICK_ALT_BIT_MASK_ADDR) == JOYSTICK_PADDLE1_ADDR)
        {
            counter_threshold = (timer_count * JOYSTICK_TIMER_COUNT_MULT) / JOYSTICK_TIMER_COUNT_DIV;
            if(counter_threshold < paddle1)
            {
                *byte = JOYSTICK_DATA_PIN_HIGH;
            }
            else
            {
                *byte = JOYSTICK_DATA_PIN_LOW;
            }
        }
    }
}

void joystick_state_set(uint8_t btn0, uint8_t btn1, uint8_t pdl0, uint8_t pdl1)
{
    if(btn0)
    {
        button0 = JOYSTICK_DATA_PIN_HIGH;
    }
    else
    {
        button0 = JOYSTICK_DATA_PIN_LOW;
    }

    if(btn1)
    {
        button1 = JOYSTICK_DATA_PIN_HIGH;
    }
    else
    {
        button1 = JOYSTICK_DATA_PIN_LOW;
    }

    // Only update if timer is not running
    if (timer_count >= JOYSTICK_TIMER_COUNT_MAX)
    {
        paddle0 = pdl0;
        paddle1 = pdl1;
    }
}
