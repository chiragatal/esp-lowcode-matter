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

#pragma once

#include <stdio.h>
#include <cstdint>

// TODO: Remove this error list from here and create low_code_utils which has all the standard error
/* Definitions for error constants. */
#define ESP_OK          0       /*!< esp_err_t value indicating success (no error) */
#define ESP_FAIL        -1      /*!< Generic esp_err_t code indicating failure */

#define ESP_ERR_NO_MEM              0x101   /*!< Out of memory */
#define ESP_ERR_INVALID_ARG         0x102   /*!< Invalid argument */
#define ESP_ERR_INVALID_STATE       0x103   /*!< Invalid state */
#define ESP_ERR_INVALID_SIZE        0x104   /*!< Invalid size */
#define ESP_ERR_NOT_FOUND           0x105   /*!< Requested resource not found */
#define ESP_ERR_NOT_SUPPORTED       0x106   /*!< Operation or feature not supported */
#define ESP_ERR_TIMEOUT             0x107   /*!< Operation timed out */
#define ESP_ERR_INVALID_RESPONSE    0x108   /*!< Received response was invalid */
#define ESP_ERR_INVALID_CRC         0x109   /*!< CRC or checksum was invalid */
#define ESP_ERR_INVALID_VERSION     0x10A   /*!< Version was invalid */
#define ESP_ERR_INVALID_MAC         0x10B   /*!< MAC address was invalid */
#define ESP_ERR_NOT_FINISHED        0x10C   /*!< Operation has not fully completed */
#define ESP_ERR_NOT_ALLOWED         0x10D   /*!< Operation is not allowed */


#define ESP_ERR_WIFI_BASE           0x3000  /*!< Starting number of WiFi error codes */
#define ESP_ERR_MESH_BASE           0x4000  /*!< Starting number of MESH error codes */
#define ESP_ERR_FLASH_BASE          0x6000  /*!< Starting number of flash error codes */
#define ESP_ERR_HW_CRYPTO_BASE      0xc000  /*!< Starting number of HW cryptography module error codes */
#define ESP_ERR_MEMPROT_BASE        0xd000  /*!< Starting number of Memory Protection API error codes */

typedef enum {
    LOW_CODE_FEATURE_ID_UNHANDLED = 0,
    LOW_CODE_FEATURE_ID_POWER = 1001,
    LOW_CODE_FEATURE_ID_BRIGHTNESS = 1002,
    LOW_CODE_FEATURE_ID_COLOR_TEMPERATURE = 1003,
    LOW_CODE_FEATURE_ID_TEMPERATURE = 4004,
    LOW_CODE_FEATURE_ID_COOLING_SETPOINT = 4005,
    LOW_CODE_FEATURE_ID_HEATING_SETPOINT = 4006,
    LOW_CODE_FEATURE_ID_MAX = UINT32_MAX
} low_code_feature_id_t;

typedef enum {
    LOW_CODE_VALUE_TYPE_INVALID = 0,
    LOW_CODE_VALUE_TYPE_BOOLEAN,
    LOW_CODE_VALUE_TYPE_INTEGER,
    LOW_CODE_VALUE_TYPE_UNSIGNED_INTEGER,
    LOW_CODE_VALUE_TYPE_FLOAT,
    LOW_CODE_VALUE_TYPE_UINT8,
    LOW_CODE_VALUE_TYPE_INT16,
    LOW_CODE_VALUE_TYPE_UINT16,
    LOW_CODE_VALUE_TYPE_STRING,
    LOW_CODE_VALUE_TYPE_OCTET_STRING,
    LOW_CODE_VALUE_TYPE_ARRAY,
    LOW_CODE_VALUE_TYPE_CUSTOM,
} low_code_feature_value_type_t;

typedef struct low_code_feature_value {
    low_code_feature_value_type_t type;
    int value_len;
    uint8_t *value;
} low_code_feature_value_t;

typedef struct low_code_feature_details {
    /** Transport layer should not use these */
    uint16_t endpoint_id;
    low_code_feature_id_t feature_id;
    union {
        struct {
            uint32_t cluster_id;
            uint32_t attribute_id;
            uint32_t command_id;
        } matter;
        struct {
            char *name;
            char *type;
        } rainmaker;
    } low_level;
} low_code_feature_details_t;

typedef struct low_code_feature_data {
    low_code_feature_details_t details;
    low_code_feature_value_t value;
    void *priv_data;
} low_code_feature_data_t;

typedef enum {
    LOW_CODE_EVENT_INVALID = 0,
    LOW_CODE_EVENT_SETUP_MODE_START,
    LOW_CODE_EVENT_SETUP_MODE_END,
    LOW_CODE_EVENT_SETUP_DEVICE_CONNECTED,
    LOW_CODE_EVENT_SETUP_STARTED,
    LOW_CODE_EVENT_SETUP_SUCCESSFUL,
    LOW_CODE_EVENT_SETUP_FAILED,
    LOW_CODE_EVENT_NETWORK_CONNECTED,
    LOW_CODE_EVENT_NETWORK_DISCONNECTED,
    LOW_CODE_EVENT_OTA_STARTED,
    LOW_CODE_EVENT_OTA_STOPPED,
    LOW_CODE_EVENT_READY,
    LOW_CODE_EVENT_IDENTIFICATION_START,
    LOW_CODE_EVENT_IDENTIFICATION_STOP,
    LOW_CODE_EVENT_IDENTIFICATION_BLINK,
    LOW_CODE_EVENT_IDENTIFICATION_BREATHE,
    LOW_CODE_EVENT_IDENTIFICATION_OKAY,
    LOW_CODE_EVENT_IDENTIFICATION_CHANNEL_CHANGE,
    LOW_CODE_EVENT_IDENTIFICATION_FINISH_EFFECT,
    LOW_CODE_EVENT_IDENTIFICATION_STOP_EFFECT,
    LOW_CODE_EVENT_TEST_MODE_LOW_CODE,
    LOW_CODE_EVENT_TEST_MODE_COMMON,
    LOW_CODE_EVENT_TEST_MODE_BLE,
    LOW_CODE_EVENT_TEST_MODE_SNIFFER,
    LOW_CODE_EVENT_FACTORY_RESET,
    LOW_CODE_EVENT_FORCED_ROLLBACK,
    LOW_CODE_EVENT_BLE_ADVERTISE,
} low_code_event_type_t;

typedef struct low_code_event {
    low_code_event_type_t event_type;
    int event_data_size;
    void *event_data;
} low_code_event_t;

typedef int (*low_code_event_callback_t)(low_code_event_t *event);
typedef int (*low_code_feature_update_callback_t)(low_code_feature_data_t *data);

/* Internal */
typedef int (*low_code_get_event_from_system_t)();
typedef int (*low_code_get_feature_update_from_system_t)();

typedef struct low_code_callback_list {
    low_code_event_callback_t event_cb;
    low_code_feature_update_callback_t feature_update_cb;
    low_code_get_event_from_system_t get_event;
    low_code_get_feature_update_from_system_t get_feature_update;
} __attribute__((packed)) low_code_callback_list_t;

int low_code_register_transport_callbacks(low_code_callback_list_t *callbacks);
int low_code_feature_update_from_transport(low_code_feature_data_t *data);
int low_code_event_from_transport(low_code_event_t *event);

/* From application */
int low_code_register_callbacks(low_code_feature_update_callback_t feature_update_from_system, low_code_event_callback_t event_from_system);
int low_code_feature_update_to_system(low_code_feature_data_t *feature);
int low_code_event_to_system(low_code_event_t *event);
int low_code_get_feature_update_from_system();
int low_code_get_event_from_system();
