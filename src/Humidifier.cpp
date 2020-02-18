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

    analogWriteFreq(100);
    analogWriteRange(100);
    _dutyCycle = 100;
}

void Humidifier::loop() {
    bool waterOK = digitalRead(_waterOKPin) == LOW;
    if (!waterOK) {
        digitalWrite(_fanPin, LOW);
        digitalWrite(_pwmPin, HIGH);
        logger.log("Stopping due to low water level.");
        _onHumidity = -1;
        _onTimestamp = 0;
        return;
    }

    if (influxClient.isDataAvailable()) {
        float humidity = influxClient.getQueryResult();
        influxClient.purgeData();
        logger.log("Humidity is %.1f", humidity);

        if (humidity < settings.getSettings()->hm.targetHumidityLow) {
            digitalWrite(_fanPin, HIGH);
            // the _pwmPin is inversed. 0/LOW means ON, 100/HIGH means off.
            analogWrite(_pwmPin, 100 - _dutyCycle);
            logger.log("Humidity lower than specified, turning on with duty cycle = %d%%", _dutyCycle);

            _onHumidity = humidity;
            _onTimestamp = millis();
        }
        
        if (humidity >= (0.98 * settings.getSettings()->hm.targetHumidityHigh)) {
            // We are pretty close to the targe humidity.
            digitalWrite(_fanPin, LOW);
            digitalWrite(_pwmPin, HIGH);
            logger.log("Humidity target reached, turning off.");

            if (_onTimestamp != 0) {
                // Recalc the required duty cycle to get the target humidity in a number of whole
                // intervals.

                // 1) Find out the humidity raise in 1 interval with 100% duty cycle.
                float cycleMillis = settings.getSettings()->ifxSettings.queryInterval * 1000.0;
                uint8_t cycles = round((millis() - _onTimestamp) / cycleMillis);
                float oneCycleHumidityRaise = (humidity - _onHumidity) * (100.0 / _dutyCycle) / cycles;
                logger.log("Humidity got from %.1f to %.1f in %d cycles with dc=%d%%", _onHumidity, humidity, cycles, _dutyCycle);
                logger.log("Maximum humidity raise on 1 cycle is %.1f", oneCycleHumidityRaise);
  
                // 2) Get the number of required cycles to reach the target humidity.
                float requiredCycles = (settings.getSettings()->hm.targetHumidityHigh - settings.getSettings()->hm.targetHumidityLow) / oneCycleHumidityRaise;
                logger.log("Required cycles to reach target humidity are %.1f", requiredCycles);

                // 3) Get the required duty cycle to reach the target humidity in a number of whole intervals.
                _dutyCycle = 100.0 * requiredCycles / ceil(requiredCycles);  // I.e. 100 * 2.7 / 3 = 90% duty cycle
                logger.log("Duty cycle for not overshooting target humidity is %d", _dutyCycle);

                // 4) Clean up.
                _onTimestamp = 0;
                _onHumidity = -1;
            }
        }
    }
}