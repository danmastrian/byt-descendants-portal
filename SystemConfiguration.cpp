#include "SystemConfiguration.h"
#include "flash_config.h"

Adafruit_FlashTransport_QSPI flashTransport;
Adafruit_SPIFlash flash(&flashTransport);
FatVolume fatfs;
SystemConfiguration sysConfig;
