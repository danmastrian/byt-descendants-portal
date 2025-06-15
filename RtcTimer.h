#pragma once

#include "RealTimeClock.h"

class RtcTimer
{
private:

  const char* label_;
  uint32_t startTime_;
  uint32_t stopTime_;
  bool outputElapsedTime_;

public:

  RtcTimer(const char* label, bool outputElapsedTime = false)
    : label_(label),
      outputElapsedTime_(outputElapsedTime)
  {
    Restart();
  }

  ~RtcTimer()
  {
    Stop();

    if (outputElapsedTime_)
    {
        OutputElapsedTime(Serial);
    }
  }

  void Restart()
  {
    startTime_ = RealTimeClock::readClockRegister();
    stopTime_ = 0;
  }

  void Stop()
  {
    if (stopTime_ != 0)
    {
      stopTime_ = RealTimeClock::readClockRegister();
    }
  }

  void OutputElapsedTime(Print& output)
  {
    output.printf("%s: %u us\n", label_, ElapsedMicroseconds());
  }

  uint32_t ElapsedMicroseconds() const
  {
    uint32_t currentTime = stopTime_;
    if (currentTime == 0)
    {
      currentTime = RealTimeClock::readClockRegister();
    }

    // Convert RTC ticks to microseconds (1 tick = 30.517578125 us)
    return (uint32_t)((currentTime - startTime_) * 30.517578125);
  }
};
