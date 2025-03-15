#include "LedStrip.h"
#include "Constants.h"

#define LED_PIN A0

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);