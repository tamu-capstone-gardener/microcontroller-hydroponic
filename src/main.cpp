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

const int READ_INTERVAL_MS = 12000;  // 12 seconds between reads (or 5 readings per minute)
const int NUM_SAMPLES = 5;

unsigned long lastReadTime = 0;
int readCount = 0;

// Accumulators so that we can average values over a minute for more accurate readings
int totalMoisture = 0;
float totalTemperature = 0;
float totalHumidity = 0;
int totalLight = 0;


// Need to work on this part or get scheduling working
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

  unsigned long now = millis();

  // Every 12 seconds, take a reading
  if (now - lastReadTime >= READ_INTERVAL_MS) {
    lastReadTime = now;

    int moisture = readMoisture();
    float temp = readTemperature();
    float hum = readHumidity();
    int light = readLightAnalog();

    // Uncomment below line for sensor readings printed to serial once every 12 seconds
    // printSensorReadings(moisture, temp, hum, light);

    totalMoisture += moisture;
    totalTemperature += temp;
    totalHumidity += hum;
    totalLight += light;
    readCount++;

    Serial.print("Collected reading ");
    Serial.println(readCount);
  }

  // After 5 readings, average and publish
  if (readCount >= NUM_SAMPLES) {
    int avgMoisture = totalMoisture / NUM_SAMPLES;
    float avgTemp = totalTemperature / NUM_SAMPLES;
    float avgHum = totalHumidity / NUM_SAMPLES;
    int avgLight = totalLight / NUM_SAMPLES;

    // Get time
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      strftime(timeStamp, sizeof(timeStamp), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
    }

    // Print and publish once every minute 
    printSensorReadings(avgMoisture, avgTemp, avgHum, avgLight);
    publishSensorData(MOISTURE_SENSOR_ID, avgMoisture, timeStamp);
    publishSensorData(TEMPERATURE_SENSOR_ID, avgTemp, timeStamp);
    publishSensorData(HUMIDITY_SENSOR_ID, avgHum, timeStamp);
    publishSensorData(LIGHT_SENSOR_ID, avgLight, timeStamp);

    // Handle auto watering
    if (!manualOverride && millis() > autoModePauseUntil) {
      if (avgMoisture > 2000) turnPumpRelayOn();
      else turnPumpRelayOff();
    }

    // Reset for next minute
    readCount = 0;
    totalMoisture = 0;
    totalTemperature = 0;
    totalHumidity = 0;
    totalLight = 0;
  }

  // MQTT loop
  mqttClient.loop();
}

