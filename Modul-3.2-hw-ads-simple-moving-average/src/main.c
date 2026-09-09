#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"

#include "esp_log.h"

#define LED_GPIO        GPIO_NUM_5

#define LDR_ADC_UNIT    ADC_UNIT_1
#define LDR_ADC_CHANNEL ADC_CHANNEL_3

#define ADC_ATTENUATION ADC_ATTEN_DB_12
#define ADC_BIT_WIDTH   ADC_BITWIDTH_DEFAULT

#define SMA_SIZE 5

static int adc_buffer[SMA_SIZE];

static int buffer_index = 0;
static int buffer_count = 0;
static int buffer_sum = 0;

#define LIGHT_ON_THRESHOLD   1700
#define LIGHT_OFF_THRESHOLD  1900

typedef enum
{
    LED_OFF,
    LED_ON

} LedState;

static LedState led_state = LED_OFF;

// TAG
static const char *TAG = "LDR";


// SMA
static int sma_add(int new_value)
{
    // Якщо buffer вже заповнений,
    // прибираємо найстаріше значення
    if (buffer_count == SMA_SIZE)
    {
        buffer_sum -= adc_buffer[buffer_index];
    }
    else
    {
        buffer_count++;
    }

    // Записуємо нове значення
    adc_buffer[buffer_index] = new_value;

    // Додаємо нове значення до суми
    buffer_sum += new_value;

    // Переміщуємо index по колу
    buffer_index++;

    if (buffer_index >= SMA_SIZE)
    {
        buffer_index = 0;
    }

    // Повертаємо середнє значення
    return buffer_sum / buffer_count;
}


// LED state machine + hysteresis

static void update_led(int filtered_value)
{
    switch (led_state)
    {
        case LED_OFF:

            if (filtered_value < LIGHT_ON_THRESHOLD)
            {
                led_state = LED_ON;

                gpio_set_level(LED_GPIO, 1);

                ESP_LOGI(
                    TAG,
                    "DARK -> LED ON | SMA: %d",
                    filtered_value
                );
            }

            break;


        case LED_ON:

            if (filtered_value > LIGHT_OFF_THRESHOLD)
            {
                led_state = LED_OFF;

                gpio_set_level(LED_GPIO, 0);

                ESP_LOGI(
                    TAG,
                    "LIGHT -> LED OFF | SMA: %d",
                    filtered_value
                );
            }

            break;
    }
}

void app_main(void)
{
    gpio_config_t led_config = {
        .pin_bit_mask = (1ULL << LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&led_config);

    // Початковий стан LED
    gpio_set_level(LED_GPIO, 0);
    

    // ADC configuration
    adc_oneshot_unit_handle_t adc_handle;

    adc_oneshot_unit_init_cfg_t adc_init_config = {
        .unit_id = LDR_ADC_UNIT
    };

    adc_oneshot_new_unit(
        &adc_init_config,
        &adc_handle
    );


    adc_oneshot_chan_cfg_t adc_channel_config = {
        .bitwidth = ADC_BIT_WIDTH,
        .atten = ADC_ATTENUATION
    };

    adc_oneshot_config_channel(
        adc_handle,
        LDR_ADC_CHANNEL,
        &adc_channel_config
    );


    ESP_LOGI(TAG, "LDR system started");

    while (1)
    {
        int raw_value = 0;

        // ADC measurement
        adc_oneshot_read(
            adc_handle,
            LDR_ADC_CHANNEL,
            &raw_value
        );

        // SMA filtering
        int filtered_value = sma_add(raw_value);

        // 3. Log values
        ESP_LOGI(
            TAG,
            "RAW: %d | SMA: %d | LED: %s",
            raw_value,
            filtered_value,
            led_state == LED_ON ? "ON" : "OFF"
        );

        // Update LED state
        update_led(filtered_value);

        // 5. Delay
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}