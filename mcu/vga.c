#include <string.h>
#include <stdint.h>

#include "vga.h"

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/binary_info.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/structs/mpu.h"
#include "hardware/structs/vreg_and_chip_reset.h"
#include "parallel.pio.h"

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

#define VIDEO_SCAN_BUFFER_OFFSET ((VGA_H_BACK_PORCH + (VGA_H_VISIBLE_AREA - VIDEO_RESOLUTION_X * 2) / 2) / 2) //44
#define VIDEO_SCAN_LINE_OFFSET ((VGA_V_BACK_PORCH + (VGA_V_VISIBLE_AREA - VIDEO_RESOLUTION_Y * 2) / 2))   //40.5 rounded down

// one last value of zero otherwise last pixel repeats to the end of the scan line
#define VIDEO_SCAN_BUFFER_LEN ((VGA_H_BACK_PORCH + VGA_H_VISIBLE_AREA) / 2 + 1)
#define VIDEO_SCAN_LINE_LEN (VGA_H_WHOLE_LINE / 2)

const uint VSYNC_PIN = 4;
const uint HSYNC_PIN = 2;
const uint PCLK_PIN = 6;
const uint R0_PIN = 7;

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

static uint16_t scan_line_buffer[VIDEO_SCAN_LINE_LEN] = {0};
static uint16_t *p_scan_line_buffer;
volatile bool scan_line_old = true;

static uint16_t scan_line;
static int16_t overscan_line;
static uint8_t overscan_line_odd;


void __attribute__((noinline, long_call, section(".time_critical"))) vga_scan_line(void)
{
    pwm_clear_irq(hsync_slice);
    scan_line_old = false;
    overscan_line = pwm_get_counter(vsync_slice) / VSYNC_SCAN_MULTIPLIER - VIDEO_SCAN_LINE_OFFSET;
    overscan_line_odd = overscan_line & 0x01;
    if (overscan_line >= 0)
    {
        scan_line = overscan_line / 2;
    }
    else
    {
        scan_line = VIDEO_SCAN_LINES + overscan_line / 2;
    }
}

void vga_init(void)
{
    p_scan_line_buffer = scan_line_buffer;

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

    int ctrl_chan = dma_claim_unused_channel(true);
    pio_dma_chan = dma_claim_unused_channel(true);

     // Setup the control channel
     dma_channel_config c = dma_channel_get_default_config(ctrl_chan); // default configs
     channel_config_set_transfer_data_size(&c, DMA_SIZE_32); // 32-bit txfers
     channel_config_set_read_increment(&c, false); // no read incrementing
     channel_config_set_write_increment(&c, false); // no write incrementing
     channel_config_set_chain_to(&c, pio_dma_chan);
     channel_config_set_dreq(&c, DREQ_PWM_WRAP0 + pclk_slice);

     dma_channel_configure(
         ctrl_chan,
         &c,
         &dma_hw->ch[pio_dma_chan].al3_read_addr_trig,
         &p_scan_line_buffer,
         1,
         false
     );

    dma_channel_config pio_dma_chan_config = dma_channel_get_default_config(pio_dma_chan);

    channel_config_set_transfer_data_size(&pio_dma_chan_config, DMA_SIZE_16);
    channel_config_set_read_increment(&pio_dma_chan_config, true);
    channel_config_set_write_increment(&pio_dma_chan_config, false);
    channel_config_set_chain_to(&pio_dma_chan_config, ctrl_chan);
    channel_config_set_dreq(&pio_dma_chan_config, DREQ_PWM_WRAP0 + pclk_slice);

    dma_channel_configure(
        pio_dma_chan,
        &pio_dma_chan_config,
        &pio->txf[sm],
        scan_line_buffer,
        VIDEO_SCAN_LINE_LEN - 1,
        true);

    pwm_set_mask_enabled ((1 << hsync_slice) | (1 << vsync_slice) | (1 << pclk_slice));
}

int16_t vga_overscan_line_get(void)
{
    return overscan_line;
}

uint16_t *vga_scan_line_buffer(void)
{
    return &scan_line_buffer[VIDEO_SCAN_BUFFER_OFFSET];
}

bool vga_overscan_line_is_odd(void)
{
    return overscan_line_odd;
}

uint16_t vga_scan_line_get(void)
{
    return scan_line;
}

void vga_wait_for_new_overscan_line(void)
{
    while(scan_line_old)
    {
    }
    scan_line_old = true;
}
