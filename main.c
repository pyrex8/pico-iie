#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/structs/vreg_and_chip_reset.h"

#define VREG_VOLTAGE_1_30 0b1111    ///< 1.30v

const uint LED_PIN = 25;
const uint TEST_PIN = 20;

int main()
{
    // Overclocking Pico with up to 436MHz works
    vreg_set_voltage(VREG_VOLTAGE_1_30);
    // overclocking at 270MHz
    set_sys_clock_khz(270000, true);
    // set_sys_clock_khz(420000, true);

    bi_decl(bi_program_description("pico-iie"));
    bi_decl(bi_1pin_with_name(LED_PIN, "On-board LED"));

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_init(TEST_PIN);
    gpio_set_dir(TEST_PIN, GPIO_OUT);

    while (1)
    {
        gpio_put(TEST_PIN, 1);

        // 1.8us on Pi Pico with 270MHz overclocking on one core


        gpio_put(TEST_PIN, 0);
        gpio_put(LED_PIN, 0);
        sleep_ms(100);
        gpio_put(LED_PIN, 1);
        sleep_ms(100);
    }
}
