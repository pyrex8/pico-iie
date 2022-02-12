
#ifndef __CLOCK_H__
#define __CLOCK_H__

#define VREG_VOLTAGE_1_30 0b1111    // 1.30v
#define CLK_FREQUENCY_HZ 270000000  // overclocking at 270MHz
#define CLK_FREQUENCY_KHZ (CLK_FREQUENCY_HZ / 1000)

void clock_init(void);

#endif /* __CLOCK_H__ */
