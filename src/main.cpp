#include <Arduino.h>
#include "esp_task_wdt.h"

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <ArduinoJson.h>
#include <LittleFS.h>
#include <Ticker.h>

#include "captivePortal.h"
#include "webUtils.h"

#include "BLE-YC01.h"

#include "config.h"


// general configuration
config_t config;
#define BUFFER_SIZE 512
static char statusJsonBuffer[BUFFER_SIZE];
#define LED_PIN 2
static uint32_t lastScan = 0;
String resetReason;

/**
 * @brief Returns a human-readable string for the reset reason.
 * @param reason esp_reset_reason_t
 * @return String
 */
String getResetReasonName(esp_reset_reason_t reason) {
  switch (reason) {
    case ESP_RST_UNKNOWN:   return "Unknown";
    case ESP_RST_POWERON:   return "Power-on";
    case ESP_RST_EXT:       return "External reset";
    case ESP_RST_SW:        return "Software reset";
    case ESP_RST_PANIC:     return "Panic/Exception";
    case ESP_RST_INT_WDT:   return "Interrupt Watchdog";
    case ESP_RST_TASK_WDT:  return "Task Watchdog";
    case ESP_RST_WDT:       return "Other Watchdog";
    case ESP_RST_DEEPSLEEP: return "Deep sleep";
    case ESP_RST_BROWNOUT:  return "Brownout";
    case ESP_RST_SDIO:      return "SDIO reset";
    default:                return "Other";
  }
}

// wifi
static bool isCaptive = false;
bool isOffline = false;
static uint32_t diconnectedAt = 0; // timestamp when the device was disconnected from WiFi

// web server
AsyncWebServer webServer(80);
Ticker restartTimer;

// MQTT
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
WiFiClient wifiClient;
WiFiClientSecure secureClient;
PubSubClient mqttClient;

/**
 * @brief Centralized reboot function with delay and cleanup.
 * @param reason String describing the reason for reboot
 * @param delayMs Delay in milliseconds before restart
 */
void requestReboot(String reason, uint32_t delayMs) {
  Serial.print("Reboot requested. Reason: ");
  Serial.println(reason);
  Serial.flush();
  
  // MQTT cleanup
  if (mqttClient.connected()) {
    DEBUG_println("Disconnecting MQTT...");
    mqttClient.disconnect();
  }
  
  // Use Ticker to delay restart
  restartTimer.once_ms(delayMs, []() {
    ESP.restart();
  });
}

/**
 * @brief Handles the MQTT client loop.
 * 
 * Only active if MQTT is configured and not in captive portal mode.
 */
void mqttLoop() {
  if ( config.mqttPort && !isCaptive) {
    if ( mqttClient.connected() ) {
      mqttClient.loop();
    }
  }    
}

/**
 * @brief HTTP GET handler for commands via /cmd endpoint.
 * @param request Pointer to AsyncWebServerRequest
 */
void handleCmd(AsyncWebServerRequest *request)
{
  request->getParam("cmd", true, false); // get the cmd parameter, but do not use it
  if (!request->hasParam("param") || !request->hasParam("value"))
  {
    DEBUG_println("Bad Request: Missing Parameters");
    request->send(400, "text/plain", "Bad Request: Missing Parameters");
    return;
  }

  String param = request->getParam("param")->value();
  String val = request->getParam("value")->value();
  DEBUG_print("handleCmd: "); DEBUG_print(param); DEBUG_print(" = "); DEBUG_println(val);

  if (param == "reboot" && val) {
    requestReboot("Web Command"); 
  } else if (param == "scan" && val) {
    // re-scan
    config.bleAddress = ""; // reset address to force re-scan
    lastScan = -config.interval; // reset last scan time
  } else if (param == "read" && val) {
    lastScan = -config.interval; // reset last scan time
  }

  request->send(200, "text/plain", "");
}

/**
 * @brief Reads the configuration from LittleFS (config.json).
 */
