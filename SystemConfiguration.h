#pragma once

#include <SPI.h>
#include <SdFat.h>
#include <Adafruit_SPIFlash.h>
#include "flash_config.h"

#define D_CONFIG "/config"
#define F_CONFIG_SYSCONFIG "/config/sysconfig.bin"

const int BRIGHTNESS_MAX = 255;
const int DMX_UNIVERSE_SIZE = 512;

extern void StartupMessage(const char* msg);

typedef struct PersistentConfigData
{
  uint16_t dmxStartChannel;
  uint8_t brightness;
  uint8_t mode;  
} PersistentConfigData;

extern Adafruit_SPIFlash flash(&flashTransport);
extern FatVolume fatfs;

class SystemConfiguration
{
private:

    PersistentConfigData defaultData { 0, 50, 0 };

public:

    uint16_t dmxStartChannel = 99;
    uint8_t brightness = 97;
    uint8_t mode = 42;
    
    const int DmxChannelCount = 5;

    bool InitializeStorage()
    {
        StartupMessage("InitializeStorage...");
        
        // Initialize flash library and check its chip ID.
        if (!flash.begin())
        {
            Serial.println(F("Error, failed to initialize flash chip!"));
            return false;
        }
        Serial.print(F("Flash chip JEDEC ID: 0x"));
        Serial.println(flash.getJEDECID(), HEX);
        StartupMessage("SD flash init OK");

        // First call begin to mount the filesystem.  Check that it returns true
        // to make sure the filesystem was mounted.
        if (!fatfs.begin(&flash))
        {
            Serial.println(F("Error, failed to mount newly formatted filesystem!"));
            return false;
        }
        StartupMessage("FAT FS init OK");

        if (!fatfs.exists(D_CONFIG))
        {
            Serial.println(F("Test directory not found, creating..."));
            fatfs.mkdir(D_CONFIG);

            if (!fatfs.exists(D_CONFIG))
            {
            Serial.println(F("Error, failed to create directory!"));
            return false;
            }
        }

        if (!fatfs.exists(F_CONFIG_SYSCONFIG))
        {
            // Initialize with default settings
            Save(defaultData);
        }

        // Now open the same file but for reading.
        File32 readFile = fatfs.open(F_CONFIG_SYSCONFIG, FILE_READ);
        if (!readFile)
        {
            Serial.println(F("Error, failed to open " F_CONFIG_SYSCONFIG " for reading!"));
            return false;
        }

        StartupMessage("Reading config");

        // Read data using the same read, find, readString, etc. functions as when
        // using the serial class.  See SD library File class for more documentation:
        //   https://www.arduino.cc/en/reference/SD

        PersistentConfigData loadedConfig;
        if (readFile.read(&loadedConfig, sizeof(loadedConfig)))
        {
            this->dmxStartChannel = loadedConfig.dmxStartChannel;
            this->brightness = loadedConfig.brightness;
            StartupMessage("Config OK");  
        }
        readFile.close();

        return true;
    }

    bool Save(const PersistentConfigData& data)
    {
        File32 writeFile = fatfs.open(F_CONFIG_SYSCONFIG, FILE_WRITE);
        if (!writeFile)
        {
            Serial.println(F("Error, failed to open " F_CONFIG_SYSCONFIG " for writing!"));
            return false;
        }
        Serial.println(F("Opened file " F_CONFIG_SYSCONFIG " for writing/appending..."));
    
        writeFile.write(&data, sizeof(data));
        writeFile.flush();
        writeFile.close();
        Serial.println(F("Wrote to file " F_CONFIG_SYSCONFIG "!"));
    }
};

extern SystemConfiguration sysConfig;
