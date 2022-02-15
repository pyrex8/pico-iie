#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "parallel.pio.h"
#include "pico/binary_info.h"
#include "hardware/structs/vreg_and_chip_reset.h"
#include "pico/multicore.h"

#include "common/rom.h"
#include "common/ram.h"
#include "common/keyboard.h"
#include "common/joystick.h"
#include "common/audio.h"
#include "common/video.h"
#include "common/disk.h"
#include "m6502/c6502.h"

#include "mcu/clock.h"
#include "mcu/led.h"
#include "mcu/test.h"

#define BACKGROUND_LOOP_DELAY_MS 16

#define LED_BLINK_DELAY_MS 500

// Pixel freq 25.175MHz for VGA Signal 640 x 480 @ 60 Hz
#define PCLK_DIVIDER_INTEGER 16
#define PCLK_DIVIDER_FRACT 12
#define PCLK_PWM_COUNT 1
#define PCLK_PWM_VALUE 1

// Vertical refresh 31.46875 kHz
#define VGA_H_VISIBLE_AREA 640
#define VGA_H_FRONT_PORCH 16
#define VGA_H_SYNC_PULSE 96
#define VGA_H_BACK_PORCH 48
#define VGA_H_WHOLE_LINE 800

#define HSYNC_DIVIDER 1
#define HSYNC__DIVIDER_INTEGER (PCLK_DIVIDER_INTEGER)
#define HSYNC__DIVIDER_FRACT (PCLK_DIVIDER_FRACT)
#define HSYNC_PWM_COUNT (VGA_H_WHOLE_LINE - 1)
#define HSYNC_PWM_VALUE (VGA_H_WHOLE_LINE - VGA_H_SYNC_PULSE)

// Screen refresh rate 60 Hz
#define VGA_V_VISIBLE_AREA 480
#define VGA_V_FRONT_PORCH 10
#define VGA_V_SYNC_PULSE 2
#define VGA_V_BACK_PORCH 33
#define VGA_V_WHOLE_FRAME 525

// // 800 / 8 = 100
#define VSYNC_CLK_MULTIPLIER 8
#define VSYNC_SCAN_MULTIPLIER (VGA_H_WHOLE_LINE / VSYNC_CLK_MULTIPLIER)

#define VSYNC_DIVIDER_INTEGER 134 // 8 * (16 + 12 / 16)
#define VSYNC_DIVIDER_FRACT 0
#define VSYNC_PWM_COUNT (VGA_V_WHOLE_FRAME * VSYNC_SCAN_MULTIPLIER - 1)
#define VSYNC_PWM_VALUE (VSYNC_SCAN_MULTIPLIER * (VGA_V_WHOLE_FRAME - VGA_V_SYNC_PULSE))

#define VGA_TO_VIDEO_SCAN_LINES_DIVIDER 2
#define VIDEO_SCAN_LINES (VGA_V_WHOLE_FRAME / VGA_TO_VIDEO_SCAN_LINES_DIVIDER)

#define VIDEO_RESOLUTION_X 280
#define VIDEO_RESOLUTION_Y 192
#define VIDEO_BORDER_X ((VGA_H_VISIBLE_AREA / 2 - VIDEO_RESOLUTION_X) / 2)
#define VIDEO_BORDER_Y ((VGA_V_VISIBLE_AREA / 2 - VIDEO_RESOLUTION_Y) / 2)
// #define VIDEO_BORDER_Y 10

#define VIDEO_COLOR 0 //0xFFFFF
#define VIDEO_BORDER_COLOR ((0x1F<<6) & 0xFFFF)

#define VIDEO_SCAN_BUFFER_OFFSET ((VGA_H_BACK_PORCH + (VGA_H_VISIBLE_AREA - VIDEO_RESOLUTION_X * 2) / 2) / 2) //44
#define VIDEO_SCAN_LINE_OFFSET ((VGA_V_BACK_PORCH + (VGA_V_VISIBLE_AREA - VIDEO_RESOLUTION_Y * 2) / 2) / 2)   //40.5 rounded down

// one last value of zero otherwise last pixel repeats to the end of the scan line
#define VIDEO_SCAN_BUFFER_LEN ((VGA_H_BACK_PORCH + VGA_H_VISIBLE_AREA) / 2 + 1)

#define VIDEO_SCAN_LINES_VISIBLE 192

const uint VSYNC_PIN = 17;
const uint HSYNC_PIN = 19;
const uint PCLK_PIN = 5;
const uint R0_PIN = 0;

