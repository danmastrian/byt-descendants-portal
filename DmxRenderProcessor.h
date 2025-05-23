#pragma once

#include "RenderController.h"

class DmxRenderProcessor : public RenderProcessor
{
private:

  const uint8_t DMX_EFFECT_MANUAL_RGBW = 25;

  uint8_t GetCurrentEffect() const;

public:

    DmxRenderProcessor();
    virtual void Render() const;
    virtual void WriteStatusString(Print& output) const;
};
