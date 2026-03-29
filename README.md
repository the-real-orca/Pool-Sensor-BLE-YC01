# Pool Sensor BLE-YC01 Project Overview

## Project Description
The **Pool Sensor BLE-YC01** project is an ESP32-based IoT application designed to monitor swimming pool water quality. It communicates with a BLE-YC01 6-in-1 sensor to read values such as pH, Chlorine (Cl), ORP, EC, Salt, TDS, and Temperature. The collected data is then transmitted to an MQTT broker for further processing and visualization (e.g., in Home Assistant) and is also accessible via a built-in web server.

## Key Features
- **BLE-YC01 Integration:** Automated scanning and data retrieval from BLE-YC01 sensors.
- **Custom Data Decoding:** Specialized algorithm to decode the proprietary BLE data format.
- **Connectivity Options:**
  - **WiFi:** Supports station mode and a captive portal for easy configuration.
  - **MQTT:** Publishes sensor data to a broker with optional TLS encryption.
  - **Web Server:** Real-time status dashboard, configuration management, and file system access.
- **OTA Updates:** Support for firmware and file system updates over the air.
- **Configuration:** Persistent storage of settings in a JSON file on LittleFS.
- **Robustness:** Built-in hardware watchdog to ensure long-term stability.

## Target Hardware
- **MCU:** ESP32 (DevKit-V1, M5Stick-C, or ESP32-C3).
- **Sensor:** BLE-YC01 6-in-1 water quality sensor.

## Technology Stack
- **Framework:** Arduino ESP32 with Platform IO
- **Libraries:**
  - `NimBLE-Arduino` for efficient BLE communication.
  - `ESPAsyncWebServer` for a non-blocking web interface.
  - `ArduinoJson` for configuration and status data.
  - `PubSubClient` for MQTT communication.
