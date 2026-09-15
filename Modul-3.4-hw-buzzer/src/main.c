#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "driver/ledc.h"
#include "driver/gptimer.h"
#include "driver/gpio.h"

#include "esp_attr.h"
#include "esp_err.h"

#define BUZZER_GPIO             GPIO_NUM_4

#define BUZZER_LEDC_MODE        LEDC_LOW_SPEED_MODE
#define BUZZER_LEDC_CHANNEL     LEDC_CHANNEL_0
#define BUZZER_LEDC_TIMER       LEDC_TIMER_0

#define BUZZER_PWM_RESOLUTION   LEDC_TIMER_10_BIT
#define BUZZER_DUTY             256

#define TICK_MS                 50
#define TIMER_RESOLUTION_HZ     1000000

typedef struct
{
    uint32_t frequency;
    uint32_t duration_ms;

} Note;


static const Note melody[] =
{
    {262, 300},     // C
    {262, 300},     // C
    {294, 600},     // D

    {262, 300},     // C
    {262, 300},     // C
    {294, 600},     // D

    {262, 300},     // C
    {262, 300},     // C
    {294, 300},     // D
    {330, 800},     // E

    {0,   500}      // Pause
};


#define MELODY_LENGTH \
    (sizeof(melody) / sizeof(melody[0]))

static gptimer_handle_t timer = NULL;

static volatile bool tick_50ms = false;

static uint32_t current_note = 0;
static uint32_t elapsed_ms = 0;


static void buzzer_init(void)
{
    // LEDC TIMER configuration
    ledc_timer_config_t timer_cfg =
    {
        .speed_mode       = BUZZER_LEDC_MODE,
        .timer_num        = BUZZER_LEDC_TIMER,
        .duty_resolution  = BUZZER_PWM_RESOLUTION,
        .freq_hz          = 1000,
        .clk_cfg          = LEDC_AUTO_CLK
    };

    ESP_ERROR_CHECK(
        ledc_timer_config(&timer_cfg)
    );


    // LEDC CHANNEL configuration
    ledc_channel_config_t channel_cfg =
    {
        .gpio_num       = BUZZER_GPIO,
        .speed_mode     = BUZZER_LEDC_MODE,
        .channel        = BUZZER_LEDC_CHANNEL,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = BUZZER_LEDC_TIMER,
        .duty           = 0,
        .hpoint         = 0
    };

    ESP_ERROR_CHECK(
        ledc_channel_config(&channel_cfg)
    );
}


static void buzzer_start(uint32_t frequency)
{
    if (frequency == 0)
    {
        return;
    }

    ESP_ERROR_CHECK(
        ledc_set_freq(
            BUZZER_LEDC_MODE,
            BUZZER_LEDC_TIMER,
            frequency
        )
    );

    ESP_ERROR_CHECK(
        ledc_set_duty(
            BUZZER_LEDC_MODE,
            BUZZER_LEDC_CHANNEL,
            BUZZER_DUTY
        )
    );

    ESP_ERROR_CHECK(
        ledc_update_duty(
            BUZZER_LEDC_MODE,
            BUZZER_LEDC_CHANNEL
        )
    );
}


static void buzzer_stop(void)
{
    ESP_ERROR_CHECK(
        ledc_stop(
            BUZZER_LEDC_MODE,
            BUZZER_LEDC_CHANNEL,
            0
        )
    );
}


static void play_current_note(void)
{
    uint32_t frequency = melody[current_note].frequency;

    if (frequency == 0)
    {
        buzzer_stop();
    }
    else
    {
        buzzer_start(frequency);
    }
}


static void buzzer_update(void)
{
    elapsed_ms += TICK_MS;

    if (elapsed_ms >= melody[current_note].duration_ms)
    {
        elapsed_ms = 0;

        current_note++;

        if (current_note >= MELODY_LENGTH)
        {
            current_note = 0;
        }

        play_current_note();
    }
}


static bool IRAM_ATTR timer_callback(
    gptimer_handle_t timer,
    const gptimer_alarm_event_data_t *edata,
    void *user_data)
{
    tick_50ms = true;

    return false;
}


static void timer_init(void)
{
    // GPTimer configuration
    gptimer_config_t timer_cfg =
    {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = TIMER_RESOLUTION_HZ
    };

    ESP_ERROR_CHECK(
        gptimer_new_timer(
            &timer_cfg,
            &timer
        )
    );


    // Callback configuration
    gptimer_event_callbacks_t callbacks =
    {
        .on_alarm = timer_callback
    };

    ESP_ERROR_CHECK(
        gptimer_register_event_callbacks(
            timer,
            &callbacks,
            NULL
        )
    );


    // Alarm configuration
    gptimer_alarm_config_t alarm_cfg =
    {
        .reload_count = 0,

        .alarm_count = TICK_MS * 1000,

        .flags.auto_reload_on_alarm = true
    };

    ESP_ERROR_CHECK(
        gptimer_set_alarm_action(
            timer,
            &alarm_cfg
        )
    );


    // Enable timer
    ESP_ERROR_CHECK(
        gptimer_enable(timer)
    );


    // Start timer
    ESP_ERROR_CHECK(
        gptimer_start(timer)
    );
}


void app_main(void)
{
    printf("Buzzer player started\n");

    // Initialize PWM
    buzzer_init();

    // Initialize 50 ms timer
    timer_init();

    // Start first note
    play_current_note();


    // Main loop
    while (1)
    {
        if (tick_50ms)
        {
            tick_50ms = false;

            buzzer_update();
        }

    }
}