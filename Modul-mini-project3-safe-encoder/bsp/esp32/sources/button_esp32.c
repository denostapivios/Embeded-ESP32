#include "button_esp32.h"
#include "button_interface.h"

#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"


// ============================================================
// Button configuration
// ============================================================

#define ENC_SW 17

#define BUTTON_DEBOUNCE_MS 50


// ============================================================
// Button state
// ============================================================

static int last_button_state = 1;
static int button_state = 1;

static TickType_t last_button_time = 0;


// ============================================================
// Initialization
// ============================================================

void button_init(void)
{
    gpio_config_t config = {
        .pin_bit_mask = (1ULL << ENC_SW),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK(
        gpio_config(&config)
    );


    last_button_state = 1;
    button_state = 1;

    last_button_time = xTaskGetTickCount();
}


// ============================================================
// Check button
// ============================================================

bool button_is_pressed(void)
{
    int reading = gpio_get_level(ENC_SW);

    TickType_t now = xTaskGetTickCount();


    // --------------------------------------------------------
    // Detect state change
    // --------------------------------------------------------

    if (reading != last_button_state)
    {
        last_button_time = now;

        last_button_state = reading;
    }


    // --------------------------------------------------------
    // Debounce
    // --------------------------------------------------------

    if (
        (now - last_button_time) >=
        pdMS_TO_TICKS(BUTTON_DEBOUNCE_MS)
    )
    {
        if (reading != button_state)
        {
            button_state = reading;


            // Pull-up:
            // 1 = released
            // 0 = pressed

            if (button_state == 0)
            {
                return true;
            }
        }
    }


    return false;
}