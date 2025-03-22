#include "Keypad.h"
#include <Arduino.h>

#define ROWS 1
#define COLS 4

char keymap[ROWS][COLS] = {
  {'1','2','3','4'}
};

Adafruit_TCA8418 customKeypad;

bool InitKeypad()
{
  if (!customKeypad.begin(TCA8418_DEFAULT_ADDR, &Wire))
  {
    return false;
  }

  customKeypad.matrix(ROWS, COLS);
  customKeypad.flush();

  return true;
}

char GetKeyPressEvent()
{
  if (customKeypad.available() == 0)
  {
    return 0;
  }

  int k = customKeypad.getEvent();
  
  bool pressed = k & 0x80;
  if (!pressed)
  {
    // Only emit an event when the key is first pressed
    return 0;
  }

  k &= 0x7F;
  k--;

  uint8_t row = k / 10;
  uint8_t col = k % 10;

  Serial.printf("Pressed row %u col %u\n", row, col);

  return keymap[row][col];
}