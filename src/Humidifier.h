#pragma once

#include "HumidMatic.h"

class Humidifier {
    public:
        Humidifier(uint8_t fanPin, uint8_t pwmPin, uint8_t waterOKPin);
        void begin();
        void loop();
    private:
        uint8_t _fanPin;
        uint8_t _pwmPin;
        uint8_t _waterOKPin;
};
