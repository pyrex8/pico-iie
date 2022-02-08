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

const uint LED_PIN = 25;
const uint TEST_PIN = 20;
const uint VSYNC_PIN = 17;
const uint HSYNC_PIN = 19;
const uint R0_PIN = 0;

PIO pio;
uint offset;
uint sm;

uint hsync_slice;
uint hsync_channel;

uint vsync_slice;
uint vsync_channel;

uint16_t scan_line;
uint16_t h_pixel;

void vga_scan_line(void)
{
    pwm_clear_irq(hsync_slice);

    scan_line = pwm_get_counter(vsync_slice) / 78 / 2;

    if ((scan_line > 40) && (scan_line < 40 + 192))
    {
        h_pixel = 0;
        while( h_pixel < 44)
        {
            h_pixel = pwm_get_counter(hsync_slice) / 22;
        }
        pio_sm_put_blocking(pio, sm, 0xFFFF);
        while( h_pixel < 44 + 280)
        {
            h_pixel = pwm_get_counter(hsync_slice) / 22;
        }
        pio_sm_put_blocking(pio, sm, 0);
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

    pwm_set_mask_enabled ((1 << hsync_slice) | (1 << vsync_slice));

    while (1)
    {

        // 1.8us on Pi Pico with 270MHz overclocking on one core

        gpio_put(TEST_PIN, 0);
        gpio_put(LED_PIN, 0);
        sleep_ms(100);
        gpio_put(TEST_PIN, 1);
        gpio_put(LED_PIN, 1);
        sleep_ms(100);
    }
}
