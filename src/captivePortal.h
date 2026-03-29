#pragma once

/**
 * @brief Sets up the captive portal and attempts to connect to WiFi.
 * 
 * This function handles the initial WiFi connection logic. If connection to 
 * the configured SSID fails, it starts an Access Point with a captive portal.
 * 
 * @return int 0 if connected to WiFi as station, 1 if in captive portal mode, -1 on error.
 */
int captivePortalSetup();

/**
 * @brief Handles background tasks for the captive portal.
 * 
 * This function should be called regularly in the main loop to process 
 * DNS requests and check for portal timeouts.
 */
void captivePortalLoop();
