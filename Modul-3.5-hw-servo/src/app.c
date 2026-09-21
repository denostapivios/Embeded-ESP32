#include "app.h"

#include "driver/ledc.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "SERVO_POT"

#define POT_GPIO        4
#define SERVO_GPIO      5
#define POT_ADC_CHANNEL ADC_CHANNEL_3

#define POT_MIN_ANGLE_DEG     0
#define POT_MAX_ANGLE_DEG     270

#define SERVO_MIN_ANGLE_DEG   0
#define SERVO_MAX_ANGLE_DEG   180

#define SERVO_MIN_PULSE_US    500
#define SERVO_MAX_PULSE_US    2400

#define SERVO_PWM_FREQUENCY   50
#define SERVO_PWM_PERIOD_US   20000

#define ADC_MAX_VALUE         4095

#define ADC_SAMPLES           16

#define CONTROL_PERIOD_MS     20

static adc_oneshot_unit_handle_t adc_handle;

static int clamp_int(int value, int min, int max)
{
    if (value < min) {
        return min;
    }

    if (value > max) {
        return max;
    }

    return value;
}

static void potentiometer_init(void)
{
    adc_oneshot_unit_init_cfg_t adc_config = {
        .unit_id = ADC_UNIT_1,
        .clk_src = 0,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    ESP_ERROR_CHECK(
        adc_oneshot_new_unit(
            &adc_config,
            &adc_handle
        )
    );

    adc_oneshot_chan_cfg_t channel_config = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_12,
    };

    ESP_ERROR_CHECK(
        adc_oneshot_config_channel(
            adc_handle,
            POT_ADC_CHANNEL,
            &channel_config
        )
    );

    ESP_LOGI(
        TAG,
        "Potentiometer initialized: GPIO%d / ADC1_CH3",
        POT_GPIO
    );
}

static int potentiometer_read(void)
{
    int sum = 0;

    for (int i = 0; i < ADC_SAMPLES; i++) {

        int raw_value = 0;

        ESP_ERROR_CHECK(
            adc_oneshot_read(
                adc_handle,
                POT_ADC_CHANNEL,
                &raw_value
            )
        );

        sum += raw_value;
    }

    return sum / ADC_SAMPLES;
}

static int adc_to_pot_angle(int adc_value)
{
    adc_value = clamp_int(
        adc_value,
        0,
        ADC_MAX_VALUE
    );

    return (
        adc_value * POT_MAX_ANGLE_DEG
    ) / ADC_MAX_VALUE;
}

static int pot_to_servo_angle(int pot_angle)
{

    if (pot_angle <= SERVO_MIN_ANGLE_DEG) {
        return SERVO_MIN_ANGLE_DEG;
    }

    if (pot_angle >= SERVO_MAX_ANGLE_DEG) {
        return SERVO_MAX_ANGLE_DEG;
    }

    return pot_angle;
}


static void servo_init(void)
{
    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,

        .duty_resolution = LEDC_TIMER_14_BIT,

        .freq_hz = SERVO_PWM_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ESP_ERROR_CHECK(
        ledc_timer_config(&timer_config)
    );


    ledc_channel_config_t channel_config = {
        .gpio_num = SERVO_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
    };

    ESP_ERROR_CHECK(
        ledc_channel_config(&channel_config)
    );

    ESP_LOGI(
        TAG,
        "SG90 initialized: GPIO%d, PWM %d Hz, 14-bit",
        SERVO_GPIO,
        SERVO_PWM_FREQUENCY
    );
}

static void servo_set_angle(int angle)
{
    angle = clamp_int(
        angle,
        SERVO_MIN_ANGLE_DEG,
        SERVO_MAX_ANGLE_DEG
    );

    int pulse_us =
        SERVO_MIN_PULSE_US +
        (
            angle *
            (SERVO_MAX_PULSE_US - SERVO_MIN_PULSE_US)
        ) /
        SERVO_MAX_ANGLE_DEG;

    uint32_t duty =
        (
            (uint64_t)pulse_us * (1 << 14)
        ) /
        SERVO_PWM_PERIOD_US;


    // Set PWM duty
    ESP_ERROR_CHECK(
        ledc_set_duty(
            LEDC_LOW_SPEED_MODE,
            LEDC_CHANNEL_0,
            duty
        )
    );

    // Apply new duty
    ESP_ERROR_CHECK(
        ledc_update_duty(
            LEDC_LOW_SPEED_MODE,
            LEDC_CHANNEL_0
        )
    );
}

void app_init(void)
{
    potentiometer_init();

    servo_init();

    // Start from the extreme left position
    servo_set_angle(0);

    ESP_LOGI(
        TAG,
        "Application initialized"
    );
}

void app_run(void)
{
    // Read potentiometer
    int adc_value = potentiometer_read();


    // ADC -> potentiometer angle
    int pot_angle = adc_to_pot_angle(adc_value);


    // Potentiometer angle -> SG90 angle
    int servo_angle = pot_to_servo_angle(pot_angle);


    // Move servo
    servo_set_angle(servo_angle);


    // Log deviation from extreme left position
    ESP_LOGI(
        TAG,
        "ADC: %d | Pot: %d deg | Servo deviation: %d deg",
        adc_value,
        pot_angle,
        servo_angle
    );


    vTaskDelay(
        pdMS_TO_TICKS(CONTROL_PERIOD_MS)
    );
}