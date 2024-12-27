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

#include <ulp_lp_core_utils.h>
#include <hal/gpio_ll.h>
#include <hal/gpio_types.h>
#include <soc/gpio_struct.h>
#include <soc/gpio_reg.h>
#include <soc/interrupt_matrix_struct.h>
#include <soc/gpio_num.h>
#include <lp_sw_timer.h>
#include <esp_amp_platform.h>
#include <low_code_transport.h>

#include <system.h>

void system_loop()
{
    system_timer_update();
}

void system_setup()
{
    low_code_transport_register_callbacks();
}

void system_timer_update()
{
    lp_sw_timer_run();
}

void system_sleep(uint32_t seconds)
{
    esp_amp_platform_delay_ms(seconds * 1000);
}

void system_delay(uint32_t seconds)
{
    esp_amp_platform_delay_ms(seconds * 1000);
}

void system_delay_ms(uint32_t ms)
{
    esp_amp_platform_delay_ms(ms);
}

void system_delay_us(uint32_t us)
{
    esp_amp_platform_delay_us(us);
}

uint32_t system_get_time()
{
    return esp_amp_platform_get_time_ms();
}

void system_enable_software_interrupt()
{
    ulp_lp_core_sw_intr_enable(true);
}

void system_set_pin_mode(int gpio_num, pin_mode_t mode)
{
    if (mode == OUTPUT) {
        gpio_ll_output_enable(&GPIO, gpio_num);
        gpio_ll_pullup_dis(&GPIO, gpio_num);
        gpio_ll_pulldown_dis(&GPIO, gpio_num);
        gpio_ll_input_disable(&GPIO, gpio_num);
        gpio_ll_od_disable(&GPIO, gpio_num);
    } else {
        gpio_ll_input_enable(&GPIO, gpio_num);
        gpio_ll_output_disable(&GPIO, gpio_num);
    }
    gpio_ll_intr_disable(&GPIO, gpio_num);
    gpio_ll_func_sel(&GPIO, gpio_num, PIN_FUNC_GPIO);
}

void system_digital_write(int gpio_num, pin_level_t level)
{
    gpio_ll_set_level(&GPIO, gpio_num, level);
}

int system_digital_read(int gpio_num)
{
    return gpio_ll_get_level(&GPIO, gpio_num);
}
