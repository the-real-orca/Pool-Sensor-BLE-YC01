
#include <Arduino.h>
#include <map>

#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <Update.h>
#include <Ticker.h>

#include <WiFi.h>
#include "webUtils.h"
#include "config.h"

const char *UPDATE_HTML =
#include "update.html.h"
    ;

const char *NOFILESYSTEM_HTML =
#include "nofilesystem.html.h"
    ;

const char *RELOADPREV_HTML =
#include "reloadprev.html.h"
    ;

String formatBytes(size_t bytes)
{
    if (bytes < 1024)
    {
        return String(bytes) + "B";
    }
    else if (bytes < (1024 * 1024))
    {
        return String(bytes / 1024.0) + "kB";
    }
    else if (bytes < (1024 * 1024 * 1024))
    {
        return String(bytes / 1024.0 / 1024.0) + "MB";
    }
    else
    {
        return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
    }
}

String getContentType(String filename)
{
    // TODO  if (server.hasArg("download")) return "application/octet-stream";
    if (filename.endsWith(".htm"))
        return "text/html";
    else if (filename.endsWith(".html"))
        return "text/html";
    else if (filename.endsWith(".css"))
        return "text/css";
    else if (filename.endsWith(".js"))
        return "application/javascript";
    else if (filename.endsWith(".png"))
        return "image/png";
    else if (filename.endsWith(".gif"))
        return "image/gif";
    else if (filename.endsWith(".jpg"))
        return "image/jpeg";
    else if (filename.endsWith(".ico"))
        return "image/x-icon";
    else if (filename.endsWith(".xml"))
        return "text/xml";
    else if (filename.endsWith(".json"))
        return "text/json";
    else if (filename.endsWith(".pdf"))
        return "application/x-pdf";
    else if (filename.endsWith(".zip"))
        return "application/x-zip";
    else if (filename.endsWith(".gz"))
        return "application/x-gzip";
    return "text/plain";
}

/**
 * @brief Template processor for the update page.
 * @param var Template variable name
 * @return String processed variable value
 */
static String templateProcessorUpdate(const String &var)
{
    if (var == "FILES")
    {
        String str = "<ul>";
        File root = LittleFS.open("/");
        File file = root.openNextFile();
        while (file)
        {
            String filename(file.name());
            str += "<li><a href='" + filename + "'>" + filename + "</a> (" + formatBytes(file.size()) + ")  <b><i><a href='del?file=" + filename + "'>(del)</a></i></b></li>";
            file = root.openNextFile();
        }
        str += "</ul>";
        return str;
    }
    else if (var == "TOTAL")
    {
        size_t total = LittleFS.totalBytes();
        return String(formatBytes(total));
    }
    else if (var == "USED")
    {
        size_t used = LittleFS.usedBytes();
        return String(formatBytes(used));
    }
    else if (var == "FREE")
    {
        size_t total = LittleFS.totalBytes();
        size_t used = LittleFS.usedBytes();
        return String(formatBytes(total - used));
    }
    else if (var == "BUILDDATE")
    {
        return String(__DATE__ + String(" ") + __TIME__);
    }
    return String();
}

/**
 * @brief Handles firmware/filesystem updates via HTTP POST.
 * @param request Pointer to AsyncWebServerRequest
 * @param filename Name of the uploaded file
 * @param index Current byte offset of the upload
 * @param data Data pointer
 * @param len Length of the data chunk
 * @param final true if this is the last chunk
 */
static void handleUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
    if (!index)
    {
        DEBUG_print("start firmware uploade");
        if (!Update.begin(UPDATE_SIZE_UNKNOWN))
        {
            // start with max available size
            Update.printError(Serial);
        }
    }
    DEBUG_print(".");
    if (Update.write(data, len) != len)
    {
        Update.printError(Serial);
    }
    if (final)
    {
        DEBUG_println("\nupload finished");
        if (Update.end(true))
        {
            requestReboot("OTA update successful");
        }
        else
        {
            Update.printError(Serial);
        }
    }
}

/**
 * @brief Handles generic file uploads to LittleFS.
 * @param request Pointer to AsyncWebServerRequest
 * @param filename Name of the uploaded file
 * @param index Current byte offset of the upload
 * @param data Data pointer
 * @param len Length of the data chunk
 * @param final true if this is the last chunk
 */
static void handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
    if (!filename.startsWith("/"))
    {
        filename = "/" + filename;
    }
    if (!index)
    {
        DEBUG_print("start upload file: ");
        DEBUG_println(filename.c_str());
        if (LittleFS.exists(filename))
        {
            LittleFS.remove(filename);
        }
        File file = LittleFS.open(filename, FILE_WRITE);
        file.close(); // Datei wird später geöffnet
    }

    File file = LittleFS.open(filename, FILE_APPEND);
    if (file)
    {
        file.write(data, len);
        file.close();
    }

    if (final)
    {
        DEBUG_print("upload finished: ");
        DEBUG_print(filename.c_str());
        DEBUG_print(" ");
        DEBUG_println(formatBytes(index + len));
    }
}

/**
 * @brief Handles file deletion from LittleFS.
 * @param request Pointer to AsyncWebServerRequest
 */
