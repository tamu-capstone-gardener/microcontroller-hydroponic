#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define LIGHT_ANALOG_PIN 13  // Analog pin
#define LIGHT_DIGITAL_PIN 12 // Digital pin

// Replace these with your WiFi network credentials
const char* ssid = "placeholder";
const char* password = "placeholder";

// MQTT Broker details for HiveMQ Public Broker
/*const char* mqtt_server = "broker.hivemq.com";  // Free Public MQTT Broker by HiveMQ
const int mqtt_port = 1883;                     // TCP Port: 1883 (unencrypted) */

WiFiClient espClient;
PubSubClient client(espClient);

// Callback function to handle incoming MQTT messages
/*void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
} */

// Connect to WiFi
/*void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to WiFi network: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}*/

// Reconnect to the MQTT broker if the connection is lost
/*void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("esp32/command");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" - retrying in 5 seconds");
      delay(5000);
    }
  }
}*/

void setup() {
  Serial.begin(9600);

  pinMode(LIGHT_DIGITAL_PIN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  //setup_wifi();
  
  //client.setServer(mqtt_server, mqtt_port);
  //client.setCallback(callback);
}

void loop() {
  /*if (!client.connected()) {
    reconnect();
  }
  client.loop(); */

  // Read from the light sensor
  int analogValue = analogRead(LIGHT_ANALOG_PIN);
  int digitalValue = digitalRead(LIGHT_DIGITAL_PIN);

  // Display values on Serial Monitor
  Serial.print("Analog Value: ");
  Serial.print(analogValue);
  Serial.print(" | Digital Value: ");
  Serial.println(digitalValue);

  // Publish sensor values every 5 seconds
  /*static unsigned long lastMsg = 0;
  unsigned long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    
    String analogMsg = "Analog: " + String(analogValue);
    String digitalMsg = "Digital: " + String(digitalValue);

    Serial.println("Publishing sensor data...");
    client.publish("esp32/light/analog", analogMsg.c_str());
    client.publish("esp32/light/digital", digitalMsg.c_str());
  } */

  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
}
