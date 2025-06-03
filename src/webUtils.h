#pragma once
#include <ESPAsyncWebServer.h>

String formatBytes(size_t bytes);
String getContentType(String filename);

void webServerInit(AsyncWebServer &webServer, bool isCaptive = false);

