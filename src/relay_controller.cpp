// relay_controller.cpp
#include "relay_controller.h"
#include "config.h"
#include <Arduino.h>

void initPumpRelay() {
    pinMode(RELAY_PUMP_PIN, OUTPUT);
    turnPumpRelayOff();
}

void initOutletRelay() {
    pinMode(RELAY_OUTLET_PIN, OUTPUT);
    turnOutletRelayOff();
}

void turnPumpRelayOn() {
    Serial.println("Turning the pump relay on");
    digitalWrite(RELAY_PUMP_PIN, HIGH);
}

void turnPumpRelayOff() {
    Serial.println("Turning the pump relay off");
    digitalWrite(RELAY_PUMP_PIN, LOW); 
}

void turnOutletRelayOn() {
    Serial.println("Turning the outlet relay on");
    digitalWrite(RELAY_OUTLET_PIN, HIGH);
}

void turnOutletRelayOff() {
    Serial.println("Turning the outlet relay off");
    digitalWrite(RELAY_OUTLET_PIN, LOW);
}