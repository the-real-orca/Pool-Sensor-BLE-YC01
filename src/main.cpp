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
char jsonBuffer[256];
#define LED_PIN 2

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
  } else if (param == "todo..." && val) {
    // TODO
  }

  request->send(200, "text/plain", "");
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

  // init filesystem
  if (!SPIFFS.begin(true))
    Serial.println(F("init SPIFFS error"));

  if (!SPIFFS.exists("/index.html"))
  {
    Serial.println(F("ERROR: Failed to read filesystem"));
    Serial.println(F("!-!-!-! build & upload filesystem image !-!-!-!"));
  }

  // read config
  File file = SPIFFS.open("/config.json", "r");
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error)
    Serial.println(F("Failed to read file, using default configuration"));
  config.wifiSSID = doc["wifiSSID"] | "SSID";
  config.wifiPassword = doc["wifiPassword"] | "";
  config.portalSSID = doc["portalSSID"] | "ESP32-Portal";
  config.portalPassword = doc["portalPassword"] | "";
  config.mqttServer = doc["mqttServer"] | "test.mosquitto.org";
  config.mqttPort = doc["mqttPort"] | 1883;
  config.mqttTLS = doc["mqttTLS"] | false;
  config.mqttTopic = doc["mqttTopic"] | "/esp32/sensor/ble-yc01";
  config.mqttUser = doc["mqttUser"] | "";
  config.mqttPassword = doc["mqttPassword"] | "";
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
  DEBUG_println("");

  // start wifi
  bool isCaptive = captivePortalSetup();

  // configure web server
  DEBUG_println("starting web server...");
  webServerInit(webServer, isCaptive);
  webServer.on("/cmd", HTTP_GET, handleCmd);
  webServer.begin();

  // NTP setup
  const char* ntpServer = "pool.ntp.org";
  configTime(0, 0, ntpServer); // configure to UTC time zone (0 offset)


  // MQTT setup
  if ( config.mqttTLS ) {
    DEBUG_println("using secure MQTT connection");
    secureClient.setInsecure();
    mqttClient.setClient(secureClient);
  } else {
    DEBUG_println("using insecure MQTT connection");
    mqttClient.setClient(wifiClient);
  }


  // TODO
  pinMode(LED_PIN, OUTPUT);
}

void loop()
{
  captivePortalLoop();
  mqttLoop();

  digitalWrite(LED_PIN, HIGH);

  Serial.println("Scanning for BLE devices...");
  auto list = BLE_YC01::scan();
  Serial.println("Scan complete.");

  for (const auto& addr : list) {
    Serial.print("Read device: ");
    Serial.println(addr.toString().c_str());
    
    BLE_YC01 device(addr);
    if (device.readData()) {
      Serial.println("Data decoded successfully:");
      auto readings = device.getReadings();
      JsonDocument doc;
      doc["device"] = device.getAddress().toString();
      doc["name"] = device.getName();
      doc["sensorType"] = device.getSensorType();
      doc["type"] = readings.type;
      doc["time"] = readings.time;
      doc["pH"] = readings.pH;
      doc["ec"] = readings.ec; // EC in mV
      doc["salt"] = readings.salt; // Salt in g/L
      doc["tds"] = readings.tds; // TDS in mg/L
      doc["orp"] = readings.orp; // ORP in mV
      doc["cl"] = readings.cl; // Chlorine in mg/L
      doc["temp"] = readings.temp; // Temperature in °C
      doc["bat"] = readings.bat; // Battery in mV
      serializeJson(doc, jsonBuffer);


      Serial.println(jsonBuffer);

      // Senden an MQTT
      if ( mqttClient.connected() ) {
        mqttClient.publish(config.mqttTopic.c_str(), jsonBuffer);
        Serial.println("MQTT sent successfully.");
      }

    } else {
      Serial.println("Failed to read data.");
    }
  }
  
  digitalWrite(LED_PIN, LOW);

  // TODO
  delay(2000);
}
