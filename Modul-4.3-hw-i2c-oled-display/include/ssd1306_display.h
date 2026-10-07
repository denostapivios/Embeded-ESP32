#ifndef SSD1306_DISPLAY_H
#define SSD1306_DISPLAY_H

#include "esp_err.h"

// Конфігурація I2C та GPIO для ESP32-S3-DevKitC-1
#define I2C_BUS_PORT        I2C_NUM_0
#define PIN_NUM_SDA         GPIO_NUM_1
#define PIN_NUM_SCL         GPIO_NUM_2
#define LCD_PIXEL_CLOCK_HZ  (400 * 1000)
#define SSD1306_I2C_ADDR    0x3C

// Прототипи функцій для роботи з дисплеєм
esp_err_t ssd1306_display_init(void);
void ssd1306_display_clear(void);
void ssd1306_display_draw_string_2x(int x, int start_page, const char *str);
esp_err_t ssd1306_display_update(void);

#endif // SSD1306_DISPLAY_H