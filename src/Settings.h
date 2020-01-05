#pragma once

#include "SettingsBase.h"
#include "WiFi.h"
#include "InfluxDBClient.h"

struct SettingsData {
    NetworkSettings network;
    InfluxDBClientSettings ifxSettings;
    struct HumidMatic {
        uint8_t targetHumidity;
    } hm;
};

class Settings: public SettingsBase<SettingsData> {
    public:
        Settings();
        SettingsData* getSettings();

    protected:
        void initializeSettings();

    private:
        SettingsData settingsData;
};
