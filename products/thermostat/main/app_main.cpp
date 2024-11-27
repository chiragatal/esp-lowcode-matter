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

#include <low_code_transport.h>
#include <device_features.h>

#include "app_driver.h"

static const char *TAG = "low_code";

int low_code_external_from_internal_feature_update(device_feature_data_t *data)
{
    uint16_t endpoint_id = data->details.endpoint_id;
    uint32_t cluster_id = data->details.cluster_id;
    uint32_t attribute_id = data->details.attribute_id;
    uint8_t *value = data->value.value;

    if (endpoint_id == 0x0001) {
        if (cluster_id == 0x0201) {  // Thermostat cluster
            if (attribute_id == 0x0000) {  // Local Temperature
                int16_t temperature = *(int16_t *)value;
                app_driver_set_temperature(temperature);
                printf("%s: Feature update: temperature: %d\n", TAG, temperature);
            } else if (attribute_id == 0x0012) {  // Occupied Cooling Setpoint
                int16_t cooling_setpoint = *(int16_t *)value;
                app_driver_set_cooling_setpoint(cooling_setpoint);
                printf("%s: Feature update: cooling setpoint: %d\n", TAG, cooling_setpoint);
            } else if (attribute_id == 0x0014) {  // Occupied Heating Setpoint
                int16_t heating_setpoint = *(int16_t *)value;
                app_driver_set_heating_setpoint(heating_setpoint);
                printf("%s: Feature update: heating setpoint: %d\n", TAG, heating_setpoint);
            }
        }
    }

    return 0;
}

int low_code_external_from_internal_event(device_feature_event_t *event)
{
    printf("%s: Received event: %d\n", TAG, event->event_type);

    switch (event->event_type) {
        case DEVICE_FEATURES_EVENT_SETUP_MODE_START:
            printf("%s: Setup mode started\n", TAG);
            break;
        case DEVICE_FEATURES_EVENT_SETUP_MODE_END:
            printf("%s: Setup mode ended\n", TAG);
            break;
        case DEVICE_FEATURES_EVENT_SETUP_DEVICE_CONNECTED:
            printf("%s: Device connected during setup\n", TAG);
            break;
        case DEVICE_FEATURES_EVENT_SETUP_STARTED:
            printf("%s: Setup process started\n", TAG);
            break;
        case DEVICE_FEATURES_EVENT_SETUP_SUCCESSFUL:
            printf("%s: Setup process successful\n", TAG);
            break;
        case DEVICE_FEATURES_EVENT_SETUP_FAILED:
            printf("%s: Setup process failed\n", TAG);
            break;
        case DEVICE_FEATURES_EVENT_NETWORK_CONNECTED:
            printf("%s: Network connected\n", TAG);
            break;
        case DEVICE_FEATURES_EVENT_NETWORK_DISCONNECTED:
            printf("%s: Network disconnected\n", TAG);
            break;
        case DEVICE_FEATURES_EVENT_OTA_STARTED:
            printf("%s: OTA update started\n", TAG);
            break;
        case DEVICE_FEATURES_EVENT_OTA_STOPPED:
            printf("%s: OTA update stopped\n", TAG);
            break;
        case DEVICE_FEATURES_EVENT_READY:
            printf("%s: Device is ready\n", TAG);
            break;
        case DEVICE_FEATURES_EVENT_IDENTIFICATION_START:
            printf("%s: Identification started\n", TAG);
            break;
        case DEVICE_FEATURES_EVENT_IDENTIFICATION_STOP:
            printf("%s: Identification stopped\n", TAG);
            break;
        case DEVICE_FEATURES_EVENT_TEST_MODE_LOW_CODE:
            printf("%s: Low code test mode is triggered for subtype: %d\n", TAG, (int)*((int*)(event->event_data)));
            break;
        case DEVICE_FEATURES_EVENT_TEST_MODE_COMMON:
            printf("%s: common test mode triggered\n", TAG);
            break;
        case DEVICE_FEATURES_EVENT_TEST_MODE_BLE:
            printf("%s: ble test mode triggered\n", TAG);
            break;
        case DEVICE_FEATURES_EVENT_TEST_MODE_SNIFFER:
            printf("%s: sniffer test mode triggered\n", TAG);
            break;
        default:
            printf("%s: Unhandled event type: %d\n", TAG, event->event_type);
            break;
    }

    return 0;
}

static int low_code_init()
{
    /* Register callbacks */
    device_feature_low_code_register_callback(low_code_external_from_internal_feature_update,
                                              low_code_external_from_internal_event);

    /* Initialize driver */
    return app_driver_init();
}

extern "C" int main()
{
    printf("%s: Starting low code external\n", TAG);

    /* Pre-Initializations: This should be called first and should always be present */
    low_code_transport_lp_core_external_register_callbacks();

    /* Setup */
    low_code_init();

    /* Loop */
    while (1) {
        device_feature_get_event();
        device_feature_get_feature();
    }
    return 0;
}
