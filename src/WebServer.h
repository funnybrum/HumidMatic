#pragma once

#include "HumidMatic.h"
#include "WebServerBase.h"
#include "WebPages.h"

class WebServer: public WebServerBase {
    public:
        WebServer(Logger* logger, NetworkSettings* networkSettings);
        void registerHandlers();
        void handle_root();
        void handle_settings();
        void handle_on();
        void handle_off();
};
