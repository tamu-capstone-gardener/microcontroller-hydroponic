#include "sensor_manager.h"
#include "config.h"
#include <DHT.h>

DHT dht(DHTPIN, DHTTYPE);

void initSensors() {
  dht.begin();
  pinMode(TDS_PIN, INPUT);
  pinMode(FLOAT_PIN, INPUT_PULLUP); // or INPUT, depending on your float sensor
  pinMode(LIGHT_ANALOG_PIN, INPUT_PULLUP); 
  // If the digital part of your light sensor is used as a threshold or interrupt, that is optional
}

float readTDS() {
  // In many TDS sensor circuits, you get an analog voltage proportional to TDS.
  // For demonstration, just read raw analog value. Real TDS circuits might require additional scaling.
  return (float)analogRead(TDS_PIN);
}

bool readFloatSensor() {
  // If the float sensor is closed-circuit when water is at a certain level, read the digital pin
  int reading = digitalRead(FLOAT_PIN);
  return (reading == LOW); 
  // or (reading == HIGH), depends on the sensor wiring 
}

int readLightAnalog() {
  return analogRead(LIGHT_ANALOG_PIN);
}

float readTemperature() {
  return dht.readTemperature();
}

float readHumidity() {
  return dht.readHumidity();
}

void printSensorReadings(float tds, bool floatSensor, float temperature, float humidity, int lightAnalog) {
    Serial.println("==== Sensor Readings ====");
    
    Serial.print("TDS (Raw Analog): ");
    Serial.println(tds);

    Serial.print("Float Sensor State: ");
    Serial.println(floatSensor ? "FLOAT IS ACTIVE" : "FLOAT IS NOT ACTIVE");

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
