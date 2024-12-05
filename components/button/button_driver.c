#include "sdkconfig.h"
#include "stdio.h"
#include "stdlib.h"

#include "riscv/rv_utils.h"
#include "soc/soc.h"
#include "hal/rtc_io_ll.h"
#include "hal/misc.h"
#include "soc/lp_aon_struct.h"
#include "hal/gpio_ll.h"

#if CONFIG_BUTTON_DRIVER_USE_LP_GPIO
#include "soc/lp_io_struct.h"
#include "soc/lp_io_reg.h"
#include "soc/gpio_num.h"
#include "ulp_lp_core_gpio.h"
#endif /* CONFIG_BUTTON_DRIVER_USE_LP_GPIO */

#if CONFIG_BUTTON_DRIVER_USE_HP_GPIO
#include "hal/gpio_types.h"
#include "soc/gpio_struct.h"
#include "soc/gpio_reg.h"
#include "soc/interrupt_matrix_struct.h"
#include "esp_amp_sw_intr.h"
#include "esp_amp_sys_info.h"
#endif /* CONFIG_BUTTON_DRIVER_USE_HP_GPIO */

#include "ulp_lp_core_print.h"
#include "ulp_lp_core_utils.h"
#include "ulp_lp_core_interrupts.h"

#include "lp_sw_timer.h"

#include "button_driver.h"

#ifdef CONFIG_MAX_BUTTON_NUM
#define MAX_BUTTON_NUM CONFIG_MAX_BUTTON_NUM
#else
#define MAX_BUTTON_NUM 4
#endif /* CONFIG_MAX_BUTTON_NUM */

#ifdef CONFIG_BUTTON_DEBOUNCE_TIME
#define BUTTON_DEBOUNCE_TIME CONFIG_BUTTON_DEBOUNCE_TIME
#else
#define BUTTON_DEBOUNCE_TIME 15
#endif /* CONFIG_BUTTON_DEBOUNCE_TIME */

#ifdef CONFIG_BUTTON_LONG_PRESS_TIME_MS
#define BUTTON_LONG_PRESS_TIME_MS CONFIG_BUTTON_LONG_PRESS_TIME_MS
#else
#define BUTTON_LONG_PRESS_TIME_MS 5000
#endif /* CONFIG_BUTTON_LONG_PRESS_TIME_MS */

#ifdef CONFIG_BUTTON_SHORT_PRESS_TIME_MS
#define BUTTON_SHORT_PRESS_TIME_MS CONFIG_BUTTON_SHORT_PRESS_TIME_MS
#else
#define BUTTON_SHORT_PRESS_TIME_MS 2000
#endif /* BUTTON_SHORT_PRESS_TIME_MS */

#if CONFIG_BUTTON_DRIVER_USE_HP_GPIO
#define GPIO_GET_LEVEL(_x) gpio_ll_get_level(&GPIO, _x)
#endif

#if CONFIG_BUTTON_DRIVER_USE_LP_GPIO
#define GPIO_GET_LEVEL(_x) ulp_lp_core_gpio_get_level(_x)
#endif

/* LP core frequency in KHz */
#define LP_CORE_FREQ_IN_KHZ 16000

typedef enum {
    BUTTON_STATE_INVALID, /* not init */
    BUTTON_STATE_INIT, /* button created */
    BUTTON_STATE_NO_PRESS, /* button not being pressed */
    BUTTON_STATE_DEBOUNCE_PRESS_T, /* transient state: debounce for press down */
    BUTTON_STATE_PRESS_DOWN, /* button press down after debounce */
    BUTTON_STATE_DEBOUNCE_RELEASE_T, /* transient state: debounce for release up, emit: short_press, long_press */
    BUTTON_STATE_RELEASE_UP, /* button release up after debounce */
    BUTTON_STATE_MAX,
} button_state_t;

typedef struct {
    button_cb_t cb;
    void *usr_data;
} button_cb_info_t;

