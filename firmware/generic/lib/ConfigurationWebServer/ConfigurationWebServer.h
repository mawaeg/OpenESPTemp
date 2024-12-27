#ifndef CONFIGURATIONWEBSERVER_H
#define CONFIGURATIONWEBSERVER_H

#include "CredentialManager.h"

class ConfigurationWebServer {
    public:
        ConfigurationWebServer(CredentialManager manager);
        ~ConfigurationWebServer();

        void run();
    private:
        CredentialManager manager;
        bool is_finished = false;
};

const char form_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
    <head>
        <meta name="color-scheme" content="light dark">
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <title>OpenESPTemp Configurator</title>
        <style>
            h1 {
                text-align: center;
            }
            .container {
                margin: 0 auto;
                display: grid;
                grid-template-columns: 1fr 1fr;
                width: 250px;
                gap: 10px;
            }

        </style>
    </head>
    <body>
        <h1>OpenESPTemp Configurator</h1>
        <form action="/submit" method="POST" class="container">
            <label for="ssid">SSID</label>
            <input type="text" id="ssid" name="ssid">
            <label for="password">Password</label>
            <input type="password" id="password" name="password">
            <label for="post_url">Post URL</label>
            <input type="text" id="post_url" name="post_url">
            <label for="authorization">Authorization</label>
            <input type="text" id="authorization" name="authorization">
            <input type="submit" value="Submit">
        </form>
    </body>
</html>
)rawliteral";

#endif /* CONFIGURATIONWEBSERVER_H */