#include <Adafruit_Keypad.h>
#include <Adafruit_GFX.h>

enum UIButton
{
    Back,
    Down,
    Up,
    OK,
};

class UIState
{
protected:

    UIState();

public:

    virtual void UpdateDisplay() = 0;
    virtual UIState *HandleButtonPress(UIButton button) = 0;
};

class UIState_Root : public UIState
{
public:

    void UpdateDisplay()
    {
        display.clearDisplay();
        display.setTextSize(1);              // Normal 1:1 pixel scale
        display.setTextColor(SSD1306_WHITE); // Draw white text
        display.setCursor(0, 0);             // Start at top-left corner
        display.println(F("SYSTEM READY"));
        display.display();
    }

    UIState *HandleButtonPress(UIButton button)
    {
    }
};

class UIState_Menu : public UIState
{
public:
    void UpdateDisplay()
    {
    }

    UIState *HandleButtonPress(UIButton button)
    {
    }
};

class UIController
{
protected:

    Adafruit_Keypad& keypad;
    Adafruit_SSD1306& display;

private:

    UIState *state;

public:

    UIController(
        Adafruit_Keypad& keypad,
        Adafruit_SSD1306& display,
        UIState *initialState)
        : keypad(keypad),
          display(display)
    {
        this->state = initialState;
    }

    void Process()
    {
        keypad.tick();

        while (keypad.available())
        {
            keypadEvent e = keypad.read();

            if (e.bit.EVENT == KEY_JUST_PRESSED)
            {
                display.clearDisplay();
                display.setTextSize(1);              // Normal 1:1 pixel scale
                display.setTextColor(SSD1306_WHITE); // Draw white text
                display.setCursor(0, 0);             // Start at top-left corner

                display.print((char)e.bit.KEY);
                display.println(F(" pressed"));
                display.display();
            }
            else if (e.bit.EVENT == KEY_JUST_RELEASED)
            {
                display.clearDisplay();
                display.setTextSize(1);              // Normal 1:1 pixel scale
                display.setTextColor(SSD1306_WHITE); // Draw white text
                display.setCursor(0, 0);             // Start at top-left corner
                display.print((char)e.bit.KEY);
                display.println(F(" released"));
                display.display();
            }
        }
    }
};
