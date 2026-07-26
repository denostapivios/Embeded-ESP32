#include <Arduino.h>

#define RED_LED_PIN  4
#define BLUE_LED_PIN 5

void setup() {
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(BLUE_LED_PIN, OUTPUT);
}

void loop() {
    digitalWrite(RED_LED_PIN, HIGH); 
    delay(300);
    digitalWrite(RED_LED_PIN, LOW);  
    delay(300);
    digitalWrite(BLUE_LED_PIN, HIGH); 
    delay(300);
    digitalWrite(BLUE_LED_PIN, LOW);  
    delay(300);
}