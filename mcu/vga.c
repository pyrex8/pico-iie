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

const uint VSYNC_PIN = 17;
const uint HSYNC_PIN = 19;
const uint PCLK_PIN = 5;
const uint R0_PIN = 0;

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
static uint16_t scan_line_blank[VIDEO_SCAN_LINE_LEN] = {0};
static uint16_t *p_scan_line_buffer;
static bool scan_line_old = true;


void __attribute__((noinline, long_call, section(".time_critical"))) vga_scan_line(void)
{
    pwm_clear_irq(hsync_slice);
    scan_line_old = false;
}

void vga_init(void)
{
//    bus_ctrl_hw->priority = BUSCTRL_BUS_PRIORITY_DMA_W_BITS | BUSCTRL_BUS_PRIORITY_DMA_R_BITS;

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
    return pwm_get_counter(vsync_slice) / VSYNC_SCAN_MULTIPLIER - VIDEO_SCAN_LINE_OFFSET;
}

uint16_t *vga_scan_line_buffer(void)
{
    return &scan_line_buffer[VIDEO_SCAN_BUFFER_OFFSET];
}

void vga_blank_scan_line_set(void)
{
    memcpy(scan_line_buffer, scan_line_blank, VIDEO_SCAN_BUFFER_LEN);
}

bool vga_scan_line_not_ready(void)
{
    return scan_line_old;
}

bool vga_scan_line_not_ready_reset(void)
{
    scan_line_old = true;

}
