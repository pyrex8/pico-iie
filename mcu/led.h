#ifndef __LED_H__
#define __LED_H__

#include <stdint.h>

// Configuration
#define LED_PIN_NUMBER 25
#define LED_RED_PIN_NUMBER 18

#define LED_BLINK_DELAY_NORMAL_MS  750
#define LED_BLINK_DELAY_LOADING_MS 100
#define LED_BLINK_DELAY_DISK_MS    300

typedef enum
{
    LED_BLINK_NORMAL = 0,
    LED_BLINK_LOADING,
    LED_BLINK_DISK,
} LedBlinkType;

void led_blink_init(uint32_t update_delay);
void led_blink_update(LedBlinkType blink_type);

void led_red_init(void);
void led_red_low(void);
void led_red_high(void);

#endif /* __LED_H__ */