void readConfig()
{
  File file = LittleFS.open("/config.json", "r");
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error)
    Serial.println(F("Failed to read file, using default configuration"));

  config.wifiSSID = doc["wifiSSID"] | "SSID";
  config.wifiPassword = doc["wifiPassword"] | "";
  config.wifiTimeout = doc["wifiTimeout"] | 600;
  config.offlineMode = doc["offlineMode"] | false;
  config.portalSSID = doc["portalSSID"] | "ESP32-Portal";
  config.portalPassword = doc["portalPassword"] | "";
  config.portalTimeout = doc["portalTimeout"] | 600;
  config.mqttServer = doc["mqttServer"] | "";
  config.mqttPort = doc["mqttPort"] | 1883;
  config.mqttTLS = doc["mqttTLS"] | false;
  config.mqttTopic = doc["mqttTopic"] | "/esp32/sensor/ble-yc01";
  config.mqttUser = doc["mqttUser"] | "";
  config.mqttPassword = doc["mqttPassword"] | "";
  config.interval = doc["interval"] | 900;
  config.name = doc["name"] | "";
  config.bleAddress = doc["bleAddress"] | "";
  file.close();


  DEBUG_println("config");
  DEBUG_print("  wifiSSID: "); DEBUG_println(config.wifiSSID);
  #if DEBUG_SECURITY
    DEBUG_print("  wifiPassword: "); DEBUG_println(config.wifiPassword);
  #else
    DEBUG_println("  wifiPassword: ***");
  #endif
  DEBUG_print("  wifiTimeout: "); DEBUG_println(config.wifiTimeout);
  DEBUG_print("  portalSSID: "); DEBUG_println(config.portalSSID);
  DEBUG_print("  portalPassword: "); DEBUG_println(config.portalPassword);
  DEBUG_print("  portalTimeout: "); DEBUG_println(config.portalTimeout);
  DEBUG_print("  mqttServer: "); DEBUG_println(config.mqttServer);
  DEBUG_print("  mqttPort: "); DEBUG_println(config.mqttPort);
  DEBUG_print("  mqttTLS: "); DEBUG_println(config.mqttTLS);
  DEBUG_print("  mqttTopic: "); DEBUG_println(config.mqttTopic);
  DEBUG_print("  mqttUser: "); DEBUG_println(config.mqttUser);
  #if DEBUG_SECURITY
    DEBUG_print("  mqttPassword: "); DEBUG_println(config.mqttPassword);
  #else
    DEBUG_println("  mqttPassword: ***");
  #endif  
  DEBUG_print("  interval: "); DEBUG_println(config.interval);
  DEBUG_print("  name: "); DEBUG_println(config.name);
  DEBUG_print("  address: "); DEBUG_println(config.bleAddress);
  DEBUG_println("");
}

/**
 * @brief Saves the current configuration to LittleFS (config.json).
 */
void saveConfig() {
  JsonDocument doc;

  // prepare JSON document
  doc["wifiSSID"]       = config.wifiSSID;
  doc["wifiPassword"]   = config.wifiPassword;
  doc["wifiTimeout"]    = config.wifiTimeout;
  doc["portalSSID"]     = config.portalSSID;
  doc["portalPassword"] = config.portalPassword;
  doc["portalTimeout"]  = config.portalTimeout;
  doc["mqttServer"]     = config.mqttServer;
  doc["mqttPort"]       = config.mqttPort;
  doc["mqttTLS"]        = config.mqttTLS;
  doc["mqttTopic"]      = config.mqttTopic;
  doc["mqttUser"]       = config.mqttUser;
  doc["mqttPassword"]   = config.mqttPassword;
  doc["interval"]       = config.interval;
  doc["name"]           = config.name;
  doc["bleAddress"]           = config.bleAddress;

  // write config file
  File file = LittleFS.open("/config.json", "w");
  if (!file) {
    Serial.println(F("Failed to open config file for writing"));
    return;
  }
  if (serializeJson(doc, file) == 0) {
    Serial.println(F("Failed to write config to file"));
  }
  file.close();
}


