#include <Arduino.h>

#define CAMERA_MODEL_AI_THINKER

int        flashPin = 4;
const char asd      = 3;

void setup() {
    pinMode(flashPin, OUTPUT);

    Serial.begin(115200);
    Serial.println("Hello!");
}

void loop() {
    digitalWrite(flashPin, HIGH);
    delay(1000);
    digitalWrite(flashPin, LOW);
    delay(1000);

    Serial.println("World!");
}