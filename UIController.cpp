#include "UIController.h"
#include "SystemConfiguration.h"
#include "Constants.h"
#include "DmxData.h"

class UIStateDummy : public UIState
{
public:
    UIStateDummy(const char *name)
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

class UIStateConfigBrightness : public UIState
{
private:
    int newValue;

public:
    UIStateConfigBrightness()
        : UIState("LED BRIGHTNESS")
    {
    }

    virtual void Activate()
    {
        newValue = sysConfig.brightness;
    }

    virtual void Render()
    {
        display.println(F(GetName()));
        display.print(F("Current Value: "));
        display.println(sysConfig.brightness);
        display.println();
        
        display.print(F("New Value: "));
        SetTextColor(true); // Inverted
        display.print(F(" < ")); // Wraparound supported
        display.print(newValue);
        display.println(F(" > "));
    }

    UIState *HandleButtonPress(UIButton button)
    {
        switch (button)
        {
        case Back:
            return parent;

        case Right:
            if (newValue < BRIGHTNESS_MAX)
            {
                newValue += BRIGHTNESS_STEP;
            }
            else
            {
                // Wrap around to minimum
                newValue = BRIGHTNESS_MIN;
            }
            SetDirty();
            break;

        case Left:
            if (newValue > BRIGHTNESS_MIN)
            {
                newValue -= BRIGHTNESS_STEP;
            }
            else
            {
                // Wrap around to maximum
                newValue = BRIGHTNESS_MAX;
            }
            SetDirty();
            break;

        case OK:
            sysConfig.brightness = newValue;
            sysConfig.Save();
            return parent;
        }

        return this;
    }
};

class UIStateConfigDmxChannel : public UIState
{
private:
    int newStartChannel;

public:
    UIStateConfigDmxChannel()
        : UIState("DMX CHANNEL")
    {
    }

    virtual void Activate()
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
        SetTextColor(true); // Inverted
        display.print(F(" < ")); // Wraparound supported
        display.print(newStartChannel);
        display.println(F(" > "));
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
            }
            else
            {
                newStartChannel = 1;
            }
            SetDirty();
            break;

        case Left:
            if (newStartChannel > 1)
            {
                newStartChannel--;
            }
            else
            {
                newStartChannel = DMX_UNIVERSE_SIZE - sysConfig.DmxChannelCount + 1;
            }
            SetDirty();
            break;

        case OK:
            sysConfig.dmxStartChannel = newStartChannel;
            sysConfig.Save();
            return parent;
        }

        return this;
    }
};

class UIStateConfigMode : public UIState
{
private:

    const int MIN_MODE = 0;
    const int MAX_MODE = 10;

    int newMode;

public:
    UIStateConfigMode()
        : UIState("MODE")
    {
    }

    virtual void Activate()
    {
        newMode = sysConfig.mode;
    }

    virtual void Render()
    {
        display.println(F(GetName()));
        display.print(F("Current: "));
        display.println(sysConfig.mode);
        display.println();

        display.print(F("New: "));
        SetTextColor(true); // Inverted
        display.print(newMode > MIN_MODE ? F(" < ") : F("   "));
        display.print(newMode);
        display.println(newMode < MAX_MODE ? F(" > ") : F("   "));
    }

    UIState *HandleButtonPress(UIButton button)
    {
        switch (button)
        {
        case Back:
            return parent;

        case Right:
            if (newMode < MAX_MODE)
            {
                newMode++;
                SetDirty();
            }
            break;

        case Left:
            if (newMode > MIN_MODE)
            {
                newMode--;
                SetDirty();
            }
            break;

        case OK:
            sysConfig.mode = newMode;
            sysConfig.Save();
            return parent;
        }

        return this;
    }
};

class UIStateDmxDump : public UIState
{
private:
    const unsigned long updatePeriodMsec = 50UL;
    unsigned long lastUpdateMsec = 0;
    unsigned long lastDmxPacketMsec = 0;
    bool showDmxStatusGlyph = false;

public:
    UIStateDmxDump();
    virtual void Tick();
    virtual void Render();
    virtual UIState* HandleButtonPress(UIButton button);
};

UIStateDmxDump::UIStateDmxDump()
    : UIState("DMX DUMP")
{
}

void UIStateDmxDump::Tick()
{
    if (millis() - lastUpdateMsec >= updatePeriodMsec)
    {
      SetDirty();
      lastUpdateMsec = millis();
    }
}

void UIStateDmxDump::Render()
{
    display.print(F(GetName()));

    if (lastDmxPacketMsec != lastDmxPacketReceivedMsec)
    {
      lastDmxPacketMsec = lastDmxPacketReceivedMsec;
      showDmxStatusGlyph = !showDmxStatusGlyph;
    }
    if (showDmxStatusGlyph)
    {
      display.print(" *");
    }
    display.println();

    display.println(F("Press BACK to exit"));
    display.println();

    uint16_t startCh = sysConfig.dmxStartChannel;
    uint16_t chCount = sysConfig.DmxChannelCount;
    const uint16_t lineCount = 4;

    for (uint16_t ch = startCh; ch < startCh + min(chCount, lineCount); ++ch)
    {
      if (ch + lineCount < startCh + chCount)
      {
        display.printf("%3d = %3d | %3d = %3d\n", ch, dmxData[ch].Value, ch + lineCount, dmxData[ch + lineCount].Value);
      }
      else
      {
        display.printf("%3d = %3d |\n", ch, dmxData[ch].Value);
      }
    }
}

UIState *UIStateDmxDump::HandleButtonPress(UIButton button)
{
    switch (button)
    {
      case Back:
          return parent;
    }

    return this;
}

