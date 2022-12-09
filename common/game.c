#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include "game.h"

#define GAME_BUTTON0_ADDR 0xC061
#define GAME_BUTTON1_ADDR 0xC062
#define GAME_PADDLE0_ADDR 0xC064
#define GAME_PADDLE1_ADDR 0xC065

#define GAME_ALT_BIT_MASK_ADDR 0xFFF7

#define GAME_TIMER_RESET_ADDR 0xC070

#define GAME_TIMER_COUNT_MULT  255
#define GAME_TIMER_COUNT_DIV  2816

// 2816 us / 31.84us

#define GAME_TIMER_COUNT_MAX  (GAME_TIMER_COUNT_DIV)

#define GAME_DATA_PIN_LOW  0x7F
#define GAME_DATA_PIN_HIGH 0xFF

static uint8_t button0 = 0;
static uint8_t button1 = 0;
static uint8_t paddle0 = 0;
static uint8_t paddle1 = 0;

static uint16_t timer_count = 0;
static uint32_t counter_threshold = 0;

void game_update(uint8_t read, uint16_t address, uint8_t *byte)
{
    if (timer_count < GAME_TIMER_COUNT_MAX)
    {
        timer_count++;
    }

    if (address == GAME_TIMER_RESET_ADDR)
    {
        timer_count = 0;
    }

    if ((address & 0xFFF0) == 0xC060)
    {
        if (read)
        {
            if ((address & GAME_ALT_BIT_MASK_ADDR) == GAME_BUTTON0_ADDR)
            {
                *byte = button0;
            }

            else if ((address & GAME_ALT_BIT_MASK_ADDR) == GAME_BUTTON1_ADDR)
            {
                *byte = button1;
            }

            else if ((address & GAME_ALT_BIT_MASK_ADDR) == GAME_PADDLE0_ADDR)
            {
                counter_threshold = (timer_count * GAME_TIMER_COUNT_MULT) / GAME_TIMER_COUNT_DIV;
                if(counter_threshold < paddle0)
                {
                    *byte = GAME_DATA_PIN_HIGH;
                }
                else
                {
                    *byte = GAME_DATA_PIN_LOW;
                }
            }

            else if ((address & GAME_ALT_BIT_MASK_ADDR) == GAME_PADDLE1_ADDR)
            {
                counter_threshold = (timer_count * GAME_TIMER_COUNT_MULT) / GAME_TIMER_COUNT_DIV;
                if(counter_threshold < paddle1)
                {
                    *byte = GAME_DATA_PIN_HIGH;
                }
                else
                {
                    *byte = GAME_DATA_PIN_LOW;
                }
            }
        }
    }
}

void game_state_set(uint8_t btn0, uint8_t btn1, uint8_t pdl0, uint8_t pdl1)
{
    if(btn0)
    {
        button0 = GAME_DATA_PIN_HIGH;
    }
    else
    {
        button0 = GAME_DATA_PIN_LOW;
    }

    if(btn1)
    {
        button1 = GAME_DATA_PIN_HIGH;
    }
    else
    {
        button1 = GAME_DATA_PIN_LOW;
    }

    // Only update if timer is not running
    if (timer_count >= GAME_TIMER_COUNT_MAX)
    {
        paddle0 = pdl0;
        paddle1 = pdl1;
    }
}

void game_btn0_set(uint8_t btn0)
{
    if(btn0)
    {
        button0 = GAME_DATA_PIN_HIGH;
    }
    else
    {
        button0 = GAME_DATA_PIN_LOW;
    }
}

void game_btn1_set(uint8_t btn1)
{
    if(btn1)
    {
        button1 = GAME_DATA_PIN_HIGH;
    }
    else
    {
        button1 = GAME_DATA_PIN_LOW;
    }
}

void game_pdl0_set(uint8_t pdl0)
{
    // Only update if timer is not running
    if (timer_count >= GAME_TIMER_COUNT_MAX)
    {
        paddle0 = pdl0;
    }
}

void game_pdl1_set(uint8_t pdl1)
{
    // Only update if timer is not running
    if (timer_count >= GAME_TIMER_COUNT_MAX)
    {
        paddle1 = pdl1;
    }
}
