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

#include "lp_rmt.h"

static rmt_hal_context_t lpRmtHalContext;

static void __rmt_hal_init(rmt_hal_context_t *hal)
{
    hal->regs = &RMT;
    rmt_ll_mem_power_by_pmu(hal->regs);
    rmt_ll_enable_mem_access_nonfifo(hal->regs, true);     // APB access the RMTMEM in nonfifo mode
    rmt_ll_enable_interrupt(hal->regs, UINT32_MAX, false); // disable all interrupt events
    rmt_ll_clear_interrupt_status(hal->regs, UINT32_MAX);  // clear all pending events
#if SOC_RMT_SUPPORT_TX_SYNCHRO
    rmt_ll_tx_clear_sync_group(hal->regs);
#endif // SOC_RMT_SUPPORT_TX_SYNCHRO
}

static void __rmt_hal_deinit(rmt_hal_context_t *hal)
{
    rmt_ll_enable_interrupt(hal->regs, UINT32_MAX, false); // disable all interrupt events
    rmt_ll_clear_interrupt_status(hal->regs, UINT32_MAX);  // clear all pending events
    rmt_ll_mem_force_power_off(hal->regs);                 // power off RMTMEM power domain forcefully
    hal->regs = NULL;
}

static void __rmt_hal_tx_channel_reset(rmt_hal_context_t *hal, uint32_t channel)
{
    rmt_ll_tx_reset_channels_clock_div(hal->regs, 1 << channel);
    rmt_ll_tx_reset_pointer(hal->regs, channel);
#if SOC_RMT_SUPPORT_TX_LOOP_COUNT
    rmt_ll_tx_reset_loop_count(hal->regs, channel);
#endif // SOC_RMT_SUPPORT_TX_LOOP_COUNT
    rmt_ll_enable_interrupt(hal->regs, RMT_LL_EVENT_TX_MASK(channel), false);
    rmt_ll_clear_interrupt_status(hal->regs, RMT_LL_EVENT_TX_MASK(channel));
}

// Initialize RMT module on LP core
bool lp_rmt_init_device(void) {

    lp_core_printf("Initialize RMT module on LP core...\n");

    // enable the bus clock for the RMT peripheral
    rmt_ll_enable_bus_clock(LP_RMT_DEFAULT_GROUP, true);
    rmt_ll_reset_register(LP_RMT_DEFAULT_GROUP);
    // hal layer initialize
    __rmt_hal_init(&lpRmtHalContext);
    // no division for group clock source, to achieve highest resolution
    rmt_ll_set_group_clock_src(lpRmtHalContext.regs, LP_RMT_DEFAULT_TX_CHANNEL, LP_RMT_DEFAULT_CLK_SRC, 1, 1, 0);
    rmt_ll_enable_group_clock(lpRmtHalContext.regs, true);

    return true;
}

// Deinitialize RMT module on LP core
bool lp_rmt_deinit_device(void) {

    lp_core_printf("Deinitialize RMT module on LP core...\n");

    // disable core clock
    rmt_ll_enable_group_clock(lpRmtHalContext.regs, false);
    // hal layer deinitialize
    __rmt_hal_deinit(&lpRmtHalContext);
    // disable bus clock
    rmt_ll_enable_bus_clock(LP_RMT_DEFAULT_GROUP, false);

    return true;
}

bool lp_rmt_create_default_tx_channel(lp_rmt_channel_t* defaultChannel) {
    // use default tx channel (channel 0)
    defaultChannel->channalId = LP_RMT_DEFAULT_TX_CHANNEL;
    // use default clock resolution (40MHz)
    defaultChannel->clkResolutionHz = LP_RMT_DEFAULT_CLK_RESOLUTION;
    // use default GPIO pin as output
    defaultChannel->gpioPin = BLINK_GPIO;
    // use default clock
    defaultChannel->groupClkResolutionHz = 0;
    return true;
}

