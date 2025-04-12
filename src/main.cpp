#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <functional>
#include <map>
#include "config.h"
#include "wifi_manager.h"
#include "mqtt_manager.h"
#include "sensor_manager.h"
#include "relay_controller.h"

/* TODO 
1. Get scheduling of lights and water working
2. Fix threshold to work with hydroponic sensors
3. Get camera working
4. Maybe fix the relay? Still have to use voltmeter to short it sometimes 
5. Control the outlet separately from the pumps (when needed)
*/

// Global time stamp buffer
char timeStamp[25];

// Misc. global flags and variables
bool manualOverride = false;
unsigned long autoModePauseUntil = 0;
float temperature, humidity;

// Sensor reading interval and averaging
const int READ_INTERVAL_MS = 12000;  // 12 seconds between reads (or 5 readings per minute)
const int NUM_SAMPLES = 5;
unsigned long lastReadTime = 0;
int readCount = 0;

// Accumulators for averaged sensor readings
// NOTE: The "moisture" variable is now used for the TDS sensor reading.
int totalMoisture = 0;      
float totalTemperature = 0;
float totalHumidity = 0;
int totalLight = 0;

// Mapping from sensor type to sensor IDs received from the Rails-side
std::map<String, String> sensorIdMap;
bool sensorsRegistered = false;  // Global flag

// Global state variables for toggling (still used for non-fertilizer controls)
bool pumpIsOn = false;
bool lightsAreOn = false;
bool outletIsOn = false;

// Define a type alias for a function that takes no parameters.
using RelayFunction = std::function<void(void)>;

// Structure to hold a control type's details.
struct Control {
  String type;         // e.g., "water", "lights", "outlet"
  RelayFunction on;    // Function to turn the control on.
  RelayFunction off;   // Function to turn the control off.
  bool* state;         // Pointer to a state variable for toggling.
};

// List of controls.
std::vector<Control> controlsList;

// Populate the list of controls.
void setupControlsList() {
  controlsList.push_back({
    "pump",  // This legacy control may be used for testing; however, fertilizer control is now handled separately.
    []() { 
      Serial.println("Turning pump ON"); 
      turnPumpNOn(); 
    },
    []() { 
      Serial.println("Turning pump OFF"); 
      turnPumpNOff(); 
    },
    &pumpIsOn
  });

  controlsList.push_back({
    "outlet",  // In the hydroponic system, the outlet controls the grow light.
    []() { 
      Serial.println("Turning outlet (light) ON"); 
      turnLightOn(); 
    },
    []() { 
      Serial.println("Turning outlet (light) OFF"); 
      turnLightOff(); 
    },
    &outletIsOn
  });
}

// --- NEW: Fertilizer Routine Handler ---
//
// This function handles the "fertilizer_routine" MQTT command.
// It expects a JSON payload with the following format:
//   {
//     "mix_1": <mL of Nitrogen>,
//     "mix_2": <mL of Phosphorus>,
//     "mix_3": <mL of Potassium>
//   }
//
// The pump on-time is computed by multiplying the requested mL by a calibration factor (ms per mL).
//
void handleFertilizerRoutine(StaticJsonDocument<256>& doc) {
  int mix1 = doc["mix_1"] | 0;  // Nitrogen (mL)
  int mix2 = doc["mix_2"] | 0;  // Phosphorus (mL)
  int mix3 = doc["mix_3"] | 0;  // Potassium (mL)

  Serial.println("=== Fertilizer Routine Command ===");
  Serial.print("Nitrogen (mix_1): ");
  Serial.println(mix1);
  Serial.print("Phosphorus (mix_2): ");
  Serial.println(mix2);
  Serial.print("Potassium (mix_3): ");
  Serial.println(mix3);

  // Convert mL to milliseconds using calibration constants defined in config.h:
  unsigned long timeN = mix1 * MS_PER_ML_N; 
  unsigned long timeP = mix2 * MS_PER_ML_P;
  unsigned long timeK = mix3 * MS_PER_ML_K;

  // Sequentially run each pump with a small pause in between.
  if (mix1 > 0) {
    Serial.print("Dispensing Nitrogen for ");
    Serial.print(timeN);
    Serial.println(" ms");
    turnPumpNOn();
    delay(timeN);
    turnPumpNOff();
    delay(1000);  // 1 sec pause
  }
  if (mix2 > 0) {
    Serial.print("Dispensing Phosphorus for ");
    Serial.print(timeP);
    Serial.println(" ms");
    turnPumpPOn();
    delay(timeP);
    turnPumpPOff();
    delay(1000);
  }
  if (mix3 > 0) {
    Serial.print("Dispensing Potassium for ");
    Serial.print(timeK);
    Serial.println(" ms");
    turnPumpKOn();
    delay(timeK);
    turnPumpKOff();
    delay(1000);
  }
  
  Serial.println("Fertilizer routine completed.");
}

