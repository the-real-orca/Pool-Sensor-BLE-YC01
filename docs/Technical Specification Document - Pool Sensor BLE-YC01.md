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

### 3.1 BLE Protocol
- **Service UUID:** `0000ff01-0000-1000-8000-00805f9b34fb`
- **Characteristic UUID:** `0000ff02-0000-1000-8000-00805f9b34fb`
- **Data Format:** Proprietary 17-byte format.
- **Decoding Algorithm:** Uses bitwise XOR/reversal and checksum verification to extract 6 measurements (pH, EC, Salt, TDS, ORP, Chlorine, Temp).

#### 3.1.1 BLE Decoding Algorithm
The raw 17-byte data received from the BLE-YC01 sensor via characteristic `0000ff02-0000-1000-8000-00805f9b34fb` undergoes a proprietary decoding process.

**1. Proprietary Bitwise Transformation (`decodeData` function):**
The received 17-byte data (`raw_data[0...16]`) is first subjected to a bitwise XOR/reversal transformation. This process iterates through the `raw_data` from the last byte towards the first, manipulating bits in an interleaved fashion between consecutive bytes.
For each byte `data[i]`, its even bits (positions 0, 2, 4, 6) and odd bits (positions 1, 3, 5, 7) are shifted and combined with bits from the adjacent byte to produce an intermediate value. This intermediate value is then bitwise NOT-ed (`~`) to derive the corresponding byte in the `decodedData` array. This complex operation effectively "unscrambles" the sensor data.

**2. Checksum Verification (`checksum` function):**
After the bitwise transformation, a simple XOR checksum is calculated over the first 16 bytes of the `decodedData` array. The 17th byte (`decodedData[16]`) is expected to contain this checksum. If the calculated checksum does not match `decodedData[16]`, the data is considered invalid and discarded.

**3. Data Mapping and Conversion:**
Upon successful decoding and checksum verification, the `decodedData` (which is effectively 17 bytes: `decodedData[0...16]`) is mapped to the `sensorReadings_t` structure as follows. Values requiring 16-bit representation are extracted using a helper function `toInt16`, which combines two `uint8_t` into an `int16_t` in big-endian format.

| Offset (bytes) | Length (bytes) | Value Name  | Conversion / Multiplier |
| :--- | :--- | :--- | :--- |
| `0-1` | `2` | (Unused/Header) | N/A |
| `2` | `1` | `readings.type` | Direct `uint8_t` value |
| `3-4` | `2` | `readings.pH` | `toInt16(offset) / 100.0` |
| `5-6` | `2` | `readings.ec` | `toInt16(offset)` (in mV) |
| `5-6` | `2` | `readings.salt` | `toInt16(offset) * 0.55` (derived from EC, in g/L) |
| `7-8` | `2` | `readings.tds` | `toInt16(offset)` (in mg/L) |
| `9-10` | `2` | `readings.orp` | `toInt16(offset)` (in mV) |
| `11-12` | `2` | `readings.cl` | `toInt16(offset) / 10.0` (in mg/L) |
| `13-14` | `2` | `readings.temp` | `toInt16(offset) / 10.0` (in °C) |
| `15-16` | `2` | `readings.bat` | `toInt16(offset)` (in mV) |
| `16` | `1` | Checksum | Calculated XOR checksum |

The `readings.rssi` value is retrieved directly from the BLE client during the connection phase. `readings.time` is the timestamp when the data was read.

### 3.2 Data Structures
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

### 3.3 Network Communication
- **MQTT:** Publishes to the configured topic every `interval` seconds. JSON payload includes all sensor readings and system status.
- **Standby Mode:** If WiFi is disconnected for more than `wifiTimeout` seconds, or if the `OFFLINE` serial command is issued, the device enters a non-blocking **Standby Mode**. In this mode, WiFi and Access Point are disabled, but BLE scanning and serial commands remain active. The system periodically attempts a WiFi reconnection every 60 seconds until successful.
- **Captive Portal:** Initiated if initial WiFi connection fails or is configured incorrectly. After a `portalTimeout`, the portal disables the AP and transitions to **Standby Mode** instead of rebooting.
- **HTTP:** REST-like API for commands (`/cmd`), status (`/status`), and configuration (`/config.json`).

