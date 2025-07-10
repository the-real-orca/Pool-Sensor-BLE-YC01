#pragma once
#include <ESPAsyncWebServer.h>

String formatBytes(size_t bytes);
String getContentType(String filename);

void scanWifiNetworks();

void webServerInit(AsyncWebServer &webServer, bool isCaptive = false);

void webUtilsLoop();

