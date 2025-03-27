// config_template.h
#ifndef CONFIG_TEMPLATE_H
#define CONFIG_TEMPLATE_H

/* 
===== Notes for New Users with ESP32: =====

1. ADC2 conflicts with WiFi (so make sure all analog read's are contained within GPIO pins 32-39)

2. GPIO pins 34-39 are analog read only (so digital reads and writes cannot occur here 
(for instance the DHT sensor does not work in this range))

3. For all digital reads and writes you are safe to use all GPIO pins apart from 34-39
    - because they are digital signals, they do not use the ADC functionality therefore not interfering with WiFi 

4. Once you have selected pins, test them using the serial monitor 

5. In our configuration we are only taking advantage of the analog light value, 
but feel free to modify the code to use the digital light instead!

*/

// WiFi Configuration
    // This is untested with networks that use 2FA or similar
#define WIFI_SSID     "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_password"

// MQTT Configuration
    // For now we are using the free and public test server as the data we are sending is not incredibly dangerous
#define MQTT_SERVER "test.mosquitto.org"
#define MQTT_PORT   1883

// Sensor Pins
    // See the notes above if you have any trouble 
#define LIGHT_ANALOG_PIN     36  // Analog pin for light sensor
#define LIGHT_DIGITAL_PIN    26  // Digital pin for light sensor
#define DHTPIN               13  // DHT sensor pin
#define DHTTYPE              DHT22
#define MOISTURE_SENSOR_PIN  39
#define RELAY_PUMP_PIN       27
#define RELAY_OUTLET_PIN     14

// Sensor and Module IDs
    // Get these values from the website (should correspond to the plant module you want to monitor) 
#define PLANT_MODULE_ID "c9dc166b-1279-4d60-901a-091b2c172859"
#define MOISTURE_SENSOR_ID "c69a9baa-d53f-458b-88e0-a3ca1975069c"
#define TEMPERATURE_SENSOR_ID "159fe615-f1dc-4702-acec-1d0cee973010"
#define HUMIDITY_SENSOR_ID "d0ad3ecd-5046-4e52-82f8-373724d2bd30"
#define LIGHT_SENSOR_ID "d0ad3ecd-5046-4e52-82f8-373724d2bd30" // same as humidity because as of right now there is no humidity on the website 

// NTP Config
#define NTP_SERVER          "pool.ntp.org"
#define GMT_OFFSET_SEC      0
#define DAYLIGHT_OFFSET_SEC 3600

#endif
