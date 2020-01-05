#include "HumidMatic.h"

char buffer[4096];

WebServer::WebServer(Logger* logger, NetworkSettings* networkSettings)
    :WebServerBase(networkSettings, logger, NULL) {
}

void WebServer::registerHandlers() {
    server->on("/", std::bind(&WebServer::handle_root, this));
    server->on("/settings", std::bind(&WebServer::handle_settings, this));
    server->on("/on", std::bind(&WebServer::handle_on, this));
    server->on("/off", std::bind(&WebServer::handle_off, this));
}

void WebServer::handle_root() {
    server->sendHeader("Location","/settings");
    server->send(303);
}

void WebServer::handle_settings() {
    bool save = false;

    wifi.parse_config_params(this, save);

    if (save) {
        settings.save();
    }

    char network_settings[strlen_P(NETWORK_CONFIG_PAGE) + 32];
    wifi.get_config_page(network_settings);


    sprintf_P(
        buffer,
        CONFIG_PAGE,
        network_settings);
    server->send(200, "text/html", buffer);
}

void WebServer::handle_on() {
    digitalWrite(PIN_PWM_OUTPUT, LOW);
    digitalWrite(PIN_FAN_OUTPUT, HIGH);
    server->send(200);
}

void WebServer::handle_off() {
    digitalWrite(PIN_PWM_OUTPUT, HIGH);
    digitalWrite(PIN_FAN_OUTPUT, LOW);
    server->send(200);
}
