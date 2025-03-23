#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <DHT.h>

// Sensor Pins
#define LIGHT_ANALOG_PIN 26  // Analog pin
#define LIGHT_DIGITAL_PIN 14 // Digital pin
#define DHTPIN 26            // Temperature sensor
#define DHTTYPE DHT22
#define MOISTURE_SENSOR_PIN 2
#define RELAY_PIN 15

float temperature, humidity;
bool manualOverride = false; // Default: auto mode

DHT dht(DHTPIN, DHT22);

void setup() {
  Serial.begin(115200);
  pinMode(LIGHT_DIGITAL_PIN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(MOISTURE_SENSOR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  dht.begin();
}

void loop() {
  // Read moisture level
  int moistureAnalogValue = analogRead(MOISTURE_SENSOR_PIN);

  // Read temperature and humidity
  temperature = dht.readTemperature();
  delay(200);
  humidity = dht.readHumidity();

  Serial.print("Moisture: ");
  Serial.print(moistureAnalogValue);
  Serial.print(" | Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C | Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  // Check for user input in Serial Monitor
  if (Serial.available() > 0) {
    char command = Serial.read();
    
    if (command == '1') {
      manualOverride = true;
      Serial.println("Manual Mode: Water Pump ON");
      digitalWrite(RELAY_PIN, HIGH);
    } 
    else if (command == '0') {
      manualOverride = true;
      Serial.println("Manual Mode: Water Pump OFF");
      digitalWrite(RELAY_PIN, LOW);
    } 
    else if (command == 'a') { // Return to automatic mode
      manualOverride = false;
      Serial.println("Automatic Mode: Soil moisture control enabled.");
    }
  }

  // Only control the pump automatically if manual override is OFF
  if (!manualOverride) {
    if (moistureAnalogValue > 2000) { 
      Serial.println("Soil is dry. Turning ON water pump.");
      digitalWrite(RELAY_PIN, HIGH);
    } else {
      Serial.println("Soil is moist. Turning OFF water pump.");
      digitalWrite(RELAY_PIN, LOW);
    }
  }

  // Blink LED
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);

  delay(1500);
}
