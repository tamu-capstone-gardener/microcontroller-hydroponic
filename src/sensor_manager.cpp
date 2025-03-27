// sensor_manager.cpp
#include "sensor_manager.h"
#include "config.h"
#include <DHT.h>
DHT dht(DHTPIN, DHTTYPE);

void initSensors() {
  dht.begin();
  pinMode(MOISTURE_SENSOR_PIN, INPUT);
  pinMode(LIGHT_DIGITAL_PIN, INPUT);
}

int readMoisture() {
  return analogRead(MOISTURE_SENSOR_PIN);
}

int readLightAnalog() {
  return analogRead(LIGHT_ANALOG_PIN);
}

int readLightDigital() {
  return digitalRead(LIGHT_DIGITAL_PIN);
}

float readTemperature() {
  return dht.readTemperature();
}

float readHumidity() {
  return dht.readHumidity();
}

void printSensorReadings(int moisture, float temperature, float humidity, int lightAnalog) {
    Serial.println("==== Sensor Readings ====");
    
    Serial.print("Moisture Sensor: ");
    Serial.println(moisture);
  
    Serial.print("Temperature Sensor: ");
    Serial.print(temperature);
    Serial.println(" °C");
  
    Serial.print("Humidity Sensor: ");
    Serial.print(humidity);
    Serial.println(" %");
  
    Serial.print("Light Sensor (Analog): ");
    Serial.println(lightAnalog);
  
    Serial.println("=========================");
  }
  