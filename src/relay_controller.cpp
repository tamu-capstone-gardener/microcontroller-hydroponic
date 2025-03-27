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
  digitalWrite(RELAY_PUMP_PIN, HIGH);
}

void turnPumpRelayOff() {
  digitalWrite(RELAY_PUMP_PIN, LOW);
  
}

void turnOutletRelayOn() {
  digitalWrite(RELAY_OUTLET_PIN, HIGH);
}

void turnOutletRelayOff() {
  digitalWrite(RELAY_OUTLET_PIN, LOW);
}