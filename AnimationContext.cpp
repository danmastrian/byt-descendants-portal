#include "AnimationContext.h"

RGBW colorMapAuradon[] = {

    // Gold
    RGBW(
        0.5 * 255,
        0.3 * 255,
        0,
        255
    ),

    // Blue
    RGBW(
        0,
        0,
        255,
        0
    ),
};

RGBW colorMapIsle[] = {

    // Purple
    RGBW(
        0.5 * 255,
        0,
        0.8 * 255,
        0
    ),

    // Green
    RGBW(
        0,
        255,
        0,
        0
    ),
};

const long distinctColorsAuradon = sizeof(colorMapAuradon) / sizeof(colorMapAuradon[0]);
const long distinctColorsIsle = sizeof(colorMapIsle) / sizeof(colorMapIsle[0]);
const long distinctColorsFinale = 10;

AnimationContext animationContext;

void AnimationContext::SetPixelColor(int pixelIndex, uint8_t r, uint8_t g, uint8_t b, uint8_t w)
{
    strip.setPixelColor(pixelIndex, r, g, b, w);
    strip.setPixelColor(pixelIndex + LED_COUNT_PER_RING, r, g, b, w);
}

void AnimationContext::SetPixelColor(int pixelIndex, uint32_t rgbw)
{
    rgbw = strip.gamma32(rgbw);
    strip.setPixelColor(pixelIndex, rgbw);
    strip.setPixelColor(pixelIndex + LED_COUNT_PER_RING, rgbw);
}

int AnimationContext::GetRunningAnimationId() const
{
    if (stopRequested || !isRunning)
        return -1;

    return animationId;
}

void AnimationContext::Start(int animationId)
{
    Serial.printf(
        "AnimationContext.Start(%d)\n",
        animationId
    );

    if (!isRunning)
    {
        startMsec = millis();
        stopRequestedAtElapsedMsec = 0;
        stopRequested = false;
        isRunning = true;
    }

    // Allow color change while running
    this->animationId = animationId;
}

void AnimationContext::Stop()
{
    Serial.printf("AnimationContext.Stop()\n");

    if (isRunning && !stopRequested)
    {
        stopRequestedAtElapsedMsec = millis() - startMsec;
        stopRequested = true;
    }
}

void AnimationContext::StopImmediate()
{
    isRunning = false;
}

bool AnimationContext::IsRunning() const
{
    return isRunning;
}

bool AnimationContext::IsStopRequested() const
{
    return stopRequested;
}

void AnimationContext::SetRotationSpeed(long pixelsPerSec)
{
    this->rotationSpeedPixelsPerSec = pixelsPerSec;
}

void AnimationContext::SetFlashBrightness(uint8_t brightness)
{
    this->flashBrightness = brightness;
}

void AnimationContext::Render() const
{
    if (!isRunning)
        return;

    unsigned long elapsedMsecMaster = millis() - startMsec;

    // How "long" is each band of color in msec
    long curvePeriodMsec = 1000;
    if (animationId == 2)
    {
        // Smaller bands of color for rainbow mode
        curvePeriodMsec = 300;
    }

    double flashWhitePercent = 0.0;
    if (flashBrightness > 0)
    {
        // Timings to sync with the portal sound effects
        const unsigned long firstFlashDurationMsec = 1000;

        // Second flash only for the finale animation
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
            // Finale has a second flash white effect
            flashWhitePercent = pow(2, -6 * ((double)(elapsedMsecMaster - secondFlashStartMsec) / (double)secondFlashDurationMsec));
        }
        
        // Delay the start of the main animation by 750 msec to overlap slightly with the flash white
        elapsedMsecMaster -= (firstFlashDurationMsec / 2);
    }

    RGBW flashWhiteColor = RGBW::AllChannels(flashWhitePercent * 255);

    bool allStopped = stopRequested;

    // Compute the color for each pixel in the ring. Duplicate the pixels in the second ring.
    for (int i = 0; i < LED_COUNT_PER_RING; ++i)
    {
        // "Local" time per pixel varies; think of it as a phase shift or time zones.
        // So each point in time maps to certain pixel color, but each pixel is in a slightly
        // different time zone.
        long elapsedMsecLocal = elapsedMsecMaster - (i * 1000 / rotationSpeedPixelsPerSec);

        // What percent of the color band has already passed? 0 = start, 0.5 = middle/brightest, 1 = end
        double percentDone = (double)(elapsedMsecLocal % curvePeriodMsec) / (double)curvePeriodMsec;
        
        // Cheaper approximation of a normal curve, so the brightest spot is in the middle
        double brightnessPercent = cos(PI * (percentDone - 0.5));
        brightnessPercent *= brightnessPercent;

        // Black out pixels that the animation has not yet reached, or that are past the stop time
        if (elapsedMsecLocal < 0 || (stopRequested && elapsedMsecLocal > stopRequestedAtElapsedMsec))
        {
            brightnessPercent = 0.0;
        }
        else
        {
            allStopped = false;
        }

        if (animationId == ANIMATION_ID_AURADON)
        {
            RGBW& baseColor = colorMapAuradon[(elapsedMsecLocal / curvePeriodMsec) % distinctColorsAuradon];
            SetPixelColor(
                i,
                baseColor
                    .AdjustBrightness(brightnessPercent)
                    .PerChannelMax(flashWhiteColor)
                    .GetRaw()
            );
        }
        else if (animationId == ANIMATION_ID_ISLE)
        {
            RGBW& baseColor = colorMapIsle[(elapsedMsecLocal / curvePeriodMsec) % distinctColorsIsle];
            SetPixelColor(
                i,
                baseColor
                    .AdjustBrightness(brightnessPercent)
                    .PerChannelMax(flashWhiteColor)
                    .GetRaw()
            );
        }
        else if (animationId == ANIMATION_ID_FINALE)
        {
            long hue = ((elapsedMsecLocal / curvePeriodMsec) % distinctColorsFinale) * 65536 / distinctColorsFinale;
            RGBW color = RGBW::FromHSV(hue, 255, brightnessPercent * 255);

            // Apply flash white effect (all channels)
            color = color.PerChannelMax(flashWhiteColor);

            SetPixelColor(i, color.GetRaw());
        }
    }

    if (allStopped)
    {
        isRunning = false;
    }
}
