#pragma once

#include "Constants.h"
#include "SystemConfiguration.h"
#include "LedStrip.h"
#include "RGBW.h"

class AnimationContext
{
private:

    unsigned long startMsec = 0;
    unsigned long stopRequestedAtElapsedMsec = 0;
    bool stopRequested = false;
    mutable bool isRunning = false;
    int animationId = 0;
    uint8_t flashBrightness = 0;
    long rotationSpeedPixelsPerSec = LED_COUNT_PER_RING / 3;

private:

    static void SetPixelColor(int pixelIndex, uint8_t r, uint8_t g, uint8_t b, uint8_t w);
    static void SetPixelColor(int pixelIndex, uint32_t rgbw);

public:

    int GetRunningAnimationId() const;

    void Start(int animationId);
    void Stop();
    void StopImmediate();
    bool IsRunning() const;
    bool IsStopRequested() const;

    void SetRotationSpeed(long pixelsPerSec);
    void SetFlashBrightness(uint8_t brightness);

    void Render() const;
};

extern AnimationContext animationContext;

#define ANIMATION_ID_AURADON    0
#define ANIMATION_ID_ISLE       1
#define ANIMATION_ID_FINALE     2
