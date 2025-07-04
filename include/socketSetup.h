#ifndef SOCKET_SETUP_H
#define SOCKET_SETUP_H

#include <WiFi.h>
#include "SPIFFS.h" 

#include "secrets.h"
#include "eventHandler.h"

#define LED 2

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

void socketBegin() {
    pinMode(LED, OUTPUT);

    Serial.print("IP to connect to client: ");
    Serial.println(WiFi.localIP());

    Serial.println("Mounting SPIFFS...");
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS mount failed!");
        return;
    }

    //SPIFFS UPLOAD LIST (debugging purposes)
    server.on("/list", HTTP_GET, []() {
        String output;
        File root = SPIFFS.open("/");
        File file = root.openNextFile();
        
        while (file) {
            output += String(file.name()) + " (" + String(file.size()) + " bytes)\n";
            file = root.openNextFile();
        }

        server.send(200, "text/plain", output);
    });


    // HTML
    server.on("/", HTTP_GET, []() {
        File file = SPIFFS.open("/index.html", "r");
        if (!file) {
            server.send(404, "text/plain", "File Not Found!");
            return;
        }
        server.streamFile(file, "text/html");
        file.close();
    });

    // JS
    server.on("/client.js", HTTP_GET, []() {
        File file = SPIFFS.open("/client.js", "r");
        if (!file) {
            server.send(404, "text/plain", "File Not Found!");
            return;
        }
        server.streamFile(file, "application/javascript");
        file.close();
    });
    server.on("/utils/timeFunctions.js", HTTP_GET, []() {
        File file = SPIFFS.open("/utils/timeFunctions.js", "r");
        if (!file) {
            server.send(404, "text/plain", "File Not Found!");
            return;
        }
        server.streamFile(file, "application/javascript");
        file.close();
    });
    server.on("/utils/sendMp3ToESP.js", HTTP_GET, []() {
        File file = SPIFFS.open("/utils/sendMp3ToESP.js", "r");
        if (!file) {
            server.send(404, "text/plain", "File Not Found!");
            return;
        }
        server.streamFile(file, "application/javascript");
        file.close();
    }); 

    // CSS
    server.on("/style.css", HTTP_GET, []() {
        File file = SPIFFS.open("/style.css", "r");
        if (!file) {
            server.send(404, "text/plain", "File Not Found!");
            return;
        }
        server.streamFile(file, "text/css");
        file.close();
    });

    server.on("/favicon.ico", HTTP_GET, []() {
        server.send(200, "image/x-icon", "");
    });

    server.begin();
    Serial.println("HTTP Server started.");

    delay(500);
    webSocket.begin();
    Serial.println("WebSocket Server started.");

    webSocket.onEvent(webSocketEvent);
}

#endif