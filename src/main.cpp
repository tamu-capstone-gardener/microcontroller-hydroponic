#include <Arduino.h>
#include <time.h>
#include "config.h"
#include "wifi_manager.h"
#include "mqtt_manager.h"
#include "sensor_manager.h"
#include "relay_controller.h"
#include <map>

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

std::map<String, String> sensorIdMap;

bool sensorsRegistered = false;  // Global flag

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String topicStr(topic);
  String initResponseTopic = "planthub/" + String(PLANT_MODULE_ID) + "/sensor_init_response";

  Serial.print("Full topic received: ");
  Serial.println(topicStr);
  Serial.print("Expected suffix: ");
  Serial.println(initResponseTopic);

  if (topicStr.equals(initResponseTopic)) {
    Serial.println("Sensor init response received on topic:");
    Serial.println(topicStr);

    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, payload, length);
    if (error) {
      Serial.println("Failed to parse sensor_init_response");
      return;
    }
    
    Serial.println("Parsed sensor init response JSON:");
    serializeJsonPretty(doc, Serial);
    Serial.println();
    
    JsonArray sensors = doc["sensors"].as<JsonArray>();
    JsonArray controls = doc["controls"].as<JsonArray>();

    Serial.print("Number of sensors received: ");
    Serial.println(sensors.size());
    Serial.print("Number of controls received: ");
    Serial.println(controls.size());

    // Map sensors from the JSON response
    for (JsonObject s : sensors) {
      String type = s["type"];
      String id = s["sensor_id"];
      sensorIdMap[type] = id;
      Serial.println("Mapped sensor: " + type + " -> " + id);
    }
    
    // Optionally, log control mapping if needed
    for (JsonObject c : controls) {
      String type = c["type"];
      String id = c["control_id"];
      Serial.println("Received control: " + type + " -> " + id);
    }
    
    // Set registration flag if sensors are mapped
    if (!sensorIdMap.empty()) {
      sensorsRegistered = true;
      Serial.println("Sensor registration completed.");
    } else {
      Serial.println("No sensors mapped from sensor init response.");
    }
    return;
  }

  String waterTopic = "planthub/" + String(PLANT_MODULE_ID) + "/water";
  if (topicStr.equals(waterTopic)) {
    Serial.println("Water command received.");
    turnPumpRelayOn();
    delay(10000);
    turnPumpRelayOff();
    return;
  }
}



void setup() {
  Serial.begin(115200);
  connectToWiFi();
  initSensors();
  initPumpRelay();
  initOutletRelay();
  // Set up MQTT with the callback function
  setupMQTT(mqttCallback);
  
  // Wait until both WiFi and MQTT are connected before sending the init message
  Serial.println("Waiting for connectivity before sending sensor init message...");
  while (WiFi.status() != WL_CONNECTED || !mqttClient.connected()) {
    Serial.print(".");
    delay(500);
    if (!mqttClient.connected()) {
      // Attempt to reconnect if MQTT is not connected
      reconnectMQTT();
    }
  }
  Serial.println("\nConnectivity established. Sending sensor init message...");
  
  delay(3000);
  // Now that we are connected, send the sensor init message
  sendSensorInitMessage();
  
  // Configure time after connections are established
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
    printSensorReadings(moisture, temp, hum, light);


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
  
    Serial.println("----- Averaged Sensor Readings -----");
    Serial.print("Moisture: "); Serial.println(avgMoisture);
    Serial.print("Temperature: "); Serial.println(avgTemp);
    Serial.print("Humidity: "); Serial.println(avgHum);
    Serial.print("Light (analog): "); Serial.println(avgLight);
  
    // Only publish sensor data if registration is complete
    if (!sensorsRegistered) {
      Serial.println("Sensors not registered yet, skipping sensor data publish.");
    } else {
      if (sensorIdMap.count("moisture")) {
        publishSensorData(sensorIdMap["moisture"].c_str(), avgMoisture, timeStamp);
      } else {
        Serial.println("Moisture sensor not registered.");
      }
      if (sensorIdMap.count("temperature")) {
        publishSensorData(sensorIdMap["temperature"].c_str(), avgTemp, timeStamp);
      } else {
        Serial.println("Temperature sensor not registered.");
      }
      if (sensorIdMap.count("humidity")) {
        publishSensorData(sensorIdMap["humidity"].c_str(), avgHum, timeStamp);
      } else {
        Serial.println("Humidity sensor not registered.");
      }
      if (sensorIdMap.count("light_analog")) {
        publishSensorData(sensorIdMap["light_analog"].c_str(), avgLight, timeStamp);
      } else {
        Serial.println("Light sensor not registered.");
      }
    }
  
    // Reset counters for the next averaging cycle
    readCount = 0;
    totalMoisture = 0;
    totalTemperature = 0;
    totalHumidity = 0;
    totalLight = 0;
  }  

  // MQTT loop
  mqttClient.loop();
}