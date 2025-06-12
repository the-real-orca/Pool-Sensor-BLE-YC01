
#include <Arduino.h>

#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <Update.h>
#include <Ticker.h>

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

static Ticker restartTick;

// format bytes
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

String templateProcessorUpdate(const String &var)
{
    if (var == "FILES")
    {
        String str = "<ul>";
        File root = SPIFFS.open("/");
        File file = root.openNextFile();
        while (file)
        {
            String filename(file.name());
            str += "<li><a href='" + filename + "'>" + filename + "</a> (" + formatBytes(file.size()) + ")  <b><a href='del?file=" + filename + "'>X</a></b></li>";
            file = root.openNextFile();
        }
        str += "</ul>";
        return str;
    }
    else if (var == "TOTAL")
    {
        size_t total = SPIFFS.totalBytes();
        return String(formatBytes(total));
    }
    else if (var == "USED")
    {
        size_t used = SPIFFS.usedBytes();
        return String(formatBytes(used));
    }
    else if (var == "FREE")
    {
        size_t total = SPIFFS.totalBytes();
        size_t used = SPIFFS.usedBytes();
        return String(formatBytes(total - used));
    }
    return String();
}

void handleUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!index) {
        DEBUG_print("start firmware uploade");
        if ( !Update.begin(UPDATE_SIZE_UNKNOWN) ) { 
            //start with max available size
            Update.printError(Serial);
        }        
    }
    DEBUG_print(".");
    if ( Update.write(data, len) != len ) {
        Update.printError(Serial);
    }
    if (final) {
        DEBUG_println("\nupload finished");
        if ( Update.end(true) ) {
            Serial.println("OTA update successfull, restarting ...");
            restartTick.once(2, []() { ESP.restart(); });            
        } else {
            Update.printError(Serial);
        }

    }
}

void handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!filename.startsWith("/")) {
        filename = "/" + filename;
    }
    if (!index) {
        DEBUG_print("start upload file: "); DEBUG_println(filename.c_str());
        if (SPIFFS.exists(filename)) {
            SPIFFS.remove(filename);
        }
        File file = SPIFFS.open(filename, FILE_WRITE);
        file.close(); // Datei wird später geöffnet
    }

    File file = SPIFFS.open(filename, FILE_APPEND);
    if (file) {
        file.write(data, len);
        file.close();
    }

    if (final) {
        DEBUG_print("upload finished: "); DEBUG_print(filename.c_str());
        DEBUG_print(" "); DEBUG_println(formatBytes(index + len));
    }
}

void handleFileDelete(AsyncWebServerRequest *request)
{
    // check if the request has a file parameter
    const AsyncWebParameter *p = request->getParam("file");
    if (!p) {
        request->send(400, "text/plain", "Bad Request: No file specified");
        return;
    }
    // get the file name from the parameter
    String filename = p->value();
    // check if the file name is empty
    if (filename.isEmpty()) {
        request->send(400, "text/plain", "Bad Request: No file specified");
        return;
    }
    // check if the file name starts with a slash
    if (!filename.startsWith("/")) {
        filename = "/" + filename; // prepend slash if missing
    }
    // check if the file name contains ".." to prevent directory traversal
    if (filename.indexOf("..") >= 0) {
        // security alert -> trying to exit web root
        request->send(500, "text/plain", "BAD ARGS");
        return;
    }
    // check if the file exists in SPIFFS
    if (!SPIFFS.exists(filename)) {
        request->send(404, "text/plain", filename + " not found");
        return;
    }
    // remove the file from SPIFFS
    if (SPIFFS.remove(filename)) {
        request->send(200, "text/html", RELOADPREV_HTML);
    } else {
        request->send(500, "text/plain", "Failed to delete " + filename);
    }
}

void webServerInit(AsyncWebServer &webServer, bool isCaptive)
{

    webServer.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

    webServer.onNotFound([isCaptive](AsyncWebServerRequest *request) {
        DEBUG_print("not found: "); DEBUG_println(request->url());
        if (!SPIFFS.exists("/index.html")) {
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

    webServer.on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", UPDATE_HTML, templateProcessorUpdate); 
    });
    webServer.on("/execupdate", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "update finished");
    }, handleUpdate);

    webServer.on("/fileupload", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "upload finished"); 
    }, handleFileUpload);
    webServer.on("/del", HTTP_GET, handleFileDelete);

    webServer.serveStatic("/config", SPIFFS, "/config.html");
    webServer.on("/config.json", HTTP_PUT, [](AsyncWebServerRequest *request) {
        request->send(200,"text/plain", "ok");
    }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        if ( index != 0 || len < 2 )
            return request->send(500, "text/plain", "BAD CONFIG");

        File file = SPIFFS.open("/config.json", "w");
        file.write(data, len);
        file.close();
        DEBUG_println("config.json saved, reboot device in 2 second");
        restartTick.once(2, []() { ESP.restart(); });
    });

    // enable CORS for all origins
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Accept, Content-Type, Authorization");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Credentials", "true");
    
}