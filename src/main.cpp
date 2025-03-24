#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <SPI.h>
#include <DHT.h>
#include <time.h>

// Sensor Pins
#define LIGHT_ANALOG_PIN 36  // Analog pin
#define LIGHT_DIGITAL_PIN 26 // Digital pin
#define DHTPIN 13            // Temperature sensor
#define DHTTYPE DHT22
#define MOISTURE_SENSOR_PIN 39
#define RELAY_PUMP_PIN 27
#define RELAY_OUTLET_PIN 14

// ID's for the server
#define PLANT_MODULE_ID "c9dc166b-1279-4d60-901a-091b2c172859"
#define MOISTURE_SENSOR "c69a9baa-d53f-458b-88e0-a3ca1975069c"
#define TEMPERATURE_SENSOR "159fe615-f1dc-4702-acec-1d0cee973010"
#define HUMIDITY_SENSOR "d0ad3ecd-5046-4e52-82f8-373724d2bd30"
#define LIGHT_SENSOR "d0ad3ecd-5046-4e52-82f8-373724d2bd30" // same as humidity because as of right now there is no humidity on the website 

/* TODO 

1. Get scheduling of lights and water working
2. Fix threshold to work with soil
3. Get camera working
4. Maybe fix the relay? still have to use voltmeter to short it sometimes 

*/

float temperature, humidity;
bool manualOverride = false; // Default: auto mode
unsigned long autoModePauseUntil = 0;


const char* ssid = "placeholder";
const char* password = "placeholder";

const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883;

char timeStamp[25] = "0000-00-00T00:00:00Z";
 
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

WiFiClient espClient;
PubSubClient client(espClient);


DHT dht(DHTPIN, DHT22);

int publishToMqttServer(PubSubClient& client, const char* sensor_id, int value, const char* timeStamp) {
  String topic = "planthub/";
  topic += sensor_id;
  topic += "/sensor_data";

  String payload = "{\"value\": " + String(value) + ", \"timestamp\": \"" + timeStamp + "\"}";

  bool published = client.publish(topic.c_str(), payload.c_str());
  return published ? 1 : 0;
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.println(topic);
  Serial.print("Message: ");

  String message;
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    message += (char)payload[i];
  }
  Serial.println();

  // Check if the topic matches our plant module ID
  String expectedTopic = String("planthub/") + PLANT_MODULE_ID + "/water";
  if (String(topic) == expectedTopic) {
    // Parse simple JSON: {"water": true}
    // for now we don't check the value as if you receive a signal it means to water
    manualOverride = true;
    autoModePauseUntil = millis() + 15000UL;  // 15 seconds from now (debounce so the pump doesn't stop watering) 
    Serial.println("MQTT: Water signal received. Turning ON water pump.");
    digitalWrite(RELAY_PUMP_PIN, HIGH);
    digitalWrite(RELAY_OUTLET_PIN, HIGH);
    delay(1500); // right now only watering for 1.5 seconds need to play 
    digitalWrite(RELAY_PUMP_PIN, LOW);
    digitalWrite(RELAY_OUTLET_PIN, LOW);
  }
}


// Connect to WiFi
void setup_wifi() {
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
}

// Reconnect to the MQTT broker if the connection is lost
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("planthub/+/water");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" - retrying in 5 seconds");
      delay(5000);
    }
  }
}  

void setup() {
  Serial.begin(115200);
  pinMode(LIGHT_DIGITAL_PIN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(MOISTURE_SENSOR_PIN, INPUT);
  pinMode(RELAY_PUMP_PIN, OUTPUT);
  pinMode(RELAY_OUTLET_PIN, OUTPUT);
  dht.begin();

  setup_wifi();
  
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Read moisture level
  int moistureAnalogValue = analogRead(MOISTURE_SENSOR_PIN);

  // Read temperature and humidity
  temperature = dht.readTemperature();
  delay(200);
  humidity = dht.readHumidity();

  int lightAnalogValue = analogRead(LIGHT_ANALOG_PIN);
  int lightDigitalValue = digitalRead(LIGHT_DIGITAL_PIN);

  unsigned long now = millis();
  static unsigned long lastMsg = now;
  Serial.println("Publishing sensor data...");
  struct tm timeinfo;

  if (getLocalTime(&timeinfo)) {
    strftime(timeStamp, sizeof(timeStamp), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
  }

  Serial.print("Moisture: ");
  Serial.print(moistureAnalogValue);
  publishToMqttServer(client, MOISTURE_SENSOR, moistureAnalogValue, timeStamp);
  Serial.print(" | Temperature: ");
  Serial.print(temperature);
  publishToMqttServer(client, TEMPERATURE_SENSOR, temperature, timeStamp);
  Serial.print(" °C | Humidity: ");
  Serial.print(humidity);
  Serial.print(" %");
  publishToMqttServer(client, HUMIDITY_SENSOR, humidity, timeStamp);
  Serial.print(" | Light Analog: ");
  Serial.print(lightAnalogValue);
  Serial.print(" | Light Digital: ");
  Serial.println(lightDigitalValue);
  publishToMqttServer(client, LIGHT_SENSOR, lightAnalogValue, timeStamp);


  // Check for user input in Serial Monitor
  if (Serial.available() > 0) {
    char command = Serial.read();
    
    if (command == '1') {
      manualOverride = true;
      Serial.println("Manual Mode: Water Pump ON");
      digitalWrite(RELAY_PUMP_PIN, HIGH);
      digitalWrite(RELAY_OUTLET_PIN, HIGH);
    } 
    else if (command == '0') {
      manualOverride = true;
      Serial.println("Manual Mode: Water Pump OFF");
      digitalWrite(RELAY_PUMP_PIN, LOW);
      digitalWrite(RELAY_OUTLET_PIN, LOW);
    } 
    else if (command == 'a') { // Return to automatic mode
      manualOverride = false;
      Serial.println("Automatic Mode: Soil moisture control enabled.");
    }
  }

  // Only control the pump automatically if manual override is OFF and the pause isn't on from remote watering
  if (!manualOverride && millis() > autoModePauseUntil) {
    if (moistureAnalogValue > 2000) { 
      Serial.println("Soil is dry. Turning ON water pump.");
      digitalWrite(RELAY_PUMP_PIN, HIGH);
      digitalWrite(RELAY_OUTLET_PIN, HIGH);
    } else {
      Serial.println("Soil is moist. Turning OFF water pump.");
      digitalWrite(RELAY_PUMP_PIN, LOW);
      digitalWrite(RELAY_OUTLET_PIN, LOW);
    }
  }

  // Blink LED
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);

  delay(2000);
}
