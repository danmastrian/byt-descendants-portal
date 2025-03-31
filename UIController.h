#pragma once

#include "Display.h"
#include "Keypad.h"

extern double fps;

enum UIButton
{
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
    UIState *state;

    UIButton TranslateKey(char key);

public:
    UIController(
        Adafruit_SSD1306 &display,
        UIState *initialState);

    void Process();
};

extern UIController uiController;