class UIStateMenu : public UIState
{
private:
    UIState **menuItems;
    int menuItemCount;
    int currentIndex;

public:
    UIStateMenu(const char *name, UIState **menuItems, int menuItemCount)
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
        
        SetTextColor(true); // Inverted
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
            UIState *newState = menuItems[currentIndex];
            newState->SetParent(this);
            return newState;
        }

        return this;
    }
};

class UIStateMain : public UIState
{
private:

    const double DmxUniverseUpdateLatencyMsecThreshold = 1000;
    const unsigned long DisplayRefreshPeriodMsec = 50UL;
    const int MenuItemCount = 4; // Yuck
    const char* StatusGlyphs = "|/-\\";
    const size_t StatusGlyphsLength = 4; // Yuck

    UIStateMenu *mainMenu;
    unsigned long lastUpdateMsec = 0;

    //unsigned long lastDmxPacketMsec = 0;
    //bool showDmxStatusGlyph = false;

    unsigned long lastDmxUniverseMsec = 0;
    bool showDmxUniverseStatusGlyph = false;
    // Duration in msec since last complete update of the DMX universe
    double dmxUniverseUpdateLatencyMsec = 40.0; // Default to the expected latency

    int statusGlyphIndex = 0;

public:
    UIStateMain();
    virtual void Tick();
    virtual void Render();
    virtual UIState* HandleButtonPress(UIButton button);
};

UIStateMain::UIStateMain()
    : UIState("MAIN")
{
    mainMenu = new UIStateMenu(
        "MAIN MENU",
        new UIState *[MenuItemCount] {
            new UIStateDmxDump(),
            new UIStateConfigDmxChannel(),
            new UIStateConfigBrightness(),
            new UIStateConfigMode(),
        },
        MenuItemCount);
}

void UIStateMain::Tick()
{
    if ((millis() - lastUpdateMsec) > DisplayRefreshPeriodMsec)
    {
        statusGlyphIndex = (statusGlyphIndex + 1) % StatusGlyphsLength;
        SetDirty();
        lastUpdateMsec = millis();
    }
}

void UIStateMain::Render()
{
    display.printf(
      "READY %c Mode %u",
      StatusGlyphs[statusGlyphIndex],
      sysConfig.mode);
    display.println();
    display.println();

    display.printf(
        " Bright   %3u (%u%%)",
        sysConfig.brightness,
        (unsigned long)(sysConfig.brightness) * 100UL / 255UL // Calculate percentage (0-100%)
        );
    display.println();

    /*
    if (lastDmxPacketMsec != lastDmxPacketReceivedMsec)
    {
      lastDmxPacketMsec = lastDmxPacketReceivedMsec;
      showDmxStatusGlyph = !showDmxStatusGlyph;
    }
    */

    if (lastDmxUniverseMsec != lastDmxUniverseUpdateCompletedMsec)
    {
        lastDmxUniverseMsec = lastDmxUniverseUpdateCompletedMsec;
        showDmxUniverseStatusGlyph = !showDmxUniverseStatusGlyph;
    }

    const double latencyUpdateWeight = 0.01;
    dmxUniverseUpdateLatencyMsec = 
        ((1.0 - latencyUpdateWeight) * dmxUniverseUpdateLatencyMsec) +
        (latencyUpdateWeight * (double)(millis() - lastDmxUniverseUpdateCompletedMsec));

    // Clamp to threshold to avoid spiking on first update
    dmxUniverseUpdateLatencyMsec = min(
        dmxUniverseUpdateLatencyMsec,
        DmxUniverseUpdateLatencyMsecThreshold);

    display.printf(
      " DMX Ch   %3u - %3u %s",
      sysConfig.dmxStartChannel,
      sysConfig.dmxStartChannel + sysConfig.DmxChannelCount - 1,
      showDmxUniverseStatusGlyph ? "*" : " "
      );

    display.println();

    display.print(F(" DMX Lag  "));
    if (dmxUniverseUpdateLatencyMsec >= DmxUniverseUpdateLatencyMsecThreshold)
    {
        display.println(F("[???]"));
    }
    else
    {
        // Round the average latency to the nearest millisecond
        display.printf("%3u ms", (unsigned long)(dmxUniverseUpdateLatencyMsec + 0.5));
        display.println();
    }

    // Round frames per second
    display.printf(" Render   %3u fps", (unsigned int)(fps + 0.5));
    display.println();
    
    display.println();
    display.println(F("Press OK for menu"));
}

UIState* UIStateMain::HandleButtonPress(UIButton button)
{
    if (button == OK)
    {
        mainMenu->SetParent(this);
        return mainMenu;
    }

    return this;
}

void UIState::UpdateDisplay()
{
    if (!isDirty)
        return;

    isDirty = false;

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    SetTextColor();;
    Render();
    display.display();
}

UIButton UIController::TranslateKey(char key)
{
    switch (key)
    {
    case '1':
        return UIButton::Back;
    case '2':
        return UIButton::Left;
    case '3':
        return UIButton::Right;
    case '4':
        return UIButton::OK;
    default:
        return UIButton::Back; // should never happen
    }
}

UIController::UIController(
    Adafruit_SSD1306 &display,
    UIState *initialState)
    : display(display),
      state(initialState)
{
}

void UIController::Process()
{
    char key = GetKeyPressEvent();
    if (key != 0)
    {
      UIState *newState = state->HandleButtonPress(TranslateKey(key));
      if (newState != state)
      {
          newState->Activate();
          newState->SetDirty();
          state = newState;
      }
    }

    state->Tick();
    state->UpdateDisplay();
}

UIStateMain uiStateMain;
UIController uiController(display, &uiStateMain);
