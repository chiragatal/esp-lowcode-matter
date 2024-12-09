#pragma once

#include "soc/gpio_num.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LED_CHANNEL_NC = -1,
    LED_CHANNEL_RED = 0,
    LED_CHANNEL_GREEN,
    LED_CHANNEL_BLUE,
    LED_CHANNEL_COLD,
    LED_CHANNEL_WARM,
    LED_CHANNEL_MAX,
} led_channel_enum_t;

/* enable clock & timer */
int led_driver_init(void);

/* disable clock & timer */
void led_driver_deinit(void);
int led_driver_set_channel(uint8_t channel, uint8_t val);

/* init channel */
int led_driver_regist_channel(uint8_t channel, gpio_num_t gpio);
int led_driver_update_channels(void);

#ifdef __cplusplus
}
#endif
