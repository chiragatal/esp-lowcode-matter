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

#include <string.h>

#include "low_code.h"

static const char *TAG = "low_code";

static low_code_event_callback_t event_to_transport = NULL;
static low_code_feature_update_callback_t feature_update_to_transport = NULL;
static low_code_feature_update_callback_t feature_update_to_application = NULL;
static low_code_event_callback_t event_to_application = NULL;

low_code_get_event_from_system_t get_event_from_system = NULL;
low_code_get_feature_update_from_system_t get_feature_from_system = NULL;

int low_code_event_from_transport(low_code_event_t *event)
{
    if (!event) {
        printf("%s: Low Code event cannot be null\n", TAG);
        return ESP_ERR_INVALID_ARG;
    }

    if (event_to_application) {
        event_to_application(event);
    }
    return ESP_OK;
}

int low_code_event_to_system(low_code_event_t *event)
{
    if (event_to_transport) {
        return event_to_transport(event);
    }
    return ESP_OK;
}

int low_code_feature_update_from_transport(low_code_feature_data_t *data)
{
    // TODO: Call customer prototype function here
    if (!data) {
        printf("%s: Low Code feature data cannot be null\n", TAG);
        return ESP_ERR_INVALID_ARG;
    }

    if (feature_update_to_application) {
        feature_update_to_application(data);
    }
    return ESP_OK;
}

int low_code_feature_update_to_system(low_code_feature_data_t *feature)
{
    if (feature_update_to_transport) {
        feature_update_to_transport(feature);
    }
    return ESP_OK;
}

int low_code_register_transport_callbacks(low_code_callback_list_t *callbacks)
{
    if (!callbacks) {
        printf("%s: Loe Code callback list cannot be null\n", TAG);
        return ESP_ERR_INVALID_ARG;
    }
    if (!callbacks->event_cb || !callbacks->feature_update_cb) {
        printf("%s: Low Code callback cannot be null\n", TAG);
        return ESP_ERR_INVALID_ARG;
    }

    if (event_to_transport || feature_update_to_transport) {
        printf("%s: Low Code transport callback already registered\n", TAG);
        return ESP_ERR_INVALID_STATE;
    }

    event_to_transport = callbacks->event_cb;
    feature_update_to_transport = callbacks->feature_update_cb;
    get_event_from_system  = callbacks->get_event;
    get_feature_from_system = callbacks->get_feature_update;

    return ESP_OK;
}

int low_code_get_event_from_system()
{
    if (get_event_from_system) {
        return get_event_from_system();
    }
    return ESP_FAIL;
}

int low_code_get_feature_update_from_system()
{
    if (get_feature_from_system) {
        return get_feature_from_system();
    }
    return ESP_FAIL;
}

int low_code_register_callbacks(low_code_feature_update_callback_t feature_update_cb, low_code_event_callback_t event_cb)
{
    if (!feature_update_cb || !event_cb) {
        printf("%s: feature_update_cb or event_cb cannot be null\n", TAG);
        return ESP_ERR_INVALID_ARG;
    }

    if (event_to_application || feature_update_to_application) {
        printf("%s: Low Code callback already registered\n", TAG);
        return ESP_ERR_INVALID_STATE;
    }

    event_to_application = event_cb;
    feature_update_to_application = feature_update_cb;

    return ESP_OK;
}
