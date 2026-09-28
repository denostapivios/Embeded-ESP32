#include "safe.h"

#include <stdio.h>
#include <string.h>

static const uint8_t CORRECT_CODE[SAFE_CODE_LENGTH] =
{
    1, 1, 1, 1
};

static SafeContext safe_context;

static void safe_reset_code(void)
{
    memset(
        safe_context.digits,
        0,
        sizeof(safe_context.digits)
    );

    safe_context.current_digit = 0;

    printf("Code reset\n");
}

static bool safe_check_code(void)
{
    return memcmp(
        safe_context.digits,
        CORRECT_CODE,
        SAFE_CODE_LENGTH
    ) == 0;
}

static void safe_print_code(void)
{
    printf(
        "Code: [%d,%d,%d,%d] | Current digit: %d\n",
        safe_context.digits[0],
        safe_context.digits[1],
        safe_context.digits[2],
        safe_context.digits[3],
        safe_context.current_digit
    );
}

void safe_init(void)
{
    safe_context.state = SAFE_LOCKED;
    safe_context.current_digit = 0;

    safe_reset_code();

    printf("\n");
    printf("=================================\n");
    printf("        SAFE LOCK STARTED        \n");
    printf("=================================\n");

    printf("State: LOCKED\n");
    printf("Enter 4-digit code\n");
    printf("Test code: [1,1,1,1]\n");

    safe_print_code();
}

void safe_handle_encoder(EncoderDirection direction)
{
    if (safe_context.state == SAFE_UNLOCKED)
    {
        return;
    }

    if (direction == ENCODER_CW)
    {
        safe_context.digits[safe_context.current_digit]++;

        if (safe_context.digits[safe_context.current_digit] > 9)
        {
            safe_context.digits[safe_context.current_digit] = 0;
        }

        printf("Encoder: +1\n");
    }
    else if (direction == ENCODER_CCW)
    {
        if (safe_context.digits[safe_context.current_digit] == 0)
        {
            safe_context.digits[safe_context.current_digit] = 9;
        }
        else
        {
            safe_context.digits[safe_context.current_digit]--;
        }

        printf("Encoder: -1\n");
    }

    safe_print_code();
}

void safe_handle_button(void)
{
    if (safe_context.state == SAFE_UNLOCKED)
    {
        printf("Safe is already unlocked\n");
        return;
    }

    printf(
        "Digit %d selected: %d\n",
        safe_context.current_digit,
        safe_context.digits[safe_context.current_digit]
    );

    if (safe_context.current_digit == SAFE_CODE_LENGTH - 1)
    {
        printf("Checking code...\n");

        if (safe_check_code())
        {
            safe_context.state = SAFE_UNLOCKED;

            printf("\n");
            printf("=============================\n");
            printf("        SAFE UNLOCKED        \n");
            printf("=============================\n");
            printf("\n");
        }
        else
        {
            safe_context.state = SAFE_LOCKED;

            printf("\n");
            printf("=============================\n");
            printf("      WRONG CODE! LOCKED     \n");
            printf("=============================\n");
            printf("\n");

            safe_reset_code();
            safe_print_code();
        }

        return;
    }

    safe_context.current_digit++;

    printf(
        "Next digit: %d\n",
        safe_context.current_digit
    );

    safe_print_code();
}

const SafeContext *safe_get_context(void)
{
    return &safe_context;
}