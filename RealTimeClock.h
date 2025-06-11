#pragma once

#include <samd.h>

class RealTimeClock
{
public:

  static void begin()
  {
    OSC32KCTRL->XOSC32K.reg = 
      OSC32KCTRL_XOSC32K_ENABLE |
      OSC32KCTRL_XOSC32K_EN32K |
      OSC32KCTRL_XOSC32K_XTALEN |
      OSC32KCTRL_XOSC32K_STARTUP(0x6); // medium startup time

    while (!OSC32KCTRL->STATUS.bit.XOSC32KRDY); // Wait for stability

    OSC32KCTRL->RTCCTRL.reg = OSC32KCTRL_RTCCTRL_RTCSEL_XOSC32K;

    MCLK->APBAMASK.reg |= MCLK_APBAMASK_RTC;

    RTC->MODE0.CTRLA.bit.ENABLE = 0;
    while (RTC->MODE0.SYNCBUSY.bit.ENABLE);

    RTC->MODE0.CTRLA.bit.SWRST = 1;
    while (RTC->MODE0.SYNCBUSY.bit.SWRST);

    RTC->MODE0.CTRLA.reg =
      RTC_MODE0_CTRLA_MODE_COUNT32 |  // 32-bit counter mode
      RTC_MODE0_CTRLA_COUNTSYNC | // Synchronize count
      RTC_MODE0_CTRLA_PRESCALER_DIV1; // no prescaler

    RTC->MODE0.CTRLA.bit.ENABLE = 1;
    while (RTC->MODE0.SYNCBUSY.bit.ENABLE);
  }

  static uint32_t readClockRegister()
  {
    //RTC->MODE0.COUNT.reg; // Trigger read
    //while (RTC->MODE0.SYNCBUSY.bit.COUNT);
    return RTC->MODE0.COUNT.reg;
  }
};
