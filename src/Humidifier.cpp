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
        _calcNextCycleDuration(humidity, settings.getSettings()->hm.targetHumidityHigh);
        // This can be invoked multiple times iff humidity is raising slow and on the next check cycle it is still
        // below the targetHumidityLow threshold.
        if (_onTimestamp = 0) {
            _onTimestamp = millis();
            _offTimestsamp = 0;
        }
    }

    if (humidityHigh || humidificationCycleCompleted) {
        digitalWrite(_fanPin, LOW);
        digitalWrite(_pwmPin, HIGH);

        if (humidityHigh) logger.log("Last humidity is %.1f, turning off", humidity);
        if (humidificationCycleCompelted) logger.log("Humidification cycle completed, turning off");

        _cycleDuration = 0;
        // This can be invoked multiple times iff humidity is falling slow and on the next check cycle it is while
        // the humidity is still above the targetHumidityHigh threshold.
        if (_offTimestamp = 0) {
            _offTimestamp = millis();
        }
    }
}

void Humidifier::_calcNextCycleDuration(float currentHumidity, float targetHumidity) {
    if (_onTimestamp == 0) {
        // There was no previous humidification cycle. Without such the next duty cycle
        // duration can not be calculated;
        return;
    }

    unsigned long now = millis();

    wifi.connect();
    int counter = 100;
    while (!wifi.isConnected() && counter > 0) {
        delay(100);
        counter--;
    }

    if (counter == 0) {
        logger.log("Failed to connect.");
        return;
    }

    logger.log("On timestamp is now - %dm", (now - _onTimestamp) / 60000);
    logger.log("Off timestamp is now - %dm", (now - _offTimestamp) / 60000);


    // 1) Find out the humidity at the start of the previous cycle.
    bool result = influxClient.query((now - _onTimestamp) / 60000);
    if (!result) {
        logger.log("Failed to get previous start cycle humidity.");
        wifi.disconnect();
        return;
    }
    float startCycleHumidity = influxClient.getQueryResult();
    influxClient.purgeData();

    // 2) Find out the humidity at the end of the previous cycle.
    result = influxClient.query((now - _offTimestamp) / 60000);
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

    wifi.disconnect();

    logger.log("Prev start/end humidity was %.1f / %.1f", startCycleHumidity, endCycleHumidity);
    logger.log("Last cycle duration was %d seconds", lastCycleDuration/1000);
    logger.log("Current humidity is %.1f, target humidity raise is %.1f", currentHumidity, targetHumidityRaise);

    if (startCycleHumidity > settings.getSettings()->hm.targetHumidityLow) {
        logger.log("Unexpected prev start cycle humidity");
        return;
    }


    if (endCycleHumidity < settings.getSettings()->hm.targetHumidityHigh) {
        logger.log("Unexpected prev end cycle humidity");
        return;
    }

    _cycleDuration = (targetHumidityRaise/lastHumidityRaise) * lastCycleDuration;
    logger.log("Next cycle duration is %d seconds", _cycleDuration / 1000);
}
