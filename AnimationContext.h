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

    int GetRunningAnimationId()
    {
        if (stopRequested || !isRunning)
            return -1;

        return animationId;
    }

    void Start(int animationId, bool isFlashEnabled)
    {
        Serial.printf(
            "AnimationContext.Start(%d, %d)\n",
            animationId,
            isFlashEnabled
        );

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
        Serial.printf("AnimationContext.Stop()\n");

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
            // Smaller bands of color for rainbow mode
            curvePeriodMsec = 300;
        }

        double flashWhitePercent = 0.0;
        if (isFlashEnabled)
        {
            // Timings to sync with the portal sound effects
            const unsigned long firstFlashDurationMsec = 1000;
            const unsigned long secondFlashStartMsec = 3500;
            const unsigned long secondFlashDurationMsec = 4000;

            if (elapsedMsecMaster < firstFlashDurationMsec)
            {
                // Exponential decay function
                flashWhitePercent = pow(2, -6 * ((double)elapsedMsecMaster / (double)firstFlashDurationMsec));
            }
            else if (
                (animationId == 2) &&
                (elapsedMsecMaster >= secondFlashStartMsec) &&
                (elapsedMsecMaster < (secondFlashStartMsec + secondFlashDurationMsec))
            )
            {
                flashWhitePercent = pow(2, -6 * ((double)(elapsedMsecMaster - secondFlashStartMsec) / (double)secondFlashDurationMsec));
            }
            
            // Delay the start of the main animation by 750 msec to overlap slightly with the flash white
            elapsedMsecMaster -= (firstFlashDurationMsec / 2);
        }

        bool allStopped = stopRequested;

        // Animation "rotates" at N pixels/sec
        const long pixelsPerSec = LED_COUNT_PER_RING / 3;

        for (int i = 0; i < LED_COUNT_PER_RING; ++i)
        {
            long elapsedMsecLocal = elapsedMsecMaster - (i * 1000 / pixelsPerSec);

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
                    // Gold
                    SetPixelColor(
                        i,
                        brightnessPercent * 0.5 * 255,
                        brightnessPercent * 0.3 * 255,
                        0,
                        max(flashWhitePercent, brightnessPercent) * 255);
                }
                else
                {
                    // Blue
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
                    // Purple
                    SetPixelColor(
                        i,
                        brightnessPercent * 0.5 * 255,
                        0,
                        brightnessPercent * 0.8 * 255,
                        max(0, flashWhitePercent * 255));
                }
                else
                {
                    // Green
                    SetPixelColor(
                        i,
                        0,
                        brightnessPercent * 255,
                        0,
                        max(0, flashWhitePercent * 255));
                }
            }
            else if (animationId == 2) // Finale (rainbow)
            {
                const uint32_t rMask = 0x00FF0000ul;
                const uint32_t gMask = 0x0000FF00ul;
                const uint32_t bMask = 0x000000FFul;
                const uint32_t wMask = 0xFF000000ul;
                const long distinctColors = 10;
                
                long hue = ((elapsedMsecLocal / curvePeriodMsec) % distinctColors) * 65536 / distinctColors;

                uint32_t color = strip.ColorHSV(hue, 255, brightnessPercent * 255);
                color = (color & ~rMask) | max(color & rMask, (uint32_t)(flashWhitePercent * 255) << 16);
                color = (color & ~gMask) | max(color & gMask, (uint32_t)(flashWhitePercent * 255) << 8);
                color = (color & ~bMask) | max(color & bMask, (uint32_t)(flashWhitePercent * 255));
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
