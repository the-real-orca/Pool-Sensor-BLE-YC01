#pragma once
#include <Arduino.h>


/*
 * Debug
 */
#define DEBUG_SERIAL_PRINT 1 // 1: enable debug serial print, 0: disable debug serial print
#define DEBUG_SECURITY 0     // 1: security disabled (passwords plaintext), 0: security enabled (passwords masked)

#if DEBUG_SERIAL_PRINT
  #define DEBUG_println(x)  Serial.println(x)
  #define DEBUG_print(x)  Serial.print(x)
  #define DEBUG_printf(x, y)  Serial.printf(x, y)
#else  
  #define DEBUG_println(x)
  #define DEBUG_print(x)
  #define DEBUG_printf(x, y)
#endif

/*
 * config structure
 */
typedef struct {
// WiFi configuration
  String portalSSID;
  String portalPassword;
  uint16_t portalTimeout;
  String wifiSSID;
  String wifiPassword;
  uint16_t wifiTimeout;
  bool offlineMode;
// MQTT configurations
  String mqttServer;
  uint16_t mqttPort;
  bool mqttTLS;
  String mqttTopic;
  String mqttUser;
  String mqttPassword;
// BLE-YC01 configurations
  uint16_t interval;
  String name;
  String address;
  // TODO: add more configuration parameters as needed

} config_t;
extern config_t config;


/**
 * @brief Centralized reboot function with delay and cleanup.
 * @param reason String describing the reason for reboot
 * @param delayMs Delay in milliseconds before restart (default 1000)
 */
void requestReboot(String reason, uint32_t delayMs = 1000);


#include <ArduinoJson.h>
template <
    typename TDestination,
    detail::enable_if_t<!detail::is_pointer<TDestination>::value, int> = 0>
void serializeConfig(TDestination& destination, bool pretty = false)
{            
    JsonDocument doc; 
    // general
    doc["name"]           = config.name;
    doc["interval"]       = config.interval;

    // WIFI
    doc["wifiSSID"]       = config.wifiSSID;
    #if DEBUG_SECURITY
    doc["wifiPassword"]   = config.wifiPassword;
    #else
    doc["wifiPassword"]   = "***";
    #endif
    doc["wifiTimeout"]    = config.wifiTimeout;
    doc["portalSSID"]     = config.portalSSID;
    doc["portalPassword"] = config.portalPassword;
    doc["portalTimeout"]  = config.portalTimeout;
    doc["offlineMode"]    = config.offlineMode;
    
    // MQTT
    doc["mqttServer"]     = config.mqttServer;
    doc["mqttPort"]       = config.mqttPort;
    doc["mqttTLS"]        = config.mqttTLS;
    doc["mqttTopic"]      = config.mqttTopic;
    doc["mqttUser"]       = config.mqttUser;
    #if DEBUG_SECURITY
    doc["mqttPassword"]   = config.mqttPassword;
    #else
    doc["mqttPassword"]   = "***";
    #endif

    // BLE
    doc["addr"]           = config.address;


    if (pretty)
        serializeJsonPretty(doc, destination);
    else
        serializeJson(doc, destination);
}