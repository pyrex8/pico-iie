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
const uint HSYNC_PIN = 18;

int main()
{
    // Overclocking Pico with up to 436MHz works
   vreg_set_voltage(VREG_VOLTAGE_1_30);
    // overclocking at 270MHz
   set_sys_clock_khz(270000, true);
    // set_sys_clock_khz(420000, true);

    bi_decl(bi_program_description("pico-iie"));

   // gpio_init(LED_PIN);
   // gpio_set_dir(LED_PIN, GPIO_OUT);
   gpio_init(TEST_PIN);
   gpio_set_dir(TEST_PIN, GPIO_OUT);

    PIO pio = pio0;
    uint offset = pio_add_program(pio, &parallel_program);
    uint sm = pio_claim_unused_sm(pio, true);
    parallel_program_init(pio, sm, offset, LED_PIN);

    gpio_set_function(VSYNC_PIN, GPIO_FUNC_PWM);
    uint vsync_slice = pwm_gpio_to_slice_num(VSYNC_PIN);
    uint vsync_channel = pwm_gpio_to_channel(VSYNC_PIN);
    pwm_set_clkdiv_int_frac (vsync_slice, 100, 0);
    pwm_set_wrap(vsync_slice, 999);
    pwm_set_chan_level(vsync_slice, vsync_channel, 500);
    pwm_set_mask_enabled (1 << vsync_slice);

    while (1)
    {

        // 1.8us on Pi Pico with 270MHz overclocking on one core

        gpio_put(TEST_PIN, 0);
        // gpio_put(LED_PIN, 0);
        pio_sm_put_blocking(pio, sm, 0);
        sleep_ms(100);
        gpio_put(TEST_PIN, 1);
        // gpio_put(LED_PIN, 1);
        pio_sm_put_blocking(pio, sm, 1);
        sleep_ms(100);
    }
}
