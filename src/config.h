#pragma once
#include <Arduino.h>


/*
 * Debug
 */
#define DEBUG_SERIAL_PRINT 1 // 1: enable debug serial print, 0: disable debug serial print

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
  String wifiSSID;
  String wifiPassword;
// other configurations
  String name;
  // TODO: add more configuration parameters as needed
} config_t;
extern config_t config;
