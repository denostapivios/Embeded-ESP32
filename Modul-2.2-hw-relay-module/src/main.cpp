#include <Arduino.h>

// Визначаємо піни відповідно до схеми
const int RELAY_CTRL_PIN = 18;  // Пін керування реле (IN)
const int RELAY_READ_PIN = 17;  // Пін зчитування стану контакту (NO)

const int TOTAL_MEASUREMENTS = 10;
unsigned long measurements[TOTAL_MEASUREMENTS];
int measurementCount = 0;

volatile unsigned long triggerTime = 0;
volatile bool contactClosed = false;

// Обробник переривання (ISR)
void IRAM_ATTR handleContactClosure() {
  triggerTime = millis();
  contactClosed = true;
}

void setup() {
  Serial.begin(115200);
  
  // Налаштування керування реле (Active LOW за замовчуванням для більшості модулів)
  pinMode(RELAY_CTRL_PIN, OUTPUT);
  digitalWrite(RELAY_CTRL_PIN, HIGH); // Вимкнено на старті
  
  // Налаштування зчитування з підтяжкою до 3.3 В
  pinMode(RELAY_READ_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RELAY_READ_PIN), handleContactClosure, FALLING);
  
  delay(1000); 
  Serial.println("\n=== Тест швидкості спрацьовування реле розпочато ===");
}

void loop() {
  if (measurementCount < TOTAL_MEASUREMENTS) {
    // 1. Повідомляємо про початок конкретного вимірювання
    Serial.print("Запуск вимірювання ");
    Serial.print(measurementCount + 1);
    Serial.println("... виконується...");

    contactClosed = false;
    triggerTime = 0;
    
    // 2. Подаємо сигнал на вмикання реле та фіксуємо стартовий час
    unsigned long startTime = millis();
    digitalWrite(RELAY_CTRL_PIN, LOW); // Вмикаємо реле
    
    // 3. Очікуємо замикання контактів через переривання (із таймаутом 150 мс)
    unsigned long waitStart = millis();
    while (!contactClosed && (millis() - waitStart < 150)) {
      // Очікування фізичного перемикання пластини реле
    }
    
    // 4. Одразу вимикаємо реле назад, щоб котушка не грілася марно
    digitalWrite(RELAY_CTRL_PIN, HIGH);
    
    // 5. Обробляємо та миттєво виводимо результат поточного кроку
    if (contactClosed) {
      unsigned long duration = triggerTime - startTime;
      measurements[measurementCount] = duration;
      
      Serial.print("-> Успішно! Час замикання контактів: ");
      Serial.print(duration);
      Serial.println(" мс\n");
      
      measurementCount++;
    } else {
      Serial.println("-> Помилка: Контакти не встигли замкнутися за 150 мс. Перевірте дроти!\n");
      delay(2000); // Даємо паузу перед наступною спробою
      return;
    }
    
    // 6. Якщо це було останнє вимірювання, виводимо підсумкову статистику
    if (measurementCount == TOTAL_MEASUREMENTS) {
      unsigned long sum = 0;
      for (int i = 0; i < TOTAL_MEASUREMENTS; i++) {
        sum += measurements[i];
      }
      float average = (float)sum / TOTAL_MEASUREMENTS;
      
      Serial.println("=== КІНЦЕВІ РЕЗУЛЬТАТИ ===");
      Serial.print("Отримані значення: ");
      for (int i = 0; i < TOTAL_MEASUREMENTS; i++) {
        Serial.print(measurements[i]);
        Serial.print(i == TOTAL_MEASUREMENTS - 1 ? " мс" : " мс, ");
      }
      Serial.println();
      Serial.print("Середній час фізичного спрацьовування: ");
      Serial.print(average, 2);
      Serial.println(" мс");
      Serial.println("================================");
    }
    
    // Робимо паузу 1.5 секунди між тестами, щоб механічні контакти повністю розійшлися та заспокоїлися [2]
    delay(1500); 
  }
}