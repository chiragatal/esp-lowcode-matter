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

#include <stdbool.h>
#include <stdint.h>
#include "esp_mac.h"
// #include "esp_clk_tree.h"
#include "hal/rmt_ll.h"
#include "hal/rmt_hal.h"
#include "hal/gpio_hal.h"
#include "hal/gpio_ll.h"
#include "hal/clk_tree_ll.h"
#include "hal/clk_tree_hal.h"
#include "soc/gpio_num.h"
#include "soc/gpio_periph.h"
#include "soc/rmt_periph.h"
#include "soc/rtc.h"
#include "ulp_lp_core_print.h"
#include "ulp_lp_core_utils.h"
#include "sdkconfig.h"


#define LP_RMT_DEFAULT_GROUP            (0)
#define LP_RMT_DEFAULT_TX_CHANNEL       (0)
#define LP_RMT_DEFAULT_CLK_RESOLUTION   XTAL_CLK_FREQ
// #define LP_RMT_DEFAULT_CLK_RESOLUTION   RTC_FAST_CLK_FREQ_APPROX

#define LP_RMT_DEFAULT_CLK_SRC          RMT_CLK_SRC_XTAL
// #define LP_RMT_DEFAULT_CLK_SRC          RMT_CLK_SRC_RC_FAST
// For ESP32C6 Only
#define CONFIG_BLINK_GPIO               (8)
#define BLINK_GPIO                      CONFIG_BLINK_GPIO

#ifdef __cplusplus
extern "C" {
#endif

// hard-coded RMTMEM Structure
typedef struct {
    struct {
        rmt_symbol_word_t symbols[48];
    } channels[4];
} rmt_block_mem_t;

extern rmt_block_mem_t RMTMEM;

struct lp_rmt_channel_t {
    size_t clkResolutionHz;
    unsigned int realClkResolutionHz;
    size_t groupClkResolutionHz;
    size_t channalId;
    size_t gpioPin;
    rmt_symbol_word_t bit0;
    rmt_symbol_word_t bit1;
    size_t readlDiv;
    bool msbFirst;
};

typedef struct lp_rmt_channel_t     lp_rmt_channel_t;
typedef rmt_dev_t                   lp_rmt_dev_t;
bool lp_rmt_send_bytes(void* dataBuffer, size_t numBits, lp_rmt_channel_t* channel);
bool lp_rmt_config_tx_channel(lp_rmt_channel_t* channel);
bool lp_rmt_create_default_tx_channel(lp_rmt_channel_t* defaultChannel);
bool lp_rmt_init_device(void);
bool lp_rmt_deinit_device(void);

#ifdef __cplusplus
}
#endif
