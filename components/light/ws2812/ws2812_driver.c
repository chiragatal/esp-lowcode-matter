#include "ws2812_driver.h"

static lp_rmt_channel_t ws2812RmtChannel;
static ws2812_buffer_t ws2812Buffer;

#define LP_RMT_DEFAULT_TX_CHANNEL       (0)
#define LP_RMT_DEFAULT_CLK_RESOLUTION   XTAL_CLK_FREQ

int ws2812_driver_init(void) {
    lp_rmt_init_device();
    return 0;
}

int ws2812_driver_regist_channel(uint8_t channel, gpio_num_t gpio)
{
    ws2812RmtChannel.channalId = LP_RMT_DEFAULT_TX_CHANNEL;
    // use default clock resolution (40MHz)
    ws2812RmtChannel.clkResolutionHz = LP_RMT_DEFAULT_CLK_RESOLUTION;
    // use default GPIO pin as output
    ws2812RmtChannel.gpioPin = gpio;
    // use default clock
    ws2812RmtChannel.groupClkResolutionHz = 0;

    // lp_rmt_create_default_tx_channel(&ws2812RmtChannel);
    
    lp_rmt_config_tx_channel(&ws2812RmtChannel);

    ws2812RmtChannel.bit0 = (rmt_symbol_word_t) {
        .level0 = 1,
        .duration0 = 14,
        .level1 = 0,
        .duration1 = 40
    };
    ws2812RmtChannel.bit1 = (rmt_symbol_word_t) {
        .level0 = 1,
        .duration0 = 40,
        .level1 = 0,
        .duration1 = 14
    };
    ws2812RmtChannel.msbFirst = true;

    // set default brightness for hsv color space
    ws2812Buffer.BrightBuffer = 10;

    return 0;
}

void ws2812_driver_deinit(void) {
    lp_rmt_deinit_device();
}

int ws2812_driver_set_channel(uint8_t channel, uint8_t val) {
    // As for WS2812, we define the unit of speed as ms (milli second)
    switch (channel) {
        case WS2812_CHANNEL_RED:
            ws2812Buffer.RGBBuffer.red = val;
        break;
        case WS2812_CHANNEL_GREEN:
            ws2812Buffer.RGBBuffer.green = val;
        break;
        case WS2812_CHANNEL_BLUE:
            ws2812Buffer.RGBBuffer.blue = val;
        break;
        case WS2812_CHANNEL_COLD:
            ws2812Buffer.CWBuffer.cold = val;
        break;
        case WS2812_CHANNEL_WARM:
            ws2812Buffer.CWBuffer.warm = val;
        break;
        case WS2812_CHANNEL_BRIGHTNESS:
            ws2812Buffer.BrightBuffer = val;
        break;
        default:
            return 1;
        break;
    }
    ws2812Buffer.lastUpdatedChannel = channel;
    return 0;
}

int ws2812_driver_update_channels(void) {
    switch (ws2812Buffer.lastUpdatedChannel) {
        case WS2812_CHANNEL_RED:
        case WS2812_CHANNEL_GREEN:
        case WS2812_CHANNEL_BLUE:
            // use RGB value as final reference
        break;
        case WS2812_CHANNEL_COLD:
        case WS2812_CHANNEL_WARM:
        case WS2812_CHANNEL_BRIGHTNESS:
            // use CW value as final reference
            cw_to_hsv(ws2812Buffer.CWBuffer, &ws2812Buffer.HSBuffer);
            hsv_to_rgb(ws2812Buffer.HSBuffer, ws2812Buffer.BrightBuffer, &ws2812Buffer.RGBBuffer);
        break;
        default:
            return 1;
        break;
    }

    // Copy color settings to RMT buffer
    ws2812Buffer.RMTBufferGRB[0] = ws2812Buffer.RGBBuffer.green;
    ws2812Buffer.RMTBufferGRB[1] = ws2812Buffer.RGBBuffer.red;
    ws2812Buffer.RMTBufferGRB[2] = ws2812Buffer.RGBBuffer.blue;
    // update color settings
    lp_rmt_send_bytes(ws2812Buffer.RMTBufferGRB, 24, &ws2812RmtChannel);

    return 0;
}
