#include <Adafruit_Keypad.h>
#include <Adafruit_GFX.h>

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

    const char* name;
    bool isDirty;

protected:

    UIState* parent;

    UIState(UIState* parent, const char* name)
        : parent(parent),
          name(name),
          isDirty(true)
    {
    }

public:

    const char* GetName()
    {
        return name;
    }

    void UpdateDisplay()
    {
        if (!isDirty)
            return;

        isDirty = false;

        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.setTextColor(SSD1306_WHITE);
        Render();
        display.display();
    }

    void SetDirty()
    {
        isDirty = true;
    }

    virtual void Render() = 0;
    
    virtual UIState *HandleButtonPress(UIButton button) = 0;
    
    virtual void Activate()
    {
    }
    
    virtual void Deactivate()
    {
    }
};

class UIStateDummy : public UIState
{
public:

    UIStateDummy(UIState* parent, const char* name)
        : UIState(parent, name)
    {
    }

    virtual void Render()
    {
        display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
        display.println(F(GetName()));
        display.setTextColor(SSD1306_WHITE);
        display.println(F("Press BACK to return"));
    }

    UIState *HandleButtonPress(UIButton button)
    {
        if (button == Back)
        {
            return parent;
        }
    }
};

class UIStateMain : public UIState
{
private:

    UIStateMenu* mainMenu;

public:

    UIStateMain()
        : UIState(nullptr, "Main")
    {
        mainMenu = new UIStateMenu(
            this,
            "Main Menu",
            new[]
            {
                UIStateDummy(this, "Item 1"),
                UIStateDummy(this, "Item 2"),
                UIStateDummy(this, "Item 3"),
            },
            3);
    }

    virtual void Render()
    {
        display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
        display.println(F("SYSTEM READY"));
        display.setTextColor(SSD1306_WHITE);
        display.println(F("Press OK for menu"));
    }

    UIState *HandleButtonPress(UIButton button)
    {
        if (button == OK)
        {
            return mainMenu;
        }
    }
};

class UIStateMenu : public UIState
{
private:

    UIState* menuItems;
    int menuItemCount;
    int currentIndex;

public:

    UIStateMenu(UIState* parent, const char* name, UIState* menuItems, int menuItemCount)
        : UIState(parent, name),
          menuItems(menuItems),
          menuItemCount(menuItemCount),
          currentIndex(0)
    {
    }

    virtual void Render()
    {
        display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
        display.println(F(GetName()));

        display.setTextColor(SSD1306_WHITE);
        display.println();
        display.print(currentIndex > 0 ? F("< ") : F("  "));
        display.print(F(menuItems[currentIndex].GetName()));
        display.println(currentIndex < (menuItemCount - 1) ? F(" >") : F("  "));
    }

    UIState *HandleButtonPress(UIButton button)
    {
        switch (button)
        {
            case Back:
                return parent;

            case Right:
                if (currentIndex < menuItemCount - 1)
                {
                    currentIndex++;
                    SetDirty();
                }
                break;

            case Left:
                if (currentIndex > 0)
                {
                    currentIndex--;
                    SetDirty();
                }
                break;

            case OK:
                return menuItems[currentIndex];
        }

        return this;
    }
};

class UIController
{
protected:

    Adafruit_Keypad& keypad;
    Adafruit_SSD1306& display;

private:

    UIState *state;

    UIButton TranslateKey(keypadEvent e)
    {
        switch (e.bit.KEY)
        {
            case '1': return Back;
            case '2': return Left;
            case '3': return Right;
            case '4': return OK;
            default: return Back; // should never happen
        }
    }

public:

    UIController(
        Adafruit_Keypad& keypad,
        Adafruit_SSD1306& display,
        UIState *initialState)
        : keypad(keypad),
          display(display),
          state(initialState)
    {
    }

    void Process()
    {
        keypad.tick();

        while (keypad.available())
        {
            keypadEvent e = keypad.read();

            if (e.bit.EVENT == KEY_JUST_PRESSED)
            {
                UIState* newState = state->HandleButtonPress(TranslateKey(e));
                if (newState != state)
                {
                    state->Deactivate();
                    newState->Activate();
                    newState->SetDirty();
                    state = newState;
                }
            }
        }

        state->UpdateDisplay();
    }
};
