## README

This is the microcontroller code that runs on an ESP32. It communicates via MQTT to our web app in order to upload data and receive control signals, such as water the plants. In this codebase you will find setup code (.py), the ESP-32 code itself (.cpp and .h), and PlatformIO specific code.  

## How to configure, run, and push code to the ESP-32

- Clone this repository
- Download and/or run VS Code
- Open the repository folder in VS Code
- Download the PlatformIO extension (for communicating with the ESP-32 like an arduino)
- Run the setup.py and push the saved data with in your terminal `pio run --target uploadfs` 
- Plug in power to the ESP-32 via USB-C
- Click checkmark on VS Code (build)
- Before clicking upload, press and hold boot button
- Click right arrow on VS Code (Upload)
- Let go of boot button once 'write to memory'
- Open Serial Monitor on your USB port at 115200 baud rate to read data
- Once you have made any modifications, tested, and understood the code, simply unplug the USB-C cable from your computer and into an outlet or an 'Always On' part of your outlet relay (if applicable)

## ESP-32 Pin Selection Guide for PlantHub

This guide will help you choose the right pins when configuring your ESP-32-based PlantHub module.

---

### General Notes

1. **Avoid ADC2 Pins for Analog Reads**
   - ADC2 conflicts with WiFi, so do not use it for analog reads.
   - Use GPIO 32–39 for analog sensors — these are on ADC1.

2. **GPIO 34–39 Are Analog-Only**
   - These pins can only be used for analog reads.
   - They cannot be used for digital input/output (e.g., DHT sensors won't work here).

3. **Safe Pins for Digital Read/Write**
   - You can safely use any GPIO pin except 34–39 for digital signals (like relays or DHT sensors).
   - These do not use ADC, so they won't interfere with WiFi.

4. **Test Your Pins**
   - After assigning pins, test them using the Serial Monitor before deploying.
   - This ensures your sensor is wired and responding correctly.

5. **Light Sensor Options**
   - Our default setup uses the analog value from the light sensor.
   - You can modify the code to use the digital signal instead, if preferred.

---

### Summary Table

| Pin Range | Type               | Usage Notes                            |
|-----------|--------------------|----------------------------------------|
| 32–39     | Analog (ADC1)      | Safe for analog reads + WiFi friendly |
| 34–39     | Analog-Only        | No digital I/O                         |
| Others    | Digital Read/Write | Safe for digital input/output         |

---

Tip: Use `analogRead()` for moisture/light sensors, and `digitalWrite()` or `digitalRead()` for relays, buttons, or digital light inputs.

## Link to other repos

| Repository Name                | Description                                                   | Link                                                                 |
|--------------------------------|---------------------------------------------------------------|----------------------------------------------------------------------|
| Ruby on Rails Server           | Web server application for our project built with Ruby on Rails | [Repository](https://github.com/tamu-capstone-gardener/rails-react)   |
| Microcontroller Code for ESP-32 | Hardware code that runs on the ESP-32                          | [Repository](https://github.com/tamu-capstone-gardener/microcontroller)|
| Team Agreement                 | Team agreement that we all signed and agreed to               | [Repository](https://github.com/tamu-capstone-gardener/team-agreement) |
| Team/Individuals Reports       | Contains all team and individual reports                      | [Repository](https://github.com/tamu-capstone-gardener/reports)        |