// Generic control command handler that loops through controlsList.
void handleControlCommand(String controlType, StaticJsonDocument<256>& doc) {
  // Retrieve parameters from JSON.
  bool toggle = doc["toggle"] | false;
  int duration = doc["duration"] | 10000;  // Default duration: 10 seconds.
  
  Serial.print("Handling command for control type: ");
  Serial.println(controlType);
  
  // Loop through the list to find the matching control.
  for (auto &ctrl : controlsList) {
    if (ctrl.type.equals(controlType)) {
      if (toggle) {
        // Toggle the control's state.
        *(ctrl.state) = !(*(ctrl.state));
        if (*(ctrl.state)) {
          Serial.print("Toggling ");
          Serial.print(ctrl.type);
          Serial.println(": turning ON");
          ctrl.on();
        } else {
          Serial.print("Toggling ");
          Serial.print(ctrl.type);
          Serial.println(": turning OFF");
          ctrl.off();
        }
      } else {
        // Activate the control for a set duration.
        Serial.print("Activating ");
        Serial.print(ctrl.type);
        Serial.print(" for ");
        Serial.print(duration);
        Serial.println(" ms");
        ctrl.on();
        delay(duration);
        ctrl.off();
      }
      // Found and processed the control; exit the loop.
      return;
    }
  }
  
  Serial.print("Control type ");
  Serial.print(controlType);
  Serial.println(" not found.");
}

// MQTT callback function.
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String topicStr(topic);

  // Process sensor init response.
  String initResponseTopic = "planthub/" + String(PLANT_MODULE_ID) + "/sensor_init_response";
  if (topicStr.equals(initResponseTopic)) {
    Serial.println("Sensor init response received on topic:");
    Serial.println(topicStr);

    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, payload, length);
    if (error) {
      Serial.println("Failed to parse sensor_init_response");
      return;
    }
    
    Serial.println("Parsed sensor init response JSON:");
    serializeJsonPretty(doc, Serial);
    Serial.println();
    
    JsonArray sensors = doc["sensors"].as<JsonArray>();
    JsonArray controls = doc["controls"].as<JsonArray>();

    Serial.print("Number of sensors received: ");
    Serial.println(sensors.size());
    Serial.print("Number of controls received: ");
    Serial.println(controls.size());

    // Map sensors from the JSON response.
    for (JsonObject s : sensors) {
      String type = s["type"];
      String id = s["sensor_id"];
      sensorIdMap[type] = id;
      Serial.println("Mapped sensor: " + type + " -> " + id);
    }
    
    // Log control mapping.
    for (JsonObject c : controls) {
      String type = c["type"];
      String id = c["control_id"];
      Serial.println("Received control: " + type + " -> " + id);
    }
    
    // Set registration flag if sensors are mapped.
    if (!sensorIdMap.empty()) {
      sensorsRegistered = true;
      Serial.println("Sensor registration completed.");
    } else {
      Serial.println("No sensors mapped from sensor init response.");
    }
    return;
  }

  // Ignore sensor data messages.
  if (topicStr.indexOf("sensor_data") != -1) {
    return;
  }
  
  // Otherwise, assume the topic is a control command.
  // Expected format: planthub/<plant_module_id>/<control_type>
  int lastSlash = topicStr.lastIndexOf('/');
  if (lastSlash == -1) {
    Serial.println("Invalid topic format for control signal");
    return;
  }
  String controlType = topicStr.substring(lastSlash + 1);
  Serial.print("Received control command for: ");
  Serial.println(controlType);
  
  // Parse the JSON payload.
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, payload, length);
  if (error) {
    Serial.println("Failed to parse control command JSON");
    return;
  }
  
  // If the control command is for a fertilizer routine, handle it separately.
  if (controlType.equals("fertilizer_routine")) {
    handleFertilizerRoutine(doc);
    return;
  }
  
  // Otherwise, dispatch the command by looping through our controls list.
  handleControlCommand(controlType, doc);
}

