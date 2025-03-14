#include "SystemConfiguration.h"
#include "Display.h"

#include <SPI.h>
#include <SdFat.h>
#include <Adafruit_SPIFlash.h>
#include "flash_config.h"

Adafruit_SPIFlash flash(&flashTransport);

FatVolume fatfs;

#define DIR_CONFIG "/config"
#define FILE_CONFIG_SYSCONFIG "/config/sysconfig.bin"

SystemConfiguration sysConfig;

bool SystemConfiguration::InitializeStorage()
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

    if (!fatfs.exists(DIR_CONFIG))
    {
        Serial.println(F("Test directory not found, creating..."));
        fatfs.mkdir(DIR_CONFIG);

        if (!fatfs.exists(DIR_CONFIG))
        {
            Serial.println(F("Error, failed to create directory!"));
            return false;
        }
    }

    if (!fatfs.exists(FILE_CONFIG_SYSCONFIG))
    {
        StartupMessage("Intializing config");
        delay(10000);

        // Default settings
        PersistentConfigData data{0, 50, 0};
        if (!Save(data))
        {
            Serial.println(F("Error, failed to create " FILE_CONFIG_SYSCONFIG " file!"));
            return false;
        }
    }

    // Now open the same file but for reading.
    File32 readFile = fatfs.open(FILE_CONFIG_SYSCONFIG, FILE_READ);
    if (!readFile)
    {
        Serial.println(F("Error, failed to open " FILE_CONFIG_SYSCONFIG " for reading!"));
        return false;
    }

    StartupMessage("Reading config");

    // Read data using the same read, find, readString, etc. functions as when
    // using the serial class.  See SD library File class for more documentation:
    //   https://www.arduino.cc/en/reference/SD

    PersistentConfigData loadedConfig;
    if (readFile.read(&loadedConfig, sizeof(loadedConfig)) == sizeof(loadedConfig))
    {
        this->dmxStartChannel = loadedConfig.dmxStartChannel;
        this->brightness = loadedConfig.brightness;
        this->mode = loadedConfig.mode;

        StartupMessage("Config OK");
    }
    readFile.close();

    return true;
}

bool SystemConfiguration::Save()
{
    PersistentConfigData data;

    data.dmxStartChannel = this->dmxStartChannel;
    data.brightness = this->brightness;
    data.mode = this->mode;

    return Save(data);
}

bool SystemConfiguration::Save(const PersistentConfigData &data)
{
    File32 writeFile = fatfs.open(FILE_CONFIG_SYSCONFIG, O_RDWR | O_CREAT | O_TRUNC | O_SYNC);
    if (!writeFile)
    {
        Serial.println(F("Error, failed to open " FILE_CONFIG_SYSCONFIG " for writing!"));
        return false;
    }
    Serial.println(F("Opened file " FILE_CONFIG_SYSCONFIG " for writing/appending..."));

    writeFile.write(&data, sizeof(data));
    writeFile.flush();
    writeFile.close();

    Serial.println(F("Wrote to file " FILE_CONFIG_SYSCONFIG "!"));
    return true;
}
