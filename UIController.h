#include <Adafruit_Keypad.h>
#include <Adafruit_GFX.h>

double fps = 0.0;

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

    UIState* parent = nullptr;

    UIState(const char* name)
        : name(name),
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

    virtual void Tick()
    {
    }
    
    void SetParent(UIState* parent)
    {
        this->parent = parent;
    }
};

class UIStateDummy : public UIState
{
public:

    UIStateDummy(const char* name)
        : UIState(name)
    {
    }

    virtual void Render()
    {
        display.println(F(GetName()));
        display.println(F("Press BACK to return"));
    }

    UIState *HandleButtonPress(UIButton button)
    {
        if (button == Back)
        {
            return parent;
        }

        return this;
    }
};

class SystemConfiguration
{
public:

    int dmxStartChannel = 120;
    const int DmxChannelCount = 5;
};

const int DMX_UNIVERSE_SIZE = 512;
SystemConfiguration sysConfig;

class UIStateConfigDmxChannel : public UIState
{
private:

    int newStartChannel;
    
public:

    UIStateConfigDmxChannel()
        : UIState("DMX CHANNEL")
    {
        newStartChannel = sysConfig.dmxStartChannel;
    }

    virtual void Render()
    {
        display.println(F(GetName()));
        display.print(F("Current: "));
        display.print(sysConfig.dmxStartChannel);
        display.print(F("-"));
        display.println(sysConfig.dmxStartChannel + sysConfig.DmxChannelCount - 1);
        display.println();
        display.print(F("New Start: "));
        display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
        display.print(F(" "));
        display.print(newStartChannel);
        display.println(F(" "));
    }

    UIState *HandleButtonPress(UIButton button)
    {
        switch (button)
        {
            case Back:
                return parent;

            case Right:
                if (newStartChannel < DMX_UNIVERSE_SIZE - sysConfig.DmxChannelCount)
                {
                    newStartChannel++;
                    SetDirty();
                }
                break;

            case Left:
                if (newStartChannel > 0)
                {
                    newStartChannel--;
                    SetDirty();
                }
                break;

            case OK:
                sysConfig.dmxStartChannel = newStartChannel;
                return parent;
        }

        return this;
    }
};

class UIStateMenu : public UIState
{
private:

    UIState** menuItems;
    int menuItemCount;
    int currentIndex;

public:

    UIStateMenu(const char* name, UIState** menuItems, int menuItemCount)
        : UIState(name),
          menuItems(menuItems),
          menuItemCount(menuItemCount),
          currentIndex(0)
    {
    }

    virtual void Render()
    {
        display.println(F(GetName()));
        display.println(F("Press BACK to exit"));
        display.println();
        display.print(currentIndex > 0 ? F(" < ") : F("   "));
        display.print(F(menuItems[currentIndex]->GetName()));
        display.println(currentIndex < (menuItemCount - 1) ? F(" > ") : F("   "));
    }

    UIState *HandleButtonPress(UIButton button)
    {
        switch (button)
        {
            case Back:
                return parent;

            case Right:
                if (currentIndex < (menuItemCount - 1))
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
                UIState* newState = menuItems[currentIndex];
                newState->SetParent(this);
                return newState;
        }

        return this;
    }
};

class UIStateMain : public UIState
{
private:

    UIStateMenu* mainMenu;
    unsigned long lastUpdateMsec = 0;

    const char* statusGlyphs = "|/-\\" ;
    int statusGlyphIndex = 0;

public:

    UIStateMain()
        : UIState("MAIN")
    {
        mainMenu = new UIStateMenu(
            "MAIN MENU",
            new UIState*[3]
            {
                new UIStateConfigDmxChannel(),
                new UIStateDummy("ITEM 2"),
                new UIStateDummy("Item 3"),
            },
            3);
    }

    virtual void Tick()
    {
        if ((millis() - lastUpdateMsec) > 100UL)
        {
            statusGlyphIndex = (statusGlyphIndex + 1) % 4;
            SetDirty();
            lastUpdateMsec = millis();
        }
    }

    virtual void Render()
    {
        display.print(F("SYSTEM READY "));
        display.print(statusGlyphs[statusGlyphIndex]);
        display.println();

        display.print(F("M 0, B 255, DMX "));
        display.print(sysConfig.dmxStartChannel);
        display.println();

        display.print(F("Idle - "));
        display.print((int)fps);
        display.println(F(" fps"));
        
        display.println(F("Press OK for menu"));
    }

    UIState *HandleButtonPress(UIButton button)
    {
        if (button == OK)
        {
            mainMenu->SetParent(this);
            return mainMenu;
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

    UIButton TranslateKey(keypadEvent& e)
    {
        switch ((char)e.bit.KEY)
        {
            case '1': return UIButton::Back;
            case '2': return UIButton::Left;
            case '3': return UIButton::Right;
            case '4': return UIButton::OK;
            default: return UIButton::Back; // should never happen
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
        state->Tick();
        
        keypad.tick();

        while (keypad.available())
        {
            keypadEvent e = keypad.read();

            if (e.bit.EVENT == KEY_JUST_PRESSED)
            {
                UIState* newState = state->HandleButtonPress(TranslateKey(e));
                if (newState != state)
                {
                    newState->SetDirty();
                    state = newState;
                }
            }
        }

        state->UpdateDisplay();
    }
};
