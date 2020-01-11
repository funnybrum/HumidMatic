#include "HumidMatic.h"

Humidifier::Humidifier(uint8_t fanPin, uint8_t pwmPin, uint8_t waterOKPin) {
    _fanPin = fanPin;
    _pwmPin = pwmPin;
    _waterOKPin = waterOKPin;
}

void Humidifier::begin() {
    pinMode(_fanPin, OUTPUT);
    digitalWrite(_fanPin, LOW);

    pinMode(_pwmPin, OUTPUT);
    digitalWrite(_pwmPin, HIGH);

    pinMode(_waterOKPin, INPUT);
}

void Humidifier::loop() {
    bool waterOK = digitalRead(_waterOKPin) == LOW;
    if (!waterOK) {
        digitalWrite(_fanPin, LOW);
        digitalWrite(_pwmPin, HIGH);
        logger.log("Stopping due to low water level.");
        return;
    } else {
        logger.log("Water level is OK.");
    }


    if (influxClient.isDataAvailable()) {
        if (influxClient.getQueryResult() < settings.getSettings()->hm.targetHumidity) {
            digitalWrite(_fanPin, HIGH);
            digitalWrite(_pwmPin, LOW);
            logger.log("Humidity lower than specified, turning on.");
        } else {
            digitalWrite(_fanPin, LOW);
            digitalWrite(_pwmPin, HIGH);
            logger.log("Humidity target reached, turning off.");
        }
        influxClient.purgeData();
    }
}