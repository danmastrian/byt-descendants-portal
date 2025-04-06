#pragma once

#include <Arduino.h>

class SystemConfiguration
{
private:
    typedef struct PersistentConfigData
    {
        uint16_t dmxStartChannel;
        uint8_t brightness;
        uint8_t mode;
        bool isLocked;
    } PersistentConfigData;

    bool Save(const PersistentConfigData &data);

public:
    uint16_t dmxStartChannel = 99;
    uint8_t brightness = 97;
    uint8_t mode = 42;
    bool isLocked = false;

    const uint16_t DmxChannelCount = 8;

    bool InitializeStorage();
    bool Save();
};

extern SystemConfiguration sysConfig;
