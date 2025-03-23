#pragma once

class RenderProcessor
{
public:

  virtual void Render() const;
};

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