static bool lp_rmt_config_gpio(lp_rmt_channel_t* channel) {

    lp_core_printf("Initialize and configure GPIO to work with RMT on LP core...\n");

    /* gpio matrix config: 1. func: gpio, 2. I/O/D */
    gpio_ll_iomux_func_sel(GPIO_PIN_MUX_REG[channel->gpioPin], PIN_FUNC_GPIO);
    gpio_ll_output_enable(&GPIO, channel->gpioPin);
    gpio_ll_pullup_en(&GPIO, channel->gpioPin);
    gpio_ll_input_disable(&GPIO, channel->gpioPin);
    gpio_ll_pulldown_dis(&GPIO, channel->gpioPin);
    gpio_ll_od_disable(&GPIO, channel->gpioPin);
    gpio_ll_sleep_sel_dis(&GPIO, channel->gpioPin);

    /* gpio matrix config: 3. peripheral func: ledc channel_x */
    REG_WRITE(GPIO_FUNC0_OUT_SEL_CFG_REG + (channel->gpioPin * 4), RMT_SIG_OUT0_IDX + channel->channalId);

    return true;
}

bool lp_rmt_config_tx_channel(lp_rmt_channel_t* channel)
{
    lp_core_printf("Configure RMT channel on LP core...\n");

    // reset channel, make sure the TX engine is not working, and events are cleared
    __rmt_hal_tx_channel_reset(&lpRmtHalContext, channel->channalId);
    // set channel clock resolution
    // channel->realClkResolutionHz = esp_clk_tree_src_get_freq_hz((soc_module_clk_t)RMT_CLK_SRC_RC_FAST, ESP_CLK_TREE_SRC_FREQ_PRECISION_CACHED, &channel->realClkResolutionHz);
    // use RTC_FAST_CLK_FREQ_APPROX if HP in Light/Deep sleep status
    // use XTAL_CLK_FREQ for more accurate control
    if (channel->groupClkResolutionHz == 0) {
        channel->groupClkResolutionHz = LP_RMT_DEFAULT_CLK_RESOLUTION;
    }
    channel->readlDiv = channel->groupClkResolutionHz / channel->clkResolutionHz;
    // Limit Clock Dividor between 1 and 256
    if (channel->readlDiv >= 256) {
        channel->readlDiv = 256;
    } else if (channel->readlDiv <= 1) {
        channel->readlDiv = 1;
    }
    // calculate realClkResolutionHz
    channel->realClkResolutionHz = channel->groupClkResolutionHz / channel->readlDiv;

    // lp_core_printf("Channel Divider: %lu\n", realDiv);
    // set clock divider
    rmt_ll_tx_set_channel_clock_div(lpRmtHalContext.regs, channel->channalId, channel->readlDiv);
    rmt_ll_tx_set_mem_blocks(lpRmtHalContext.regs, channel->channalId, 1);
    // set limit threshold, after transmit ping_pong_symbols size, an interrupt event would be generated
    rmt_ll_tx_set_limit(lpRmtHalContext.regs, channel->channalId, 48);
    // disable carrier modulation by default, can re-enable by `rmt_apply_carrier()`
    rmt_ll_tx_enable_carrier_modulation(lpRmtHalContext.regs, channel->channalId, false);
    // idle level is determined by register value
    rmt_ll_tx_fix_idle_level(lpRmtHalContext.regs, channel->channalId, 0, true);
    // always enable tx wrap, both DMA mode and ping-pong mode rely this feature
    rmt_ll_tx_enable_wrap(lpRmtHalContext.regs, channel->channalId, false);
    // disable interrupt
    rmt_ll_enable_interrupt(lpRmtHalContext.regs, 0b11111111111111, false);

    lp_rmt_config_gpio(channel);

    return true;
}

bool lp_rmt_send_bytes(void* dataBuffer, size_t numBits, lp_rmt_channel_t* channel) {

    // lp_core_printf("Begin to send Bytes through RMT channel %lu\n", channel->channalId);
    rmt_ll_tx_stop(lpRmtHalContext.regs, channel->channalId);
    // ulp_lp_core_delay_us(50);
    if (numBits >= 48) {
        lp_core_printf("Unsupported RMT transmission size: %lu. %d Maximum\n", numBits, 47);
        return false;
    }

    // write contents bit pattern to RMT memory
    for (size_t i = 0; i < numBits; i++) {
        RMTMEM.channels[channel->channalId].symbols[i] = (((uint8_t*)dataBuffer)[i>>3] & (1 << (channel->msbFirst ? (7 - i % 8) : (i % 8)))) ? channel->bit1 : channel->bit0;
    }
    RMTMEM.channels[channel->channalId].symbols[numBits] = (rmt_symbol_word_t)((uint32_t)(0));

    rmt_ll_tx_start(lpRmtHalContext.regs, channel->channalId);

    return true;
}
