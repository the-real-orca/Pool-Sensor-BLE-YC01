#include <Arduino.h>

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
static char jsonBuffer[BUFFER_SIZE];
#define LED_PIN 2
static time_t lastScan = 0;

// wifi
static bool isCaptive = false;

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
  if ( config.mqttPort ) {
    if ( mqttClient.connected() ) {
      mqttClient.loop();
    } else {
      mqttClient.setServer(config.mqttServer.c_str(), config.mqttPort);
      DEBUG_print("connecting to MQTT-broker... ");
      if ( mqttClient.connect("BLE-YC01", config.mqttUser.c_str(), config.mqttPassword.c_str()) ) {
        mqttClient.loop();
        DEBUG_println("ok");
      } else {
        DEBUG_print(" \nerror, rc=");
        DEBUG_println(mqttClient.state());
      }
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
    lastScan = 0; // reset last scan time
  } else if (param == "read" && val) {
    lastScan = 0; // reset last scan time
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
  config.portalSSID = doc["portalSSID"] | "ESP32-Portal";
  config.portalPassword = doc["portalPassword"] | "";
  config.mqttServer = doc["mqttServer"] | "";
  config.mqttPort = doc["mqttPort"] | 0;
  config.mqttTLS = doc["mqttTLS"] | false;
  config.mqttTopic = doc["mqttTopic"] | "/esp32/sensor/ble-yc01";
  config.mqttUser = doc["mqttUser"] | "";
  config.mqttPassword = doc["mqttPassword"] | "";
  config.interval = doc["interval"] | 900; // every 15 minutes
  config.name = doc["name"] | "";
  config.address = doc["addr"] | "";

  file.close();

  DEBUG_println("config");
  DEBUG_print("  wifiSSID: "); DEBUG_println(config.wifiSSID);
  DEBUG_print("  wifiPassword: "); DEBUG_println(config.wifiPassword);
  DEBUG_print("  portalSSID: "); DEBUG_println(config.portalSSID);
  DEBUG_print("  portalPassword: "); DEBUG_println(config.portalPassword);
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
  doc["portalSSID"]     = config.portalSSID;
  doc["portalPassword"] = config.portalPassword;
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
  
  strcpy(jsonBuffer, "{}"); // reset json buffer

  // init filesystem
  if (!SPIFFS.begin(true))
    Serial.println(F("init SPIFFS error"));

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
      request->send(200, "application/json", jsonBuffer); 
  });
  webServer.begin();

  // NTP setup
  const char* ntpServer = "pool.ntp.org";
  configTime(0, 0, ntpServer); // configure to UTC time zone (0 offset)


  // MQTT setup
  if ( config.mqttServer.isEmpty() || config.mqttPort == 0 || isCaptive ) {
    config.mqttPort = 0;  // disable MQTT if no server is configured or in captive portal mode
  }
  if ( config.mqttPort )
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


  // configure status LED
  pinMode(LED_PIN, OUTPUT);
}


void loop()
{
  time_t now;
  time(&now);

  // handle network tasks
  captivePortalLoop();
  mqttLoop();

  // scan for BLE devices & read data
  if ( (now - lastScan) > config.interval) { // scan and read data every x sceconds
    Serial.println("Scanning for BLE devices...");
    digitalWrite(LED_PIN, HIGH);
    auto list = BLE_YC01::scan();
    Serial.println("Scan complete.");

    JsonDocument doc;
    doc["time"] = now;

    for (const auto& addr : list) {
      Serial.print("Read device: ");
      Serial.println(addr.toString().c_str());
      
      // save address to config if no device address is set
      if ( config.address.isEmpty() ) {
        config.address = addr.toString().c_str();
        saveConfig();
      }

      // read data from device if address matches
      if ( compareBLEAddress(addr, config.address) ) {
        BLE_YC01 device(addr, config.name);
        if ( device.readData() ) {
          Serial.println("Data decoded successfully:");
          auto readings = device.getReadings();
          doc["status"] = "data read successfully"; // status message
          doc["name"] = device.getName();
          doc["addr"] = device.getAddress().toString();
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

          lastScan = now;
        } else {
          doc["status"] = "failed to read data";
          Serial.println("Failed to read data.");
        }
          break; // exit loop after reading matching device
      }

    }

    if (lastScan != now) {
      if ( !doc.containsKey("status") ) {
        Serial.println("No matching device found.");
      }
      doc["name"] = config.name;
      doc["addr"] = config.address;
      doc["sensorType"] = "unknown";
      doc["type"] = 0; // no readings available
      doc["pH"] = 0.0;
      doc["ec"] = 0.0;
      doc["salt"] = 0.0;
      doc["tds"] = 0.0;
      doc["orp"] = 0.0;
      doc["cl"] = 0.0;
      doc["temp"] = 0.0;
      doc["bat"] = 0.0;
      doc["bleRSSI"] = 0; // no RSSI available
    }

    // WiFi and MQTT information
    doc["wifiSSID"] = isCaptive ? config.portalSSID : config.wifiSSID; // WiFi SSID
    doc["wifiRSSI"] = WiFi.RSSI(); // WiFi RSSI
    doc["wifiIP"] = isCaptive ? WiFi.softAPIP().toString() : WiFi.localIP().toString(); // WiFi IP address
    doc["mqttServer"] = config.mqttServer; // MQTT server
    doc["mqttConnected"] = mqttClient.connected();

    serializeJson(doc, jsonBuffer);

    DEBUG_println(jsonBuffer);

    // Senden an MQTT
    if ( mqttClient.connected() ) {
      if ( mqttClient.publish(config.mqttTopic.c_str(), jsonBuffer) ) {
        DEBUG_print("MQTT sent successfully: "); DEBUG_println(config.mqttTopic);
      } else {
        DEBUG_print("MQTT send failed: "); 
      }
      mqttClient.loop();
    }

    digitalWrite(LED_PIN, LOW);
  }

  // wait
  delay(1000);
}
