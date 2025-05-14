#include "UIController.h"
#include "SystemConfiguration.h"
#include "Constants.h"
#include "DmxData.h"
#include "AnimationContext.h"
#include "RenderController.h"

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

        SetTextColor();
        display.println();
        display.print(F("Press "));
        SetTextColor(true); // Inverted
        display.print(F(" OK "));
        SetTextColor();
        display.println(F(" to save"));
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
        display.print(F(" - "));
        display.println(sysConfig.dmxStartChannel + sysConfig.DmxChannelCount - 1);
        display.println();

        display.print(F("New Start: "));
        SetTextColor(true); // Inverted
        display.print(F(" < ")); // Wraparound supported
        display.print(newStartChannel);
        display.println(F(" > "));

        SetTextColor();
        display.println();
        display.print(F("Press "));
        SetTextColor(true); // Inverted
        display.print(F(" OK "));
        SetTextColor();
        display.println(F(" to save"));
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

class UIStateShowMode : public UIState
{
private:
    int consecutiveBackPresses = 0;

public:
    UIStateShowMode()
        : UIState("RUN SHOW")
    {
    }

    virtual void Activate()
    {
        consecutiveBackPresses = 0;
    }

    virtual void Render()
    {
        display.setTextSize(2);
        display.println(F(GetName()));

        display.setTextSize(1);
        display.println(F(""));
    }

    UIState *HandleButtonPress(UIButton button)
    {
        if (button == UIButton::Back && !animationContext.IsRunning())
        {
            ++consecutiveBackPresses;

            if (consecutiveBackPresses == 5)
            {
                return parent;
            }

            return this;
        }

        consecutiveBackPresses = 0;

        switch (button)
        {
        case Back:
            //if (animationContext.IsStopRequested())
            //{
            //    animationContext.StopImmediate();
            //}
            //else
            //{
                animationContext.Stop();
            //}
            break;

        case Right: // Isle of the Lost
            animationContext.Start(ANIMATION_ID_ISLE);
            break;

        case Left: // Auradon
            animationContext.Start(ANIMATION_ID_AURADON);
            break;

        case OK: // Finale (rainbow)
            animationContext.Start(ANIMATION_ID_FINALE);
            break;
        }

        return this;
    }
};

class UIStateManualTest : public UIState
{
private:
    const int ANIMATION_COUNT = 5; // TODO
    int selectedAnimationIndex = 0;

public:
    UIStateManualTest()
        : UIState("MANUAL TEST")
    {
    }

    virtual void Activate()
    {
    }

    virtual void Render()
    {
        display.println(F(GetName()));
        display.println();

        display.print(F("Animation: "));
        SetTextColor(true); // Inverted
        display.print(F(" < ")); // Wraparound supported
        display.print(selectedAnimationIndex);
        display.println(F(" > "));

        SetTextColor();
        display.println();
        display.print(F("Press "));
        SetTextColor(true); // Inverted
        display.print(F(" OK "));
        SetTextColor();
        display.println(F(" to run"));
    }

    UIState *HandleButtonPress(UIButton button)
    {
        switch (button)
        {
        case Back:
            return parent;

        case Right:
            // start animation 0
            SetDirty();
            break;

        case Left:
            // start animation 1
            SetDirty();
            break;

        case OK:
            // stop animation
            break;
        }

        return this;
    }
};

class UIStateEnableLock : public UIState
{
private:

    UIState* homeState;

public:
    UIStateEnableLock(UIState* homeState)
        : UIState("LOCK INTERFACE"),
            homeState(homeState)
    {
    }

    virtual void Render()
    {
        display.println(F(GetName()));
        display.println();
        display.println();

        display.print(F("Press "));
        SetTextColor(true); // Inverted
        display.print(F(" OK "));
        SetTextColor();
        display.println(F(" to lock"));

        display.println();

        display.print(F("Press "));
        SetTextColor(true); // Inverted
        display.print(F(" BACK "));
        SetTextColor();
        display.println(F(" to exit"));
    }

    UIState *HandleButtonPress(UIButton button)
    {
        switch (button)
        {
        case Back:
            return parent;

        case OK:
            sysConfig.isLocked = true;
            sysConfig.Save();
            return homeState;
        }

        return this;
    }
};

class UIStateDisableLock : public UIState
{
private:

    static const int CodeLength = 3;
    const char* UnlockCode = "425"; // TODO: Make this configurable
    static const char UnspecifiedCodeChar = '_';

    UIState* homeState;
    char code[CodeLength + 1] = { 0 };
    int selectedDigit = 0;
    int codeIndex = 0;

    void ResetCode()
    {
        codeIndex = 0;
        memset(code, UnspecifiedCodeChar, CodeLength);
    }

public:

    UIStateDisableLock(UIState* homeState)
        : UIState("UNLOCK INTERFACE"),
            homeState(homeState)
    {
    }

