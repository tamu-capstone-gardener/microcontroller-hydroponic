// mqtt_manager.cpp
#include "mqtt_manager.h"
#include "config.h"
#include <WiFiClient.h>


WiFiClient espClient;
PubSubClient mqttClient(espClient);

void setupMQTT(void (*callback)(char*, byte*, unsigned int)) {
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(callback);
  mqttClient.setBufferSize(2048);
}

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (mqttClient.connect(clientId.c_str())) {
      String subscription = "planthub/" + String(PLANT_MODULE_ID) + "/+";
      mqttClient.subscribe(subscription.c_str());

    } else {
      delay(5000);
    }
  }
}

int publishSensorData(const char* sensor_id, int value, const char* timestamp) {
  String topic = "planthub/" + String(sensor_id) + "/sensor_data";
  String payload = "{\"value\": " + String(value) + ", \"timestamp\": \"" + timestamp + "\"}";
  
  // Serial logging
  Serial.print("Publishing: ");
  Serial.print(value);
  Serial.print(" at ");
  Serial.print(timestamp);
  Serial.print(" for sensor: ");
  Serial.println(sensor_id);
  
  return mqttClient.publish(topic.c_str(), payload.c_str());
}

void sendSensorInitMessage() {
  Serial.println("Starting sensor init message...");

  if (!FILESYSTEM.begin(true)) {
    Serial.println("Filesystem mount failed");
    return;
  }

  File file = FILESYSTEM.open("/sensors.json", "r");
  if (!file) {
    Serial.println("Failed to open sensors.json");
    return;
  }

  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    Serial.println("Failed to parse sensors.json");
    return;
  }

  String out;
  serializeJson(doc, out);
  String topic = "planthub/" + String(PLANT_MODULE_ID) + "/init_sensors";

  if (!mqttClient.connected()) {
    Serial.println("MQTT not connected — reconnecting before publish.");
    reconnectMQTT();
    delay(1000); // Give a brief moment after reconnecting
  }

  // Print state before publishing
  Serial.print("MQTT state before publish: ");
  Serial.println(mqttClient.state());

  bool success = mqttClient.publish(topic.c_str(), out.c_str());
  
  if (!success) {
    int state = mqttClient.state();
    Serial.print("MQTT publish failed, state: ");
    Serial.println(state);
  }

  Serial.println("Sent sensor init message:");
  Serial.println("Topic: " + topic);
  Serial.println("Payload: " + out);
  Serial.println(success ? "✅ MQTT publish succeeded" : "❌ MQTT publish FAILED");
}



