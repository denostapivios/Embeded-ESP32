#ifndef BUTTON_ESP32_H
#define BUTTON_ESP32_H

#include <stdbool.h>

void button_esp32_init(void);

bool button_esp32_is_pressed(void);

#endif