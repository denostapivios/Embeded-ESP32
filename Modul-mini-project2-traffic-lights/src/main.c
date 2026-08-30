#include <stdint.h>
#include <stdbool.h>

#include "driver/gpio.h"
#include "esp_timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define RED_LED_PIN GPIO_NUM_4
#define YELLOW_LED_PIN GPIO_NUM_5
#define GREEN_LED_PIN GPIO_NUM_18

#define GREEN_TIME_MS 5000
#define GREEN_BLINK_TIME_MS 3000
#define YELLOW_TIME_MS 3000
#define RED_TIME_MS 5000
#define RED_YELLOW_TIME_MS 2000

#define BLINK_INTERVAL_MS 500

typedef enum
{
    STATE_GREEN,
    STATE_GREEN_BLINK,
    STATE_YELLOW,
    STATE_RED,
    STATE_RED_YELLOW

} TrafficLightState;

static void setLights(bool red, bool yellow, bool green)
{
    gpio_set_level(RED_LED_PIN, red);
    gpio_set_level(YELLOW_LED_PIN, yellow);
    gpio_set_level(GREEN_LED_PIN, green);
}

void app_main(void)
{
    gpio_config_t ledConfig = {0};

    ledConfig.pin_bit_mask =
        (1ULL << RED_LED_PIN) |
        (1ULL << YELLOW_LED_PIN) |
        (1ULL << GREEN_LED_PIN);

    ledConfig.mode = GPIO_MODE_OUTPUT;
    ledConfig.pull_up_en = GPIO_PULLUP_DISABLE;
    ledConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    ledConfig.intr_type = GPIO_INTR_DISABLE;

    gpio_config(&ledConfig);

    TrafficLightState currentState = STATE_GREEN;

    int64_t stateStartTime = esp_timer_get_time() / 1000;
    int64_t lastBlinkTime = stateStartTime;

    bool greenLedState = true;

    setLights(false, false, true);

    while (1)
    {
        int64_t currentTime = esp_timer_get_time() / 1000;

        switch (currentState)
        {
            case STATE_GREEN:

                if (currentTime - stateStartTime >= GREEN_TIME_MS)
                {
                    currentState = STATE_GREEN_BLINK;
                    stateStartTime = currentTime;
                    lastBlinkTime = currentTime;

                    greenLedState = true;

                    setLights(false, false, true);
                }

                break;

            case STATE_GREEN_BLINK:

                if (currentTime - stateStartTime >= GREEN_BLINK_TIME_MS)
                {
                    currentState = STATE_YELLOW;
                    stateStartTime = currentTime;

                    setLights(false, true, false);

                    break;
                }

                if (currentTime - lastBlinkTime >= BLINK_INTERVAL_MS)
                {
                    lastBlinkTime = currentTime;

                    greenLedState = !greenLedState;

                    gpio_set_level(GREEN_LED_PIN, greenLedState);
                }

                break;

            case STATE_YELLOW:

                if (currentTime - stateStartTime >= YELLOW_TIME_MS)
                {
                    currentState = STATE_RED;
                    stateStartTime = currentTime;

                    setLights(true, false, false);
                }

                break;

            case STATE_RED:

                if (currentTime - stateStartTime >= RED_TIME_MS)
                {
                    currentState = STATE_RED_YELLOW;
                    stateStartTime = currentTime;

                    setLights(true, true, false);
                }

                break;

            case STATE_RED_YELLOW:

                if (currentTime - stateStartTime >= RED_YELLOW_TIME_MS)
                {
                    currentState = STATE_GREEN;
                    stateStartTime = currentTime;

                    setLights(false, false, true);
                }

                break;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}