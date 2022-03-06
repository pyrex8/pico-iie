#include <inttypes.h>
#include <string.h>
#include "video.h"
#include "video_char_data.h"

#define VIDEO_RESOLUTION_X 280
#define VIDEO_RESOLUTION_Y 192
#define VIDEO_BUFFER_SIZE (VIDEO_RESOLUTION_X)

#define TEXT_BYTES 8
#define TEXT_BYTES_MASK 0x07

#define VGA_COLORS 3
#define VGA_LINE_BYTES 50
#define SCAN_LINE_BYTES (VGA_COLORS * VGA_LINE_BYTES)
#define H_SYNC_COUNTER_START 410 //406 to 414, 20 counts = 1 pixel

#define H_BUFFER_OFFSET 4
#define H_BUFFER_VISIBLE 35

#define VGA_BLACK  0x0000
#define VGA_GREEN  0x0608
#define VGA_PURPLE 0xF9D6
#define VGA_ORANGE 0x12DD
#define VGA_BLUE   0xE502
#define VGA_WHITE  0xFFDF

enum HCOLOR
{
    BLACK = 0,
    PURPLE,
    GREEN,
    GREEN_1,
    PURPLE_1,
    BLUE,
    ORANGE_0,
    ORANGE_1,
    BLUE_1,
    WHITE,
    HCOLOR_LENGTH,
};

const uint16_t hcolor[] =
{
    VGA_BLACK,
    VGA_PURPLE,
    VGA_GREEN,
    VGA_GREEN,
    VGA_PURPLE,
    VGA_BLUE,
    VGA_ORANGE,
    VGA_ORANGE,
    VGA_BLUE,
    VGA_WHITE,
};

typedef enum
{
    VIDEO_TEXT_MODE = 0,
    VIDEO_GRAPHICS_MODE
} VideoModeStatus;

typedef enum
{
    VIDEO_PAGE_1 = 0,
    VIDEO_PAGE_2
} VideoPageStatus;

static uint8_t video_char_set[CHARACTER_SET_ROM_SIZE];
static uint16_t video_buffer[VIDEO_BUFFER_SIZE] = {0};
static uint8_t video_mode = VIDEO_TEXT_MODE;
static uint8_t video_page = VIDEO_PAGE_1;
static uint16_t video_scan_line;

void video_init(void)
{
    memcpy(video_char_set, char_rom, CHARACTER_SET_ROM_SIZE);
}

void video_update(uint8_t read, uint16_t address, uint8_t *byte)
{
    if (address == 0xC019)
    {
        if (video_scan_line < VIDEO_RESOLUTION_Y)
        {
            *byte = 0x80;
        }
        else
        {
            *byte = 0;
        }
    }

    if (address == 0xC054)
    {
        video_page = VIDEO_PAGE_1;
    }
    if (address == 0xC055)
    {
        video_page = VIDEO_PAGE_2;
    }

    if (address == 0xC050)
    {
        video_mode = VIDEO_GRAPHICS_MODE;
    }

    if (address == 0xC051)
    {
        video_mode = VIDEO_TEXT_MODE;
    }
}


void video_hires_line_update(uint16_t video_line_number, uint8_t *video_line_data)
{
    uint16_t data = 0;
    uint16_t pixel_pre = 0;
    uint16_t pixel_post =  0;
    uint16_t data_extended = 0;
    uint8_t address_odd = 0;
    uint8_t color_offset = 0;
    uint8_t color = 0;
    uint8_t pixel = 0;
    uint8_t pixel_left1 = 0;
    uint8_t pixel_right1 = 0;

    if (video_line_number < VIDEO_RESOLUTION_Y)
    {
        for(int j = 0; j < VIDEO_BYTES_PER_LINE; j++)
        {
            data = video_line_data[j];
            pixel_pre = (video_line_data[j - 1] & 0x60)>>5;
            pixel_post = video_line_data[j + 1] & 3;
            address_odd = (j & 1)<<1;
            color_offset = (data & 0x80)>>5;
            data_extended = pixel_pre + ((data & 0x7F)<<2) + (pixel_post<<9);

            for(int i = 0; i < 7; i++)
            {
                color = BLACK;
                pixel = (data_extended >> (i + 2)) & 1;
                pixel_left1 = (data_extended >> (i + 1)) & 1;
                pixel_right1 = (data_extended >> (i + 3)) & 1;

                if (pixel)
                {
                    if (pixel_left1 || pixel_right1)
                    {
                        color = WHITE;
                    }
                    else
                    {
                        color = color_offset + address_odd +  (i & 1) + 1;
                    }
                }
                else
                {
                    if (pixel_left1 && pixel_right1)
                    {

                        color = color_offset + address_odd +  1 - (i & 1) + 1;
                    }
                }
                video_buffer[j*7 + i] |= hcolor[color];
            }
        }
    }
}

void video_text_line_update(uint16_t video_line_number, uint8_t *video_line_data)
{
    uint16_t text_char = 0;
    uint16_t data = 0;
    uint8_t color = 0;
    uint8_t pixel_color = WHITE;
    uint8_t pixel = 0;
    uint16_t rom_char = 0;
    uint16_t rom_char_offset = 0;

    if (video_line_number < VIDEO_RESOLUTION_Y)
    {
        for(int j = 0; j < VIDEO_BYTES_PER_LINE; j++)
        {
            text_char = video_line_data[j];

            if((text_char & 0xC0) == 0x40)
            {
                // text_char &= 0x3F;
                // if(text_char < 0x20)
                // {
                //     text_char += 0x40;
                // }
            }

            rom_char = text_char * TEXT_BYTES;
            rom_char_offset = video_line_number & TEXT_BYTES_MASK;
            data = video_char_set[rom_char + rom_char_offset];

            for(int i = 0; i < 7; i++)
            {
                color = pixel_color;
                pixel = (data >> (i)) & 1;
                if (pixel)
                {
                    color = BLACK;
                }
                video_buffer[j*7 + i] |= hcolor[color];
            }
        }
    }
}

void video_scan_line_set(uint16_t line)
{
    video_scan_line = line;
}

void video_buffer_clear(void)
{
    memset(video_buffer, 0, VIDEO_BUFFER_SIZE * 2);
}

void video_buffer_get(uint16_t *buffer)
{
    memcpy(buffer, video_buffer, VIDEO_BUFFER_SIZE * 2);
}

bool video_is_mode_text(void)
{
    return video_mode == VIDEO_TEXT_MODE;
}

uint16_t video_page_get(void)
{
    return video_page;
}