#### 3.3.1 HTTP API Endpoints

##### `/cmd` (GET)
This endpoint allows executing specific commands on the device via HTTP GET requests.

*   **Method:** `GET`
*   **Parameters:**
    *   `param` (required): The command to execute. Supported values include:
        *   `reboot`: Initiates a software reboot of the ESP32.
        *   `scan`: Forces an immediate re-scan for BLE devices.
        *   `read`: Forces an immediate BLE read cycle.
    *   `value` (required): A placeholder value, typically `1` or `true`, to trigger the command.
*   **Example Request:** `GET /cmd?param=reboot&value=1`
*   **Response:**
    *   `200 OK`: On successful command execution.
    *   `400 Bad Request`: If required parameters (`param`, `value`) are missing.

##### `/status` (GET)
This endpoint provides the current operational status of the device and the latest sensor readings in JSON format. It uses the centralized `updateStatusJson()` function to ensure consistency across all interfaces.

*   **Method:** `GET`
*   **Parameters:** None
*   **Response:**
    *   `200 OK`: A JSON object containing:
        *   Current timestamp.
        *   Configured device name.
        *   Current operational status (e.g., "data read successfully", "no matching device found").
        *   Last read sensor values (pH, EC, Salt, TDS, ORP, CL, Temp, Bat, BLE RSSI).
        *   WiFi status (SSID, RSSI, IP address).
        *   MQTT connection status and server address.
        *   Standby mode status.
        *   ESP32 reset reason.
*   **Example Response (JSON):**
    ```json
    {
        "time": 1678886400,
        "name": "PoolSensor1",
        "status": "data read successfully",
        "bleAddress": "AA:BB:CC:DD:EE:FF",
        "sensorType": "BLE-YC01",
        "type": 1,
        "pH": 7.2,
        "ec": 1500,
        "salt": 0.825,
        "tds": 750,
        "orp": 300,
        "cl": 0.5,
        "temp": 25.5,
        "bat": 3800,
        "bleRSSI": -75,
        "wifiSSID": "MyWiFi",
        "wifiRSSI": -60,
        "wifiIP": "192.168.1.100",
        "mqttServer": "mqtt.example.com",
        "mqttConnected": true,
        "resetReason": "Power-on"
    }
    ```

##### `/config.json` (GET)
This endpoint allows retrieving the current device configuration. Sensitive information like WiFi and MQTT passwords are masked if `DEBUG_SECURITY` is `0` (production mode).

*   **Method:** `GET`
*   **Parameters:** None
*   **Response:**
    *   `200 OK`: A JSON object containing the current `config_t` structure. Passwords are masked (`***`) if `DEBUG_SECURITY` is `0`.

##### `/config.json` (PUT)
This endpoint allows updating the device configuration by providing a JSON payload. The device validates the incoming JSON, saves it to `/config.json` on LittleFS, and then triggers a reboot. Passwords are protected from accidental overwriting by `***` if `DEBUG_SECURITY` is `1`.

*   **Method:** `PUT`
*   **Parameters:**
    *   Request Body: A JSON object representing the `config_t` structure.
*   **Example Request (JSON Body):**
    ```json
    {
        "wifiSSID": "NewSSID",
        "wifiPassword": "NewPassword",
        "mqttServer": "new.mqtt.server",
        "interval": 600
    }
    ```
*   **Response:**
    *   `200 OK`: On successful configuration update and reboot initiation.
    *   `400 Bad Request`: If the request body is not valid JSON.
    *   `500 Internal Server Error`: If there is an issue opening or writing to the `/config.json` file.


### 3.4 Persistent Storage
- **Filesystem:** LittleFS (Serial Peripheral Interface Flash File System).
- **Configuration File:** `/config.json` (JSON format).

