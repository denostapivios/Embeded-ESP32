#include <stdint.h>

#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define BUTTON_PIN GPIO_NUM_18
#define DEBOUNCE_DELAY_MS 50

typedef enum
{
    STATE_IDLE,
    STATE_DEBOUNCE
} ButtonState;

static const char *TAG = "BUTTON";

void app_main(void)
{
    gpio_config_t buttonConfig = {0};

    buttonConfig.pin_bit_mask = (1ULL << BUTTON_PIN);
    buttonConfig.mode = GPIO_MODE_INPUT;
    buttonConfig.pull_up_en = GPIO_PULLUP_DISABLE;
    buttonConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    buttonConfig.intr_type = GPIO_INTR_DISABLE;

    gpio_config(&buttonConfig);

    ButtonState currentState = STATE_IDLE;

    int lastButtonState = gpio_get_level(BUTTON_PIN);

    int64_t lastDebounceTime = 0;

    int counter = 0;

    while (1)
    {
        int currentPinState = gpio_get_level(BUTTON_PIN);

        int64_t currentMillis = esp_timer_get_time() / 1000;

        switch (currentState)
        {
            case STATE_IDLE:

                if (lastButtonState == 0 && currentPinState == 1)
                {
                    counter++;

                    ESP_LOGI(TAG, "Count: %d", counter);

                    currentState = STATE_DEBOUNCE;
                    lastDebounceTime = currentMillis;
                }

                lastButtonState = currentPinState;

                break;

            case STATE_DEBOUNCE:

                if (currentMillis - lastDebounceTime >= DEBOUNCE_DELAY_MS)
                {
                    lastButtonState = gpio_get_level(BUTTON_PIN);

                    currentState = STATE_IDLE;
                }

                break;
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}