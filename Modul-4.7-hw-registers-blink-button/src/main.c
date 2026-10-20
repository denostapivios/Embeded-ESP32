#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_timer.h"

#include "driver/gpio.h"
#include "soc/gpio_struct.h"

// ============================================================
// GPIO configuration
// ============================================================

#define LED_GPIO            GPIO_NUM_4
#define BUTTON_GPIO         GPIO_NUM_5

// ============================================================
// Blink configuration
// ============================================================

#define SLOW_INTERVAL_MS    500
#define FAST_INTERVAL_MS    100

// Debounce time
#define DEBOUNCE_TIME_MS    30

// Timer callback interval
#define TIMER_INTERVAL_US   1000

// ============================================================
// Application state
// ============================================================

typedef enum
{
    BLINK_MODE_SLOW = 0,
    BLINK_MODE_FAST
} blink_mode_t;

static volatile blink_mode_t current_mode = BLINK_MODE_SLOW;

static volatile bool led_state = false;

static int64_t last_blink_time_us = 0;

// Button debounce state
static bool button_raw_state = true;
static bool button_stable_state = true;

static int64_t button_change_time_us = 0;

// Timer handle
static esp_timer_handle_t app_timer;

// ============================================================
// GPIO initialization
// ============================================================

static void gpio_init_custom(void)
{
    // LED output configuration
    gpio_config_t led_config = {
        .pin_bit_mask = (1ULL << LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    ESP_ERROR_CHECK(gpio_config(&led_config));

    // Button input configuration
    gpio_config_t button_config = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    ESP_ERROR_CHECK(gpio_config(&button_config));

    // Ensure LED starts OFF
    GPIO.out_w1tc = (1U << LED_GPIO);
}

// ============================================================
// Direct GPIO register access
// ============================================================

static inline void led_on(void)
{
    GPIO.out_w1ts = (1U << LED_GPIO);
    led_state = true;
}

static inline void led_off(void)
{
    GPIO.out_w1tc = (1U << LED_GPIO);
    led_state = false;
}

static inline bool button_is_pressed(void)
{
    // GPIO.in contains input levels for GPIO0..GPIO31.
    // Button uses pull-up, so LOW means pressed.
    return (GPIO.in & (1U << BUTTON_GPIO)) == 0;
}

// ============================================================
// Blink mode management
// ============================================================

static int64_t get_blink_interval_us(void)
{
    if (current_mode == BLINK_MODE_FAST)
    {
        return (int64_t)FAST_INTERVAL_MS * 1000;
    }

    return (int64_t)SLOW_INTERVAL_MS * 1000;
}

static void toggle_blink_mode(void)
{
    if (current_mode == BLINK_MODE_SLOW)
    {
        current_mode = BLINK_MODE_FAST;
    }
    else
    {
        current_mode = BLINK_MODE_SLOW;
    }

    // Restart the blink interval after changing the mode.
    last_blink_time_us = esp_timer_get_time();
}

// ============================================================
// Button handling with debounce
// ============================================================

static void button_update(int64_t now_us)
{
    bool raw_state = !button_is_pressed();

    // Detect raw input changes.
    if (raw_state != button_raw_state)
    {
        button_raw_state = raw_state;
        button_change_time_us = now_us;
    }

    // Accept the new state after the debounce interval.
    if ((now_us - button_change_time_us) >=
        ((int64_t)DEBOUNCE_TIME_MS * 1000))
    {
        if (button_stable_state != button_raw_state)
        {
            button_stable_state = button_raw_state;

            // Button is active LOW.
            // The logical state is inverted above:
            // false means pressed.
            if (!button_stable_state)
            {
                toggle_blink_mode();
            }
        }
    }
}

// ============================================================
// Blink handling
// ============================================================

static void blink_update(int64_t now_us)
{
    int64_t interval_us = get_blink_interval_us();

    if ((now_us - last_blink_time_us) >= interval_us)
    {
        last_blink_time_us = now_us;

        if (led_state)
        {
            led_off();
        }
        else
        {
            led_on();
        }
    }
}

// ============================================================
// Timer callback
// ============================================================

static void app_timer_callback(void *arg)
{
    (void)arg;

    int64_t now_us = esp_timer_get_time();

    button_update(now_us);
    blink_update(now_us);
}

// ============================================================
// Application entry point
// ============================================================

void app_main(void)
{
    gpio_init_custom();

    // Initialize button state.
    button_raw_state = !button_is_pressed();
    button_stable_state = button_raw_state;

    int64_t now_us = esp_timer_get_time();

    button_change_time_us = now_us;
    last_blink_time_us = now_us;

    // Start in slow mode.
    current_mode = BLINK_MODE_SLOW;

    const esp_timer_create_args_t timer_args = {
        .callback = app_timer_callback,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "blink_button_timer"
    };

    ESP_ERROR_CHECK(
        esp_timer_create(&timer_args, &app_timer)
    );

    ESP_ERROR_CHECK(
        esp_timer_start_periodic(
            app_timer,
            TIMER_INTERVAL_US
        )
    );
}