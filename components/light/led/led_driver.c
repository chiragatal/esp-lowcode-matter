#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "ulp_lp_core_print.h"
#include "ulp_lp_core_utils.h"

#include "hal/ledc_types.h"
#include "hal/ledc_ll.h"
#include "soc/ledc_struct.h"

#include "hal/clk_tree_ll.h"

#include "hal/gpio_ll.h"
#include "soc/gpio_num.h"
#include "soc/gpio_periph.h"
#include "soc/gpio_struct.h"

#include "hal/pmu_types.h"
#include "soc/pmu_icg_mapping.h"
#include "soc/pmu_struct.h"

#include "lp_sw_timer.h"

#include "led_driver.h"

/**
 * To simplify hal driver development, we auto-generated settings for ledc (pwm) controller
 *
 * XTAL_CLK timer is used for ledc timer. Frequency is 40MHz
 * pwm frequency is set to 4000Hz
 * pwm duty resolution is set to 10bit (0 ~ 1023)
 * pwm fade step is set to 10% (0%, 10%, 20%, ... 100%)
 *
 * can keep it here in led_driver
 */
#define LEDC_FREQ           4000
#define LEDC_DEFAULT_HPOINT 0
#define LEDC_DUTY_FADE_STEP 10
#define LEDC_DUTY_RES       LEDC_TIMER_11_BIT
#define LEDC_MAX_DUTY       BIT(LEDC_DUTY_RES)
#define LEDC_SPEED_MODE     LEDC_LOW_SPEED_MODE
#define LEDC_TIMER_SEL      LEDC_TIMER_1
#define LEDC_CLK_DIV        0x1d3
#define LEDC_CLK_SRC        LEDC_SLOW_CLK_RC_FAST

static uint32_t channel_mask = 0x00000000;

/* setting for ledc timer: clock source, clock divider, duty resolution */
static ledc_slow_clk_sel_t glb_clk = LEDC_CLK_SRC; /* clock source */
static uint32_t clock_divider = LEDC_CLK_DIV;
static ledc_timer_bit_t duty_resolution = LEDC_DUTY_RES;
static ledc_timer_t timer_sel = LEDC_TIMER_SEL; /* ledc timer: 0-3 */
static ledc_mode_t speed_mode = LEDC_SPEED_MODE;
static uint32_t hpoint = LEDC_DEFAULT_HPOINT;

int led_driver_set_channel(uint8_t channel, uint8_t val)
{

    /* LEDC_CHn_HPOINT: config hpoint */
    ledc_ll_set_hpoint(&LEDC, speed_mode, channel, hpoint);
    /* LEDC_CHn_DUTY: config duty */
    ledc_ll_set_duty_int_part(&LEDC, speed_mode, channel, (LEDC_MAX_DUTY * val / 100));
    ledc_ll_set_duty_start(&LEDC, speed_mode, channel, true);
    /* LEDC_PARA_UP_CHn: enable the configuration above */
    if (speed_mode == LEDC_LOW_SPEED_MODE) {
        ledc_ll_ls_channel_update(&LEDC, speed_mode, channel);
    }
    return 0;
}

int led_driver_regist_channel(uint8_t channel, gpio_num_t gpio)
{

    if (channel <= LED_CHANNEL_NC || channel >= LED_CHANNEL_MAX) return -1;
    if (gpio <= GPIO_NUM_NC || gpio >= GPIO_NUM_MAX) {
        printf("%s: Invalid gpio num: %d\n", __func__, gpio);
        return -1;
    }

    /* gpio matrix config: 1. func: gpio, 2. I/O/D */
    gpio_ll_iomux_func_sel(GPIO_PIN_MUX_REG[gpio], PIN_FUNC_GPIO);
    gpio_ll_set_level(&GPIO, gpio, 0 /* output_invert */);
    gpio_ll_output_enable(&GPIO, gpio);
    gpio_ll_input_disable(&GPIO, gpio);
    gpio_ll_pulldown_dis(&GPIO, gpio);
    gpio_ll_pullup_dis(&GPIO, gpio);
    gpio_ll_od_disable(&GPIO, gpio);
    gpio_ll_sleep_sel_dis(&GPIO, gpio);

    /* gpio matrix config: 3. peripheral func: ledc channel_x */
    REG_WRITE(GPIO_FUNC0_OUT_SEL_CFG_REG + (gpio * 4), LEDC_LS_SIG_OUT0_IDX + channel); /* channel */

    /* enable signal output on channel ch */
    ledc_ll_set_sig_out_en(&LEDC, speed_mode, channel, true);
    /* bind to channel timer */
    ledc_ll_bind_channel_timer(&LEDC, speed_mode, channel, timer_sel);
    /* LEDC_PARA_UP_CHn: enable the configuration above */
    if (speed_mode == LEDC_LOW_SPEED_MODE) {
        ledc_ll_ls_channel_update(&LEDC, speed_mode, channel);
    }

    /* record enabled channel */
    channel_mask |= BIT(channel);

    return 0;
}

void led_driver_deinit(void)
{
    /* disable ledc timer */
    // TODO: disable ledc timer

    /* diable channel output */
    for (int ch=LED_CHANNEL_NC+1; ch<LED_CHANNEL_MAX; ch++) {
        if (channel_mask | BIT(ch)) {
            ledc_ll_set_sig_out_en(&LEDC, speed_mode, ch, false);
            ledc_ll_set_duty_start(&LEDC, speed_mode, ch, false);
            if (speed_mode == LEDC_LOW_SPEED_MODE) {
                ledc_ll_ls_channel_update(&LEDC, speed_mode, ch);
            }
            channel_mask &= ~(BIT(ch));
        }
    }
}

int led_driver_init(void)
{
    // printf("%s() called\n", __func__);

    /* init ledc bus clock */
    ledc_ll_enable_bus_clock(true);
    ledc_ll_enable_reset_reg(false);

    /* keep ledc clock alive during light sleep */
    uint32_t val = PMU.hp_sys[PMU_MODE_HP_SLEEP].icg_func;
    PMU.hp_sys[PMU_MODE_HP_SLEEP].icg_func = (val | BIT(PMU_ICG_FUNC_ENA_LEDC) | BIT(PMU_ICG_FUNC_ENA_IOMUX));

    /* enable ledc clock */
    ledc_ll_enable_clock(&LEDC, true);
    ledc_ll_set_slow_clk_sel(&LEDC, glb_clk);
    printf("glb_clk=%d\n", glb_clk);

    /* set clock divider & duty resolution */
    ledc_ll_set_clock_divider(&LEDC, speed_mode, timer_sel, clock_divider);
    ledc_ll_set_duty_resolution(&LEDC, speed_mode, timer_sel, duty_resolution);
    if (speed_mode == LEDC_LOW_SPEED_MODE) {
        ledc_ll_ls_timer_update(&LEDC, speed_mode, timer_sel);
    }

    /* resume timer and reset, very important */
    ledc_ll_timer_resume(&LEDC, speed_mode, timer_sel);
    ledc_ll_timer_rst(&LEDC, speed_mode, timer_sel);

    return 0;
}

int led_driver_update_channels(void) {
    return 0;
}
