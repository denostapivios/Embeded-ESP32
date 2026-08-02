#include <Arduino.h>

constexpr uint8_t BUTTON_PIN = 18;

volatile uint32_t pulseCounter = 0;

void IRAM_ATTR buttonInterrupt()
{
    // Рахуємо кожен перехід LOW → HIGH
    pulseCounter++;
}

void setup()
{
    Serial.begin(115200);

    // На модулі кнопки вже є підтягування
    pinMode(BUTTON_PIN, INPUT);

    // Натискання твоєї кнопки: LOW → HIGH
    attachInterrupt(
        digitalPinToInterrupt(BUTTON_PIN),
        buttonInterrupt,
        RISING
    );

    Serial.println("Початок вимірювання");
}

void loop()
{
    static uint32_t previousCounter = 0;

    noInterrupts();
    uint32_t currentCounter = pulseCounter;
    interrupts();

    if (currentCounter != previousCounter)
    {
        Serial.printf(
            "Загальна кількість імпульсів: %lu\n",
            static_cast<unsigned long>(currentCounter)
        );

        previousCounter = currentCounter;
    }
}