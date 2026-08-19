#include <Arduino.h>

enum class LedState : uint8_t {
    Off,
    On
};

struct Led {
    uint8_t pin;
    uint32_t interval;
    uint32_t previousMillis;
    LedState state;
};

Led redLed = {
    4,
    200,
    0,
    LedState::Off
};

Led blueLed = {
    5,
    500,
    0,
    LedState::Off
};

Led greenLed = {
    6,
    1000,
    0,
    LedState::Off
};

void setLedState(Led& led, LedState state) {
    led.state = state;

    digitalWrite(
        led.pin,
        state == LedState::On ? HIGH : LOW
    );
}

void updateLed(Led& led, uint32_t currentMillis) {

    if (currentMillis - led.previousMillis < led.interval) {
        return;
    }

    led.previousMillis = currentMillis;

    if (led.state == LedState::Off) {
        setLedState(led, LedState::On);
    } else {
        setLedState(led, LedState::Off);
    }
}

void setup() {
    pinMode(redLed.pin, OUTPUT);
    pinMode(blueLed.pin, OUTPUT);
    pinMode(greenLed.pin, OUTPUT);

    setLedState(redLed, LedState::Off);
    setLedState(blueLed, LedState::Off);
    setLedState(greenLed, LedState::Off);
}

void loop() {
    const uint32_t currentMillis = millis();

    updateLed(redLed, currentMillis);
    updateLed(blueLed, currentMillis);
    updateLed(greenLed, currentMillis);
}