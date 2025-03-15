#include "SystemConfiguration.h"
#include "Keypad.h"
#include "Display.h"
#include "UIController.h"
#include "LedStrip.h"
#include "Constants.h"

void DisplayTestPattern()
{
  strip.clear();
  strip.setBrightness(sysConfig.brightness);

  for (int i = 0; i < LED_COUNT; i++)
  {
      strip.setPixelColor(
        i,
        (i % 4) == 0 ? 255 : 0,
        (i % 4) == 1 ? 255 : 0,
        (i % 4) == 2 ? 255 : 0,
        (i % 4) == 3 ? 255 : 0
      );
  }

  strip.show();
}

void setup()
{
  Serial.begin(115200);
  Serial.println("hello serial");
  
  InitializeDisplay();
  StartupMessage("Starting up...");

  customKeypad.begin();
  StartupMessage("Keypad OK");
  
  if (!sysConfig.InitializeStorage())
  {
    SystemPanic("Config load failed");
  }
  StartupMessage("Flash config OK");

  strip.begin();
  DisplayTestPattern();
  StartupMessage("LED init OK");
  delay(2000);
}

double fps = 0.0;
int currentPixel = 0;

void loop()
{
  unsigned long startTimeUsec = micros();

  uiController.Process();

  strip.setBrightness(sysConfig.brightness);
  strip.clear();

  strip.setPixelColor(currentPixel++, 0, 0, 0, 255);
  if (currentPixel >= LED_COUNT)
    currentPixel = 0;
  
  strip.show();

  unsigned long endTimeUsec = micros();
  unsigned long elapsedUsec = endTimeUsec - startTimeUsec;

  Serial.println(elapsedUsec);
  fps = (double)1000000.0 / (double)elapsedUsec;
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait)
{
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this loop:
  for (long firstPixelHue = 0; firstPixelHue < 5 * 65536; firstPixelHue += 256)
  {
    // strip.rainbow() can take a single argument (first pixel hue) or
    // optionally a few extras: number of rainbow repetitions (default 1),
    // saturation and value (brightness) (both 0-255, similar to the
    // ColorHSV() function, default 255), and a true/false flag for whether
    // to apply gamma correction to provide 'truer' colors (default true).
    strip.rainbow(firstPixelHue);
    // Above line is equivalent to:
    // strip.rainbow(firstPixelHue, 1, 255, 255, true);
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}
