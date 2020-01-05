#include "HumidMatic.h"

Logger logger = Logger();
Settings settings = Settings();

WiFiManager wifi = WiFiManager(&logger, &settings.getSettings()->network);
WebServer webServer = WebServer(&logger, &settings.getSettings()->network);
InfluxDBClient influxClient = InfluxDBClient(&logger,
                                             &wifi,
                                             &settings.getSettings()->ifxSettings,
                                             &settings.getSettings()->network);


void setup()
{ 
    Serial.begin(74880);
    while (! Serial) {
        delay(1);
    }

    pinMode(PIN_FAN_OUTPUT, OUTPUT);
    digitalWrite(PIN_FAN_OUTPUT, LOW);

    pinMode(PIN_PWM_OUTPUT, OUTPUT);
    digitalWrite(PIN_PWM_OUTPUT, HIGH);

    pinMode(PIN_WATER_LEVEL, INPUT);

    // Fan on.
    pinMode(PIN_FAN_OUTPUT, HIGH);

    settings.begin();
    wifi.begin();
    webServer.begin();
    influxClient.begin();

    wifi.connect();
}

bool isWaterOK() {
    return digitalRead(PIN_WATER_LEVEL) == HIGH;
}

void loop() {
    wifi.loop();
    webServer.loop();
    settings.loop();

    delay(100);
}
