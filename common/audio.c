#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include "audio.h"

#define TOGGLES_PER_RESET_MAX 0x400
#define DATA_TOGGLE_CLK_INDEX_MAX (TOGGLES_PER_RESET_MAX - 1)

static uint8_t audio_data_last = 0;
static uint16_t audio_clk = 0;
static uint16_t last_index = 0;
static uint16_t data_toggle_clk_cycle[TOGGLES_PER_RESET_MAX] = {0};



void audio_data_clear(void)
{
    last_index = 0;
    audio_clk = 0;
}

void audio_update(uint8_t data)
{
    audio_clk++;
    if (data != audio_data_last)
    {
        data_toggle_clk_cycle[last_index] = audio_clk;
        audio_data_last = data;
        if (last_index < DATA_TOGGLE_CLK_INDEX_MAX)
        {
            last_index++;
        }
    }
}

uint16_t audio_data(uint16_t index)
{
    uint16_t rtn_index = index & DATA_TOGGLE_CLK_INDEX_MAX;
    return data_toggle_clk_cycle[rtn_index];
}

uint16_t audio_data_length(void)
{
   return last_index;
}