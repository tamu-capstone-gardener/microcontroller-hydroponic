import json

# TODO:
    # Add a configurator for DHT as opposed to two separate temp. and humidity sensors

config_template = '''\
// Auto-generated by setup.py

#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID     "YOUR_SSID"
#define WIFI_USERNAME "YOUR_USERNAME" // only needed for secure (typically non-personal) networks
#define WIFI_PASSWORD "YOUR_PASSWORD"

// MQTT Configuration
#define MQTT_SERVER   "test.mosquitto.org"
#define MQTT_PORT     1883

// Plant Module ID
#define PLANT_MODULE_ID "{plant_module_id}"

// Sensor Pins
#define DHTPIN 13 // TODO: Need to make this automatically generated from the setup.py 
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

default_sensors = {
    "MOISTURE": 39,
    "TEMPERATURE": 13,
    "HUMIDITY": 12,
    "LIGHT_ANALOG": 36
}

default_controls = {
    "RELAY_PUMP": 27,
    "RELAY_OUTLET": 14
}

def format_define(name, value):
    return f"#define {name}_PIN {value}"

def main():
    print("🌿 Welcome to PlantHub ESP32 Configurator")
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
            active_controls.append(name.replace("RELAY_", "").lower())

    # Write config.h
    with open("include/config.h", "w") as f:
        f.write(config_template.format(
            plant_module_id=plant_module_id,
            sensor_pins="\n".join(sensor_pins),
            control_pins="\n".join(control_pins)
        ))

    print("✅ Wrote config.h")

    # Write sensors.json for config broadcast
    with open("data/sensors.json", "w") as f:
        json.dump({
            "plant_module_id": plant_module_id,
            "sensors": [ {"type": s} for s in active_sensors ],
            "controls": [ {"type": c} for c in active_controls ]
        }, f, indent=2)

    print("✅ Wrote sensors.json")

if __name__ == "__main__":
    main()
