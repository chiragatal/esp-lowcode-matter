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

#include "app_driver.h"

static const char *TAG = "app_driver";

int app_driver_init()
{
    printf("%s: Initializing thermostat driver\n", TAG);
    /* Add thermostat initialization code here */

    return 0;
}

int app_driver_set_temperature(int16_t temperature)
{
    printf("%s: Setting temperature: %d\n", TAG, temperature);
    /* Add code to update the temperature reading */

    return 0;
}

int app_driver_set_cooling_setpoint(int16_t cooling_setpoint)
{
    printf("%s: Setting cooling setpoint: %d\n", TAG, cooling_setpoint);
    /* Add code to control the cooling setpoint */

    return 0;
}

int app_driver_set_heating_setpoint(int16_t heating_setpoint)
{
    printf("%s: Setting heating setpoint: %d\n", TAG, heating_setpoint);
    /* Add code to control the heating setpoint */

    return 0;
}
