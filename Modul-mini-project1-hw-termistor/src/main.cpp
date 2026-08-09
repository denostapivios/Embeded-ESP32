#include <Arduino.h>

constexpr u_int8_t ADC_PIN = 4;

void setup() {
  Serial.begin(115200);
// Роздільність АЦП: 12 біт, значення 0–4095
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
}

void loop() {
// RAW-значення АЦП: 0–4095
  int rawValue = analogRead(ADC_PIN);

// Калібрована напруга від ESP32
  uint32_t measuredMillivolts = analogReadMilliVolts(ADC_PIN);
  float measuredVoltage = measuredMillivolts / 1000.0f;
   
  // Опір NTC рахуємо через калібровану напругу
  float resistance =
      10000.0f * measuredVoltage /
      (3.3f - measuredVoltage);

  // Температура
  float temperature = 1.0f /
    ((1.0f / 298.15f) +
     (1.0f / 3950.0f) * logf(resistance / 10000.0f))
    - 273.15f;

 Serial.printf("Raw: %d | Resistance: %2f : Temperature: %2f V\n", rawValue, resistance, temperature);
 delay(1000);
}