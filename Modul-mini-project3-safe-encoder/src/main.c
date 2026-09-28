#include "safe.h"

#include "encoder_interface.h"
#include "button_interface.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    encoder_init();
    button_init();

    vTaskDelay(pdMS_TO_TICKS(1000));

    safe_init();

    while (1)
    {
        EncoderDirection direction =
            encoder_get_direction();

        if (direction != ENCODER_NO_MOVEMENT)
        {
            safe_handle_encoder(direction);
        }

        if (button_is_pressed())
        {
            safe_handle_button();
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}