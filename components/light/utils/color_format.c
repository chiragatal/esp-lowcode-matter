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

#include "color_format.h"

void hsv_to_rgb(HS_color_t HS, uint8_t brightness, RGB_color_t *RGB)
{
    uint16_t hue = HS.hue % 360;
    uint16_t hi = hue / 60;
    uint16_t remainder = (hue - (hi * 60)) * 255 / 60;
    uint16_t sBrightness = brightness * 255 / 100;
    uint16_t sSaturation = HS.saturation * 255 / 100;
    uint16_t P = sBrightness * (255 - sSaturation) / 255;
    uint16_t Q = (sBrightness * (255 - (sSaturation * remainder) / 255)) / 255;
    uint16_t T = (sBrightness * (255 - (sSaturation * (255 - remainder)) / 255)) / 255;

    switch (hi) {
    case 0:
        RGB->red = sBrightness;
        RGB->green = T;
        RGB->blue = P;
        break;

    case 1:
        RGB->red = Q;
        RGB->green = sBrightness;
        RGB->blue = P;
        break;

    case 2:
        RGB->red = P;
        RGB->green = sBrightness;
        RGB->blue = T;
        break;

    case 3:
        RGB->red = P;
        RGB->green = Q;
        RGB->blue = sBrightness;
        break;

    case 4:
        RGB->red = T;
        RGB->green = P;
        RGB->blue = sBrightness;
        break;

    case 5:
        RGB->red = sBrightness;
        RGB->green = P;
        RGB->blue = Q;
        break;

    default:
        break;
    }
}

void rgb2hs(RGB_color_t RGB, HS_color_t *HS) {
    int16_t min = RGB.red < RGB.green ? RGB.red : RGB.green;
    min = min < RGB.blue ? min : RGB.blue;

    int16_t max = RGB.red > RGB.green ? RGB.red : RGB.green;
    max = max > RGB.blue ? max : RGB.blue;

    int16_t delta = max - min;

    if (max != 0) {
        HS->saturation = delta * 100 / max;
    } else {
        HS->hue = 0;
        HS->saturation = 0;
        return;
    }

    int16_t hue = 0;
    int16_t r = RGB.red;
    int16_t g = RGB.green;
    int16_t b = RGB.blue;

    if (RGB.red == max) {
        // between yellow & magenta
        hue = (60 * (g - b)) / delta;
    } else if (RGB.green == max) {
        // between cyan & yellow
        hue = 120 + (60 * (b - r)) / delta;
    } else {
        hue = 240 + (60 * (r - g)) / delta;
    }

    if (hue < 0) {
        hue += 360;
    } else if (hue >= 360) {
        hue -= 360;
    }

    HS->hue = hue;


    return;

}

// A Table from color temperature to hue and saturation.
// hue = temp_table[(temp - 600) / 100].hue
// saturation= temp_table[(temp - 600) / 100].saturation
// 600<= temp <= 10000
const HS_color_t temp_table[] = {
    {4, 100},  {8, 100},  {11, 100}, {14, 100}, {16, 100}, {18, 100}, {20, 100}, {22, 100}, {24, 100}, {25, 100},
    {27, 100}, {28, 100}, {30, 100}, {31, 100}, {31, 95},  {30, 89},  {30, 85},  {29, 80},  {29, 76},  {29, 73},
    {29, 69},  {28, 66},  {28, 63},  {28, 60},  {28, 57},  {28, 54},  {28, 52},  {27, 49},  {27, 47},  {27, 45},
    {27, 43},  {27, 41},  {27, 39},  {27, 37},  {27, 35},  {27, 33},  {27, 31},  {27, 30},  {27, 28},  {27, 26},
    {27, 25},  {27, 23},  {27, 22},  {27, 21},  {27, 19},  {27, 18},  {27, 17},  {27, 15},  {28, 14},  {28, 13},
    {28, 12},  {29, 10},  {29, 9},   {30, 8},   {31, 7},   {32, 6},   {34, 5},   {36, 4},   {41, 3},   {49, 2},
    {0, 0},    {294, 2},  {265, 3},  {251, 4},  {242, 5},  {237, 6},  {233, 7},  {231, 8},  {229, 9},  {228, 10},
    {227, 11}, {226, 11}, {226, 12}, {225, 13}, {225, 13}, {224, 14}, {224, 14}, {224, 15}, {224, 15}, {223, 16},
    {223, 16}, {223, 17}, {223, 17}, {223, 17}, {222, 18}, {222, 18}, {222, 19}, {222, 19}, {222, 19}, {222, 19},
    {222, 20}, {222, 20}, {222, 20}, {222, 21}, {222, 21}};

void temp_to_hs(uint32_t temperature, HS_color_t *HS)
{
    if (temperature < 600) {
        HS->hue = 0;
        HS->saturation = 100;
        return;
    }
    if (temperature > 10000) {
        HS->hue = 222;
        HS->saturation = 21 + (temperature - 10000) * 41 / 990000;
        return;
    }
    HS->hue = temp_table[(temperature - 600) / 100].hue;
    HS->saturation = temp_table[(temperature - 600) / 100].saturation;
}

// remap temperature to cold/warm (larger temperature range)
void temp_to_cw(uint32_t temperature, CW_white_t *CW)
{
    if (temperature < 300) {
        CW->cold = 0;
        CW->warm = 100;
        return;
    }
    if (temperature > 10300) {
        CW->cold = 100;
        CW->warm = 0;
        return;
    }
    CW->cold = temperature/100-3;
    CW->warm = 100-CW->cold;
}

void cw_to_temp(CW_white_t CW, uint32_t* temperature) {
    // scale CW to 0~100
    CW.cold = 100 * CW.cold / (CW.cold + CW.warm);
    CW.warm = 100 - CW.cold;
    *temperature = (CW.cold + 3) * 100;
}

void cw_to_hsv(CW_white_t CW, HS_color_t* HS) {
    uint32_t temperature = 0;
    cw_to_temp(CW, &temperature);
    temp_to_hs(temperature, HS);
}
