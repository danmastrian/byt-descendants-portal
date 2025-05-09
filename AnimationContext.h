#pragma once

#include "Constants.h"
#include "SystemConfiguration.h"
#include "LedStrip.h"

class AnimationContext
{
private:

    unsigned long startMsec = 0;
    unsigned long stopRequestedAtElapsedMsec = 0;
    bool stopRequested = false;
    bool isRunning = false;
    int animationId = 0;
    bool isFlashEnabled = false;

private:

    void SetPixelColor(int pixelIndex, uint8_t r, uint8_t g, uint8_t b, uint8_t w)
    {
        strip.setPixelColor(pixelIndex, r, g, b, w);
        strip.setPixelColor(pixelIndex + LED_COUNT_PER_RING, r, g, b, w);
    }

    void SetPixelColor(int pixelIndex, uint32_t rgbw)
    {
        strip.setPixelColor(pixelIndex, rgbw);
        strip.setPixelColor(pixelIndex + LED_COUNT_PER_RING, rgbw);
    }

public:
    void Start(int animationId, bool isFlashEnabled)
    {
        if (!isRunning)
        {
            startMsec = millis();
            stopRequestedAtElapsedMsec = 0;
            stopRequested = false;
            isRunning = true;
            this->isFlashEnabled = isFlashEnabled;
        }

        // Allow color change while running
        this->animationId = animationId;
    }

    void Stop()
    {
        if (isRunning && !stopRequested)
        {
            stopRequestedAtElapsedMsec = millis() - startMsec;
            stopRequested = true;
        }
    }

    void StopImmediate()
    {
        isRunning = false;
    }

    bool IsRunning() const
    {
        return isRunning;
    }

    bool IsStopRequested() const
    {
        return stopRequested;
    }

    void Render()
    {
        if (!isRunning)
            return;

        unsigned long elapsedMsecMaster = millis() - startMsec;

        long curvePeriodMsec = 1000;
        if (animationId == 2)
        {
            // Smaller bands of color
            curvePeriodMsec = 300;
        }
        
        const unsigned long flashWhiteDurationMsec = 1000;

        double flashWhitePercent = 0.0;
        if (isFlashEnabled)
        {
            if (elapsedMsecMaster < flashWhiteDurationMsec)
            {
                // flashWhitePercent = 1. - (elapsedMsecMaster / flashWhiteDurationMsec.);

                // Exponential decay function
                flashWhitePercent = pow(2, -6 * ((double)elapsedMsecMaster / (double)flashWhiteDurationMsec));
            }
            
            // Delay the start of the main animation by 750 msec to overlap sligtly with the flash white
            elapsedMsecMaster -= 750;
        }

        bool allStopped = stopRequested;

        for (int i = 0; i < LED_COUNT_PER_RING; ++i)
        {
            long elapsedMsecLocal = elapsedMsecMaster - (i * 1000 / 288); // propagate at N pixels/sec

            double percentDone = (double)(elapsedMsecLocal % curvePeriodMsec) / (double)curvePeriodMsec;
            // double brightnessPercent = sin(percentDone * PI);
            double brightnessPercent = cos(PI * (percentDone - 0.5));
            brightnessPercent *= brightnessPercent;

            if (elapsedMsecLocal < 0 || (stopRequested && elapsedMsecLocal > stopRequestedAtElapsedMsec))
            {
                brightnessPercent = 0.0;
            }
            else
            {
                allStopped = false;
            }

            // Auradon
            if (animationId == 0)
            {
                if ((elapsedMsecLocal / curvePeriodMsec) % 2 == 0)
                {
                    SetPixelColor(
                        i,
                        brightnessPercent * 0.5 * 255,
                        brightnessPercent * 0.3 * 255,
                        0,
                        max(flashWhitePercent, brightnessPercent) * 255);
                }
                else
                {
                    SetPixelColor(
                        i,
                        0,
                        0,
                        brightnessPercent * 255,
                        max(0, flashWhitePercent * 255));
                }
            }
            else if (animationId == 1) // Isle
            {
                if ((elapsedMsecLocal / curvePeriodMsec) % 2 == 0)
                {
                    SetPixelColor(
                        i,
                        brightnessPercent * 0.5 * 255,
                        0,
                        brightnessPercent * 0.8 * 255,
                        max(0, flashWhitePercent * 255));
                }
                else
                {
                    SetPixelColor(
                        i,
                        0,
                        brightnessPercent * 255,
                        0,
                        max(0, flashWhitePercent * 255));
                }
            }
            else if (animationId == 2) // Finale
            {
                const uint32_t wMask = 0xFF000000ul;
                const long distinctColors = 10;
                
                long hue = ((elapsedMsecLocal / curvePeriodMsec) % distinctColors) * 65536 / distinctColors;

                uint32_t color = strip.ColorHSV(hue, 255, brightnessPercent * 255);
                color = (color & ~wMask) | max(color & wMask, (uint32_t)(flashWhitePercent * 255) << 24);
                color = strip.gamma32(color);

                SetPixelColor(i, color);
            }
        }

        if (allStopped)
        {
            isRunning = false;
        }
    }
};

extern AnimationContext animationContext;
