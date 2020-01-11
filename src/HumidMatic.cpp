#include "HumidMatic.h"

Logger logger = Logger();
Settings settings = Settings();

WiFiManager wifi = WiFiManager(&logger, &settings.getSettings()->network);
WebServer webServer = WebServer(&logger, &settings.getSettings()->network);
InfluxDBClient influxClient = InfluxDBClient(&logger,
                                             &wifi,
                                             &settings.getSettings()->ifxSettings,
                                             &settings.getSettings()->network);
Humidifier humidifier = Humidifier(PIN_FAN_OUTPUT, PIN_PWM_OUTPUT, PIN_WATER_LEVEL);

void setup()
{ 
    Serial.begin(74880);
    while (! Serial) {
        delay(1);
    }

    humidifier.begin();
    settings.begin();
    wifi.begin();
    webServer.begin();
    influxClient.begin();

    wifi.connect();
}

void loop() {
    wifi.loop();
    webServer.loop();
    settings.loop();
    influxClient.loop();
    humidifier.loop();

    delay(100);
}
