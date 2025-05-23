#pragma once

#include "RenderController.h"

class IdleRenderProcessor : public RenderProcessor
{
public:

  IdleRenderProcessor()
    : RenderProcessor("IDLE")
  {
  }

  virtual void Render() const
  {
  }

  virtual void WriteStatusString(Print& output) const
  {
    output.print(F("IDLE"));
  }
};