    virtual void Activate()
    {
        selectedDigit = 0;
        ResetCode();
    }

    virtual void Render()
    {
        display.println(F(GetName()));
        display.println(F("Enter code:"));
        display.println();
        
        display.printf("     [%c] [%c] [%c]\n", code[0], code[1], code[2]);
        display.println();

        // Digit selection
        display.println();
        display.print(F("   "));
        for (int i = 0; i < 10; i++)
        {
            if (i == 5)
            {
                display.println();
                display.print(F("   "));
            }

            SetTextColor(i == selectedDigit);
            display.printf(" %c ", i + '0');
            SetTextColor();
        }
        display.println();
    }

    UIState *HandleButtonPress(UIButton button)
    {
        switch (button)
        {
            case Back:
            
                return parent;

            case OK:

                code[codeIndex] = selectedDigit + '0';
                codeIndex++;

                if (codeIndex == CodeLength)
                {
                    if (!strcmp(code, UnlockCode))
                    {
                        // Correct code entered, unlock the interface
                        sysConfig.isLocked = false;
                        sysConfig.Save();
                        return homeState;
                    }
                    else
                    {
                        // Incorrect code entered, reset the code
                        ResetCode();
                    }
                }

                SetDirty();
                break;

            case Left:
                if (selectedDigit > 0)
                {
                    selectedDigit--;
                }
                else
                {
                    selectedDigit = 9;
                }
                SetDirty();
                break;

            case Right:
                if (selectedDigit < 9)
                {
                    selectedDigit++;
                }
                else
                {
                    selectedDigit = 0;
                }
                SetDirty();
                break;
        }

        return this;
    }
};

class UIStateConfigMode : public UIState
{
private:

    const int MIN_MODE = 0;
    const int MAX_MODE = RenderProcessorCount - 1;

    int newMode;

public:
    UIStateConfigMode()
        : UIState("RENDER MODE")
    {
    }

    virtual void Activate()
    {
        newMode = sysConfig.mode;
    }

    virtual void Render()
    {
        display.println(F(GetName()));
        display.println();
        display.println(F("Current Mode: "));
        display.printf("%d %s\n", sysConfig.mode, renderProcessors[sysConfig.mode]->GetName());
        display.println();

        display.println(F("New Mode: "));
        SetTextColor(true); // Inverted
        display.print(newMode > MIN_MODE ? F(" < ") : F("   "));
        display.printf("%d %s", newMode, renderProcessors[newMode]->GetName());
        display.println(newMode < MAX_MODE ? F(" > ") : F("   "));

        SetTextColor();
        display.println();
        display.print(F("Press "));
        SetTextColor(true); // Inverted
        display.print(F(" OK "));
        SetTextColor();
        display.println(F(" to save"));
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

    // How often to refresh the display in msec
    const unsigned long UpdatePeriodMsec = 50UL;

    // Number of lines to display in the dump (up to 2 channels per line)
    const uint16_t DumpLineCount = 4;

    unsigned long lastUpdateMsec = 0;
    unsigned long lastDmxPacketMsec = 0;
    bool showDmxStatusGlyph = false;
    int page = 0;

    void DumpChannelValues(uint16_t startCh, uint16_t chCount);

public:

    UIStateDmxDump();
    virtual void Tick();
    virtual void Render();
    virtual void Activate();
    virtual UIState* HandleButtonPress(UIButton button);
};

UIStateDmxDump::UIStateDmxDump()
    : UIState("DMX DUMP")
{
}

void UIStateDmxDump::Tick()
{
    if (millis() - lastUpdateMsec >= UpdatePeriodMsec)
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

    display.printf(
        " %c %s\n",
        showDmxStatusGlyph ? '*' : ' ',
        page == 0 ? "(THIS DEV)" : "");

    display.print(F("Press "));
    SetTextColor(true); // Inverted
    display.print(F(" BACK "));
    SetTextColor();
    display.println(F(" to exit"));
    display.println();

    if (page == 0)
    {
        // Dump only the channels that this device is configured to use
        DumpChannelValues(sysConfig.dmxStartChannel, sysConfig.DmxChannelCount);
    }
    else
    {
        // Dump a page of channels anywhere in the DMX universe
        DumpChannelValues(((page - 1) * DumpLineCount * 2) + 1, DumpLineCount * 2);
    }
}

void UIStateDmxDump::DumpChannelValues(uint16_t startCh, uint16_t chCount)
{
    for (uint16_t ch = startCh; ch < startCh + min(chCount, DumpLineCount); ++ch)
    {
      if (ch + DumpLineCount < startCh + chCount)
      {
        display.printf("%3d = %3d | %3d = %3d\n", ch, dmxData[ch].Value, ch + DumpLineCount, dmxData[ch + DumpLineCount].Value);
      }
      else
      {
        display.printf("%3d = %3d |\n", ch, dmxData[ch].Value);
      }
    }
}

void UIStateDmxDump::Activate()
{
    lastUpdateMsec = 0;
    lastDmxPacketMsec = 0;
    showDmxStatusGlyph = false;
    page = 0;
}

UIState *UIStateDmxDump::HandleButtonPress(UIButton button)
{
    switch (button)
    {
        case Back:
            return parent;

        case Right:
            if (page < (DMX_UNIVERSE_SIZE / (DumpLineCount * 2)))
            {
                page++;
            }
            else
            {
                page = 0;
            }
            SetDirty();
            break;

        case Left:
            if (page > 0)
            {
                page--;
            }
            else
            {
                page = (DMX_UNIVERSE_SIZE / (DumpLineCount * 2));
            }
            SetDirty();
            break;
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
        display.print(F("Press "));
        SetTextColor(true); // Inverted
        display.print(F(" BACK "));
        SetTextColor();
        display.println(F(" to exit"));
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

    // Consider the DMX latency to be 'unknown' if it exceeds this threshold
    const double DmxUniverseUpdateLatencyMsecThreshold = 1000;

    // How often to refresh the display in msec
    const unsigned long DisplayRefreshPeriodMsec = 50UL;
    const int MenuItemCount = 6; // Yuck

    const char* StatusGlyphs = "|/-\\";
    const size_t StatusGlyphsLength = 4; // Yuck

    // Weighting factor for latency update (2% current, 98% previous)
    const double LatencyUpdateWeight = 0.02;

    UIStateMenu *mainMenu;
    UIState *unlockScreen;
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
            new UIStateShowMode(),
            new UIStateConfigMode(),
            new UIStateDmxDump(),
            new UIStateConfigDmxChannel(),
            new UIStateConfigBrightness(),
            new UIStateEnableLock(this),
        },
        MenuItemCount);

