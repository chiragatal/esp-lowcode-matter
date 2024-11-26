// Copyright 2022 Espressif Systems (Shanghai) PTE LTD
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <esp_amp.h>
#include <ulp_lp_core_utils.h>
#include <device_features.h>
#include <low_code_transport.h>

#define ESP_AMP_ENDPOINT_DEVICE_FEATURE 0
#define ESP_AMP_ENDPOINT_EVENT 1
#define ESP_AMP_EVENT_SUBCORE_READY (1 << 0)

#define BUF_SIZE 256

static esp_amp_rpmsg_dev_t esp_amp_device = {0};
static esp_amp_rpmsg_ept_t esp_amp_endpoint_device_feature = {0};
static esp_amp_rpmsg_ept_t esp_amp_endpoint_event = {0};

static const char *TAG = "transport_lp_core_external";

static uint8_t buffer[BUF_SIZE];

static int transport_lp_core_external_to_internal_send_event(device_feature_event_t *event)
{
    size_t buffer_size = sizeof(device_feature_event_t) + event->event_data_size;
    void *buffer = esp_amp_rpmsg_create_message(&esp_amp_device, buffer_size, ESP_AMP_RPMSG_DATA_DEFAULT);
    if (buffer == NULL) {
        printf("%s: esp_amp_rpmsg_create_message failed\n", TAG);
        return ESP_ERR_NO_MEM;
    }
    memcpy(buffer, event, sizeof(device_feature_event_t));
    memcpy(buffer + sizeof(device_feature_event_t), event->event_data, event->event_data_size);
    int ret = esp_amp_rpmsg_send_nocopy(&esp_amp_device, &esp_amp_endpoint_event, ESP_AMP_ENDPOINT_EVENT, buffer, buffer_size);
    if (ret != 0) {
        printf("%s: esp_amp_rpmsg_send_nocopy failed\n", TAG);
        return ESP_FAIL;
    }
    // TODO: Check if the created `buffer` is freed later on, so that we don't have any memory leak
    return ESP_OK;
}

static int transport_lp_core_external_to_internal_feature_update(device_feature_data_t *data)
{
    size_t buffer_size = sizeof(device_feature_data_t) + data->value.value_len;
    void *buffer = esp_amp_rpmsg_create_message(&esp_amp_device, buffer_size, ESP_AMP_RPMSG_DATA_DEFAULT);
    if (buffer == NULL) {
        printf("%s: esp_amp_rpmsg_create_message failed\n", TAG);
        return ESP_ERR_NO_MEM;
    }
    memcpy(buffer, data, sizeof(device_feature_data_t));
    memcpy(buffer + sizeof(device_feature_data_t), data->value.value, data->value.value_len);
    int ret = esp_amp_rpmsg_send_nocopy(&esp_amp_device, &esp_amp_endpoint_device_feature, ESP_AMP_ENDPOINT_DEVICE_FEATURE, buffer, buffer_size);
    if (ret != 0) {
        printf("%s: esp_amp_rpmsg_send_nocopy failed\n", TAG);
        return ESP_FAIL;
    }
    return ESP_OK;
}

static int external_from_internal_event_cb(void* msg_data, uint16_t data_len, uint16_t src_addr, void* rx_cb_data) {
    device_feature_event_t event;
    memcpy(&event, msg_data, sizeof(device_feature_event_t));
    if (event.event_data_size > BUF_SIZE) {
        printf("%s: event daata exceeds the buffer size of: %d\n", TAG, BUF_SIZE);
        return 0;
    }
    event.event_data = buffer;
    memcpy(event.event_data, msg_data + sizeof(device_feature_event_t), event.event_data_size);
    int ret = device_features_external_from_internal_event_update(&event);
    esp_amp_rpmsg_destroy(&esp_amp_device, msg_data);
    return 0;
}

static int external_from_internal_data_cb(void* msg_data, uint16_t data_len, uint16_t src_addr, void* rx_cb_data) {
    device_feature_data_t data;
    memcpy(&data, msg_data, sizeof(device_feature_data_t));
    if (data.value.value_len > BUF_SIZE) {
        printf("%s: feature data value_len exceeds the buffer size of: %d\n", TAG, BUF_SIZE);
        return 0;
    }
    data.value.value = buffer;
    memcpy(data.value.value, msg_data + sizeof(device_feature_data_t), data.value.value_len);
    int ret = device_features_external_from_internal_feature_update(&data);
    esp_amp_rpmsg_destroy(&esp_amp_device, msg_data);
    return 0;
}

static int transport_lp_core_external_init(void)
{
    int ret;

    /* init esp amp component */
    ret = esp_amp_init();
    if (ret != 0) {
        printf("%s: esp_amp_init failed\n", TAG);
        return ret;
    }
    ret = esp_amp_rpmsg_sub_init(&esp_amp_device, true, true);
    if (ret != 0) {
        printf("%s: esp_amp_rpmsg_sub_init failed\n", TAG);
        return ret;
    }

    esp_amp_rpmsg_create_endpoint(&esp_amp_device, ESP_AMP_ENDPOINT_EVENT, external_from_internal_event_cb, NULL, &esp_amp_endpoint_event);
    esp_amp_rpmsg_create_endpoint(&esp_amp_device, ESP_AMP_ENDPOINT_DEVICE_FEATURE, external_from_internal_data_cb, NULL, &esp_amp_endpoint_device_feature);

    esp_amp_event_notify(ESP_AMP_EVENT_SUBCORE_READY);

    return ESP_OK;
}

static int transport_lp_core_external_get_event()
{
    // TODO: Try to call the API which polls for particular endpoint only
    esp_amp_rpmsg_poll(&esp_amp_device);
    return ESP_OK;
}

static int transport_lp_core_external_get_feature()
{
    // TODO: Try to call the API which polls for particular endpoint only
    esp_amp_rpmsg_poll(&esp_amp_device);
    return ESP_OK;
}

int low_code_transport_lp_core_external_register_callbacks()
{
    int ret;
    ret = transport_lp_core_external_init();
    if (ret != ESP_OK) {
        printf("%s: transport_lp_core_external_init failed\n", TAG);
        return ret;
    }

    device_feature_callback_list_t callbacks_list = {
        .send_event_cb = transport_lp_core_external_to_internal_send_event,
        .send_update_cb = transport_lp_core_external_to_internal_feature_update,
        .get_event = transport_lp_core_external_get_event,
        .get_feature = transport_lp_core_external_get_feature
    };
    ret = device_feature_transport_register_callback(&callbacks_list);
    if (ret != ESP_OK) {
        printf("%s: Failed to register device_feature_internal callback\n", TAG);
    }
    return ret;
}
