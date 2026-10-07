#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ssd1306_display.h"

static const char *TAG = "MAIN_APP";

void app_main(void)
{
    ESP_LOGI(TAG, "Запуск головного модуля...");

    // 1. Ініціалізація дисплея
    ESP_ERROR_CHECK(ssd1306_display_init());

    // 2. Очищення фреймбуфера та малювання тексту
    ssd1306_display_clear();
    ssd1306_display_draw_string_2x(16, 3, "BeetRoot");

    // 3. Оновлення зображення на дисплеї
    ESP_ERROR_CHECK(ssd1306_display_update());

    ESP_LOGI(TAG, "Текст 'beetroot' виведено на екран.");

    // Головний цикл
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}