#include <Arduino.h>
#include "esp_task_wdt.h"

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <ArduinoJson.h>
#include <SPIFFS.h>
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

// wifi
static bool isCaptive = false;
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

void mqttLoop() {
  if ( config.mqttPort && !isCaptive) {
    if ( mqttClient.connected() ) {
      mqttClient.loop();
    }
  }    
}

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
    restartTimer.once(0.5, []() { ESP.restart(); }); 
  } else if (param == "scan" && val) {
    // re-scan
    config.address = ""; // reset address to force re-scan
    lastScan = -config.interval; // reset last scan time
  } else if (param == "read" && val) {
    lastScan = -config.interval; // reset last scan time
  }

  request->send(200, "text/plain", "");
}

void readConfig()
{
  File file = SPIFFS.open("/config.json", "r");
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error)
    Serial.println(F("Failed to read file, using default configuration"));

  config.wifiSSID = doc["wifiSSID"] | "SSID";
  config.wifiPassword = doc["wifiPassword"] | "";
  config.wifiTimeout = doc["wifiTimeout"] | 600;
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
  config.address = doc["addr"] | "";
  file.close();


  DEBUG_println("config");
  DEBUG_print("  wifiSSID: "); DEBUG_println(config.wifiSSID);
  DEBUG_print("  wifiPassword: "); DEBUG_println(config.wifiPassword);
  DEBUG_print("  wifiTimeout: "); DEBUG_println(config.wifiTimeout);
  DEBUG_print("  portalSSID: "); DEBUG_println(config.portalSSID);
  DEBUG_print("  portalPassword: "); DEBUG_println(config.portalPassword);
  DEBUG_print("  portalTimeout: "); DEBUG_println(config.portalTimeout);
  DEBUG_print("  mqttServer: "); DEBUG_println(config.mqttServer);
  DEBUG_print("  mqttPort: "); DEBUG_println(config.mqttPort);
  DEBUG_print("  mqttTLS: "); DEBUG_println(config.mqttTLS);
  DEBUG_print("  mqttTopic: "); DEBUG_println(config.mqttTopic);
  DEBUG_print("  mqttUser: "); DEBUG_println(config.mqttUser);
  DEBUG_print("  mqttPassword: "); DEBUG_println(config.mqttPassword);
  DEBUG_print("  interval: "); DEBUG_println(config.interval);
  DEBUG_print("  name: "); DEBUG_println(config.name);
  DEBUG_print("  address: "); DEBUG_println(config.address);
  DEBUG_println("");
}

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
  doc["addr"]           = config.address;

  // write config file
  File file = SPIFFS.open("/config.json", "w");
  if (!file) {
    Serial.println(F("Failed to open config file for writing"));
    return;
  }
  if (serializeJson(doc, file) == 0) {
    Serial.println(F("Failed to write config to file"));
  }
  file.close();
}


void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("\n\napplication started");
  Serial.print("Build date: ");
  Serial.print(__DATE__);
  Serial.print(" time: ");
  Serial.println(__TIME__);
  
  strcpy(statusJsonBuffer, "{}"); // reset json buffer

  // init filesystem
  if (!SPIFFS.begin(true))
    Serial.println(F("init SPIFFS error"));

  delay (500); // wait for filesystem to be ready

  if (!SPIFFS.exists("/index.html"))
  {
    Serial.println(F("ERROR: Failed to read filesystem"));
    Serial.println(F("!-!-!-! build & upload filesystem image !-!-!-!"));
  }

  // read config
  readConfig();

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

}


