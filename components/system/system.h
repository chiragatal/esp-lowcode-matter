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

#include <stdint.h>

typedef enum {
    INPUT,
    OUTPUT
} pin_mode_t;

typedef enum {
    LOW = 0,
    HIGH,
} pin_level_t;


void system_loop();
void system_setup();
void system_timer_update();
void system_sleep(uint32_t seconds);
void system_delay(uint32_t seconds);
void system_delay_ms(uint32_t ms);
void system_delay_us(uint32_t us);
uint32_t system_get_time();
void system_enable_software_interrupt();
void system_set_pin_mode(int gpio_num, pin_mode_t mode);
void system_digital_write(int gpio_num, pin_level_t level);
int system_digital_read(int gpio_num);
