// mqtt_manager.cpp
#include "mqtt_manager.h"
#include "config.h"
#include <WiFiClient.h>

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void setupMQTT(void (*callback)(char*, byte*, unsigned int)) {
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(callback);
}

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (mqttClient.connect(clientId.c_str())) {
      mqttClient.subscribe("planthub/+/water");
    } else {
      delay(5000);
    }
  }
}

int publishSensorData(const char* sensor_id, int value, const char* timestamp) {
  String topic = "planthub/" + String(sensor_id) + "/sensor_data";
  String payload = "{\"value\": " + String(value) + ", \"timestamp\": \"" + timestamp + "\"}";
  return mqttClient.publish(topic.c_str(), payload.c_str());
}
