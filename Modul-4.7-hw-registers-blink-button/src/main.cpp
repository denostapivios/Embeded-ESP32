
#include <Arduino.h>
#define RGB_LED_PIN       48
#define FEEDBACK_ADC_PIN  1
#define ADC_SAMPLES       16
#define TARGET_VALUE      1700
#define KP                0.12f
#define KI                0.01f
#define KD                0.0f

const float integralLimit = 5000.0f;

#define CONTROL_INTERVAL  20
#define SERIAL_INTERVAL   200
#define MAX_PWM_STEP      5

float P = 0.0f;
float I = 0.0f;
float D = 0.0f;

float Kp = KP;
float Ki = KI;
float Kd = KD;

float previousMeasurement = 0.0f;

int targetVal = TARGET_VALUE;
int currentADC = 0;

int pwmOutput = 150;

unsigned long previousControlTime = 0;
unsigned long previousSerialTime = 0;

int readFeedbackADC() {

    long sum = 0;

    for (int i = 0; i < ADC_SAMPLES; i++) {
        sum += analogRead(FEEDBACK_ADC_PIN);
    }

    return sum / ADC_SAMPLES;
}

int pidController(float current, float dt) {

    float error = targetVal - current;

    P = Kp * error;

    // Integral
    float newIntegral = I + error * dt;

    newIntegral = constrain(
        newIntegral,
        -integralLimit,
        integralLimit
    );

    D = -Kd * (current - previousMeasurement) / dt;

    float output = P + I + D;

    float candidateOutput = output + Ki * (newIntegral - I);

    if (!((candidateOutput > 255.0f && error > 0.0f) ||
          (candidateOutput < 0.0f && error < 0.0f))) {

        I = newIntegral;
    }

    output = P + Ki * I + D;

    output = constrain(output, 0.0f, 255.0f);

    previousMeasurement = current;

    return (int)output;
}

void updateLED(int brightness) {

    brightness = constrain(brightness, 0, 255);

    neopixelWrite(
        RGB_LED_PIN,
        brightness,
        brightness,
        brightness
    );
}

void setup() {

    Serial.begin(115200);

    analogReadResolution(12);

    analogSetPinAttenuation(
        FEEDBACK_ADC_PIN,
        ADC_11db
    );

    updateLED(pwmOutput);

    delay(500);

    currentADC = readFeedbackADC();

    previousMeasurement = currentADC;

    previousControlTime = millis();
    previousSerialTime = millis();

    Serial.println();
    Serial.println("================================");
    Serial.println("PI LIGHT CONTROLLER");
    Serial.println("ESP32-S3");
    Serial.println("================================");

    Serial.printf("Initial ADC: %d\n", currentADC);
    Serial.printf("Target ADC: %d\n", targetVal);
    Serial.printf("Initial PWM: %d\n", pwmOutput);

    Serial.println("Controller started!");
    Serial.println();
}

void loop() {

    unsigned long currentTime = millis();

    if (currentTime - previousControlTime >= CONTROL_INTERVAL) {

        float dt = (currentTime - previousControlTime) / 1000.0f;

        previousControlTime = currentTime;

        currentADC = readFeedbackADC();

        int requestedPWM = pidController(currentADC, dt);

        int difference = requestedPWM - pwmOutput;

        difference = constrain(
            difference,
            -MAX_PWM_STEP,
            MAX_PWM_STEP
        );

        pwmOutput += difference;

        updateLED(pwmOutput);
    }

    if (currentTime - previousSerialTime >= SERIAL_INTERVAL) {

        previousSerialTime = currentTime;

        Serial.printf(
            "ADC: %d | TARGET: %d | P: %.2f | I: %.2f | D: %.2f | PWM: %d\n",
            currentADC,
            targetVal,
            P,
            I,
            D,
            pwmOutput
        );
    }
}