/**
 * @brief Standard Arduino setup function.
 * 
 * Initializes serial, filesystem, configuration, WiFi/Captive Portal, 
 * Web Server, MQTT, and Watchdog.
 */
void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("\n\napplication starting ...");
  #if DEBUG_SECURITY
    Serial.println("!!! WARNING: DEBUG_SECURITY IS ENABLED - CONFIGURATION EXPOSED !!!");
  #endif
  Serial.print("Build date: ");
  Serial.print(__DATE__);
  Serial.print(" time: ");
  Serial.println(__TIME__);
  
  strcpy(statusJsonBuffer, "{\"status\": \"init\"}"); // reset json buffer

  // init filesystem
  if (!LittleFS.begin(true))
    Serial.println(F("init LittleFS error"));

  delay (100); // wait for filesystem to be ready

  if (!LittleFS.exists("/index.html"))
  {
    Serial.println(F("ERROR: Failed to read filesystem"));
    Serial.println(F("!-!-!-! build & upload filesystem image !-!-!-!"));
  }

  // read config
  readConfig();

  // get reset reason
  resetReason = getResetReasonName(esp_reset_reason());
  DEBUG_print("Reset reason: "); DEBUG_println(resetReason);

  // start wifi
  isCaptive = captivePortalSetup();

  // configure web server
  DEBUG_println("starting web server...");
  webServerInit(webServer, isCaptive);
  webServer.on("/cmd", HTTP_GET, handleCmd);
  webServer.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "application/json", statusJsonBuffer); 
  });
  webServer.begin();

  // NTP setup
  const char* ntpServer = "pool.ntp.org";
  configTime(0, 0, ntpServer); // configure to UTC time zone (0 offset)


  // MQTT setup
  if ( config.mqttServer.isEmpty() || config.mqttPort == 0 ) {
    config.mqttPort = 0;  // disable MQTT if no server is configured or in captive portal mode
  }
  if ( config.mqttPort && !isCaptive )  // only configure MQTT client if not in captive portal mode
  {
    // configure MQTT client
    if ( config.mqttTLS ) {
      DEBUG_println("using secure MQTT connection");
      secureClient.setInsecure();
      mqttClient.setClient(secureClient);
    } else {
      DEBUG_println("using insecure MQTT connection");
      mqttClient.setClient(wifiClient);
    }
    mqttClient.setBufferSize(BUFFER_SIZE + MQTT_MAX_HEADER_SIZE + 10);
  }

  // reset BLE scan
  lastScan = -config.interval;

  // configure status LED
  pinMode(LED_PIN, OUTPUT);


  // start watchdog
  esp_task_wdt_init(20, true); // 20 seconds
  esp_task_wdt_add(NULL);

  Serial.println("\ninit complete\n");

}


/**
 * @brief Handles serial commands for the Serial API.
 * 
 * Commands:
 * - RESET: Restarts the ESP32.
 * - SCAN: Forces a re-scan for BLE devices.
 * - READ: Forces an immediate BLE read.
 * - STATUS: Prints the current status JSON to Serial.
 * - SET_CONFIG: Receives a new config.json via Serial.
 */
