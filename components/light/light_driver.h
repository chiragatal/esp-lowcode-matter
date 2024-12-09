#pragma once

#include <stdint.h>
#include <inttypes.h>
#include <color_format.h>

#include "soc/gpio_num.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LIGHT_CHANNEL_COMB_INVALID = 0,
    LIGHT_CHANNEL_COMB_1CH_C,
    LIGHT_CHANNEL_COMB_1CH_W,
    LIGHT_CHANNEL_COMB_2CH_CW,
    LIGHT_CHANNEL_COMB_3CH_RGB,
    LIGHT_CHANNEL_COMB_5CH_RGBCW,
    LIGHT_CHANNEL_COMB_MAX,
} light_channel_comb_t;

/**
 * @brief 
 * ideally should select driver in sdkconfig, however lp doesnot support sdkconfig
 * we can use -DCONFIG_XXX in project_settings.cmake to pass extra settings to ulp project
*/
typedef enum {
    LIGHT_DEVICE_TYPE_LED = 0,
    LIGHT_DEVICE_TYPE_WS2812,
    LIGHT_DEVICE_TYPE_MAX,
} light_device_type_t;

typedef union {
    struct {
        gpio_num_t red;
        gpio_num_t green;
        gpio_num_t blue;
        gpio_num_t cold;
        gpio_num_t warm;
    } led_io;
    struct {
        gpio_num_t ctrl_io;
    } ws2812_io;
} light_io_conf_t;

/**
 * apis such as color,brightness,effect should be kept in light product level
 * in bus/device level, only focus on how to manipulate a specific controller such as ledc, iic
 * in ledc, this means update to a single channel; in ws2812, this means construct RMT packets and send
 * ledc should not have any knowledge of combination of channels. This information should be managed by light product driver
 * dev_if_t should be only init, deinit, write, read, update
 * reserve enough parameters to support both ws2812 and ledc
 * parameter channel has no meaning to ws2812
 * 
 * we hardcode and hide the duty into led driver, by default duty resolution is 4096
 * light_matter_set_hue(hue) => light_driver_set_hue(hue) => HS_to_RGB(hue) => 
 * - light_hal_dev_write(channel_R/G/B, (int *)val);
 * - light_hal_dev_write(NULL, (RGB_t *)val);
 * 
 * we can construct a compound argument for function of type `light_hal_dev_write_t`
 * to achieve a more complicated and cut-through manipulation of hardware duty fading from product level to hal driver level
 * e.g. when using led_hal, construct a compound argument to include fade out parameters to tell hal_dev to execute a fade out
 */

/**
 * at least should know how many channels to initialize
*/

typedef  int (* light_dev_init_t) (void);
typedef void (* light_dev_deinit_t) (void);
typedef  int (* light_dev_set_channel_t) (uint8_t channel, uint8_t val); /* write(channel, val) */
typedef  int (* light_dev_get_channel_t) (uint8_t channel, uint8_t *val); /* not implemented */
typedef  int (* light_dev_update_channels_t) (void);    /* use device-specific internal buffer to update the status of device */
typedef  int (* light_dev_regist_channel_t) (uint8_t channel, gpio_num_t gpio);

// TODO: not sure update() is needed or not. Update() allows us to configure multiple channels one by one and switch on them in the same time
typedef void (* light_hal_dev_update_t) (int channel); /* not implemented */

// this is the common handle among ledc, iic or ws2812
typedef void *light_dev_handle_t;

typedef struct {
    light_dev_handle_t handle;
    light_dev_init_t init;
    light_dev_deinit_t deinit;
    light_dev_set_channel_t set_channel;
    light_dev_get_channel_t get_channel;
    light_dev_update_channels_t update_channels;
    light_dev_regist_channel_t regist_channel;
} light_dev_if_t;

typedef enum {
    LIGHT_WORK_MODE_INVALID, /* single channel */
    LIGHT_WORK_MODE_COLOR, /* rgb or rgbcw */
    LIGHT_WORK_MODE_WHITE, /* cw  or rgb */
    LIGHT_WORK_MODE_MAX,
} light_work_mode_t;


typedef struct {
    light_device_type_t device_type;
    light_channel_comb_t channel_comb;
    light_io_conf_t io_conf;
    int min_brightness;
    int max_brightness;
} light_driver_config_t;


typedef enum {
    LIGHT_EFFECT_INVALID,
    LIGHT_EFFECT_BLINK,
    LIGHT_EFFECT_BREATHE,
} light_effect_type_t;

/**
 * @brief light effect settings
*/
typedef struct {
    light_effect_type_t type; /**< type of light effect */
    light_work_mode_t mode; /**< working mode of this effect */
    union {
        RGB_color_t RGB; /**< RGB color for the effect */
        uint32_t cct; /**< color temperature for the effect */
        HS_color_t HS;  /* HSV color space for the effect */
        CW_white_t CW;  /* Cold/Warm space for the effect */
    } color;
        int8_t max_brightness;  /**< max brightness */
        int8_t min_brightness;  /**< min brightness */
} light_effect_config_t;


int light_driver_init(light_driver_config_t *config);
int light_driver_set_power(uint8_t val);
int light_driver_set_brightness(uint8_t val);
int light_driver_set_hue(uint16_t val);
int light_driver_set_saturation(uint8_t val);
int light_driver_set_temperature(uint32_t val);
int light_driver_set_color_mode(uint8_t val);

/**
 * @brief stop old effect and start a new one
 * 
 * @param effect which effect want to set
 * @param speed how long it takes for the next effect to complete a period
 * @param total_ms how long the next effect lasts. -1 means running forever, 
 *        and need to call light_driver_effect_stop() to explicitly stop the effect
 */
void light_driver_effect_start(light_effect_config_t *effect, int speed, int total_ms);

/**
 * @brief stop the current running effect
 */
void light_driver_effect_stop(void);

#ifdef __cplusplus
}
#endif
