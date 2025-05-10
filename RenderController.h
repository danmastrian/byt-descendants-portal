#pragma once

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
};

extern RenderProcessor* renderProcessors[];
extern const int RenderProcessorCount;

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
};