#define VGA_BLACK  0x0000
#define VGA_GREEN  0x0608
#define VGA_PURPLE 0xF9D6
#define VGA_ORANGE 0x12DD
#define VGA_BLUE   0xE502
#define VGA_WHITE  0xFFDF

#define UART_ID uart1
#define UART_BAUD_RATE 230400

#define UART_TX_PIN 20
#define UART_RX_PIN 21

typedef enum
{
    MEMORY_WRITE = 0,
    MEMORY_READ
} MemoryRWStatus;

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

const uint16_t hcolor[] = {VGA_BLACK, VGA_GREEN, VGA_PURPLE, VGA_WHITE, VGA_BLACK, VGA_ORANGE, VGA_BLUE, VGA_WHITE};

static C6502_interface interface_c;
static uint8_t video_mode = VIDEO_TEXT_MODE;
static uint8_t video_page = VIDEO_PAGE_1;

PIO pio;
uint offset;
uint sm;
int pio_dma_chan;

uint hsync_slice;
uint hsync_channel;

uint vsync_slice;
uint vsync_channel;

int pclk_slice;
uint pclk_channel;

uint16_t scan_line;
uint16_t h_pixel;

uint16_t scan_line_buffer[VIDEO_SCAN_BUFFER_LEN] = {0};
uint16_t scan_line_blank[VIDEO_SCAN_BUFFER_LEN] = {0};
uint16_t scan_line_border[VIDEO_SCAN_BUFFER_LEN] = {0};
uint16_t scan_line_image[VIDEO_SCAN_BUFFER_LEN] = {0};

uint8_t uart_char = 0;

void main_init(void)
{
    rom_init();
    ram_init();
    c6502_init();
}

void main_run(uint8_t clk_cycles)
{
    for(int i = 0; i < clk_cycles; i++)
    {
        c6502_update(&interface_c);
        ram_update(interface_c.rw, interface_c.address, &interface_c.data);
        rom_update(interface_c.rw, interface_c.address, &interface_c.data);
//        disk_update(interface_c.rw, interface_c.address, &interface_c.data);

        keyboard_update(interface_c.rw, interface_c.address, &interface_c.data);
        joystick_update(interface_c.rw, interface_c.address, &interface_c.data);
        // Disable speaker if using game audio
//        speaker_update(interface_c.rw, interface_c.address, &interface_c.data);
        // game_update(interface_c.rw, interface_c.address, &interface_c.data);

        if (interface_c.address == 0xC019)
        {
            if (scan_line < VIDEO_SCAN_LINES_VISIBLE)
            {
                interface_c.data = 0x80;
            }
            else
            {
                interface_c.data = 0;
            }
        }

        if (interface_c.address == 0xC054)
        {
            video_page = VIDEO_PAGE_1;
        }
        if (interface_c.address == 0xC055)
        {
            video_page = VIDEO_PAGE_2;
        }

        if (interface_c.address == 0xC050)
        {
            video_mode = VIDEO_GRAPHICS_MODE;
        }

        if (interface_c.address == 0xC051)
        {
            video_mode = VIDEO_TEXT_MODE;
        }

        //$C100 - $C7FF (49408 - 51199): Peripheral Card Memory
        if (interface_c.address == 0xC100)
        {
        }
        if (interface_c.address == 0xC101)
        {
        }
    }
}

void __not_in_flash_func(vga_scan_line)(void)
{
    dma_hw->ch[pio_dma_chan].al3_read_addr_trig = scan_line_buffer;
    test0_pin_high();
    multicore_fifo_push_blocking(0);
    pwm_clear_irq(hsync_slice);

    scan_line = pwm_get_counter(vsync_slice) / VSYNC_SCAN_MULTIPLIER / 2;

    if ((scan_line > VIDEO_SCAN_LINE_OFFSET) &&
        (scan_line < VIDEO_SCAN_LINE_OFFSET + VIDEO_RESOLUTION_Y))
    {
        memcpy(scan_line_buffer, scan_line_image, VIDEO_SCAN_BUFFER_LEN * 2);
    }
    else
    {
        if ((scan_line > VIDEO_SCAN_LINE_OFFSET - VIDEO_BORDER_Y) &&
            (scan_line < VIDEO_SCAN_LINE_OFFSET + VIDEO_RESOLUTION_Y + VIDEO_BORDER_Y))
        {
            memcpy(scan_line_buffer, scan_line_border, VIDEO_SCAN_BUFFER_LEN * 2);
        }
        else
        {
            memcpy(scan_line_buffer, scan_line_blank, VIDEO_SCAN_BUFFER_LEN * 2);
        }
    }
    test0_pin_low();

}

