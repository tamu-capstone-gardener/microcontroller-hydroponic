// Auto-generated by setup.py

#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID     "placeholder"
#define WIFI_USERNAME "placeholder"
#define WIFI_PASSWORD "placeholder"

// MQTT Configuration
#define MQTT_SERVER   "test.mosquitto.org"
#define MQTT_PORT     1883

// Plant Module ID
#define PLANT_MODULE_ID "8c6b5dcb-6451-4bfd-b9c2-bbf18e4c449a"

// Sensor Pins
#define DHTPIN 13 // Need to make this automatically generated from the setup.py 
#define DHTTYPE 22
#define MOISTURE_PIN 39
#define TEMPERATURE_PIN 13
#define HUMIDITY_PIN 12
#define LIGHT_ANALOG_PIN 36

// Control Pins
#define RELAY_PUMP_PIN 27
#define RELAY_OUTLET_PIN 14

// NTP Config
#define NTP_SERVER          "pool.ntp.org"
#define GMT_OFFSET_SEC      0
#define DAYLIGHT_OFFSET_SEC 3600

#endif
