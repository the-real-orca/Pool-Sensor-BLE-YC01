# Technical Specification Document (TSD) - Pool Sensor BLE-YC01

## 1. System Architecture
The system consists of an ESP32 microcontroller communicating with a BLE-YC01 sensor and a central MQTT broker/web client.

### 1.1 Components
- **BLE-YC01 Handler:** Manages scanning and reading from the sensor.
- **Web Interface:** Asynchronous web server for status and configuration.
- **Network Manager:** Handles WiFi (Station and AP) and MQTT connections.
- **Configuration Manager:** JSON-based persistent storage on LittleFS.

## 2. Definitions

### 2.1 Terms and Abbreviations
| Abbreviation | Definition |
| :--- | :--- |
| **BLE** | Bluetooth Low Energy |
| **MQTT** | Message Queuing Telemetry Transport |
| **NTP** | Network Time Protocol |
| **LittleFS** | Little File System for microcontrollers |
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
- **Filesystem:** LittleFS (Serial Peripheral Interface Flash File System).
- **Configuration File:** `/config.json` (JSON format).

### 2.5 Reliability
- **Watchdog:** Hardware task watchdog (20 seconds). Resets are explicitly triggered before and after BLE scans and before each device read to prevent false triggers during long operations.
- **Reboot Logic:** Centralized reboot handler (`requestReboot`) ensures:
    - Orderly disconnection from MQTT broker.
    - Serial logging of the reboot reason.
    - Configurable delay (default 2s) to allow ongoing network operations (like HTTP responses) to complete.
- **Reset Reason:** The device tracks the reason for the last restart using `esp_reset_reason()`. This information is:
    - Output to the serial console on startup.
    - Included in the status JSON as `resetReason`.
    - Published via MQTT and displayed in the Web UI.
- **Network Time:** NTP synchronization with `pool.ntp.org` for timestamping data.

### 2.7 Security
The system implements multiple layers of security to protect sensitive configuration data:
- **`DEBUG_SECURITY` Flag:** A build-time flag (`src/config.h`) that enables/disables sensitive features.
    - **Disabled (Production):**
        - WiFi and MQTT passwords are masked (`***`) in log outputs and Serial API responses.
        - `config.json` PUT requests are ignored.
    - **Enabled (Debug):**
        - All features and logs are fully accessible.
- **Startup Protection:** The system issues an explicit warning message via Serial console on startup, indicating whether the device is running in a secured or exposed state.
- **Web UI Warning:** The configuration web interface displays a prominent security warning if `DEBUG_SECURITY` is active.

### 2.6 Internal State Machine
To ensure non-blocking operation and responsiveness of the Web UI and Serial API, the system uses a state machine in the main loop for BLE operations.

| State | Description |
| :--- | :--- |
| `BLE_IDLE` | Waiting for the next measurement interval. |
| `BLE_START_SCAN` | Initiates an asynchronous NimBLE scan (3 seconds duration). |
| `BLE_SCANNING` | Scan is running in the background. System remains responsive to other tasks. |
| `BLE_PROCESS_RESULTS` | Scan results are analyzed, devices are connected/read, and data is published via MQTT. |

### 2.7 Serial API
The ESP32 provides a non-blocking Serial API for configuration and control via the serial port (Baudrate 115200). It uses an internal buffer and only executes commands upon receiving a newline (`\n`).

| Command | Description |
| :--- | :--- |
| **RESET** | Restarts the ESP32. |
| **SCAN** | Clears the stored BLE address and forces a re-scan. |
| **READ** | Forces an immediate BLE read cycle. |
| **STATUS** | Prints the current status JSON to the serial output. |
| **SET_CONFIG** | Saves a new configuration provided as a JSON argument. (Blocked if `DEBUG_SECURITY` is 0). |
| **GET_CONFIG** | Returns the current configuration. WiFi and MQTT passwords are masked if `DEBUG_SECURITY` is 0. |

#### 2.6.1 SET_CONFIG command usage
1. Send `SET_CONFIG <json>` via serial (e.g., `SET_CONFIG {"wifiSSID": "mySSID", "wifiPassword": "myPassword"}`).
2. The device validates the JSON, saves it to LittleFS, and reboots.
3. If the JSON is valid, it responds with `Config saved successfully.` before rebooting.


## 3. Build & Deployment
- **Platform:** PlatformIO (Core `espressif32`).
- **Framework:** Arduino.
- **Board Configuration:** Defined in `platformio.ini` for multiple ESP32 variants.
