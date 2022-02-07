#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "parallel.pio.h"
#include "pico/binary_info.h"
#include "hardware/structs/vreg_and_chip_reset.h"


#define VREG_VOLTAGE_1_30 0b1111    ///< 1.30v

const uint LED_PIN = 25;
const uint TEST_PIN = 20;
const uint VSYNC_PIN = 17;
const uint HSYNC_PIN = 19;
const uint R0_PIN = 3;

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

    PIO pio = pio0;
    uint offset = pio_add_program(pio, &parallel_program);
    uint sm = pio_claim_unused_sm(pio, true);
    parallel_program_init(pio, sm, offset, R0_PIN);

    // VGA clock = 480 MHz / 2 / 19 = 12.63 MHz
    // VSYNC = 480 MHz / 2 / 7600 / 525 = 60.15 Hz
    // HSYNC = 480 MHz / 2 / 7600 = 31.58 kHz

    // 550MHz / 2 / 8000 = 34.375

    // Screen refresh rate	60 Hz
    // Vertical refresh	31.46875 kHz
    // Pixel freq.	25.175 MHz
    // 25.175 / 2 = 12.5875

    // Pico VGA clock = 270Mz / 12.6MHz =
    // 214285 =  5 x 5 x 5 x 21


    gpio_set_function(HSYNC_PIN, GPIO_FUNC_PWM);
    uint hsync_slice = pwm_gpio_to_slice_num(HSYNC_PIN);
    uint hsync_channel = pwm_gpio_to_channel(HSYNC_PIN);
    pwm_set_clkdiv_int_frac (hsync_slice, 1, 0);
    pwm_set_wrap(hsync_slice, 8579);
    pwm_set_chan_level(hsync_slice, hsync_channel, 7658);

    gpio_set_function(VSYNC_PIN, GPIO_FUNC_PWM);
    uint vsync_slice = pwm_gpio_to_slice_num(VSYNC_PIN);
    uint vsync_channel = pwm_gpio_to_channel(VSYNC_PIN);
    pwm_set_clkdiv_int_frac (vsync_slice, 110, 0);
    pwm_set_wrap(vsync_slice, 40949);
    pwm_set_chan_level(vsync_slice, vsync_channel, 40793);

    pwm_set_mask_enabled ((1 << hsync_slice) | (1 << vsync_slice));

    pio_sm_put_blocking(pio, sm, 0xFFFF);
    // pio_sm_put_blocking(pio, sm, 0);

    while (1)
    {

        // 1.8us on Pi Pico with 270MHz overclocking on one core

        pio_sm_put_blocking(pio, sm, 0xFFFF);
        gpio_put(TEST_PIN, 0);
        gpio_put(LED_PIN, 0);
        sleep_ms(1);
        pio_sm_put_blocking(pio, sm, 0);
        gpio_put(TEST_PIN, 1);
        gpio_put(LED_PIN, 1);

        sleep_ms(1);
    }
}