void setup() {
  Serial.begin(115200);
  connectToWiFi();
  initSensors();

  // For the hydroponic setup, initialize the three fertilizer pump relays and the light relay.
  initPumpN();
  initPumpP();
  initPumpK();
  initLightRelay();
  
  // Initialize MQTT with our callback.
  setupMQTT(mqttCallback);
  
  // Initialize our controls list.
  setupControlsList();
  
  Serial.println("Waiting for connectivity before sending sensor init message...");
  while (WiFi.status() != WL_CONNECTED || !mqttClient.connected()) {
    Serial.print(".");
    delay(500);
    if (!mqttClient.connected()) {
      reconnectMQTT();
    }
  }
  Serial.println("\nConnectivity established. Sending sensor init message...");
  delay(3000);
  sendSensorInitMessage();
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
}

void loop() {
  if (!mqttClient.connected()) reconnectMQTT();
  mqttClient.loop();

  unsigned long now = millis();

  // Every 12 seconds, take a reading.
  if (now - lastReadTime >= READ_INTERVAL_MS) {
    lastReadTime = now;

    // Instead of reading "moisture", we now read the TDS sensor value.
    // For compatibility, we still call the variable "moisture".
    int moisture = readTDS();
    float temp = readTemperature();
    float hum = readHumidity();
    int light = readLightAnalog();

    // Print sensor readings (note that "moisture" now represents TDS).
    printSensorReadings(moisture, temp, hum, light);

    totalMoisture += moisture;
    totalTemperature += temp;
    totalHumidity += hum;
    totalLight += light;
    readCount++;

    Serial.print("Collected reading ");
    Serial.println(readCount);
  }

  // After 5 readings, average and publish.
  if (readCount >= NUM_SAMPLES) {
    int avgMoisture = totalMoisture / NUM_SAMPLES;
    float avgTemp = totalTemperature / NUM_SAMPLES;
    float avgHum = totalHumidity / NUM_SAMPLES;
    int avgLight = totalLight / NUM_SAMPLES;
  
    // Get time stamp.
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      strftime(timeStamp, sizeof(timeStamp), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
    }
  
    Serial.println("----- Averaged Sensor Readings -----");
    Serial.print("TDS (moisture): "); Serial.println(avgMoisture);
    Serial.print("Temperature: "); Serial.println(avgTemp);
    Serial.print("Humidity: "); Serial.println(avgHum);
    Serial.print("Light (analog): "); Serial.println(avgLight);
  
    // Only publish sensor data if registration is complete.
    if (!sensorsRegistered) {
      Serial.println("Sensors not registered yet, skipping sensor data publish.");
    } else {
      if (sensorIdMap.count("moisture")) {
        publishSensorData(sensorIdMap["moisture"].c_str(), avgMoisture, timeStamp);
      } else {
        Serial.println("Moisture sensor not registered.");
      }
      if (sensorIdMap.count("temperature")) {
        publishSensorData(sensorIdMap["temperature"].c_str(), avgTemp, timeStamp);
      } else {
        Serial.println("Temperature sensor not registered.");
      }
      if (sensorIdMap.count("humidity")) {
        publishSensorData(sensorIdMap["humidity"].c_str(), avgHum, timeStamp);
      } else {
        Serial.println("Humidity sensor not registered.");
      }
      if (sensorIdMap.count("light_analog")) {
        publishSensorData(sensorIdMap["light_analog"].c_str(), avgLight, timeStamp);
      } else {
        Serial.println("Light sensor not registered.");
      }
    }
  
    // Reset counters for the next averaging cycle.
    readCount = 0;
    totalMoisture = 0;
    totalTemperature = 0;
    totalHumidity = 0;
    totalLight = 0;
  }  

  // Continue MQTT loop.
  mqttClient.loop();
}