typedef struct {
    uint16_t long_press_time;
    uint16_t short_press_time;
    lp_sw_timer_handle_t timer_debounce;
    lp_sw_timer_handle_t timer_long_press;
    uint32_t tick_press_down; /* tick when button is push down */
    gpio_num_t gpio;
    uint8_t state: 4;
    uint8_t active_level: 1;
    uint8_t valid: 1;
    button_cb_info_t cb_info[BUTTON_EVENT_MAX];
} button_dev_t;

static button_dev_t g_btn_list[MAX_BUTTON_NUM];

static void iot_button_isr_handler(uint32_t pin_mask);

#if CONFIG_BUTTON_DRIVER_USE_HP_GPIO
static bool hp_gpio_intr_handler_registered = false;

static int hp_gpio_sw_intr_handler(void *arg)
{
    // TODO: Instead use atomic_cmp_exchg since maincore can reuse the memory, causing the race condition
    uint32_t *pin_mask = (uint32_t*) esp_amp_sys_info_get(0x00, NULL);
    iot_button_isr_handler(*pin_mask);
    pin_mask = 0;
    return 0;
}
#endif

static void btn_timer_cb_debounce(lp_sw_timer_handle_t timer, void *args)
{
    button_dev_t *button = (button_dev_t *)args;
    switch (button->state) {
    case BUTTON_STATE_DEBOUNCE_PRESS_T:
        if (GPIO_GET_LEVEL(button->gpio) == button->active_level) {
            button->state = BUTTON_STATE_PRESS_DOWN;
            button->tick_press_down = RV_READ_CSR(mcycle);
            lp_sw_timer_start(button->timer_long_press);
        }
        else {
            button->state = BUTTON_STATE_NO_PRESS;
        }
        break;
    case BUTTON_STATE_DEBOUNCE_RELEASE_T:
        if (GPIO_GET_LEVEL(button->gpio) != button->active_level) {
            button->state = BUTTON_STATE_RELEASE_UP;

            /* stop long press start counting */
            lp_sw_timer_stop(button->timer_long_press);

            /* press up event */
            if (button->cb_info[BUTTON_PRESS_UP].cb) {
                button->cb_info[BUTTON_PRESS_UP].cb(button, button->cb_info[BUTTON_PRESS_UP].usr_data);
            }

            uint32_t press_time = (RV_READ_CSR(mcycle) - button->tick_press_down) / LP_CORE_FREQ_IN_KHZ;
            if (press_time >= button->long_press_time) {
                /* long press event */
                if (button->cb_info[BUTTON_LONG_PRESS_UP].cb) {
                    button->cb_info[BUTTON_LONG_PRESS_UP].cb(button, button->cb_info[BUTTON_LONG_PRESS_UP].usr_data);
                }
            }
            else if (press_time < button->short_press_time) {
                /* short press event */
                if (button->cb_info[BUTTON_SINGLE_CLICK].cb) {
                    button->cb_info[BUTTON_SINGLE_CLICK].cb(button, button->cb_info[BUTTON_SINGLE_CLICK].usr_data);
                }
            }

            button->state = BUTTON_STATE_NO_PRESS;
        }
        else {
            button->state = BUTTON_STATE_PRESS_DOWN;
        }
        break;
    default:
        button->state = button->state;
        break;
    }
}

static void btn_timer_cb_long_press(lp_sw_timer_handle_t timer, void *args)
{
    button_dev_t *button = (button_dev_t *)args;
    switch(button->state) {
    case BUTTON_STATE_PRESS_DOWN:
        if (button->cb_info[BUTTON_LONG_PRESS_START].cb) {
            button->cb_info[BUTTON_LONG_PRESS_START].cb(button, button->cb_info[BUTTON_LONG_PRESS_START].usr_data);
        }
        break;
    default:
        button->state = button->state;
        break;
    }
}

