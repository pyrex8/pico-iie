
#include "led.h"
#include "pico/stdlib.h"

const uint LED_PIN = LED_PIN_NUMBER;

static uint32_t blink_delay[] =
{
    [LED_BLINK_NORMAL] = LED_BLINK_DELAY_NORMAL_MS,
    [LED_BLINK_LOADING] = LED_BLINK_DELAY_LOADING_MS,
    [LED_BLINK_DISK] = LED_BLINK_DELAY_DISK_MS,
};

void led_blink_init(uint32_t update_delay)
{
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    if (update_delay > 0)
    {
        int size = sizeof blink_delay / sizeof blink_delay[0];
        for (int i = 0; i < size; i++)
        {
            blink_delay[i] /= update_delay;
        }
    }
}

void led_blink_update(LedBlinkType blink_type)
{
    static uint32_t led_counter = 0;
    static uint8_t led_is_on = 0;
    uint32_t max_count = blink_delay[blink_type];

    led_counter++;
    if(led_counter > max_count)
    {
      led_counter = 0;

      if(led_is_on)
      {
        led_is_on = 0;
        gpio_put(LED_PIN, 0);
      }
      else
      {
        led_is_on = 1;
        gpio_put(LED_PIN, 1);
      }
    }

}
