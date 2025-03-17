#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <SPI.h>
#include <DHT.h>
#include <time.h>

#define LIGHT_ANALOG_PIN 36  // Analog pin means GIO36
#define LIGHT_DIGITAL_PIN 39 // Digital pin
#define DHTPIN 26 // THIS MEANS GIOP26 aka pin 10
#define DHTTYPE DHT22
#define MOISTURE_SENSOR_PIN 34

float temperature, humidity;

DHT dht(DHTPIN, DHT22);

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

void printSensorData(float lightAnalogValue, float lightDigitalValue, float moistureAnalogValue, float temperature, float humidity) {
  Serial.print("Analog Light: ");
  Serial.print(lightAnalogValue);
  Serial.print(" | Digital Light: ");
  Serial.print(lightDigitalValue);
  Serial.print(" | Moisture Value: ");
  Serial.print(moistureAnalogValue);
  Serial.print(" | Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C | Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
}

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
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
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

// Setup function
void setup() {
  Serial.begin(115200);

  pinMode(LIGHT_DIGITAL_PIN, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(MOISTURE_SENSOR_PIN, INPUT);

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
  // Read from the light sensor
  int lightAnalogValue = analogRead(LIGHT_ANALOG_PIN);
  int lightDigitalValue = digitalRead(LIGHT_DIGITAL_PIN);

  // Read from Moisture Sensor  
  // Higher value means less moisture
  // Lower value means more moisture
  int moistureAnalogValue = analogRead(MOISTURE_SENSOR_PIN);

  // Read from Temperature Sensor
  temperature = dht.readTemperature();
  delay(200);
  humidity = dht.readHumidity();

  printSensorData(lightAnalogValue, lightDigitalValue, moistureAnalogValue, temperature, humidity);

  // Publish sensor values every 5 seconds
  static unsigned long lastMsg = 0;
  unsigned long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    Serial.println("Publishing sensor data...");
    struct tm timeinfo;

    if (getLocalTime(&timeinfo)) {
      strftime(timeStamp, sizeof(timeStamp), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
    }

    int status = publishToMqttServer(client, "7b2295a0-c5c7-49a1-989c-00612cf22882", lightAnalogValue, timeStamp);

    if (status) {
      Serial.println("Publish successful");
    } else {
      Serial.println("Publish failed");
    }
  }

  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);

  delay(2500); // Delay to not overheat the sensor
}
