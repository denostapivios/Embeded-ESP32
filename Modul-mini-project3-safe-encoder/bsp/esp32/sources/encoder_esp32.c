#include "encoder_esp32.h"
#include "encoder_interface.h"

#include "driver/gpio.h"
#include "driver/pulse_cnt.h"
#include "esp_err.h"


// ============================================================
// Encoder pins
// ============================================================

#define ENC_A 6
#define ENC_B 5


// ============================================================
// Encoder configuration
// ============================================================

// 1 mechanical tick = 2 PCNT counts
#define COUNTS_PER_TICK 2


// ============================================================
// PCNT
// ============================================================

static pcnt_unit_handle_t unit = NULL;
static pcnt_channel_handle_t channel = NULL;


// ============================================================
// Encoder accumulator
// ============================================================

static int tick_accumulator = 0;


// ============================================================
// Initialization
// ============================================================

void encoder_init(void)
{
    // --------------------------------------------------------
    // GPIO configuration
    // --------------------------------------------------------

    gpio_config_t io = {
        .pin_bit_mask = (1ULL << ENC_A) | (1ULL << ENC_B),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK(
        gpio_config(&io)
    );


    // --------------------------------------------------------
    // PCNT unit
    // --------------------------------------------------------

    pcnt_unit_config_t unit_cfg = {
        .high_limit = 100,
        .low_limit = -100,
    };

    ESP_ERROR_CHECK(
        pcnt_new_unit(
            &unit_cfg,
            &unit
        )
    );


    // --------------------------------------------------------
    // Glitch filter
    // --------------------------------------------------------

    pcnt_glitch_filter_config_t filter_cfg = {
        .max_glitch_ns = 10000,
    };

    ESP_ERROR_CHECK(
        pcnt_unit_set_glitch_filter(
            unit,
            &filter_cfg
        )
    );


    // --------------------------------------------------------
    // PCNT channel
    // --------------------------------------------------------

    pcnt_chan_config_t channel_cfg = {
        .edge_gpio_num = ENC_A,
        .level_gpio_num = ENC_B,
    };

    ESP_ERROR_CHECK(
        pcnt_new_channel(
            unit,
            &channel_cfg,
            &channel
        )
    );


    // --------------------------------------------------------
    // Edge action
    // --------------------------------------------------------

    ESP_ERROR_CHECK(
        pcnt_channel_set_edge_action(
            channel,
            PCNT_CHANNEL_EDGE_ACTION_INCREASE,
            PCNT_CHANNEL_EDGE_ACTION_DECREASE
        )
    );


    // --------------------------------------------------------
    // Level action
    // --------------------------------------------------------

    ESP_ERROR_CHECK(
        pcnt_channel_set_level_action(
            channel,
            PCNT_CHANNEL_LEVEL_ACTION_KEEP,
            PCNT_CHANNEL_LEVEL_ACTION_INVERSE
        )
    );


    // --------------------------------------------------------
    // Start PCNT
    // --------------------------------------------------------

    ESP_ERROR_CHECK(
        pcnt_unit_enable(unit)
    );

    ESP_ERROR_CHECK(
        pcnt_unit_clear_count(unit)
    );

    ESP_ERROR_CHECK(
        pcnt_unit_start(unit)
    );
}


// ============================================================
// Get encoder direction
// ============================================================

EncoderDirection encoder_get_direction(void)
{
    int count = 0;

    ESP_ERROR_CHECK(
        pcnt_unit_get_count(
            unit,
            &count
        )
    );


    if (count == 0)
    {
        return ENCODER_NO_MOVEMENT;
    }


    tick_accumulator += count;


    ESP_ERROR_CHECK(
        pcnt_unit_clear_count(unit)
    );


    // --------------------------------------------------------
    // Clockwise
    // --------------------------------------------------------

    if (tick_accumulator >= COUNTS_PER_TICK)
    {
        tick_accumulator -= COUNTS_PER_TICK;

        return ENCODER_CW;
    }


    // --------------------------------------------------------
    // Counter-clockwise
    // --------------------------------------------------------

    if (tick_accumulator <= -COUNTS_PER_TICK)
    {
        tick_accumulator += COUNTS_PER_TICK;

        return ENCODER_CCW;
    }


    return ENCODER_NO_MOVEMENT;
}