### 3.5 Reliability
- **Watchdog:** Hardware task watchdog (20 seconds). Resets are explicitly triggered before and after BLE scans and before each device read to prevent false triggers during long operations.
- **MQTT Robustness:** The system periodically attempts to reconnect to the MQTT broker every 10 seconds if the connection is lost. The status JSON is updated immediately after a successful reconnect.
- **Reboot Logic:** Centralized reboot handler (`requestReboot`) ensures:
    - Orderly disconnection from MQTT broker.
    - Serial logging of the reboot reason.
    - Configurable delay (default 2s) to allow ongoing network operations (like HTTP responses) to complete.
- **Reset Reason:** The device tracks the reason for the last restart using `esp_reset_reason()`. This information is:
    - Output to the serial console on startup.
    - Included in the status JSON as `resetReason`.
    - Published via MQTT and displayed in the Web UI.
- **Network Time:** NTP synchronization with `pool.ntp.org` for timestamping data.

#### 3.5.1 Status LED Meaning
The onboard LED (GPIO 2) provides visual feedback on the system's operational status:
- **Solid ON:** BLE scan is currently active.
- **Solid OFF:** System is idle or performing non-BLE related tasks.
- **Quick Blinks (not yet implemented):** Could indicate data read success or specific errors.

### 3.6 Security
The system implements multiple layers of security to protect sensitive configuration data:
- **`DEBUG_SECURITY` Flag:** A build-time flag (`src/config.h`) that enables/disables sensitive features.
    - **Disabled (Production):**
        - WiFi and MQTT passwords are masked (`***`) in log outputs and Serial API responses.
        - `config.json` PUT requests are ignored.
    - **Enabled (Debug):**
        - All features and logs are fully accessible.
        - `config.json` PUT and `SET_CONFIG` are allowed, but passwords are protected from being overwritten by masks (`***`).
- **Startup Protection:** The system issues an explicit warning message via Serial console on startup, indicating whether the device is running in a secured or exposed state.
- **Web UI Warning:** The configuration web interface displays a prominent security warning if `DEBUG_SECURITY` is active.

### 3.7 Internal State Machine
To ensure non-blocking operation and responsiveness of the Web UI and Serial API, the system uses a state machine in the main loop for BLE operations.

| State | Description |
| :--- | :--- |
| `BLE_IDLE` | Waiting for the next measurement interval. |
| `BLE_START_SCAN` | Initiates an asynchronous NimBLE scan (3 seconds duration). |
| `BLE_SCANNING` | Scan is running in the background. System remains responsive to other tasks. |
| `BLE_PROCESS_RESULTS` | Scan results are analyzed, devices are connected/read, and data is published via MQTT. |

### 3.8 Serial API
The ESP32 provides a non-blocking Serial API for configuration and control via the serial port (Baudrate 115200). It uses an internal buffer and only executes commands upon receiving a newline (`\n`).

| Command | Description |
| :--- | :--- |
| **RESET** | Restarts the ESP32. |
| **OFFLINE** | Switches the device into **Standby Mode**, disabling all network operations. |
| **SCAN** | Clears the stored BLE address and forces a re-scan. |
| **READ** | Forces an immediate BLE read cycle. |
| **STATUS** | Prints the current status JSON (generated via `updateStatusJson()`) to the serial output. |
| **SET_CONFIG** | Saves a new configuration provided as a JSON argument. (Blocked if `DEBUG_SECURITY` is 0). |
| **GET_CONFIG** | Returns the current configuration. WiFi and MQTT passwords are masked if `DEBUG_SECURITY` is 0. |

#### 3.8.1 SET_CONFIG command usage
1. Send `SET_CONFIG <json>` via serial (e.g., `SET_CONFIG {"wifiSSID": "mySSID", "wifiPassword": "myPassword"}`).
2. The device validates the JSON, saves it to LittleFS, and reboots.
3. If the JSON is valid, it responds with `Config saved successfully.` before rebooting.


## 4. Build & Deployment
- **Platform:** PlatformIO (Core `espressif32`).
- **Framework:** Arduino.
- **Board Configuration:** Defined in `platformio.ini` for multiple ESP32 variants.
