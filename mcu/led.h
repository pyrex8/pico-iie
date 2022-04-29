#ifndef __LED_H__
#define __LED_H__

#include <stdbool.h>

// Configuration
#define LED_RED_PIN_NUMBER 18
#define LED_GREEN_PIN_NUMBER 16

void led_red_init(void);
void led_red_set(bool value);

void led_green_init(void);
void led_green_set(bool value);

#endif /* __LED_H__ */