button_handle_t iot_button_create(const button_config_t *config)
{

#if CONFIG_BUTTON_DRIVER_USE_HP_GPIO
    if(!hp_gpio_intr_handler_registered && esp_amp_sw_intr_add_handler(SW_INTR_ID_0, hp_gpio_sw_intr_handler, NULL) == 0) {
        hp_gpio_intr_handler_registered = true;
    }
#endif

    button_dev_t *button = NULL;

    int long_press_time = config->long_press_time;
    int short_press_time = config->short_press_time;

    if (long_press_time == 0) {
        long_press_time = BUTTON_LONG_PRESS_TIME_MS;
    }

    if (short_press_time == 0) {
        short_press_time = BUTTON_SHORT_PRESS_TIME_MS;
    }

    for (int i=0; i<MAX_BUTTON_NUM; i++) {
        if (g_btn_list[i].valid == 0) {
            button = &g_btn_list[i];
            break;
        }
    }

    if (button == NULL) {
        printf("No space to create button\r\n");
        return NULL;
    }

#if CONFIG_BUTTON_DRIVER_USE_LP_GPIO
    if (config->gpio_num < LP_IO_NUM_0 || config->gpio_num > LP_IO_NUM_7) {
        printf("Invalid gpio %d\r\n", config->gpio_num);
        return NULL;
    }
#endif /* CONFIG_BUTTON_DRIVER_USE_LP_GPIO */

#if CONFIG_BUTTON_DRIVER_USE_HP_GPIO
    if (config->gpio_num <= GPIO_NUM_NC || config->gpio_num >= GPIO_NUM_MAX) {
        printf("Invalid gpio %d\r\n", config->gpio_num);
        return NULL;
    }
#endif /* CONFIG_BUTTON_DRIVER_USE_HP_GPIO */

    /* init timer to debounce */
    lp_sw_timer_config_t timer_debounce_cfg = {
        .arg = button,
        .handler = btn_timer_cb_debounce,
        .periodic = false,
        .timeout_ms = BUTTON_DEBOUNCE_TIME,
    };
    lp_sw_timer_handle_t timer_debounce = lp_sw_timer_create(&timer_debounce_cfg);

    if (timer_debounce) {
        button->timer_debounce = timer_debounce;
    } else {
        printf("Failed to create timer\r\n");
        iot_button_delete(button);
        return NULL;
    }

    /* init timer to long press */
    lp_sw_timer_config_t timer_long_press_cfg = {
        .arg = button,
        .handler = btn_timer_cb_long_press,
        .periodic = false,
        .timeout_ms = long_press_time,
    };

    lp_sw_timer_handle_t timer_long_press = lp_sw_timer_create(&timer_long_press_cfg);

    if (timer_long_press) {
        button->timer_long_press = timer_long_press;
    } else {
        printf("Failed to create timer\r\n");
        iot_button_delete(button);
        return NULL;
    }

    /* init button */
    button->active_level = config->active_level;
    button->gpio = config->gpio_num;
    button->long_press_time = config->long_press_time;
    button->long_press_time = long_press_time;
    button->short_press_time = short_press_time;

    printf("long press time: %u, short press time: %u, debounce time: %u\r\n", 
                button->long_press_time, button->short_press_time, BUTTON_DEBOUNCE_TIME);
#if CONFIG_BUTTON_DRIVER_USE_LP_GPIO
    /* init gpio */
    ulp_lp_core_gpio_init(button->gpio);

    ulp_lp_core_gpio_pullup_disable(button->gpio);

    ulp_lp_core_gpio_pulldown_disable(button->gpio);

    if (config->pullup_en) {
        ulp_lp_core_gpio_pullup_enable(button->gpio);
    }
    if (config->pulldown_en) {
        ulp_lp_core_gpio_pulldown_enable(button->gpio);
    }

    /* enable gpio interrupt */
    ulp_lp_core_gpio_input_enable(button->gpio);
    ulp_lp_core_gpio_intr_enable(button->gpio, LP_IO_INTR_ANYEDGE);
#endif /* CONFIG_BUTTON_DRIVER_USE_LP_GPIO */

#if CONFIG_BUTTON_DRIVER_USE_HP_GPIO
    /* if gpio is selected as rtc, change it back to digital io */
    if (button->gpio >= GPIO_NUM_0 && button->gpio <= GPIO_NUM_7) {
        uint32_t sel_mask = HAL_FORCE_READ_U32_REG_FIELD(LP_AON.gpio_mux, gpio_mux_sel);
        sel_mask &= ~BIT(button->gpio);
        HAL_FORCE_MODIFY_U32_REG_FIELD(LP_AON.gpio_mux, gpio_mux_sel, sel_mask);
    }

    gpio_ll_pullup_dis(&GPIO, button->gpio);
    gpio_ll_pulldown_dis(&GPIO, button->gpio);

    if (config->pullup_en) {
        gpio_ll_pullup_en(&GPIO, button->gpio);
    }
    if (config->pulldown_en) {
        gpio_ll_pulldown_en(&GPIO, button->gpio);
    }

    gpio_ll_intr_enable_on_core(&GPIO, 0, button->gpio);
    gpio_ll_input_enable(&GPIO, button->gpio);
    gpio_ll_set_intr_type(&GPIO, button->gpio, GPIO_INTR_ANYEDGE);

#endif /* CONFIG_BUTTON_DRIVER_USE_HP_GPIO */

    button->valid = true;
    button->state = BUTTON_STATE_NO_PRESS;
    return button;
}