    unlockScreen = new UIStateDisableLock(this);
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
    SetTextColor(true); // Inverted
    display.printf(" READY ");

    SetTextColor(false);
    display.printf(
      " %c Mode %u %s",
      StatusGlyphs[statusGlyphIndex],
      sysConfig.mode,
      sysConfig.isLocked ? "LOCK" : "");
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

    dmxUniverseUpdateLatencyMsec = 
        ((1.0 - LatencyUpdateWeight) * dmxUniverseUpdateLatencyMsec) +
        (LatencyUpdateWeight * (double)(millis() - lastDmxUniverseUpdateCompletedMsec));

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
        display.println(F("NO SIGNAL"));
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
    
    display.printf(" Status   ");
    renderer.WriteStatusString(display);
    display.println();

    display.print(F("Press "));
    SetTextColor(true); // Inverted
    display.print(F(" OK "));
    SetTextColor();
    if (sysConfig.isLocked)
    {
        display.println(F(" to unlock"));
    }
    else
    {
        display.println(F(" for menu"));
    }
}

UIState* UIStateMain::HandleButtonPress(UIButton button)
{
    if (button == OK)
    {
        if (sysConfig.isLocked)
        {
            unlockScreen->SetParent(this);
            return unlockScreen;
        }
        else
        {
            mainMenu->SetParent(this);
            return mainMenu;
        }
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
    SetTextColor();
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
        return UIButton::None; // should never happen
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
    const unsigned long now = millis();

    UIState *newState = nullptr;

    KeypadEvent event = GetKeyPressEvent();
    if (event.KeyCode != 0)
    {
        // An event has occurred
        if (event.IsPressed)
        {
            keyPressedAtMsec = now;
            currentKeyPressed = event.KeyCode;
            isKeyHeld = false;

            newState = state->HandleButtonPress(TranslateKey(event.KeyCode));
        }
        else
        {
            keyPressedAtMsec = 0;
            currentKeyPressed = 0;
            isKeyHeld = false;
        }
    }
    else if (currentKeyPressed != 0)
    {
        // No new event, but a key is still pressed
        UIButton button = TranslateKey(currentKeyPressed);

        // Only support repeat for left/right/back buttons
        if (button != UIButton::OK)
        {
            unsigned long msecSinceLastEvent = now - keyPressedAtMsec;
            unsigned long msecThreshold = isKeyHeld ? KEY_REPEAT_PERIOD_MSEC : KEY_HOLD_THRESHOLD_MSEC;
            
            if (msecSinceLastEvent >= msecThreshold)
            {
                newState = state->HandleButtonPress(button);
                keyPressedAtMsec = now; // Update the key pressed time to now
                isKeyHeld = true; // Repeat events will be sent more frequently
            }
        }
    }

    if (newState && (newState != state))
    {
        newState->Activate();
        newState->SetDirty();
        state = newState;
    }

    state->Tick();
    state->UpdateDisplay();
}

UIStateMain uiStateMain;
UIController uiController(display, &uiStateMain);
