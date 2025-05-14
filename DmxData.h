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

extern uint8_t GetLogicalDmxChannelValue(uint16_t channel);

#define LOGICAL_DMX_CHANNEL_EFFECT_MODE       0
#define LOGICAL_DMX_CHANNEL_MASTER_BRIGHTNESS 1
#define LOGICAL_DMX_CHANNEL_ROTATION_SPEED    2
#define LOGICAL_DMX_CHANNEL_FLASH_BRIGHTNESS  3
#define LOGICAL_DMX_CHANNEL_COLOR_R           4
#define LOGICAL_DMX_CHANNEL_COLOR_G           5
#define LOGICAL_DMX_CHANNEL_COLOR_B           6
#define LOGICAL_DMX_CHANNEL_COLOR_W           7
