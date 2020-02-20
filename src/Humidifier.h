#pragma once

#include "HumidMatic.h"

class Humidifier {
    public:
        Humidifier(uint8_t fanPin, uint8_t pwmPin, uint8_t waterOKPin);
        void begin();
        void loop();
    private:
        void _calcNextCycleDuration(float currentHumidity, float targetHumidity);
        
        uint8_t _fanPin;
        uint8_t _pwmPin;
        uint8_t _waterOKPin;
        // Keep a timestamp when humidifier was turned on and off last time. These will be used
        // before the next humidifier ON cycle to calculate the required duration to get to the
        // target humidity levels.
        unsigned long _onTimestamp;
        unsigned long _offTimestamp;

        // The following will controll how long the humidifier should work. If it is 0 - this is
        // not in effect because this is the first humidification cycle. If > 0 the humidification
        // will be stopped when the _onTimestamp + _cycleDuration millis are reached.
        unsigned long _cycleDuration;
};
