#ifndef SAFE_TYPES_H
#define SAFE_TYPES_H

#include <stdint.h>
#include <stdbool.h>

#define SAFE_CODE_LENGTH 4

typedef enum
{
    SAFE_LOCKED,
    SAFE_UNLOCKED
} SafeState;

typedef enum
{
    ENCODER_NO_MOVEMENT,
    ENCODER_CW,
    ENCODER_CCW
} EncoderDirection;

typedef struct
{
    uint8_t digits[SAFE_CODE_LENGTH];
    uint8_t current_digit;
    SafeState state;
} SafeContext;

#endif