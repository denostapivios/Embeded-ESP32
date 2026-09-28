#ifndef ENCODER_ESP32_H
#define ENCODER_ESP32_H

#include "safe_types.h"

void encoder_esp32_init(void);

EncoderDirection encoder_esp32_get_direction(void);

#endif