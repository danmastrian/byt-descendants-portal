#pragma once

#include <Adafruit_Keypad.h>
#include <Adafruit_Keypad_Ringbuffer.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_TCA8418.h>

extern Adafruit_TCA8418 customKeypad;

bool InitKeypad();
char GetKeyPressEvent();