int iot_button_delete(button_handle_t btn_handle)
{
    button_dev_t *button = (button_dev_t *) btn_handle;

    if (button < &(g_btn_list[0]) || button > &(g_btn_list[MAX_BUTTON_NUM])) {
        return -1;
    }

    /* delete timer */
    lp_sw_timer_delete(button->timer_debounce);
    lp_sw_timer_delete(button->timer_long_press);

    for (int i=0; i<BUTTON_EVENT_MAX; i++) {
        button->cb_info[i].cb = NULL;
        button->cb_info[i].usr_data = NULL;
    }
    button->state = BUTTON_STATE_INVALID;
    button->valid = 0;

    return 0;
}

int iot_button_register_cb(button_handle_t btn_handle, button_event_t event, button_cb_t cb, void *usr_data)
{
    button_dev_t *button = (button_dev_t *) btn_handle;
    if (button < &(g_btn_list[0]) || button > &(g_btn_list[MAX_BUTTON_NUM])) {
        return -1;
    }
    button->cb_info[event].cb = cb;
    button->cb_info[event].usr_data = usr_data;
    return 0;
}

int iot_button_unregister_cb(button_handle_t btn_handle, button_event_t event)
{
    button_dev_t *button = (button_dev_t *) btn_handle;

    if (button < &(g_btn_list[0]) || button > &(g_btn_list[MAX_BUTTON_NUM])) {
        return -1;
    }

    button->cb_info[event].cb = NULL;
    button->cb_info[event].usr_data = NULL;

    return 0;
}

static void iot_button_isr_handler(uint32_t pin_mask)
{
    /* loop against all buttons */
    for (int i=0; i<MAX_BUTTON_NUM; i++) {
        if (g_btn_list[i].valid) {
            /* the GPIO under current interrupt is button */
            if (BIT(g_btn_list[i].gpio) & pin_mask) {
                button_dev_t *button = &g_btn_list[i];
                switch (button->state) {
                case BUTTON_STATE_NO_PRESS:
                    if (GPIO_GET_LEVEL(button->gpio) == button->active_level) {
                        button->state = BUTTON_STATE_DEBOUNCE_PRESS_T;
                        lp_sw_timer_start(button->timer_debounce);
                    }
                    break;
                case BUTTON_STATE_PRESS_DOWN:
                    if (GPIO_GET_LEVEL(button->gpio) != button->active_level) {
                        button->state = BUTTON_STATE_DEBOUNCE_RELEASE_T;
                        lp_sw_timer_start(button->timer_debounce);
                    }
                    break;
                default:
                    button->state = button->state;
                    break;
                }
            }
        } 
    }
}
