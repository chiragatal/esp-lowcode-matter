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
#include <low_code.h>
#include <low_code_transport.h>

#define ESP_AMP_ENDPOINT_FEATURE 0
#define ESP_AMP_ENDPOINT_EVENT 1
#define ESP_AMP_EVENT_SUBCORE_READY (1 << 0)

#define BUF_SIZE 256

static esp_amp_rpmsg_dev_t esp_amp_device = {0};
static esp_amp_rpmsg_ept_t esp_amp_endpoint_feature = {0};
static esp_amp_rpmsg_ept_t esp_amp_endpoint_event = {0};

static const char *TAG = "low_code_transport";

static uint8_t buffer[BUF_SIZE];

static int low_code_transport_event_to_system(low_code_event_t *event)
{
    size_t buffer_size = sizeof(low_code_event_t) + event->event_data_size;
    void *buffer = esp_amp_rpmsg_create_message(&esp_amp_device, buffer_size, ESP_AMP_RPMSG_DATA_DEFAULT);
    if (buffer == NULL) {
        printf("%s: esp_amp_rpmsg_create_message failed\n", TAG);
        return ESP_ERR_NO_MEM;
    }
    memcpy(buffer, event, sizeof(low_code_event_t));
    memcpy(buffer + sizeof(low_code_event_t), event->event_data, event->event_data_size);
    int ret = esp_amp_rpmsg_send_nocopy(&esp_amp_device, &esp_amp_endpoint_event, ESP_AMP_ENDPOINT_EVENT, buffer, buffer_size);
    if (ret != 0) {
        printf("%s: esp_amp_rpmsg_send_nocopy failed\n", TAG);
        return ESP_FAIL;
    }
    // TODO: Check if the created `buffer` is freed later on, so that we don't have any memory leak
    return ESP_OK;
}

static int low_code_transport_feature_update_to_system(low_code_feature_data_t *data)
{
    size_t buffer_size = sizeof(low_code_feature_data_t) + data->value.value_len;
    void *buffer = esp_amp_rpmsg_create_message(&esp_amp_device, buffer_size, ESP_AMP_RPMSG_DATA_DEFAULT);
    if (buffer == NULL) {
        printf("%s: esp_amp_rpmsg_create_message failed\n", TAG);
        return ESP_ERR_NO_MEM;
    }
    memcpy(buffer, data, sizeof(low_code_feature_data_t));
    memcpy(buffer + sizeof(low_code_feature_data_t), data->value.value, data->value.value_len);
    int ret = esp_amp_rpmsg_send_nocopy(&esp_amp_device, &esp_amp_endpoint_feature, ESP_AMP_ENDPOINT_FEATURE, buffer, buffer_size);
    if (ret != 0) {
        printf("%s: esp_amp_rpmsg_send_nocopy failed\n", TAG);
        return ESP_FAIL;
    }
    return ESP_OK;
}

static int from_system_event_cb(void* msg_data, uint16_t data_len, uint16_t src_addr, void* rx_cb_data) {
    low_code_event_t event;
    memcpy(&event, msg_data, sizeof(low_code_event_t));
    if (event.event_data_size > BUF_SIZE) {
        printf("%s: event daata exceeds the buffer size of: %d\n", TAG, BUF_SIZE);
        return 0;
    }
    event.event_data = buffer;
    memcpy(event.event_data, msg_data + sizeof(low_code_event_t), event.event_data_size);
    int ret = low_code_event_from_transport(&event);
    esp_amp_rpmsg_destroy(&esp_amp_device, msg_data);
    return 0;
}

static int from_system_data_cb(void* msg_data, uint16_t data_len, uint16_t src_addr, void* rx_cb_data) {
    low_code_feature_data_t data;
    memcpy(&data, msg_data, sizeof(low_code_feature_data_t));
    if (data.value.value_len > BUF_SIZE) {
        printf("%s: feature data value_len exceeds the buffer size of: %d\n", TAG, BUF_SIZE);
        return 0;
    }
    data.value.value = buffer;
    memcpy(data.value.value, msg_data + sizeof(low_code_feature_data_t), data.value.value_len);
    int ret = low_code_feature_update_from_transport(&data);
    esp_amp_rpmsg_destroy(&esp_amp_device, msg_data);
    return 0;
}

static int low_code_transport_init(void)
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

    esp_amp_rpmsg_create_endpoint(&esp_amp_device, ESP_AMP_ENDPOINT_EVENT, from_system_event_cb, NULL, &esp_amp_endpoint_event);
    esp_amp_rpmsg_create_endpoint(&esp_amp_device, ESP_AMP_ENDPOINT_FEATURE, from_system_data_cb, NULL, &esp_amp_endpoint_feature);

    esp_amp_event_notify(ESP_AMP_EVENT_SUBCORE_READY);

    return ESP_OK;
}

static int low_code_transport_get_event_from_system()
{
    // TODO: Try to call the API which polls for particular endpoint only
    esp_amp_rpmsg_poll(&esp_amp_device);
    return ESP_OK;
}

static int low_code_transport_get_feature_update_from_system()
{
    // TODO: Try to call the API which polls for particular endpoint only
    esp_amp_rpmsg_poll(&esp_amp_device);
    return ESP_OK;
}

int low_code_transport_register_callbacks()
{
    int ret;
    ret = low_code_transport_init();
    if (ret != ESP_OK) {
        printf("%s: low_code_transport_init failed\n", TAG);
        return ret;
    }

    low_code_callback_list_t callbacks_list = {
        .event_cb = low_code_transport_event_to_system,
        .feature_update_cb = low_code_transport_feature_update_to_system,
        .get_event = low_code_transport_get_event_from_system,
        .get_feature_update = low_code_transport_get_feature_update_from_system
    };
    ret = low_code_register_transport_callbacks(&callbacks_list);
    if (ret != ESP_OK) {
        printf("%s: Failed to register Low Code transport callbacks\n", TAG);
    }
    return ret;
}
