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

#include <low_code.h>
#include <button_driver.h>
#include <relay_driver.h>
#include <light_driver.h>

#include "app_priv.h"

#define BUTTON_GPIO_NUM 9
#define RELAY_GPIO_NUM 2
#define INDICATOR_GPIO_NUM 3

static const char *TAG = "app_driver";

static bool socket_state = false;

static void app_driver_toggle_socket_state_button_callback(void *arg, void *data)
{
    socket_state = !socket_state;
    printf("%s: Set socket state to %d\n", TAG, socket_state);
    app_driver_set_socket_state(socket_state);

    /* Update the feature */
    low_code_feature_data_t update_data = {
        .details = {
            .endpoint_id = 1,
            .feature_id = LOW_CODE_FEATURE_ID_POWER
        },
        .value = {
            .type = LOW_CODE_VALUE_TYPE_BOOLEAN,
            .value_len = sizeof(bool),
            .value = (uint8_t*)&socket_state,
        },
    };

    low_code_feature_update_to_system(&update_data);
}

static void app_driver_trigger_factory_reset_button_callback(void *arg, void *data)
{
    /* Update by sending event */
    low_code_event_t event = {
        .event_type = LOW_CODE_EVENT_FACTORY_RESET
    };

    low_code_event_to_system(&event);
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

int app_driver_event_handler(low_code_event_t *event)
{
    /* Get the events. Approriate indicators should be shown to the user based on the event. */
    printf("%s: Received event: %d\n", TAG, event->event_type);
    light_effect_config_t effect_config = {
        .type = LIGHT_EFFECT_INVALID,
        .mode = LIGHT_WORK_MODE_WHITE, /* Since it is a single channel LED */
        .max_brightness = 100,
        .min_brightness = 10
    };

    /* Handle the events from low_code_event_type_t */
    switch (event->event_type) {
        case LOW_CODE_EVENT_SETUP_MODE_START:
            printf("%s: Setup mode started\n", TAG);
            /* Start Indication */
            effect_config.type = LIGHT_EFFECT_BLINK;
            light_driver_effect_start(&effect_config, 2000, 0);
            break;
        case LOW_CODE_EVENT_SETUP_MODE_END:
            printf("%s: Setup mode ended\n", TAG);
            /* Stop Indication */
            light_driver_effect_stop();
            break;
        case LOW_CODE_EVENT_SETUP_DEVICE_CONNECTED:
            printf("%s: Device connected during setup\n", TAG);
            break;
        case LOW_CODE_EVENT_SETUP_STARTED:
            printf("%s: Setup process started\n", TAG);
            break;
        case LOW_CODE_EVENT_SETUP_SUCCESSFUL:
            printf("%s: Setup process successful\n", TAG);
            break;
        case LOW_CODE_EVENT_SETUP_FAILED:
            printf("%s: Setup process failed\n", TAG);
            break;
        case LOW_CODE_EVENT_NETWORK_CONNECTED:
            printf("%s: Network connected\n", TAG);
            break;
        case LOW_CODE_EVENT_NETWORK_DISCONNECTED:
            printf("%s: Network disconnected\n", TAG);
            break;
        case LOW_CODE_EVENT_OTA_STARTED:
            printf("%s: OTA update started\n", TAG);
            break;
        case LOW_CODE_EVENT_OTA_STOPPED:
            printf("%s: OTA update stopped\n", TAG);
            break;
        case LOW_CODE_EVENT_READY:
            printf("%s: Device is ready\n", TAG);
            break;
        case LOW_CODE_EVENT_IDENTIFICATION_START:
            printf("%s: Identification started\n", TAG);
            break;
        case LOW_CODE_EVENT_IDENTIFICATION_STOP:
            printf("%s: Identification stopped\n", TAG);
            break;
        case LOW_CODE_EVENT_TEST_MODE_LOW_CODE:
            printf("%s: Low code test mode is triggered for subtype: %d\n", TAG, (int)*((int*)(event->event_data)));
            break;
        case LOW_CODE_EVENT_TEST_MODE_COMMON:
            printf("%s: common test mode triggered\n", TAG);
            break;
        case LOW_CODE_EVENT_TEST_MODE_BLE:
            printf("%s: ble test mode triggered\n", TAG);
            break;
        case LOW_CODE_EVENT_TEST_MODE_SNIFFER:
            printf("%s: sniffer test mode triggered\n", TAG);
            break;
        default:
            printf("%s: Unhandled event type: %d\n", TAG, event->event_type);
            break;
    }

    return 0;
}
