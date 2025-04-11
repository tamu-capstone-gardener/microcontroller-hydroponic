#include "relay_controller.h"
#include "config.h"
#include <Arduino.h>

// Initialize the pump relays and light relay.
void initPumpN() {
    pinMode(RELAY_N_PIN, OUTPUT);
    digitalWrite(RELAY_N_PIN, LOW);
}

void initPumpP() {
    pinMode(RELAY_P_PIN, OUTPUT);
    digitalWrite(RELAY_P_PIN, LOW);
}

void initPumpK() {
    pinMode(RELAY_K_PIN, OUTPUT);
    digitalWrite(RELAY_K_PIN, LOW);
}

void initLightRelay() {
    pinMode(RELAY_LIGHT_PIN, OUTPUT);
    digitalWrite(RELAY_LIGHT_PIN, LOW);
}

// Pump control functions.
void turnPumpNOn() {
    Serial.println("Turning Nitrogen pump ON");
    digitalWrite(RELAY_N_PIN, HIGH);
}

void turnPumpNOff() {
    Serial.println("Turning Nitrogen pump OFF");
    digitalWrite(RELAY_N_PIN, LOW);
}

void turnPumpPOn() {
    Serial.println("Turning Phosphorus pump ON");
    digitalWrite(RELAY_P_PIN, HIGH);
}

void turnPumpPOff() {
    Serial.println("Turning Phosphorus pump OFF");
    digitalWrite(RELAY_P_PIN, LOW);
}

void turnPumpKOn() {
    Serial.println("Turning Potassium pump ON");
    digitalWrite(RELAY_K_PIN, HIGH);
}

void turnPumpKOff() {
    Serial.println("Turning Potassium pump OFF");
    digitalWrite(RELAY_K_PIN, LOW);
}

// Light relay control.
void turnLightOn() {
    Serial.println("Turning light ON");
    digitalWrite(RELAY_LIGHT_PIN, HIGH);
}

void turnLightOff() {
    Serial.println("Turning light OFF");
    digitalWrite(RELAY_LIGHT_PIN, LOW);
}
