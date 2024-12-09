// Copyright 2021 Espressif Systems (Shanghai) CO LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t hue;
    uint8_t saturation;
} HS_color_t;

typedef struct {
    uint8_t cold;
    uint8_t warm;
} CW_white_t;

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} RGB_color_t;

void temp_to_hs(uint32_t temperature, HS_color_t *HS);

void rgb2hs(RGB_color_t RGB, HS_color_t *HS);

void temp_to_cw(uint32_t temperature, CW_white_t *CW);

void hsv_to_rgb(HS_color_t HS, uint8_t brightness, RGB_color_t *RGB);

void cw_to_temp(CW_white_t CW, uint32_t* temperature);

void cw_to_hsv(CW_white_t CW, HS_color_t* HS);

#ifdef __cplusplus
}
#endif
