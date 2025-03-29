#pragma once

#include <Arduino.h>
#include "Constants.h"

typedef struct
{
  uint8_t Value;                  // The actual value of the channel (0-255)
  unsigned long LastUpdatedMsec; // Timestamp of when this value was last updated in milliseconds since boot
} DmxChannelData;

extern DmxChannelData dmxData[DMX_UNIVERSE_SIZE + 1];
extern unsigned long lastDmxPacketReceivedMsec;
extern unsigned long lastDmxUniverseUpdateCompletedMsec;
extern uint16_t dmxChannelsPendingSinceLastCompleteUpdate;
