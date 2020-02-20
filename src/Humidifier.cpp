#include "HumidMatic.h"
#include <math.h>

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
        _onTimestamp = 0;
        _cycleDuration = 0;
        return;
    }

    float humidity = influxClient.getQueryResult();
    bool isDataAvailable = influxClient.isDataAvailable();
    if (isDataAvailable) {
        logger.log("Humidity is %.1f", humidity);
        influxClient.purgeData();
    }

    bool humidityLow = isDataAvailable &&
        (humidity < settings.getSettings()->hm.targetHumidityLow);

    bool humidityHigh = isDataAvailable &&
        (humidity >= settings.getSettings()->hm.targetHumidityHigh);

    bool humidificationCycleCompleted = _cycleDuration > 0 &&
         (millis() - _onTimestamp > _cycleDuration);

    if (humidityLow) {
        digitalWrite(_fanPin, HIGH);
        digitalWrite(_pwmPin, LOW);
        logger.log("Last humidity is %.1f, turning on.", humidity);
        _calcNextCycleDuration(humidity, settings.getSettings()->hm.targetHumidityLow);
        _onTimestamp = millis();
    }
    
    if (humidityHigh) {
        digitalWrite(_fanPin, LOW);
        digitalWrite(_pwmPin, HIGH);
        logger.log("Last humidity is %.1f, turning off", humidity);
        _cycleDuration = 0;
        _offTimestamp = millis();
    }
    
    if (humidificationCycleCompleted) {
        digitalWrite(_fanPin, LOW);
        digitalWrite(_pwmPin, HIGH);
        logger.log("Humidification cycle completed, turning off");
        _cycleDuration = 0;
        _offTimestamp = millis();
    }
}

void Humidifier::_calcNextCycleDuration(float currentHumidity, float targetHumidity) {
    if (_onTimestamp == 0) {
        // There was no previous humidification cycle. Without such the next duty cycle
        // duration can not be calculated;
        return;
    }

    wifi.connect();

    // 1) Find out the humidity at the start of the previous cycle.
    bool result = influxClient.query((millis() - _onTimestamp) / 60000);
    if (!result) {
        logger.log("Failed to get previous start cycle humidity.");
        wifi.disconnect();
        return;
    }
    float startCycleHumidity = influxClient.getQueryResult();
    influxClient.purgeData();

    // 2) Find out the humidity at the end of the previous cycle.
    result = influxClient.query((millis() - _offTimestamp) / 60000);
    if (!result) {
        logger.log("Failed to get previous end cycle humidity.");
        wifi.disconnect();
        return;
    }
    float endCycleHumidity = influxClient.getQueryResult();
    influxClient.purgeData();

    // 3) Based on the previous cycle duration and the humidity raise in it - calculate the
    // required cycle length to reach the upper humidity threshold.
    float lastHumidityRaise = endCycleHumidity - startCycleHumidity;
    unsigned long lastCycleDuration = _offTimestamp - _onTimestamp;
    float targetHumidityRaise = targetHumidity - currentHumidity;
    _cycleDuration = (targetHumidityRaise/lastHumidityRaise) * lastCycleDuration;

    wifi.disconnect();
    logger.log("Prev start/end humidity was %.1f / %.1f", startCycleHumidity, endCycleHumidity);
    logger.log("Last cycle duration was %d seconds", lastCycleDuration/1000);
    logger.log("Current humidity is %.1f, target humidity raise is %.1f", currentHumidity, targetHumidityRaise);
    logger.log("Next cycle duration is %d seconds", _cycleDuration / 1000);
}