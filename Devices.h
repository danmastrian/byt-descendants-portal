#define SSD1306_NO_SPLASH 1 // doesn't work, needs to be global compiler option

#include <Adafruit_Keypad.h>
#include <Adafruit_Keypad_Ringbuffer.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN A0
#define LED_COUNT (144 * 12)

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

#define KEYPAD_PID1332
#define R1 5
#define R2 99
#define R3 99
#define R4 99
#define C3 9
#define C4 10
#define C1 11
#define C2 12
#include "keypad_config.h"

extern Adafruit_SSD1306 display;
extern Adafruit_NeoPixel strip;
extern Adafruit_Keypad customKeypad;
