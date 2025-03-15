#include "LedStrip.h"
#include "Constants.h"

int8_t pins[] = {
    SCK,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
};

Adafruit_NeoPXL8 strip(LED_COUNT, pins, NEO_GRBW + NEO_KHZ800);
