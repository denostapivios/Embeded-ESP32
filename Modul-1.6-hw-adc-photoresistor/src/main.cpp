#include <Arduino.h>

constexpr uint8_t ADC_PIN = 4;

void setup() {
  Serial.begin(115200);

  // Роздільність АЦП: 12 біт, значення 0–4095
  analogReadResolution(12);

  analogSetAttenuation(ADC_11db); 
}

void loop() {
  // RAW-значення АЦП
  int rawValue = analogRead(ADC_PIN);

 // Обчислення напруги за формулою:
  float voltage = rawValue * (3.3 / 4095.0); 

  // Калібрована напруга в мілівольтах
  uint32_t measuredMillivolts = analogReadMilliVolts(ADC_PIN); 
  float measuredVoltage = measuredMillivolts / 1000.0f;
  
  Serial.printf("Raw: %d | Voltage: %.2f | Measured voltage: %.2f V\n", rawValue, voltage, measuredVoltage);
  delay(1000);
}