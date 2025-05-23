#pragma once

#include "RenderController.h"

class ShowRenderProcessor : public RenderProcessor
{
private:

  const uint8_t sensoryFriendlyMaxBrightness = 20;

  bool isSensoryFriendly;

public:

  ShowRenderProcessor(bool isSensoryFriendly);
  virtual void Render() const;
  virtual void WriteStatusString(Print& output) const;
};
