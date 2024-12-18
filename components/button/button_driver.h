#pragma once

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (* button_cb_t)(void *button_handle, void *usr_data);
typedef void *button_handle_t;

typedef enum {
    BUTTON_PRESS_DOWN = 0,
    BUTTON_PRESS_UP,
    BUTTON_SINGLE_CLICK,
    BUTTON_LONG_PRESS_START,
    BUTTON_LONG_PRESS_UP,
    BUTTON_EVENT_MAX,
} button_event_t;

typedef struct {
    uint16_t long_press_time;      /**< Trigger time(ms) for long press, if 0 default to BUTTON_LONG_PRESS_TIME_MS */
    uint16_t short_press_time;     /**< Trigger time(ms) for click, if 0 default to BUTTON_SHORT_PRESS_TIME_MS */
    int gpio_num;          /**< num of gpio */
    uint8_t pullup_en:1;           /**< enable gpio pullup */
    uint8_t pulldown_en:1;         /**< enable gpio pulldown */
    uint8_t active_level: 1;       /**< gpio level when press down */
} button_config_t;

/**
 * @brief Create a button
 *
 * @param config pointer of button configuration, must corresponding the button type
 *
 * @return A handle to the created button, or NULL in case of error.
 */
button_handle_t button_driver_create(const button_config_t *config);

/**
 * @brief Delete a button
 *
 * @param btn_handle A button handle to delete
 */
int button_driver_delete(button_handle_t btn_handle);

/**
 * @brief Register the button event callback function.
 *
 * @param btn_handle A button handle to register
 * @param event Button event
 * @param cb Callback function.
 * @param usr_data user data
 */
int button_driver_register_cb(button_handle_t btn_handle, button_event_t event, button_cb_t cb, void *usr_data);

/**
 * @brief Unregister all the callbacks associated with the event.
 *
 * @param btn_handle A button handle to unregister
 * @param event Button event
 */
int button_driver_unregister_cb(button_handle_t btn_handle, button_event_t event);

#ifdef __cplusplus
}
#endif
