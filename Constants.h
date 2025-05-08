#pragma once

#include <Arduino.h>

const int BRIGHTNESS_STEP = 5;
const int BRIGHTNESS_MIN = 5;
const int BRIGHTNESS_MAX = 255;
const int DMX_UNIVERSE_SIZE = 512;

const int LED_COUNT_PER_CHANNEL = (144 * 2); // 144 pixels/m * N meters
const int LED_COUNT_PER_RING = LED_COUNT_PER_CHANNEL * 3; // 3 channels per ring
const int LED_COUNT_TOTAL = LED_COUNT_PER_RING * 2; // 2 adjacent rings