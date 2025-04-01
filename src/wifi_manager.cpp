// wifi_manager.cpp
#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "wifi_manager.h"


void connectToWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());
}
