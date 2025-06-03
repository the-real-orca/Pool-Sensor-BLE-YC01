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

// web server
AsyncWebServer webServer(80);
Ticker restartTimer;


#define LED_PIN 2

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

  // import config
  config.name = doc["name"] | "TODO";
  config.wifiSSID = doc["wifiSSID"] | "SSID";
  config.wifiPassword = doc["wifiPassword"] | "";
  config.portalSSID = doc["portalSSID"] | "ESP32-Portal";
  config.portalPassword = doc["portalPassword"] | "";
  // TODO additional config parameters
  file.close();

  DEBUG_println("config");
  DEBUG_print("  name: ");
  DEBUG_println(config.name);
  DEBUG_print("  wifiSSID: ");
  DEBUG_println(config.wifiSSID);
  DEBUG_print("  wifiPassword: ");
  DEBUG_println(config.wifiPassword);
  DEBUG_print("  portalSSID: ");
  DEBUG_println(config.portalSSID);
  DEBUG_print("  portalPassword: ");
  DEBUG_println(config.portalPassword);
  DEBUG_println("");

  // start wifi
  bool isCaptive = captivePortalSetup();

  // configure web server

  DEBUG_println("starting web server...");
  webServerInit(webServer, isCaptive);
  webServer.on("/cmd", HTTP_GET, handleCmd);
  webServer.begin();

  // TODO
  pinMode(LED_PIN, OUTPUT);
}

void loop()
{
  captivePortalLoop();

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
      Serial.printf("Type: %d, pH: %.2f, EC: %.2f mV, Salt: %.2f g/L, TDS: %.2f mg/L, ORP: %.2f mV, Chlorine: %.2f mg/L, Temp: %.2f °C, Battery: %.2f mV\n",
        readings.type, readings.pH, readings.ec, readings.salt, readings.tds,
        readings.orp, readings.chlor, readings.temp, readings.bat);
    } else {
      Serial.println("Failed to read data.");
    }
  }

  // TODO
  digitalWrite(LED_PIN, HIGH);
  delay(2000);
  digitalWrite(LED_PIN, LOW);
  delay(2000);
}
