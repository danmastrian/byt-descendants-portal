#include "DmxData.h"
#include "SystemConfiguration.h"

DmxChannelData dmxData[DMX_UNIVERSE_SIZE + 1] = { 0 };
unsigned long lastDmxPacketReceivedMsec = 0;
unsigned long lastDmxUniverseUpdateCompletedMsec = 0;
uint16_t dmxChannelsPendingSinceLastCompleteUpdate = DMX_UNIVERSE_SIZE;

uint8_t GetLogicalDmxChannelValue(uint16_t channel)
{
    if (channel >= SystemConfiguration::DmxChannelCount)
        return 0;

    return dmxData[sysConfig.dmxStartChannel + channel].Value;
}