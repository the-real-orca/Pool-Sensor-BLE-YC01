#pragma once
#include <ESPAsyncWebServer.h>

/**
 * @brief Helper function to format a byte count into a human readable string.
 * @param bytes Size in bytes
 * @return String formatted size (B, kB, MB, GB)
 */
String formatBytes(size_t bytes);

/**
 * @brief Get the content type for a given filename based on its extension.
 * @param filename File name with extension
 * @return String MIME type
 */
String getContentType(String filename);

/**
 * @brief Scans for available WiFi networks and updates the internal JSON list.
 */
void scanWifiNetworks();

/**
 * @brief Initializes the web server handlers.
 * @param webServer Reference to AsyncWebServer object
 * @param isCaptive true if the device is currently in captive portal mode
 */
void webServerInit(AsyncWebServer &webServer, bool isCaptive = false);

/**
 * @brief Handles background tasks for the web server and utilities.
 */
void webUtilsLoop();
