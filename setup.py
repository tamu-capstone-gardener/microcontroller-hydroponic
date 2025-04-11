import json

# Auto-generated configuration template for Hydroponic PlantHub ESP32.
# This header file is generated by setup.py and will contain configuration
# for WiFi, MQTT, sensors, and control signal pins for the hydroponic system.

config_template = '''\
// Auto-generated by setup.py for Hydroponic PlantHub ESP32

#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID     "YOUR_SSID"
#define WIFI_USERNAME "YOUR_USERNAME" // Only needed for secure networks
#define WIFI_PASSWORD "YOUR_PASSWORD"

// MQTT Configuration
#define MQTT_SERVER   "test.mosquitto.org"
#define MQTT_PORT     1883

// Plant Module ID
#define PLANT_MODULE_ID "{plant_module_id}"

// Sensor Pins
// DHT sensor for Temperature & Humidity
#define DHTPIN 13
#define DHTTYPE 22
{sensor_pins}

// Control Pins
{control_pins}

// NTP Config
#define NTP_SERVER          "pool.ntp.org"
#define GMT_OFFSET_SEC      0
#define DAYLIGHT_OFFSET_SEC 3600

#endif
'''

# Update default sensors for Hydroponics:
# - TDS: used in place of the old moisture sensor (default on pin 34)
# - FLOAT: water level sensor (default on pin 11)
# - LIGHT_ANALOG: analog input from the light sensor (default on pin 36)
default_sensors = {
    "TDS": 34,
    "FLOAT": 11,
    "LIGHT_ANALOG": 36
}

# Update default controls for Hydroponics:
# - RELAY_N: Nitrogen pump relay (default on pin 25)
# - RELAY_P: Phosphorus pump relay (default on pin 26)
# - RELAY_K: Potassium pump relay (default on pin 27)
# - RELAY_LIGHT: Grow light relay (default on pin 10)
default_controls = {
    "RELAY_N": 25,
    "RELAY_P": 26,
    "RELAY_K": 27,
    "RELAY_LIGHT": 10
}

def format_define(name, value):
    return f"#define {name}_PIN {value}"

def main():
    print("🌿 Welcome to PlantHub ESP32 Hydroponic Configurator")
    plant_module_id = input("Enter your Plant Module UUID (from Rails): ").strip()

    sensor_pins = []
    control_pins = []
    active_sensors = []
    active_controls = []

    print("\nConfigure SENSORS:")
    for name, default in default_sensors.items():
        use = input(f"Include {name}? [Y/n]: ").strip().lower()
        if use in ["", "y", "yes"]:
            pin = input(f"  GPIO pin for {name} (default {default}): ").strip() or str(default)
            sensor_pins.append(format_define(name, pin))
            active_sensors.append(name.lower())

    print("\nConfigure CONTROL SIGNALS:")
    for name, default in default_controls.items():
        use = input(f"Include {name}? [Y/n]: ").strip().lower()
        if use in ["", "y", "yes"]:
            pin = input(f"  GPIO pin for {name} (default {default}): ").strip() or str(default)
            control_pins.append(format_define(name, pin))
            # Remove the "RELAY_" prefix for the sensor broadcast configuration
            active_controls.append(name.replace("RELAY_", "").lower())

    # Write config.h to the include/ directory.
    with open("include/config.h", "w") as f:
        f.write(config_template.format(
            plant_module_id=plant_module_id,
            sensor_pins="\n".join(sensor_pins),
            control_pins="\n".join(control_pins)
        ))
    print("✅ Wrote config.h")

    # Write sensors.json for configuration broadcast.
    with open("data/sensors.json", "w") as f:
        json.dump({
            "plant_module_id": plant_module_id,
            "sensors": [ {"type": s} for s in active_sensors ],
            "controls": [ {"type": c} for c in active_controls ]
        }, f, indent=2)
    print("✅ Wrote sensors.json")

if __name__ == "__main__":
    main()
