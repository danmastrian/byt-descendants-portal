#pragma once

#include "RenderController.h"

extern void rainbow();

class TestRenderProcessor : public RenderProcessor
{
public:

  TestRenderProcessor()
    : RenderProcessor("TEST")
  {
  }

  virtual void Render() const
  {
    rainbow();
  }

  virtual void WriteStatusString(Print& output) const
  {
    output.print(F("TEST"));
  }
};
