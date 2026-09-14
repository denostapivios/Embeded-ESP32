#include "app.h"

#include <stdio.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"

// GPIO
#define POT_GPIO        GPIO_NUM_4
#define BUTTON_GPIO     GPIO_NUM_5
#define LED_GPIO        GPIO_NUM_16
#define MOTOR_GPIO      GPIO_NUM_17


// PWM
#define PWM_FREQUENCY   5000
#define PWM_RESOLUTION  LEDC_TIMER_10_BIT

#define LED_CHANNEL     LEDC_CHANNEL_0
#define MOTOR_CHANNEL   LEDC_CHANNEL_1

#define PWM_MAX         1023

// ADC
// GPIO4 = ADC1_CHANNEL_3
#define POT_ADC_CHANNEL ADC_CHANNEL_3

// Application state
typedef enum
{
    MODE_LED,
    MODE_MOTOR

} ControlMode;


static ControlMode current_mode = MODE_LED;

static adc_oneshot_unit_handle_t adc_handle;

static const char *TAG = "APP";

// ADC initialization
static void adc_init(void)
{
    adc_oneshot_unit_init_cfg_t adc_config = {
        .unit_id = ADC_UNIT_1,
    };

    ESP_ERROR_CHECK(
        adc_oneshot_new_unit(&adc_config, &adc_handle)
    );


    adc_oneshot_chan_cfg_t channel_config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };

    ESP_ERROR_CHECK(
        adc_oneshot_config_channel(
            adc_handle,
            POT_ADC_CHANNEL,
            &channel_config
        )
    );
}


// PWM initialization
static void pwm_init(void)
{
    // PWM timer
    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = PWM_RESOLUTION,
        .freq_hz = PWM_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK
    };

    ESP_ERROR_CHECK(
        ledc_timer_config(&timer_config)
    );


    // LED PWM
    ledc_channel_config_t led_config = {
        .gpio_num = LED_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LED_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };

    ESP_ERROR_CHECK(
        ledc_channel_config(&led_config)
    );


    // Motor PWM
    ledc_channel_config_t motor_config = {
        .gpio_num = MOTOR_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = MOTOR_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };

    ESP_ERROR_CHECK(
        ledc_channel_config(&motor_config)
    );
}

// Button initialization
static void button_init(void)
{
    gpio_config_t button_config = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    ESP_ERROR_CHECK(
        gpio_config(&button_config)
    );
}

// Set PWM
static void set_led_pwm(uint32_t duty)
{
    if (duty > PWM_MAX)
        duty = PWM_MAX;

    ESP_ERROR_CHECK(
        ledc_set_duty(
            LEDC_LOW_SPEED_MODE,
            LED_CHANNEL,
            duty
        )
    );

    ESP_ERROR_CHECK(
        ledc_update_duty(
            LEDC_LOW_SPEED_MODE,
            LED_CHANNEL
        )
    );
}


static void set_motor_pwm(uint32_t duty)
{
    if (duty > PWM_MAX)
        duty = PWM_MAX;

    ESP_ERROR_CHECK(
        ledc_set_duty(
            LEDC_LOW_SPEED_MODE,
            MOTOR_CHANNEL,
            duty
        )
    );

    ESP_ERROR_CHECK(
        ledc_update_duty(
            LEDC_LOW_SPEED_MODE,
            MOTOR_CHANNEL
        )
    );
}

// Application initialization
void app_init(void)
{
    adc_init();
    pwm_init();
    button_init();

    ESP_LOGI(TAG, "Application initialized");
    ESP_LOGI(TAG, "MODE: LED");
}

// Main application
void app_run(void)
{
    int adc_raw = 0;

    bool button_previous = false;

    while (1)
    {
        // Read potentiometer
        ESP_ERROR_CHECK(
            adc_oneshot_read(
                adc_handle,
                POT_ADC_CHANNEL,
                &adc_raw
            )
        );

        // Convert ADC → PWM
        // ADC: 0...4095
        // PWM: 0...1023
        uint32_t pwm_duty =
            ((uint32_t)adc_raw * PWM_MAX) / 4095;


        // Read button
        bool button_pressed =
            gpio_get_level(BUTTON_GPIO);

        // Detect button press

        if (button_pressed && !button_previous)
        {
            if (current_mode == MODE_LED)
            {
                current_mode = MODE_MOTOR;

                // LED off when switching
                set_led_pwm(0);

                ESP_LOGI(TAG, "MODE: MOTOR");
            }
            else
            {
                current_mode = MODE_LED;

                // Motor off when switching
                set_motor_pwm(0);

                ESP_LOGI(TAG, "MODE: LED");
            }

            // Simple debounce
            vTaskDelay(pdMS_TO_TICKS(50));
        }

        button_previous = button_pressed;

        // Control selected device
        if (current_mode == MODE_LED)
        {
            set_led_pwm(pwm_duty);
            set_motor_pwm(0);
        }
        else
        {
            set_motor_pwm(pwm_duty);
            set_led_pwm(0);
        }

        // Debug information
        ESP_LOGI(
            TAG,
            "ADC: %d | PWM: %lu | MODE: %s",
            adc_raw,
            pwm_duty,
            current_mode == MODE_LED ? "LED" : "MOTOR"
        );


        vTaskDelay(pdMS_TO_TICKS(100));
    }
}