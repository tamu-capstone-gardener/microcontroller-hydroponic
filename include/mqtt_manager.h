// mqtt_manager.h
#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#define FILESYSTEM LittleFS

extern PubSubClient mqttClient;
void setupMQTT(void (*callback)(char*, byte*, unsigned int));
void reconnectMQTT();
int publishSensorData(const char* sensor_id, int value, const char* timestamp);
void sendSensorInitMessage();
#endif
