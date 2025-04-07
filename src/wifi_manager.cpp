#include <WiFi.h>
#include "esp_wpa2.h" // Include for WPA2 Enterprise
#include "config.h"   // Your config.h should define WIFI_SSID, WIFI_USERNAME, WIFI_PASSWORD


// for TAMU WiFi
/*void connectToWiFi() {
  WiFi.disconnect(true);  // Reset WiFi

  WiFi.mode(WIFI_STA);
  esp_wifi_sta_wpa2_ent_enable();

  // Set enterprise credentials
  esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)WIFI_USERNAME, strlen(WIFI_USERNAME));
  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)WIFI_USERNAME, strlen(WIFI_USERNAME));
  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)WIFI_PASSWORD, strlen(WIFI_PASSWORD));

  WiFi.begin(WIFI_SSID);  // Just SSID — no password here

  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi.");
  Serial.println(WiFi.localIP());
}*/

void connectToWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());
}