void handleSerialApi() {
  static String serialBuffer = "";
  
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      String line = serialBuffer;
      serialBuffer = ""; // reset buffer
      line.trim();
      if (line.length() == 0) continue;

      int spaceIndex = line.indexOf(' ');
      String cmd = (spaceIndex == -1) ? line : line.substring(0, spaceIndex);
      String arg = (spaceIndex == -1) ? "" : line.substring(spaceIndex + 1);
      cmd.trim(); cmd.toUpperCase();
      arg.trim();

      if (cmd == "RESET") {
        requestReboot("Serial RESET");
        Serial.flush();
      } else if (cmd == "OFFLINE") {
        Serial.println("Offline Mode\n");
        WiFi.mode(WIFI_OFF);
        config.offlineMode = true;
      } else if (cmd == "SCAN") {
        Serial.println("Forcing re-scan...\n");
        config.bleAddress = "";
        lastScan = millis()/1000 - config.interval;
      } else if (cmd == "READ") {
        Serial.println("Forcing immediate read...\n");
        lastScan = millis()/1000 - config.interval;
      } else if (cmd == "STATUS") {
        Serial.println(statusJsonBuffer);
      } else if (cmd == "SET_CONFIG") {
        if (arg.length() == 0) {
          Serial.println("Usage: SET_CONFIG <json>");
        } else {
          JsonDocument doc;
          DeserializationError error = deserializeJson(doc, arg);
          if (!error) {
            File file = LittleFS.open("/config.json", "w");
            if (file) {
              // Only update passwords if not masked
              if (doc["wifiPassword"] == "***") doc["wifiPassword"] = config.wifiPassword;
              if (doc["mqttPassword"] == "***") doc["mqttPassword"] = config.mqttPassword;
              
              if (serializeJson(doc, file) > 0) {
                file.close();
                Serial.println("Config saved successfully.\n");
                Serial.flush();
                requestReboot("Serial SET_CONFIG");
              } else {
                Serial.println("Failed to write config to file.");
                file.close();
              }
            } else {
              Serial.println("Failed to open config file for writing.");
            }
          } else {
            Serial.print("JSON deserialization error: ");
            Serial.println(error.c_str());
          }
        }
      } else if (cmd == "GET_CONFIG") {
        Serial.println("Current configuration:");
        // Serialize config to JSON and print to Serial
        serializeConfig(Serial, true);
        Serial.println(); // Add a newline for better readability
      } else {
        Serial.print("Unknown command: ");
        Serial.println(cmd);
        Serial.println("Available commands: RESET, OFFLINE, SCAN, READ, STATUS, SET_CONFIG, GET_CONFIG\n");
      }
      Serial.flush();
    } else if (c != '\r') {
      serialBuffer += c;
      // Safety: limit buffer size to prevent memory exhaustion
      if (serialBuffer.length() > 2048) {
        Serial.println("Error: Serial buffer overflow\n"); Serial.flush();
        serialBuffer = "";
      }
    }
  }
}

enum BLEState {
  BLE_IDLE,
  BLE_START_SCAN,
  BLE_SCANNING,
  BLE_PROCESS_RESULTS
};

static BLEState bleState = BLE_IDLE;

/**
 * @brief Standard Arduino loop function.
 * 
 * Handles network tasks, BLE scanning/reading, and MQTT data publishing.
 */
