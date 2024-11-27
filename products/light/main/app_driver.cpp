// Copyright 2024 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdio.h>

#include <light_driver.h>

#include "app_driver.h"

#define COLD_CHANNEL_IO 4
#define WARM_CHANNEL_IO 6

static const char *TAG = "app_driver";

int app_driver_init()
{
    printf("%s: Initializing light driver\n", TAG);
    light_driver_config_t cfg = {
        .device_type = LIGHT_DEVICE_TYPE_LED,
        .channel_comb = LIGHT_CHANNEL_COMB_2CH_CW,
        .io_conf = {
            .led_io = {
                .cold = COLD_CHANNEL_IO,
                .warm = WARM_CHANNEL_IO,
            },
        },
        .min_brightness = 0,
        .max_brightness = 100,
    };
    light_driver_init(&cfg);
    light_driver_set_power(true);
    light_driver_set_brightness(100);
    light_driver_set_temperature(4000);
    return 0;
}

int app_driver_set_light_state(bool state)
{
    printf("%s: Setting light state: %s\n", TAG, state ? "ON" : "OFF");
    return light_driver_set_power(state);
}

int app_driver_set_light_brightness(uint8_t brightness)
{
    brightness = brightness * 100 / 255;
    printf("%s: Setting light brightness: %d\n", TAG, brightness);
    return light_driver_set_brightness(brightness);
}

int app_driver_set_light_temperature(uint16_t temperature)
{
    temperature = 1000000 / temperature;
    printf("%s: Setting light temperature: %d\n", TAG, temperature);
    return light_driver_set_temperature(temperature);
}
