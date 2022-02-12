#include "clock.h"
#include "pico/stdlib.h"

void clock_init(void)
{
    vreg_set_voltage(VREG_VOLTAGE_1_30);
    set_sys_clock_khz(CLK_FREQUENCY_KHZ, true);
}
