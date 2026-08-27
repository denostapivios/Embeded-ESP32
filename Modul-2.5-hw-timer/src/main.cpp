#include <Arduino.h>

constexpr uint8_t RELAY_PIN = 18;

constexpr uint32_t FAN_PERIOD_SEC = 10;
constexpr uint32_t FAN_WORK_TIME_SEC = 5;
constexpr uint32_t FAN_OFF_TIME_SEC =
    FAN_PERIOD_SEC - FAN_WORK_TIME_SEC;

enum class RelayState : uint8_t
{
    Off = HIGH,
    On = LOW
};

hw_timer_t *timer = nullptr;

volatile bool fanRunning = false;
volatile uint32_t secondsToSwitch = FAN_PERIOD_SEC;

void setRelay(RelayState state)
{
    digitalWrite(RELAY_PIN, static_cast<uint8_t>(state));
}

void IRAM_ATTR onTimer()
{
    secondsToSwitch--;

    if (secondsToSwitch > 0)
    {
        return;
    }

    if (fanRunning)
    {
        setRelay(RelayState::Off);

        fanRunning = false;
        secondsToSwitch = FAN_OFF_TIME_SEC;
    }
    else
    {
        setRelay(RelayState::On);

        fanRunning = true;
        secondsToSwitch = FAN_WORK_TIME_SEC;
    }
}

void setup()
{
    pinMode(RELAY_PIN, OUTPUT);

    setRelay(RelayState::Off);

    timer = timerBegin(0, 80, true);

    timerAttachInterrupt(
        timer,
        &onTimer,
        true
    );

    timerAlarmWrite(
        timer,
        1000000,
        true
    );

    timerAlarmEnable(timer);
}

void loop()
{
}