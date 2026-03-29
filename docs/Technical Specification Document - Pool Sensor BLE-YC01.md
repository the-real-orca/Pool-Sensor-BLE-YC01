# Technical Specification Document (TSD) - Pool Sensor BLE-YC01

## 1. System Architecture
The system consists of an ESP32 microcontroller communicating with a BLE-YC01 sensor and a central MQTT broker/web client.

### 1.1 Components
- **BLE-YC01 Handler:** Manages scanning and reading from the sensor.
- **Web Interface:** Asynchronous web server for status and configuration.
- **Network Manager:** Handles WiFi (Station and AP) and MQTT connections.
- **Configuration Manager:** JSON-based persistent storage on SPIFFS.

## 2. Definitions

### 2.1 Terms and Abbreviations
| Abbreviation | Definition |
| :--- | :--- |
| **BLE** | Bluetooth Low Energy |
| **MQTT** | Message Queuing Telemetry Transport |
| **NTP** | Network Time Protocol |
| **SPIFFS** | Serial Peripheral Interface Flash File System |
| **TSD** | Technical Specification Document |
| **UUID** | Universally Unique Identifier |

## 3. Technical Details

### 2.1 BLE Protocol
- **Service UUID:** `0000ff01-0000-1000-8000-00805f9b34fb`
- **Characteristic UUID:** `0000ff02-0000-1000-8000-00805f9b34fb`
- **Data Format:** Proprietary 17-byte format.
- **Decoding Algorithm:** Uses bitwise XOR/reversal and checksum verification to extract 6 measurements (pH, EC, Salt, TDS, ORP, Chlorine, Temp).

### 2.2 Data Structures
#### sensorReadings_t
```cpp
struct sensorReadings_t {
    uint8_t type;
    time_t time;
    int16_t rssi;
    float pH;
    float ec;
    float salt;
    float tds;
    float orp;
    float cl;
    float temp;
    float bat;
};
```
#### config_t
```cpp
typedef struct {
  String portalSSID, portalPassword;
  uint16_t portalTimeout;
  String wifiSSID, wifiPassword;
  uint16_t wifiTimeout;
  String mqttServer;
  uint16_t mqttPort;
  bool mqttTLS;
  String mqttTopic, mqttUser, mqttPassword;
  uint16_t interval;
  String name, address;
} config_t;
```

### 2.3 Network Communication
- **MQTT:** Publishes to the configured topic every `interval` seconds. JSON payload includes all sensor readings and system status.
- **HTTP:** REST-like API for commands (`/cmd`), status (`/status`), and configuration (`/config.json`).

### 2.4 Persistent Storage
- **Filesystem:** SPIFFS (Serial Peripheral Interface Flash File System).
- **Configuration File:** `/config.json` (JSON format).

### 2.5 Reliability
- **Watchdog:** Hardware task watchdog (20 seconds).
- **Network Time:** NTP synchronization with `pool.ntp.org` for timestamping data.

## 3. Build & Deployment
- **Platform:** PlatformIO (Core `espressif32`).
- **Framework:** Arduino.
- **Board Configuration:** Defined in `platformio.ini` for multiple ESP32 variants.
