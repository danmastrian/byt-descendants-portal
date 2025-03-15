#pragma once

#include <Adafruit_NeoPXL8.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

extern Adafruit_NeoPXL8 strip;
