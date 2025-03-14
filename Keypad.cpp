#include "Keypad.h"

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

Adafruit_Keypad customKeypad = Adafruit_Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
