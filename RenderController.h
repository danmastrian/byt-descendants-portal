#pragma once

#include <Arduino.h>

class RenderProcessor
{
private:

  const char* name;

protected:

  RenderProcessor(const char* name)
    : name(name)
  {
  }

public:

  virtual void Render() const;

  const char* GetName() const
  {
    return name;
  }

  virtual void WriteStatusString(Print& output) const;
};

extern RenderProcessor* renderProcessors[];
extern const int RenderProcessorCount;
extern bool manualRenderMode;

class RenderController
{
private:

  const RenderProcessor* processor;

public:

  RenderController(const RenderProcessor* processor)
  {
    this->processor = processor;
  }

  void Render() const;
  void SetProcessor(const RenderProcessor* processor);

  void WriteStatusString(Print& output) const;
};

extern RenderController renderer;
