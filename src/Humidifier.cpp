#include "HumidMatic.h"
#include <math.h>

Humidifier::Humidifier(uint8_t fanPin, uint8_t pwmPin, uint8_t waterOKPin) {
    _fanPin = fanPin;
    _pwmPin = pwmPin;
    _waterOKPin = waterOKPin;
}

void Humidifier::begin() {
    pinMode(_fanPin, OUTPUT);
    pinMode(_pwmPin, OUTPUT);
    pinMode(_waterOKPin, INPUT);

    stop(true);
}

void Humidifier::loop() {
    if (!waterOK()) {
        logger.log("Stopping due to low water level.");
        stop();
        return;
    }

    if (_cycleDuration > 0 && millis() - _onTimestamp > _cycleDuration) {
        stop();
        logger.log("Cycle completed. Stopping.");
    }

    if (!influxClient.isDataAvailable()) {
        // No reason to go futher.
        return;
    }

    float humidity = influxClient.getQueryResult();
    influxClient.purgeData();
    logger.log("Humidity is %.1f", humidity);

    if (isRunning) {
        if (humidity >= settings.getSettings()->hm.targetHumidityHigh) {
            logger.log("Humidity above threshold. Stopping.");
            stop();
        }
    } else {
        if (humidity < settings.getSettings()->hm.targetHumidityLow) {
            logger.log("Humidity below threshold. Starting.");
            calcNextCycleDuration(humidity, settings.getSettings()->hm.targetHumidityHigh);
            start();
        }
    }
}

void Humidifier::cleanUp() {
    _onTimestamp = 0;
    _offTimestamp = 0;
    _cycleDuration = 0;
}

void Humidifier::calcNextCycleDuration(float currentHumidity, float targetHumidity) {
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

    if (!wifi.isConnected()) {
        logger.log("Failed to connect.");
        wifi.disconnect();
        cleanUp();
        return;
    }

    uint16_t prevCycleStartedBefore = (now - _onTimestamp) / 60000; 
    uint16_t prevCycleCompletedBefore = (now - _offTimestamp) / 60000;

    logger.log("Prev cycle started before %dm and completed before %dm",
        prevCycleStartedBefore,
        prevCycleCompletedBefore);

    // 1) Find out the humidity at the start of the previous cycle.
    influxClient.query(prevCycleStartedBefore);
    if (!influxClient.isDataAvailable()) {
        logger.log("Failed to get previous start cycle humidity.");
        wifi.disconnect();
        cleanUp();
        return;
    }
    float startCycleHumidity = influxClient.getQueryResult();
    influxClient.purgeData();

    // 2) Find out the humidity at the end of the previous cycle.
    influxClient.query((now - _offTimestamp) / 60000);
    if (!influxClient.isDataAvailable()) {
        logger.log("Failed to get previous end cycle humidity.");
        wifi.disconnect();
        cleanUp();
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

    if (startCycleHumidity > settings.getSettings()->hm.targetHumidityLow ||
        startCycleHumidity >= endCycleHumidity+1) {
        logger.log("Possibly incorrect prev cycle start/end humidity");
        cleanUp();
        return;
    }

    cleanUp();
    _cycleDuration = (targetHumidityRaise/lastHumidityRaise) * lastCycleDuration;
    logger.log("Next cycle duration is %d seconds", _cycleDuration / 1000);
}

void Humidifier::start() {
    if (isRunning) {
        // Already running.
        return;
    }

    digitalWrite(_fanPin, HIGH);
    digitalWrite(_pwmPin, LOW);

    isRunning = true;
    _onTimestamp = millis();
}

void Humidifier::stop(boolean force) {
    if (!isRunning && !force) {
        // Already stopped.
        return;
    }

    digitalWrite(_fanPin, LOW);
    digitalWrite(_pwmPin, HIGH);

    isRunning = false;
    _offTimestamp = millis();
    _cycleDuration = 0;
}

boolean Humidifier::waterOK() {
    return digitalRead(_waterOKPin) == LOW;
}