static void handleFileDelete(AsyncWebServerRequest *request)
{
    // check if the request has a file parameter
    const AsyncWebParameter *p = request->getParam("file");
    if (!p)
    {
        request->send(400, "text/plain", "Bad Request: No file specified");
        return;
    }
    // get the file name from the parameter
    String filename = p->value();
    // check if the file name is empty
    if (filename.isEmpty())
    {
        request->send(400, "text/plain", "Bad Request: No file specified");
        return;
    }
    // check if the file name starts with a slash
    if (!filename.startsWith("/"))
    {
        filename = "/" + filename; // prepend slash if missing
    }
    // check if the file name contains ".." to prevent directory traversal
    if (filename.indexOf("..") >= 0)
    {
        // security alert -> trying to exit web root
        request->send(500, "text/plain", "BAD ARGS");
        return;
    }
    // check if the file exists in LittleFS
    if (!LittleFS.exists(filename))
    {
        request->send(404, "text/plain", filename + " not found");
        return;
    }
    // remove the file from LittleFS
    if (LittleFS.remove(filename))
    {
        request->send(200, "text/html", RELOADPREV_HTML);
    }
    else
    {
        request->send(500, "text/plain", "Failed to delete " + filename);
    }
}

/**
 * @brief Internal structure for temporary WiFi scanning results.
 */
struct WifiEntry
{
    String ssid;
    int32_t rssi;
    uint8_t encryptionType;
    uint8_t channel;
};

bool scanWifi = false;
String wifiListJson = "[]";
void scanWifiNetworks()
{
    std::map<String, WifiEntry> bestBySSID;

    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; ++i)
    {
        String ssid = WiFi.SSID(i);
        int32_t rssi = WiFi.RSSI(i);
        uint8_t enc = WiFi.encryptionType(i);
        uint8_t chan = WiFi.channel(i);

        if (ssid.length() == 0)
            continue; // ignore empty SSIDs

        if (bestBySSID.find(ssid) == bestBySSID.end() || rssi > bestBySSID[ssid].rssi)
        {
            bestBySSID[ssid] = {ssid, rssi, enc, chan};
        }
    }

    wifiListJson = "";
    for (auto const &pair : bestBySSID)
    {
        if (wifiListJson.isEmpty())
            wifiListJson = "[";
        else
            wifiListJson += ",";
        wifiListJson += "{";
        wifiListJson += "\"ssid\":\"" + pair.second.ssid + "\"";
        wifiListJson += ",\"enc\": " + String(pair.second.encryptionType);
        wifiListJson += ",\"rssi\": " + String(pair.second.rssi);
        wifiListJson += "}";
    }
    wifiListJson += "]";
    DEBUG_println("WiFi Scan completed.");
}

void webServerInit(AsyncWebServer &webServer, bool isCaptive)
{

    webServer.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    webServer.onNotFound([isCaptive](AsyncWebServerRequest *request)
        {
        DEBUG_print("not found: "); DEBUG_println(request->url());
        if (!LittleFS.exists("/index.html")) {
            // if no filesystem, send no filesystem html
            DEBUG_println("no filesystem, sending no filesystem html");
            request->send(200, "text/html", NOFILESYSTEM_HTML);
        } else {
            if ( isCaptive ) {
                DEBUG_println("captive portal mode, redirecting to index.html");
                request->send(200, "text/html", "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='0; url=/'></head><body>Captive Portal</body></html>");
            } else {
                request->send(404, "text/plain", "Not found");
            }
        } 
    });

    webServer.on("/update", HTTP_GET, [](AsyncWebServerRequest *request)
        { request->send(200, "text/html", UPDATE_HTML, templateProcessorUpdate); });
    webServer.on("/execupdate", HTTP_POST, [](AsyncWebServerRequest *request)
        { request->send(200, "text/plain", "update finished"); }, handleUpdate);

    webServer.on("/fileupload", HTTP_POST, [](AsyncWebServerRequest *request)
        { request->send(200, "text/plain", "upload finished"); }, handleFileUpload);
    webServer.on("/del", HTTP_GET, handleFileDelete);

    webServer.on("/config", HTTP_GET, [](AsyncWebServerRequest *request)
        {
            request->send(LittleFS, "/config.html", "text/html");
            DEBUG_println("config -> config.html");
            scanWifi = true; // trigger wifi scan on config load
        });

    webServer.on("/config.json", HTTP_PUT, [](AsyncWebServerRequest *request)
        { request->send(200, "text/plain", "ok"); }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
            {
                if ( index != 0 || len < 2 )
                    return request->send(500, "text/plain", "BAD CONFIG");

                File file = LittleFS.open("/config.json", "w");
                file.write(data, len);
                file.close();
                requestReboot("Config saved");
            });
    webServer.on("/wifiList", HTTP_GET, [](AsyncWebServerRequest *request)
        {
            request->send(200, "application/json", wifiListJson);
            scanWifi = true; // trigger wifi scan on config load
        });

    // enable CORS for all origins
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Accept, Content-Type, Authorization");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Credentials", "true");
}

void webUtilsLoop()
{
    if (scanWifi)
    {
        scanWifi = false;
        scanWifiNetworks();
    }
}
