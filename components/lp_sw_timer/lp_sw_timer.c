#include "stdlib.h"
#include "stdbool.h"
#include "stdint.h"
#include "limits.h"

#include "riscv/rv_utils.h"
#include "ulp_lp_core_print.h"
#include "lp_sw_timer.h"

#ifdef CONFIG_LP_SW_TIMER_MAX_ITEMS
#define LP_SW_TIMER_MAX_ITEMS CONFIG_LP_SW_TIMER_MAX_ITEMS
#else
#define  LP_SW_TIMER_MAX_ITEMS 10
#endif /* CONFIG_LP_SW_TIMER_MAX_ITEMS */

#define LP_CORE_FREQ_IN_KHZ 16000

typedef struct {
    bool active; /* false means a suspended/uninitialized timer */
    bool valid; /* whether this timer is valid or not */
    bool periodic; /* auto-reload the timer if it is periodic */
    uint32_t last_tick; /* last tick */
    int64_t remain_ticks; /* remain ticks to call the timer callback */
    int timeout_ms; /* timeout period */
    lp_sw_timer_cb_t handler; /* callback */
    void *arg;
} lp_sw_timer_t;

/* zerocode timer */
static lp_sw_timer_t g_timers[LP_SW_TIMER_MAX_ITEMS];

/**
 * @brief create zerocode timer
 *
 * @return handle if on available entry, return NULL
*/
lp_sw_timer_handle_t lp_sw_timer_create(lp_sw_timer_config_t *config)
{
    if (config->handler == NULL) {
        lp_core_printf("%s: Invalid handler\n", __func__);
        return NULL;
    }

    if (config->periodic == true && config->timeout_ms == 0) {
        lp_core_printf("%s: Invalid periodic timer with timeout_ms=0\n", __func__);
        return NULL;
    }

    lp_sw_timer_t *timer = NULL;

    for (int i=0; i<LP_SW_TIMER_MAX_ITEMS; i++) {
        if (!g_timers[i].valid) {
            timer = &(g_timers[i]);
            break;
        }
    }

    if (timer) {
        timer->active = false; /* initial is inactive */
        timer->handler = config->handler;
        timer->arg = config->arg;
        timer->valid = true;
        timer->timeout_ms = config->timeout_ms;
        timer->periodic = config->periodic;
    }
    else {
        lp_core_printf("Lack of memory for lp_sw_timer\n");
    }

    return (lp_sw_timer_handle_t)timer;
}

/**
 * @brief delete a timer
*/
int lp_sw_timer_delete(lp_sw_timer_handle_t timer_handle)
{
    if (timer_handle == NULL){
        return -1;
    }

    lp_sw_timer_t *timer = (lp_sw_timer_t *)timer_handle;
    timer->active = false;
    timer->handler = NULL;
    timer->arg = NULL;
    timer->remain_ticks = -1;
    timer->valid = false;
    return 0;
}

/**
 * @brief start timer
 *
*/
int lp_sw_timer_start(lp_sw_timer_handle_t timer_handle)
{
    lp_sw_timer_t *timer = (lp_sw_timer_t *)timer_handle;
    if (timer == NULL || timer->valid == false){
        lp_core_printf("%s: Invalid timer\n", __func__);
        return -1;
    }

    /* calculate remain_ticks */
    timer->remain_ticks = (int64_t)(timer->timeout_ms) * LP_CORE_FREQ_IN_KHZ;

    if (timer->remain_ticks == 0) {
        /* if remain_ticks=0, call handler immediately and stop timer */
        timer->handler(timer_handle, timer->arg);
        lp_sw_timer_stop(timer);
    }
    else {
        /* update timer's last tick */
        timer->last_tick = RV_READ_CSR(mcycle);
        timer->active = 1;
    }

    return 0;
}

/**
 * @brief stop a timer
 *
 * update the last_tick & remain_ticks immediately
*/
int lp_sw_timer_stop(lp_sw_timer_handle_t timer_handle)
{
    lp_sw_timer_t *timer = (lp_sw_timer_t *)timer_handle;

    if (timer == NULL || timer->valid == false){
        lp_core_printf("%s: Invalid timer\n", __func__);
        return -1;
    }

    if(timer->active == false) return 0;
    timer->active = false;
    return 0;
}


/**
 * @brief call from main function to update the timer list
*/
void lp_sw_timer_run(void)
{
    for (int i=0; i<LP_SW_TIMER_MAX_ITEMS; i++) {
        if (g_timers[i].valid && g_timers[i].active) {
            /* calculate escaped ticks & update last tick */
            uint32_t tick = RV_READ_CSR(mcycle); /* bugfix: time_gap overflow when timer started inside another timer cb */
            uint32_t time_gap = tick - g_timers[i].last_tick;
            g_timers[i].last_tick = tick; /* update last_tick */
            g_timers[i].remain_ticks -= (int64_t)time_gap; /* update remain_ticks */

            if (g_timers[i].remain_ticks <= 0) {
                /* handler may delete/stop timer. update timer status before executing handler */
                if (g_timers[i].periodic) {
                    /* periodic, reload timer. if not, stop timer */
                    lp_sw_timer_start(&(g_timers[i]));
                } else {
                    lp_sw_timer_stop(&(g_timers[i]));
                }

                /* call handler */
                g_timers[i].handler(&(g_timers[i]), g_timers[i].arg);
            }
        }
    }
}
