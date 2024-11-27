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
#include <ulp_lp_core_utils.h>

#include <device_features.h>
#include <button_driver.h>
#include <relay_driver.h>
#include <light_driver.h>

#include "app_driver.h"

#define BUTTON_GPIO_NUM 0
#define RELAY_GPIO_NUM 2
#define INDICATOR_GPIO_NUM 3

static const char *TAG = "app_driver";

static bool socket_state = false;

static void app_driver_toggle_socket_state_button_callback(void *arg, void *data)
{
    socket_state = !socket_state;
    printf("%s: Set socket state to %d\n", TAG, socket_state);
    app_driver_set_socket_state(socket_state);

    /* Update the device feature */
    device_feature_data_t update_data = {
        .details = {
            .endpoint_id = 0x0001,
            .cluster_id = 0x0006,
            .attribute_id = 0x0000,
        },
        .value = {
            .type = DEVICE_FEATURES_VALUE_TYPE_BOOLEAN,
            .value_len = sizeof(bool),
            .value = (uint8_t*)&socket_state,
        },
    };

    device_features_external_to_internal_feature_update(&update_data);
}

static void app_driver_trigger_factory_reset_button_callback(void *arg, void *data)
{
    /* Update by sending event */
    device_feature_event_t event = {
        .event_type = DEVICE_FEATURES_EVENT_FACTORY_RESET
    };

    device_features_external_to_internal_event_update(&event);
    printf("%s: Factory reset triggered\n", TAG);
}

int app_driver_init()
{
    /* Initialize relay */
    relay_driver_init(RELAY_GPIO_NUM);

    /* Enable intterupts for button */
    ulp_lp_core_sw_intr_enable(true);

    /* Initialize button */
    button_config_t btn_cfg = {
        .gpio_num = BUTTON_GPIO_NUM,
        .pullup_en = 1,
        .active_level = 0,
    };
    button_handle_t btn_handle = iot_button_create(&btn_cfg);
    if (!btn_handle) {
        printf("Failed to create the button");
        return -1;
    }

    /* Register callback to toggle socket state on button click */
    iot_button_register_cb(btn_handle, BUTTON_SINGLE_CLICK, app_driver_toggle_socket_state_button_callback, NULL);

    /* Register callback to factory reset the device on button long press */
    iot_button_register_cb(btn_handle, BUTTON_LONG_PRESS_UP, app_driver_trigger_factory_reset_button_callback, NULL);

    /* Initialise the light indicator */
    light_driver_config_t cfg = {
        .device_type = LIGHT_DEVICE_TYPE_LED,
        .channel_comb = LIGHT_CHANNEL_COMB_1CH_W,
        .io_conf = {
            .led_io = {
                .warm = INDICATOR_GPIO_NUM,
            },
        },
        .min_brightness = 0,
        .max_brightness = 100,
    };
    light_driver_init(&cfg);
    light_driver_set_power(socket_state);

    printf("%s: App driver initialized\n", TAG);
    return 0;
}

int app_driver_set_socket_state(bool state)
{
    /* Set relay state */
    socket_state = state;
    printf("%s: Set socket state to %d\n", TAG, state);
    relay_driver_set_power(RELAY_GPIO_NUM, state);
    light_driver_set_power(state);
    return 0;
}
