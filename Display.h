#pragma once

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SSD1306_NO_SPLASH // doesn't work, needs to be global compiler option

extern void StartupMessage(const char* msg);
extern void SystemPanic(const char* msg);
extern void InitializeDisplay();

extern Adafruit_SSD1306 display;
