#include "TestRenderProcessor.h"

void rainbow()
{
  const long cyclePeriodMsec = 10000;
  long firstPixelHue = (millis() % cyclePeriodMsec) * 65536 / cyclePeriodMsec;

  // strip.rainbow() can take a single argument (first pixel hue) or
  // optionally a few extras: number of rainbow repetitions (default 1),
  // saturation and value (brightness) (both 0-255, similar to the
  // ColorHSV() function, default 255), and a true/false flag for whether
  // to apply gamma correction to provide 'truer' colors (default true).
  strip.rainbow(firstPixelHue);
  // Above line is equivalent to:
  // strip.rainbow(firstPixelHue, 1, 255, 255, true);
}
