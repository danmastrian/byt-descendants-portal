#include "ShowRenderProcessor.h"
#include "SystemConfiguration.h"
#include "AnimationContext.h"

ShowRenderProcessor::ShowRenderProcessor(bool isSensoryFriendly)
    : RenderProcessor(isSensoryFriendly ? "MANUAL SENS" : "MANUAL NORMAL"),
        isSensoryFriendly(isSensoryFriendly)
{
}

void ShowRenderProcessor::Render() const
{
    if (isSensoryFriendly)
    {
        strip.setBrightness(min(sysConfig.brightness, sensoryFriendlyMaxBrightness));
        animationContext.SetFlashBrightness(0);
    }
    else
    {
        strip.setBrightness(sysConfig.brightness);
        animationContext.SetFlashBrightness(255);
    }

    animationContext.SetRotationSpeed(LED_COUNT_PER_RING / 3);

    animationContext.Render();
}

void ShowRenderProcessor::WriteStatusString(Print& output) const
{
    if (animationContext.IsRunning())
    {
        if (animationContext.IsStopRequested())
        {
        output.print(F("STOP "));
        }

        output.print(animationContext.GetRunningAnimationId());
    }
    else
    {
        output.print(F("---"));
    }
}