void loop()
{
  time_t now;
  time(&now);
  uint32_t uptime = millis()/1000; // get current timestamp in seconds

  // reset watchdog
  esp_task_wdt_reset();

  // handle serial API
  handleSerialApi();

  // handle network tasks
  captivePortalLoop();
  mqttLoop();
  webUtilsLoop();

  if ( !config.offlineMode && !isCaptive && !WiFi.isConnected()) {
    if ( !diconnectedAt ) {
      diconnectedAt = uptime; // set disconnection timestamp
      DEBUG_println("WiFi disconnected, waiting for reconnection...");
    } else {
      // restart if disconnected from WiFi for more than wifiTimeout seconds ... starts captive portal or reconnect to WiFi
      if ( (uptime - diconnectedAt) > config.wifiTimeout ) {
        DEBUG_println("restarting due to WiFi disconnection timeout");
        WiFi.disconnect(true, true);
        delay(50);
        WiFi.mode(WIFI_OFF);
        delay(500);
        requestReboot("WiFi Timeout");
      }
    }
  } else {
    diconnectedAt = 0; // reset disconnection timestamp
  }
    
  // BLE State Machine
  switch (bleState) {
    case BLE_IDLE:
      if ((uptime - lastScan) > config.interval) {
        bleState = BLE_START_SCAN;
      }
      break;

    case BLE_START_SCAN:
      Serial.println("Scanning for BLE devices (async)...");
      digitalWrite(LED_PIN, HIGH);
      if (BLE_YC01::startScan(3)) {
        bleState = BLE_SCANNING;
      } else {
        DEBUG_println("Failed to start BLE scan");
        lastScan = uptime; // Delay retry
        bleState = BLE_IDLE;
        digitalWrite(LED_PIN, LOW);
      }
      break;

    case BLE_SCANNING:
      if (!BLE_YC01::isScanning()) {
        bleState = BLE_PROCESS_RESULTS;
      }
      break;

    case BLE_PROCESS_RESULTS: {
      auto list = BLE_YC01::getFoundDevices();
      
      JsonDocument doc;
      doc["time"] = now;
      doc["name"] = config.name;

      bool found = false;
      for (const auto& addr : list) {
        esp_task_wdt_reset();
        Serial.print("Read device: ");
        Serial.println(addr.toString().c_str());
        
        if ( config.bleAddress.isEmpty() || compareBLEAddress(addr, config.bleAddress) ) {
          BLE_YC01 device(addr, config.name);
          sensorReadings_t readings = {0};
          if ( device.readData() ) {
            readings = device.getReadings();
          }
          
          if ( readings.type ) {
            Serial.println("Data decoded successfully:");
            doc["status"] = "data read successfully";
            doc["bleAddress"] = device.getAddress().toString();
            doc["sensorType"] = device.getSensorType();
            doc["type"] = readings.type;
            doc["pH"] = readings.pH;
            doc["ec"] = readings.ec;
            doc["salt"] = readings.salt;
            doc["tds"] = readings.tds;
            doc["orp"] = readings.orp;
            doc["cl"] = readings.cl;
            doc["temp"] = readings.temp;
            doc["bat"] = readings.bat;
            doc["bleRSSI"] = readings.rssi;
            found = true;
          }
          break; 
        }
      }

      if (!found) {
        doc["status"] = list.empty() ? "no devices found" : "no matching device found";
        doc["bleAddress"] = config.bleAddress;
        doc["sensorType"] = "unknown";
        doc["type"] = 0;
        Serial.println(doc["status"].as<String>());
      }

      // WiFi and MQTT information
      doc["wifiSSID"] = isCaptive ? config.portalSSID : config.wifiSSID;
      doc["wifiRSSI"] = WiFi.RSSI();
      doc["wifiIP"] = isCaptive ? WiFi.softAPIP().toString() : WiFi.localIP().toString();
      doc["mqttServer"] = config.mqttServer;
      doc["mqttConnected"] = mqttClient.connected();
      doc["resetReason"] = resetReason;

      serializeJson(doc, statusJsonBuffer);
      DEBUG_println(statusJsonBuffer);

      // MQTT Publishing
      if ( config.mqttPort && !isCaptive ) {
        if ( !mqttClient.connected() ) {
          DEBUG_print("connecting to MQTT-broker... ");
          mqttClient.setServer(config.mqttServer.c_str(), config.mqttPort);
          if ( mqttClient.connect("BLE-YC01", config.mqttUser.c_str(), config.mqttPassword.c_str()) ) {
            mqttClient.loop();
            DEBUG_println("ok");
          } else {
            DEBUG_print(" \nerror, rc=");
            DEBUG_println(mqttClient.state());
          }
        }
        if ( mqttClient.connected() ) {
          mqttClient.publish(config.mqttTopic.c_str(), statusJsonBuffer);
          mqttClient.loop();
        }
      }

      lastScan = uptime;
      digitalWrite(LED_PIN, LOW);
      bleState = BLE_IDLE;
      break;
    }
  }

  // wait
  delay(10); // Reduced delay for better responsiveness
}
