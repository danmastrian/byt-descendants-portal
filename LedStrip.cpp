#include "LedStrip.h"
#include "Constants.h"

int8_t pins[] = {
    SCK,
    5,
    6,
    9,
    10,
    11,
    -1, // Unused channel
    -1, // Unused channel
};

Adafruit_NeoPXL8 strip(LED_COUNT_PER_CHANNEL, pins, NEO_GRBW + NEO_KHZ800);
