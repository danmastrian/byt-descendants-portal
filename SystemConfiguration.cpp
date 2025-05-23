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

// #define STRICT_CONFIG_VALIDATION

SystemConfiguration sysConfig;

bool SystemConfiguration::InitializeStorage()
{
    StartupMessage("SD flash init...");

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
        // Use SdFat_format example sketch from Adafruit_SPIFlash library
        // to initialize flash on the board with a FAT filesystem.
        Serial.println(F("Error, failed to mount FAT filesystem!"));
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
        StartupMessage("CONFIG RESET!");
        delay(10000);

        // Default settings
        PersistentConfigData data{0, 50, 0};
        if (!Save(data))
        {
            Serial.println(F("Error, failed to create " FILE_CONFIG_SYSCONFIG " file!"));
            return false;
        }
    }

    StartupMessage("Reading config");

    // Now open the same file but for reading.
    File32 readFile = fatfs.open(FILE_CONFIG_SYSCONFIG, FILE_READ);
    if (!readFile)
    {
        Serial.println(F("Error, failed to open " FILE_CONFIG_SYSCONFIG " for reading!"));
        return false;
    }

    // Read data using the same read, find, readString, etc. functions as when
    // using the serial class.  See SD library File class for more documentation:
    //   https://www.arduino.cc/en/reference/SD

    PersistentConfigData loadedConfig = {0, 0, 0, false};
    size_t bytesRead = readFile.read(&loadedConfig, sizeof(loadedConfig));

    #ifdef STRICT_CONFIG_VALIDATION
    if (bytesRead != sizeof(loadedConfig))
    {
      readFile.close();
      return false;
    }
#endif

    readFile.close();

    this->dmxStartChannel = loadedConfig.dmxStartChannel;
    this->brightness = loadedConfig.brightness;
    this->mode = loadedConfig.mode;
    this->isLocked = loadedConfig.isLocked;

    StartupMessage("Config OK");
    return true;
}

bool SystemConfiguration::Save()
{
    PersistentConfigData data;

    data.dmxStartChannel = this->dmxStartChannel;
    data.brightness = this->brightness;
    data.mode = this->mode;
    data.isLocked = this->isLocked;

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
