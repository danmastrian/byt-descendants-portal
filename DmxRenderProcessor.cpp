#include "DmxRenderProcessor.h"
#include "DmxData.h"
#include "SystemConfiguration.h"
#include "AnimationContext.h"

uint8_t DmxRenderProcessor::GetCurrentEffect() const
{
return GetLogicalDmxChannelValue(LOGICAL_DMX_CHANNEL_EFFECT_MODE) / 10;
}

DmxRenderProcessor::DmxRenderProcessor()
    : RenderProcessor("DMX")
{
}

void DmxRenderProcessor::Render() const
{
    uint8_t effect = GetCurrentEffect();

    if (effect == DMX_EFFECT_MANUAL_RGBW)
    {
        // Manually set RGB for the whole ring
        strip.fill(
            RGBW(
                GetLogicalDmxChannelValue(LOGICAL_DMX_CHANNEL_COLOR_R),
                GetLogicalDmxChannelValue(LOGICAL_DMX_CHANNEL_COLOR_G),
                GetLogicalDmxChannelValue(LOGICAL_DMX_CHANNEL_COLOR_B),
                GetLogicalDmxChannelValue(LOGICAL_DMX_CHANNEL_COLOR_W)
            ).GetRaw(),
            0,
            LED_COUNT_TOTAL
            );
        return;
    }

    switch (effect)
    {
        case 0:
            animationContext.Stop();
            break;

        default:
            animationContext.Start(effect - 1);
            break;
    }

    animationContext.SetRotationSpeed(
        // Set a minimum rotation speed, to avoid DMX setting it too low
        max(15, GetLogicalDmxChannelValue(LOGICAL_DMX_CHANNEL_ROTATION_SPEED)) * 10
    );

    animationContext.SetFlashBrightness(
        GetLogicalDmxChannelValue(LOGICAL_DMX_CHANNEL_FLASH_BRIGHTNESS)
    );

    strip.setBrightness(
        GetLogicalDmxChannelValue(LOGICAL_DMX_CHANNEL_MASTER_BRIGHTNESS)
    );

    animationContext.Render();
}

void DmxRenderProcessor::WriteStatusString(Print& output) const
{
    if (GetCurrentEffect() == DMX_EFFECT_MANUAL_RGBW)
    {
        output.printf(
            "%02X%02X%02X%02X",
            GetLogicalDmxChannelValue(LOGICAL_DMX_CHANNEL_COLOR_R),
            GetLogicalDmxChannelValue(LOGICAL_DMX_CHANNEL_COLOR_G),
            GetLogicalDmxChannelValue(LOGICAL_DMX_CHANNEL_COLOR_B),
            GetLogicalDmxChannelValue(LOGICAL_DMX_CHANNEL_COLOR_W)
            );
        return;
    }

    output.print(F("DMX "));

    if (animationContext.IsStopRequested() && animationContext.IsRunning())
    {
        output.print(F("STOP..."));
    }
    else if (animationContext.IsRunning())
    {
        output.print(animationContext.GetRunningAnimationId());
        output.print(F(" @ "));
        output.print(GetLogicalDmxChannelValue(LOGICAL_DMX_CHANNEL_MASTER_BRIGHTNESS));
    }
    else
    {
        output.print(F("---"));
    }
}
