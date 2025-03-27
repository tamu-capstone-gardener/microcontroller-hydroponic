#include <Arduino.h>
#include <time.h>
#include "config.h"
#include "wifi_manager.h"
#include "mqtt_manager.h"
#include "sensor_manager.h"
#include "relay_controller.h"

/* TODO 

1. Get scheduling of lights and water working
2. Fix threshold to work with soil
3. Get camera working
4. Maybe fix the relay? still have to use voltmeter to short it sometimes 
5. Control the outlet separately from the pump (when needed)

*/

char timeStamp[25];
bool manualOverride = false;
unsigned long autoModePauseUntil = 0;
float temperature, humidity;

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String expected = String("planthub/") + PLANT_MODULE_ID + "/water";
  if (String(topic) == expected) {
    manualOverride = true;
    autoModePauseUntil = millis() + 15000UL;
    turnPumpRelayOn();
    delay(1500);
    turnPumpRelayOff();
  }
}

void setup() {
  Serial.begin(115200);
  connectToWiFi();
  initSensors();
  initPumpRelay();
  initOutletRelay();
  setupMQTT(mqttCallback);
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
}

void loop() {
  if (!mqttClient.connected()) reconnectMQTT();
  mqttClient.loop();

  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    strftime(timeStamp, sizeof(timeStamp), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
  }

  int moisture = readMoisture();
  temperature = readTemperature();
  humidity = readHumidity();
  int lightAnalog = readLightAnalog();

  publishSensorData(MOISTURE_SENSOR_ID, moisture, timeStamp);
  publishSensorData(TEMPERATURE_SENSOR_ID, temperature, timeStamp);
  publishSensorData(HUMIDITY_SENSOR_ID, humidity, timeStamp);
  publishSensorData(LIGHT_SENSOR_ID, lightAnalog, timeStamp);

  if (!manualOverride && millis() > autoModePauseUntil) {
    if (moisture > 2000) turnPumpRelayOn();
    else turnPumpRelayOff();
  }

  delay(2000);
}
