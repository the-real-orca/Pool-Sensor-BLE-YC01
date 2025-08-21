#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <DNSServer.h>

#include "captivePortal.h"

#include "config.h"

#define WIFI_TIMEOUT 5000 // timeout for WiFi connection attempts in ms
IPAddress apIP(192, 168, 4, 1);

DNSServer dnsServer;
bool wiFiConnecting = false;
bool captiveMode = false;
unsigned long timestamp = 0;
unsigned long lastWifiUpdate = 0;

int captivePortalSetup()
{
    WiFi.disconnect(true, true);  // drop connection & clear credentials
    delay(50);
    WiFi.mode(WIFI_OFF);          // this calls esp_wifi_stop internally
    delay(50);

    while (true)
    {
        timestamp = millis();
        wl_status_t wifiStatus = WiFi.status();
        if (!captiveMode)
        {
            if (!wiFiConnecting)
            {
                // try to connect to WiFi
                lastWifiUpdate = timestamp;
                wiFiConnecting = true;
                dnsServer.stop();
                DEBUG_println("WiFi connect...");
                WiFi.mode(WIFI_STA);
                WiFi.begin(config.wifiSSID.c_str(), config.wifiPassword.c_str());
            }
            else
            {
                if (wifiStatus == WL_CONNECTED)
                {
                    DEBUG_print("WiFi successfully connected with IP: ");
                    DEBUG_println(WiFi.localIP());
                    captivePortalLoop();
                    return 0; // exit captive portal if connected
                }
                else
                {
                    if ((timestamp - lastWifiUpdate) > WIFI_TIMEOUT)
                    {
                        // switch to captive portal mode if not connected
                        WiFi.mode(WIFI_OFF);
                        delay(100);
                        lastWifiUpdate = timestamp;
                        wiFiConnecting = false;
                        captiveMode = true;
                        DEBUG_println("WiFi connection failed...");
                    }
                }
            }
        }
        else
        {
            // captive portal mode
            if (!wiFiConnecting)
            {
                DEBUG_println("starting captive portal AP");
                WiFi.mode(WIFI_AP);
                if (!WiFi.softAP(config.portalSSID.c_str(), config.portalPassword.c_str()))
                {
                    DEBUG_println("softAP failed");
                }
                delay(200);

                lastWifiUpdate = timestamp;
                wiFiConnecting = true;

                Serial.print("AP SSID: "); Serial.println(config.portalSSID);
                Serial.print("AP password: "); Serial.println(config.portalPassword);
                Serial.print("AP IP address: "); Serial.println(WiFi.softAPIP());

                // Setup the DNS server redirecting all the domains
                dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
                dnsServer.start(53, "*", WiFi.softAPIP());

                captivePortalLoop();
                return 1; // captive portal mode active
            }
        }
        delay(100);
    }
    return -1; // should never reach here
}

void captivePortalLoop()
{
    if (captiveMode)
    {
        dnsServer.processNextRequest();

        // check captive portal timeout
        if (config.portalTimeout > 0 && millis() > ((uint32_t) config.portalTimeout * 1000) )
        {
            DEBUG_println("Captive portal timeout reached");
            if ( WiFi.softAPgetStationNum() ) {
                DEBUG_println("waiting for clients to disconnect...");
                config.portalTimeout += 30; // extend timeout by 30 seconds
            } else {
                DEBUG_println("stopping AP and reboot...");
                WiFi.softAPdisconnect(true);
                WiFi.disconnect(true);
                captiveMode = false;
                wiFiConnecting = false;
                delay(500);
                ESP.restart(); // restart the ESP32
            }
        }
    }
}