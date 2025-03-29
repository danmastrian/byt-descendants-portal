#include "DmxData.h"

DmxChannelData dmxData[DMX_UNIVERSE_SIZE + 1] = { 0 };
unsigned long lastDmxPacketReceivedMsec = 0;
unsigned long lastDmxUniverseUpdateCompletedMsec = 0;
uint16_t dmxChannelsPendingSinceLastCompleteUpdate = DMX_UNIVERSE_SIZE;
