#include <Arduino.h>

// Визначення пінів
#define RED_LED_PIN 4
#define BLUE_LED_PIN 5
#define EXTERNAL_BUTTON_PIN 18 
#define BOOT_BUTTON_PIN 0 

uint16_t blinkDelay = 1000;

void setup() {
  // Ініціалізація послідовного інтерфейсу для відладки
   Serial.begin(115200);
   delay(1000);

   // Налаштування GPIO
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(EXTERNAL_BUTTON_PIN, INPUT);
  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);
  Serial.println("GPIO система ініціалізована");
}

void loop() {
  // Зчитування стану кнопки
  uint8_t externalButtonState = digitalRead(EXTERNAL_BUTTON_PIN);
  uint8_t bootButtonState = digitalRead(BOOT_BUTTON_PIN);

  // Швидкий режим
  if (externalButtonState == HIGH) {
      blinkDelay = 100;
      delay(50);
  }
// Повільний режим
  else if (bootButtonState == LOW) {
      blinkDelay = 600;
      delay(50);
  }

  digitalWrite(RED_LED_PIN, HIGH);
  digitalWrite(BLUE_LED_PIN, LOW);
  delay(blinkDelay);

  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(BLUE_LED_PIN, HIGH);
  delay(blinkDelay);
}
