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


#define VREG_VOLTAGE_1_30 0b1111    ///< 1.30v
#define SCAN_LINE_BUFFER_LEN (280 + 44 + 1)

const uint LED_PIN = 25;
const uint TEST_PIN = 21;
const uint VSYNC_PIN = 17;
const uint HSYNC_PIN = 19;
const uint PCLK_PIN = 20;
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

uint16_t scan_line;
uint16_t h_pixel;

uint16_t scan_line_buffer[SCAN_LINE_BUFFER_LEN] = {0};
uint16_t scan_line_blank[SCAN_LINE_BUFFER_LEN] = {0};
uint16_t scan_line_image[SCAN_LINE_BUFFER_LEN] = {0};

void __not_in_flash_func(vga_scan_line)(void)
{
    dma_hw->ch[pio_dma_chan].al3_read_addr_trig = scan_line_buffer;
    pwm_clear_irq(hsync_slice);

    scan_line = pwm_get_counter(vsync_slice) / 78 / 2;

    if ((scan_line > 40) && (scan_line < 40 + 192))
    {
        memcpy(scan_line_buffer, scan_line_image, SCAN_LINE_BUFFER_LEN * 2);
    }
    else
    {
        memcpy(scan_line_buffer, scan_line_blank, SCAN_LINE_BUFFER_LEN * 2);
    }
}

int main()
{
    // Overclocking Pico with up to 420MHz eventually
   vreg_set_voltage(VREG_VOLTAGE_1_30);
    // overclocking at 270MHz
   set_sys_clock_khz(270000, true);
    // set_sys_clock_khz(420000, true);

    bi_decl(bi_program_description("pico-iie"));

   gpio_init(LED_PIN);
   gpio_set_dir(LED_PIN, GPIO_OUT);
   gpio_init(TEST_PIN);
   gpio_set_dir(TEST_PIN, GPIO_OUT);

   for (int i = 0; i < SCAN_LINE_BUFFER_LEN; i++)
   {
       if ((i > 44) && (i < SCAN_LINE_BUFFER_LEN - 1))
       {
           scan_line_image[i] = 0xFFFF;
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
    irq_set_enabled(PWM_IRQ_WRAP, true);

    pwm_set_clkdiv_int_frac (hsync_slice, 1, 0);
    pwm_set_wrap(hsync_slice, 8579);
    pwm_set_chan_level(hsync_slice, hsync_channel, 7658);

    gpio_set_function(VSYNC_PIN, GPIO_FUNC_PWM);
    vsync_slice = pwm_gpio_to_slice_num(VSYNC_PIN);
    vsync_channel = pwm_gpio_to_channel(VSYNC_PIN);
    pwm_set_clkdiv_int_frac (vsync_slice, 110, 0);
    pwm_set_wrap(vsync_slice, 40949);
    pwm_set_chan_level(vsync_slice, vsync_channel, 40793);

    gpio_set_function(PCLK_PIN, GPIO_FUNC_PWM);
    pclk_slice = pwm_gpio_to_slice_num(PCLK_PIN);
    pclk_channel = pwm_gpio_to_channel(PCLK_PIN);
    pwm_set_clkdiv_int_frac (pclk_slice, 1, 0);
    pwm_set_wrap(pclk_slice, 21);
    pwm_set_chan_level(pclk_slice, pclk_channel, 10);

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
        SCAN_LINE_BUFFER_LEN,
        true);

    pwm_set_mask_enabled ((1 << hsync_slice) | (1 << vsync_slice) | (1 << pclk_slice));

    while (1)
    {

        // 1.8us on Pi Pico with 270MHz overclocking on one core

        gpio_put(TEST_PIN, 0);
        gpio_put(LED_PIN, 0);
        sleep_ms(500);
        gpio_put(TEST_PIN, 1);
        gpio_put(LED_PIN, 1);
        sleep_ms(500);
    }
}
