#pragma once

#include "Display.h"
#include "Keypad.h"

extern double fps;

enum UIButton
{
    None,
    Back,
    Left,
    Right,
    OK,
};

class UIState
{
private:

    const char *name;
    bool isDirty;

protected:

    UIState *parent = nullptr;

    UIState(const char *name)
        : name(name),
          isDirty(true)
    {
    }

    void SetTextColor(bool inverted = false)
    {
        if (inverted)
        {
            display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
        }
        else
        {
            display.setTextColor(SSD1306_WHITE);
        }
    }

    virtual void Render() = 0;

public:

    const char *GetName()
    {
        return name;
    }

    void UpdateDisplay();

    void SetDirty()
    {
        isDirty = true;
    }

    virtual UIState *HandleButtonPress(UIButton button) = 0;

    virtual void Activate()
    {
    }

    virtual void Tick()
    {
    }

    void SetParent(UIState *parent)
    {
        this->parent = parent;
    }
};

class UIController
{
protected:
    Adafruit_SSD1306 &display;

private:

    // Minimum time to hold a key before it is considered a long press (hold)
    const unsigned long KEY_HOLD_THRESHOLD_MSEC = 500UL;

    // Time between repeat events for a key that is being held down
    const unsigned long KEY_REPEAT_PERIOD_MSEC = 25UL;

    // The current state of the UI
    UIState *state;

    // The time of the last key press event
    unsigned long keyPressedAtMsec = 0;

    // The key that is currently pressed
    char currentKeyPressed = 0;

    // Whether the current key is being held down (for repeat events)
    bool isKeyHeld = false;

    UIButton TranslateKey(char key);

public:
    UIController(
        Adafruit_SSD1306 &display,
        UIState *initialState);

    void Process();
};

extern UIController uiController;
