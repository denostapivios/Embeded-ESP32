#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <driver/uart.h>
#include <driver/gpio.h>

#include <esp_log.h>


// ============================================================
// UART
// ============================================================

#define UART_PORT_NUM UART_NUM_1

#define TX_PIN GPIO_NUM_17
#define RX_PIN GPIO_NUM_18


// ============================================================
// GPIO
// ============================================================

#define BUTTON_BOOT_PIN GPIO_NUM_5
#define LED_PIN GPIO_NUM_4


static const char *TAG = "ESP32_S3_ISR";


// ============================================================
// BUTTON ISR
// ============================================================

static volatile bool button_pressed_flag = false;
static volatile uint32_t last_isr_time = 0;


// ============================================================
// LED STATE
// ============================================================

static bool led_state = false;


// ============================================================
// UART RX DEBOUNCE
// ============================================================

static uint32_t last_command_time = 0;


// ============================================================
// GPIO ISR
// ============================================================

static void IRAM_ATTR gpio_boot_isr_handler(void* arg)
{
    uint32_t current_time = xTaskGetTickCountFromISR();

    // Debounce кнопки 200 ms
    if ((current_time - last_isr_time) > pdMS_TO_TICKS(200))
    {
        button_pressed_flag = true;
        last_isr_time = current_time;
    }
}


// ============================================================
// PERIPHERALS
// ============================================================

void init_peripherals(void)
{
    // --------------------------------------------------------
    // UART1
    // --------------------------------------------------------

    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    uart_param_config(
        UART_PORT_NUM,
        &uart_config
    );

    uart_set_pin(
        UART_PORT_NUM,
        TX_PIN,
        RX_PIN,
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE
    );

    uart_driver_install(
        UART_PORT_NUM,
        256,
        256,
        0,
        NULL,
        0
    );


    // --------------------------------------------------------
    // BUTTON
    // --------------------------------------------------------

    gpio_config_t button_config = {
        .pin_bit_mask = (1ULL << BUTTON_BOOT_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };

    gpio_config(&button_config);


    // --------------------------------------------------------
    // LED
    // --------------------------------------------------------

    gpio_config_t led_config = {
        .pin_bit_mask = (1ULL << LED_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    gpio_config(&led_config);

    // Початковий стан LED
    led_state = false;

    gpio_set_level(
        LED_PIN,
        0
    );


    // --------------------------------------------------------
    // GPIO INTERRUPT
    // --------------------------------------------------------

    gpio_install_isr_service(0);

    gpio_isr_handler_add(
        BUTTON_BOOT_PIN,
        gpio_boot_isr_handler,
        (void*)BUTTON_BOOT_PIN
    );
}


// ============================================================
// APP MAIN
// ============================================================

void app_main(void)
{
    init_peripherals();

    ESP_LOGI(
        TAG,
        "ESP32-S3 initialized. Waiting for buttons..."
    );


    uint8_t rx_data[1];


    while (1)
    {
        // ====================================================
        // ESP32 BUTTON
        // ====================================================

        if (button_pressed_flag)
        {
            button_pressed_flag = false;

            // ESP32 -> STM32
            // '1' = toggle STM32 LED

            uart_write_bytes(
                UART_PORT_NUM,
                "1",
                1
            );

            ESP_LOGI(
                TAG,
                "ESP32 button -> STM32: command '1'"
            );
        }


        // ====================================================
        // UART RECEIVE
        // STM32 -> ESP32
        // ====================================================

        int len = uart_read_bytes(
            UART_PORT_NUM,
            rx_data,
            1,
            0
        );

        if (len > 0)
        {
            // STM32 -> ESP32
            // '2' = toggle ESP32 LED

            if (rx_data[0] == '2')
            {
                uint32_t current_time = xTaskGetTickCount();

                // Захист від повторного отримання команди
                // протягом 300 ms
                if ((current_time - last_command_time) >= pdMS_TO_TICKS(300))
                {
                    last_command_time = current_time;

                    // Toggle програмного стану LED
                    led_state = !led_state;

                    gpio_set_level(
                        LED_PIN,
                        led_state
                    );

                    ESP_LOGI(
                        TAG,
                        "STM32 -> ESP32: LED %s",
                        led_state ? "ON" : "OFF"
                    );
                }
            }
        }


        vTaskDelay(pdMS_TO_TICKS(10));
    }
}