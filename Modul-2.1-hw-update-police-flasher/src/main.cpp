#include <Arduino.h>

enum class LedState : uint8_t {
    Off,
    On
};

enum class BlinkPhase : uint8_t {
    RedOn,
    RedOff,
    BlueOn,
    BlueOff
};

class Led {
public:
    explicit Led(uint8_t pin)
        : pin_(pin) {
    }

    void init() {
        pinMode(pin_, OUTPUT);
        set(LedState::Off);
    }

    void set(LedState state) {
        digitalWrite(pin_, state == LedState::On ? HIGH : LOW);
    }

private:
    uint8_t pin_;
};

constexpr uint8_t RED_LED_PIN = 4;
constexpr uint8_t BLUE_LED_PIN = 5;

constexpr uint32_t BLINK_INTERVAL_MS = 300;

Led redLed(RED_LED_PIN);
Led blueLed(BLUE_LED_PIN);

void setup() {
    redLed.init();
    blueLed.init();

    redLed.set(LedState::On);
    blueLed.set(LedState::Off);
}

void loop() {
    static BlinkPhase phase = BlinkPhase::RedOn;
    static uint32_t previousMillis = 0;

    const uint32_t currentMillis = millis();

    if (currentMillis - previousMillis < BLINK_INTERVAL_MS) {
        return;
    }

    previousMillis = currentMillis;

    switch (phase) {
        case BlinkPhase::RedOn:
            redLed.set(LedState::Off);
            phase = BlinkPhase::RedOff;
            break;

        case BlinkPhase::RedOff:
            blueLed.set(LedState::On);
            phase = BlinkPhase::BlueOn;
            break;

        case BlinkPhase::BlueOn:
            blueLed.set(LedState::Off);
            phase = BlinkPhase::BlueOff;
            break;

        case BlinkPhase::BlueOff:
            redLed.set(LedState::On);
            phase = BlinkPhase::RedOn;
            break;
    }
}