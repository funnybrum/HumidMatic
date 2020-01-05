#pragma once

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>

#include "user_interface.h"

#include "esp8266-base.h"

#define HTTP_PORT 80
#define HOSTNAME "humid-matic"

#define PIN_WATER_LEVEL 13
#define PIN_PWM_OUTPUT 4
#define PIN_FAN_OUTPUT 12

extern Logger logger;
extern Settings settings;
extern WiFiManager wifi;
