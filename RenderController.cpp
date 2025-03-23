#include "RenderController.h"
#include "LedStrip.h"
#include "SystemConfiguration.h"

void RenderController::Render() const
{
  strip.setBrightness(sysConfig.brightness);
  strip.clear();

  processor->Render();

  strip.show();
}

void RenderController::SetProcessor(const RenderProcessor* processor)
{
  this->processor = processor;
}