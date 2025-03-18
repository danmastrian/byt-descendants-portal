#include "SystemConfiguration.h"
#include "Keypad.h"
#include "Display.h"
#include "UIController.h"
#include "LedStrip.h"
#include "Constants.h"
#include <Wire.h>
#include "wiring_private.h"

#define I2C_DEV_ADDR 0x55

unsigned long lastDmxPacketReceivedMsec = 0;
uint8_t dmxData[513] = { 0 };

TwoWire myWire(&sercom4, A2, A3);

void SERCOM4_0_Handler() { myWire.onService(); }
void SERCOM4_1_Handler() { myWire.onService(); }
void SERCOM4_2_Handler() { myWire.onService(); }
void SERCOM4_3_Handler() { myWire.onService(); }

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

void onReceive(int len) 
{
  Serial.printf("onReceive[%d]: ", len);

  uint8_t recvBuf[len];
  for (int i = 0; i < len; ++i)
  {
    if (myWire.available())
    {
      recvBuf[i] = myWire.read();
    }
    else
    {
      Serial.printf("I2C PACKET READ ERROR: len = %d bytes, failed read at byte %d\n", len, i);
    }
  }

  uint8_t startCh = recvBuf[0]; // BUGBUG: CHANNELS are uint16!
  uint8_t chCount = recvBuf[1];

  if (len != chCount + 2)
  {
    Serial.printf("MALFORMED I2C PACKET: len = %d bytes, expected chCount = %d\n", len, chCount);
    return;
  }

  lastDmxPacketReceivedMsec = millis();
  
  for (uint8_t i = 0; i < chCount; ++i)
  {
    dmxData[startCh + i] = recvBuf[2 + i];
    //Serial.write(dmxData[chIdx]);
    Serial.printf("[%03d:%03d]", startCh + i, dmxData[startCh + i]);
  }
  Serial.println();
}

void setup()
{
  Serial.begin(115200);
  Serial.println("hello serial");

  pinPeripheral(A2, PIO_SERCOM_ALT);
  pinPeripheral(A3, PIO_SERCOM_ALT);

  myWire.onReceive(onReceive);
  //myWire.onRequest(onRequest);
  myWire.begin((uint8_t)I2C_DEV_ADDR);
  //myWire.setClock(100000);

  pinPeripheral(A2, PIO_SERCOM_ALT);
  pinPeripheral(A3, PIO_SERCOM_ALT);
  
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
//int currentPixel = 0;

void loop()
{
  unsigned long startTimeUsec = micros();

  uiController.Process();

  strip.setBrightness(sysConfig.brightness);
  strip.clear();

  //strip.setPixelColor(currentPixel++, 0, 0, 0, 255);
  //if (currentPixel >= LED_COUNT)
  //  currentPixel = 0;
  rainbow(0);
  
  strip.show();

  unsigned long endTimeUsec = micros();
  unsigned long elapsedUsec = endTimeUsec - startTimeUsec;

  //Serial.println(elapsedUsec);
  fps = (double)1000000.0 / (double)elapsedUsec;
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait)
{
  long firstPixelHue = millis() % 65536;

  // strip.rainbow() can take a single argument (first pixel hue) or
  // optionally a few extras: number of rainbow repetitions (default 1),
  // saturation and value (brightness) (both 0-255, similar to the
  // ColorHSV() function, default 255), and a true/false flag for whether
  // to apply gamma correction to provide 'truer' colors (default true).
  strip.rainbow(firstPixelHue);
  // Above line is equivalent to:
  // strip.rainbow(firstPixelHue, 1, 255, 255, true);
}
