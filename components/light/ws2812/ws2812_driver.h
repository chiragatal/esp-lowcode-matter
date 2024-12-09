#pragma once

#include "lp_sw_timer.h"
#include "color_format.h"
#include "lp_rmt.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initialize ws2812
int ws2812_driver_init(void);

// Deinitialize ws2812
void ws2812_driver_deinit(void);

typedef enum {
    WS2812_CHANNEL_NC = -1,
    WS2812_CHANNEL_RED = 0,
    WS2812_CHANNEL_GREEN,
    WS2812_CHANNEL_BLUE,
    WS2812_CHANNEL_COLD,
    WS2812_CHANNEL_WARM,
    WS2812_CHANNEL_BRIGHTNESS,
    WS2812_CHANNEL_MAX,
} ws2812_channel_enum_t;

struct ws2812_buffer_t {
    RGB_color_t RGBBuffer;
    HS_color_t HSBuffer;
    CW_white_t CWBuffer;
    // uint32_t TempBuffer;
    uint8_t BrightBuffer;
    uint8_t RMTBufferGRB[3];
    uint32_t FadeSpeed;
    ws2812_channel_enum_t lastUpdatedChannel;
};

typedef struct ws2812_buffer_t ws2812_buffer_t;

int ws2812_driver_set_channel(uint8_t channel, uint8_t val);
int ws2812_driver_regist_channel(uint8_t channel, gpio_num_t gpio);
int ws2812_driver_update_channels(void);
// int ws2812_driver_set_channel_fade(uint8_t channel, uint8_t start, uint8_t end, uint32_t speed);
// int ws2812_driver_start_channel_fade(void);
// int ws2812_driver_stop_channel_fade(uint8_t channel);


#ifdef __cplusplus
}
#endif