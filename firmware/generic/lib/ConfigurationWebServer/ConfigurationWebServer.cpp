#include <Arduino.h>

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "ConfigurationWebServer.h"

AsyncWebServer server(80);


ConfigurationWebServer::ConfigurationWebServer(CredentialManager manager){
    this->manager = manager;
}

ConfigurationWebServer::~ConfigurationWebServer(){
}


void ConfigurationWebServer::run(){
    WiFi.softAP("OpenESPTemp Configurator");

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", form_html);
    });
    server.on("/submit", HTTP_POST, [this](AsyncWebServerRequest *request){
        bool is_ok = true;
        for (int i = 0; i < credential_keys.size(); i++){
            if (request->hasParam(credential_keys[i], true)){
            manager.put(credential_keys[i], request->getParam(credential_keys[i], true)->value().c_str());

            } else {
                is_ok = false;
            }
        }

        if (is_ok == true){
            request->send(200, "text/plain", "Configuration received");
            this->is_finished = true;
        }
        else {
            request->send(400, "text/plain", "Bad Request");
        }

    });
    server.begin();

    while (!this->is_finished){
        __asm__("nop\n\t"); // Needed (otherwise we wont escape from the loop when finished)
    }

    delay(2000);
    server.end();
    WiFi.softAPdisconnect();
}
