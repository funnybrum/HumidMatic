#include "HumidMatic.h"

char buffer[4096];

WebServer::WebServer(Logger* logger, NetworkSettings* networkSettings)
    :WebServerBase(networkSettings, logger, NULL) {
}

void WebServer::registerHandlers() {
    server->on("/", std::bind(&WebServer::handle_root, this));
    server->on("/settings", std::bind(&WebServer::handle_settings, this));
}

void WebServer::handle_root() {
    server->sendHeader("Location","/settings");
    server->send(303);
}

void WebServer::handle_settings() {
    bool save = false;

    wifi.parse_config_params(this, save);
    influxClient.parse_config_params(this, save);
    process_setting("humidity", settings.getSettings()->hm.targetHumidity, save);


    if (save) {
        settings.save();
    }

    char network_settings[strlen_P(NETWORK_CONFIG_PAGE) + 32];
    wifi.get_config_page(network_settings);

    char influx_settings[strlen_P(INFLUXDB_CLIENT_CONFIG_PAGE) + 128];
    influxClient.get_config_page(influx_settings);

    sprintf_P(
        buffer,
        CONFIG_PAGE,
        network_settings,
        influx_settings,
        settings.getSettings()->hm.targetHumidity);
    server->send(200, "text/html", buffer);
}