void loop()
{
  time_t now;
  time(&now);
  uint32_t uptime = millis()/1000; // get current timestamp in seconds

  // reset watchdog
  esp_task_wdt_reset();


  // handle network tasks
  captivePortalLoop();
  mqttLoop();
  webUtilsLoop();


  if ( !isCaptive && !WiFi.isConnected()) {
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
        ESP.restart();
      }
    }
  } else {
    diconnectedAt = 0; // reset disconnection timestamp
  }
    
  // scan for BLE devices & read data
  if ( (uptime - lastScan) > config.interval) { // scan and read data every x sceconds
    // BLE
    Serial.println("Scanning for BLE devices...");
    digitalWrite(LED_PIN, HIGH);
    auto list = BLE_YC01::scan();
    Serial.println("Scan complete.");

    JsonDocument doc;
    doc["time"] = now;
    doc["name"] = config.name;
 

    for (const auto& addr : list) {
      Serial.print("Read device: ");
      Serial.println(addr.toString().c_str());
      
      // read data from device if address matches or address is empty
      if ( config.address.isEmpty() || compareBLEAddress(addr, config.address) ) {
        BLE_YC01 device(addr, config.name);
        sensorReadings_t readings = {0};
        if ( device.readData() ) {
          readings = device.getReadings();
        }
        if ( readings.type ) {
          Serial.println("Data decoded successfully:");

          doc["status"] = "data read successfully"; // status message
          doc["addr"] = device.getAddress().toString(); // device address
          doc["sensorType"] = device.getSensorType();
          doc["type"] = readings.type;

          // readings
          doc["pH"] = readings.pH;
          doc["ec"] = readings.ec; // EC in mV
          doc["salt"] = readings.salt; // Salt in g/L
          doc["tds"] = readings.tds; // TDS in mg/L
          doc["orp"] = readings.orp; // ORP in mV
          doc["cl"] = readings.cl; // Chlorine in mg/L
          doc["temp"] = readings.temp; // Temperature in °C
          doc["bat"] = readings.bat; // Battery in mV
          doc["bleRSSI"] = readings.rssi; // BLE RSSI

          lastScan = uptime;
        } else {
          doc["status"] = "failed to read data";
          doc["bleRSSI"] = readings.rssi; // BLE RSSI
          doc["sensorType"] = "unknown";
          Serial.println("Failed to read data.");
        }

        break; // exit loop after reading matching device
      }

    }
    digitalWrite(LED_PIN, LOW);

    if (lastScan != uptime) {
      if ( doc["status"].isNull() ) {
        doc["status"] = "no matching device found"; // status message
        Serial.println("No matching device found.");
      }
      doc["addr"] = config.address;
      doc["sensorType"] = "unknown";
      doc["type"] = 0; // no readings available
      // doc["pH"] = 0.0;
      // doc["ec"] = 0.0;
      // doc["salt"] = 0.0;
      // doc["tds"] = 0.0;
      // doc["orp"] = 0.0;
      // doc["cl"] = 0.0;
      // doc["temp"] = 0.0;
      // doc["bat"] = 0.0;
      // doc["bleRSSI"] = 0; // no RSSI available

//      lastScan = uptime - (config.interval/2);
      lastScan = uptime - config.interval + 10; // next scan in 10 seconds
    }

    // WiFi and MQTT information
    doc["wifiSSID"] = isCaptive ? config.portalSSID : config.wifiSSID; // WiFi SSID
    doc["wifiRSSI"] = WiFi.RSSI(); // WiFi RSSI
    doc["wifiRSSI"] = WiFi.RSSI();
    doc["wifiIP"] = isCaptive ? WiFi.softAPIP().toString() : WiFi.localIP().toString(); // WiFi IP address
    doc["mqttServer"] = config.mqttServer; // MQTT server
    doc["mqttConnected"] = mqttClient.connected();

    serializeJson(doc, statusJsonBuffer);

    DEBUG_println(statusJsonBuffer);

    // MQTT
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
        if ( mqttClient.publish(config.mqttTopic.c_str(), statusJsonBuffer) ) {
          DEBUG_print("MQTT sent successfully: "); DEBUG_println(config.mqttTopic);
        } else {
          DEBUG_print("MQTT send failed: "); 
        }
        mqttClient.loop();
      }
    }

  }

  // wait
  delay(100);
}
