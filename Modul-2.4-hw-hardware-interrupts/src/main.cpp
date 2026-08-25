#include <Arduino.h>

constexpr uint8_t BUTTON_PIN = 18;

enum ButtonState {
  STATE_IDLE,      // Очікування натискання
  STATE_DEBOUNCE   // Захисний інтервал після змінення стану
};

ButtonState currentState = STATE_IDLE;

bool lastButtonState = HIGH; // HIGH = не натиснуто (при INPUT_PULLUP)
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50; // 50 мс паузи для стабілізації

int counter = 0;

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT);
  lastButtonState = digitalRead(BUTTON_PIN);
}

void loop() {
  bool currentPinState = digitalRead(BUTTON_PIN);
  unsigned long currentMillis = millis();

  switch (currentState) {
    
    case STATE_IDLE:
      // Перевіряємо, чи змінився стан піна з HIGH на LOW (натискання)
      if (lastButtonState == LOW && currentPinState == HIGH) {
        // 1. Одразу фіксуємо перше натискання!
        counter++;
        Serial.print("Count: ");
        Serial.println(counter);

        // 2. Переходимо в режим Debounce і запам'ятовуємо час
        currentState = STATE_DEBOUNCE;
        lastDebounceTime = currentMillis;
      }
      lastButtonState = currentPinState;
      break;

    case STATE_DEBOUNCE:
      // Чекаємо 50 мс, щоб ігнорувати будь-який контактний шум
      if (currentMillis - lastDebounceTime >= debounceDelay) {
        // Оновлюємо актуальний стан після завершення дребезгу
        lastButtonState = digitalRead(BUTTON_PIN);
        currentState = STATE_IDLE;
      }
      break;
  }
}