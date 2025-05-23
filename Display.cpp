#include "Display.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void InitializeDisplay()
{
    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }

    display.display();
    // delay(1000); // skip splash screen
}

void StartupMessage(const char *msg)
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.setTextColor(SSD1306_WHITE);

    display.println(F("STARTING UP..."));
    display.println();
    display.println(msg);
    display.display();

    Serial.println(msg);

    delay(50);
}

void SystemPanic(const char *msg)
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.setTextColor(SSD1306_WHITE);

    display.println(F("< SYSTEM PANIC >"));
    display.println();
    display.println(msg);
    display.display();

    Serial.println(F("< SYSTEM PANIC >"));
    Serial.println(msg);
    
    // Halt
    while (true) { delay(1000); }
}