int core1_main(void)
{

    while (1)
    {
        multicore_fifo_pop_blocking();
        test1_pin_high();
        main_run(1);
        test1_pin_low();
    }
}

int main(void)
{
   clock_init();
   led_blink_init(BACKGROUND_LOOP_DELAY_MS);
   led_red_init();
   test0_pin_init();
   test1_pin_init();

   uart_init(UART_ID, UART_BAUD_RATE);
   gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
   gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

   multicore_launch_core1(core1_main);

   main_init();


   int i = 0;
   for (i = 0; i < VIDEO_SCAN_BUFFER_LEN; i++)
   {
       if ((i > VIDEO_SCAN_BUFFER_OFFSET) && (i < VIDEO_SCAN_BUFFER_OFFSET + VIDEO_RESOLUTION_X))
       {
            scan_line_image[i] = hcolor[(i>>2) & 0x07];
       }
       else
       {
            scan_line_image[i] = VGA_BLACK;
       }
   }

    pio = pio0;
    offset = pio_add_program(pio, &parallel_program);
    sm = pio_claim_unused_sm(pio, true);
    parallel_program_init(pio, sm, offset, R0_PIN);

    gpio_set_function(HSYNC_PIN, GPIO_FUNC_PWM);
    hsync_slice = pwm_gpio_to_slice_num(HSYNC_PIN);
    hsync_channel = pwm_gpio_to_channel(HSYNC_PIN);

    pwm_clear_irq(hsync_slice);
    pwm_set_irq_enabled(hsync_slice, true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, vga_scan_line);
    irq_set_priority(PWM_IRQ_WRAP, 0);
    irq_set_enabled(PWM_IRQ_WRAP, true);

    pwm_set_clkdiv_int_frac (hsync_slice, HSYNC__DIVIDER_INTEGER, HSYNC__DIVIDER_FRACT);
    pwm_set_wrap(hsync_slice, HSYNC_PWM_COUNT);
    pwm_set_chan_level(hsync_slice, hsync_channel, HSYNC_PWM_VALUE);

    gpio_set_function(VSYNC_PIN, GPIO_FUNC_PWM);
    vsync_slice = pwm_gpio_to_slice_num(VSYNC_PIN);
    vsync_channel = pwm_gpio_to_channel(VSYNC_PIN);
    pwm_set_clkdiv_int_frac (vsync_slice, VSYNC_DIVIDER_INTEGER, VSYNC_DIVIDER_FRACT);
    pwm_set_wrap(vsync_slice, VSYNC_PWM_COUNT);
    pwm_set_chan_level(vsync_slice, vsync_channel, VSYNC_PWM_VALUE);

    gpio_set_function(PCLK_PIN, GPIO_FUNC_PWM);
    pclk_slice = pwm_gpio_to_slice_num(PCLK_PIN);
    pclk_channel = pwm_gpio_to_channel(PCLK_PIN);
    pwm_set_clkdiv_int_frac (pclk_slice, PCLK_DIVIDER_INTEGER, PCLK_DIVIDER_FRACT);
    pwm_set_wrap(pclk_slice, PCLK_PWM_COUNT);
    pwm_set_chan_level(pclk_slice, pclk_channel, PCLK_PWM_VALUE);

    pio_dma_chan = dma_claim_unused_channel(true);
    dma_channel_config pio_dma_chan_config = dma_channel_get_default_config(pio_dma_chan);

    channel_config_set_transfer_data_size(&pio_dma_chan_config, DMA_SIZE_16);
    channel_config_set_read_increment(&pio_dma_chan_config, true);
    channel_config_set_write_increment(&pio_dma_chan_config, false);
    channel_config_set_dreq(&pio_dma_chan_config, DREQ_PWM_WRAP0 + pclk_slice);

    dma_channel_configure(
        pio_dma_chan,
        &pio_dma_chan_config,
        &pio->txf[sm],
        scan_line_buffer,
        VIDEO_SCAN_BUFFER_LEN,
        true);

    pwm_set_mask_enabled ((1 << hsync_slice) | (1 << vsync_slice) | (1 << pclk_slice));

    while (1)
    {
        // led_blink_update(LED_BLINK_NORMAL);
        // sleep_ms(BACKGROUND_LOOP_DELAY_MS);
        // if (uart_is_readable(UART_ID))
        // {
        //     uart_char = uart_getc(UART_ID);
        //     if (uart_char == 'a')
        //     {
        //         led_red_high();
        //     }
        //     if (uart_char == 's')
        //     {
        //         led_red_low();
        //     }
        // }
    }
}
