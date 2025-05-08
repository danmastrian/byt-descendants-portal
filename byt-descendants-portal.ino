#include "Constants.h"
#include "SystemConfiguration.h"
#include "Keypad.h"
#include "Display.h"
#include "UIController.h"
#include "LedStrip.h"
#include "RenderController.h"
#include <Wire.h>
#include "wiring_private.h"
#include "DmxData.h"
#include <CRC32.h>

#define STATIC_ASSERT(condition) typedef char p__LINE__[ (condition) ? 1 : -1];
#define ASSERT_TYPE_SIZE(type, bytes) STATIC_ASSERT(sizeof(type) == (bytes))

ASSERT_TYPE_SIZE(long, 4)
ASSERT_TYPE_SIZE(int, 4)
ASSERT_TYPE_SIZE(uint8_t, 1)

// 2 bytes for start channel, 1 byte for channel count
const int I2C_PACKET_HEADER_BYTES = sizeof(uint16_t) + sizeof(uint8_t);
const int I2C_PACKET_METADATA_BYTES = I2C_PACKET_HEADER_BYTES + sizeof(crc_size_t);
const uint8_t I2C_DEV_ADDR = 0x55;

#define DMX_INPUT_PIN_SDA A2
#define DMX_INPUT_PIN_SCK A3

// SERCOM setup for the secondary i2c bus that receives the DMX data forwarded from the other MCU
TwoWire myWire(&sercom4, DMX_INPUT_PIN_SDA, DMX_INPUT_PIN_SCK);

void SERCOM4_0_Handler() { myWire.onService(); }
void SERCOM4_1_Handler() { myWire.onService(); }
void SERCOM4_2_Handler() { myWire.onService(); }
void SERCOM4_3_Handler() { myWire.onService(); }

class AnimationContext
{
  unsigned long startMsec = 0;
  unsigned long stopRequestedAtElapsedMsec = 0;
  bool stopRequested = false;
  bool isRunning = false;
  int animationId = 0;

  const long curvePeriodMsec = 1000;

public:
  void Start()
  {
    if (!isRunning)
    {
      startMsec = millis();
      stopRequestedAtElapsedMsec = 0;
      stopRequested = false;
      isRunning = true;
    }
  }
  
  void Stop()
  {
    if (isRunning && !stopRequested)
    {
      stopRequestedAtElapsedMsec = millis() - startMsec;
      stopRequested = true;
    }
  }

  void Render()
  {
    if (!isRunning)
      return;

    unsigned long elapsedMsecMaster = millis() - startMsec;
    
    const unsigned long flashWhiteDurationMsec = 1000;

    double flashWhitePercent = 0.0;
    if (elapsedMsecMaster < flashWhiteDurationMsec)
    {
      //flashWhitePercent = 1. - (elapsedMsecMaster / flashWhiteDurationMsec.);

      // Exponential decay function
      flashWhitePercent = pow(2, -6 * ((double)elapsedMsecMaster / (double)flashWhiteDurationMsec));

      // Delay the start of the main animation by 750 msec to overlap sligtly with the flash white
      elapsedMsecMaster -= 750; 
    }

    bool allStopped = true;

    // TODO duplicate values for secondary ring
    for (int i = 0; i < LED_COUNT_TOTAL; ++i)
    {
      long elapsedMsecLocal = elapsedMsecMaster - (i * 1000 / 288); // propagate at N pixels/sec

      double percentDone = (double)(elapsedMsecLocal % curvePeriodMsec) / (double)curvePeriodMsec;
      //double brightnessPercent = sin(percentDone * PI);
      double brightnessPercent = cos(PI * (percentDone - 0.5));
      brightnessPercent *= brightnessPercent;

      if (elapsedMsecLocal < 0 || (stopRequested && elapsedMsecLocal > stopRequestedAtElapsedMsec))
      {
        brightnessPercent = 0.0;
      }
      else
      {
        allStopped = false;
      }

      if (!stopRequested)
        allStopped = false;

      // Auradon
      /*
      if ((elapsedMsecLocal / curvePeriodMsec) % 2 == 0)
      {
        strip.setPixelColor(
          i,
          brightnessPercent * 0.5 * 255,
          brightnessPercent * 0.3 * 255,
          0,
          brightnessPercent * 255
        );
      }
      else
      {
        strip.setPixelColor(
          i,
          0,
          0,
          brightnessPercent * 255,
          0
        );
      }
        */

      if ((elapsedMsecLocal / curvePeriodMsec) % 2 == 0)
      {
        strip.setPixelColor(
          i,
          brightnessPercent * 0.5 * 255,
          0,
          brightnessPercent * 0.8 * 255,
          max(0, flashWhitePercent * 255)
        );
      }
      else
      {
        strip.setPixelColor(
          i,
          0,
          brightnessPercent * 255,
          0,
          max(0, flashWhitePercent * 255)
        );
      }
    }

    if (allStopped)
    {
      isRunning = false;
    }
  }
};

AnimationContext animationContext;

class TestRenderProcessor : public RenderProcessor
{
public:

  virtual void Render() const
  {
    //rainbow();
    animationContext.Start();
    animationContext.Render();
  }
};

class IdleRenderProcessor : public RenderProcessor
{
public:

  virtual void Render() const
  {
    // TODO
  }
};

class DmxRenderProcessor : public RenderProcessor
{
public:

  virtual void Render() const
  {

  }
};

RenderProcessor* renderProcessors[] = {
  new TestRenderProcessor(),
  new IdleRenderProcessor(),
  new DmxRenderProcessor(),
};

class RootRenderProcessor : public RenderProcessor
{
public:

  virtual void Render() const
  {
    if (sysConfig.mode >= sizeof(renderProcessors) / sizeof(renderProcessors[0]))
      return;

    renderProcessors[sysConfig.mode]->Render();
  }
};

// Global data
double fps = 0.0;
RootRenderProcessor rrp;
RenderController renderer(&rrp);
volatile unsigned long lastIsrUsec = 0;
CRC32 crc;

void DisplayTestPattern()
{
  strip.clear();
  strip.setBrightness(sysConfig.brightness);

  for (int i = 0; i < LED_COUNT_PER_CHANNEL; i++)
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

// i2c ISR for DMX data coming from the other MCU
void onReceive(int len) 
{
  //unsigned long isrStartUsec = micros();

  uint8_t recvBuf[len];
  for (int i = 0; i < len; ++i)
  {
    recvBuf[i] = myWire.read();
  }

  uint16_t startCh = (((uint16_t)recvBuf[0]) << 8) | (uint16_t)recvBuf[1];
  uint8_t chCount = recvBuf[2];

  if (len != chCount + I2C_PACKET_METADATA_BYTES)
  {
    Serial.printf("MALFORMED I2C PACKET: len = %d bytes, expected chCount = %u, startCh = %u\n", len, chCount, startCh);
    return;
  }

  crc.reset();
  crc.add(recvBuf, len - sizeof(crc_size_t)); // Exclude CRC bytes
  crc_size_t crcValueActual = crc.calc();
  
  crc_size_t crcValueExpected;
  memcpy(
    &crcValueExpected,
    recvBuf + len - sizeof(crc_size_t), // Get the last 4 bytes which is the CRC32 value
    sizeof(crc_size_t));

  if (crcValueActual != crcValueExpected)
  {
    Serial.printf("CRC MISMATCH: Actual CRC = %08X, Expected CRC = %08X\n", crcValueActual, crcValueExpected);
    return; // Ignore this packet since the CRC doesn't match
  }

  lastDmxPacketReceivedMsec = millis();
  
  for (uint8_t i = 0; i < chCount; ++i)
  {
    DmxChannelData& ch = dmxData[startCh + i];
    
    if (ch.LastUpdatedMsec <= lastDmxUniverseUpdateCompletedMsec)
    {
      // Reduce the count of pending channels since the full DMX universe was updated
      dmxChannelsPendingSinceLastCompleteUpdate--;
    }

    if (dmxChannelsPendingSinceLastCompleteUpdate == 0)
    {
      // All channels have been received
      lastDmxUniverseUpdateCompletedMsec = lastDmxPacketReceivedMsec;
      
      // Reset the count of pending channels for the next cycle
      dmxChannelsPendingSinceLastCompleteUpdate = DMX_UNIVERSE_SIZE;
    }

    ch.Value = recvBuf[I2C_PACKET_HEADER_BYTES + i];
    ch.LastUpdatedMsec = lastDmxPacketReceivedMsec;
  }

  //unsigned long isrEndUsec = micros();

  //lastIsrUsec = isrEndUsec - isrStartUsec;
}

void setup()
{
  memset(dmxData, 0, sizeof(dmxData));

  Serial.begin(115200);
  Serial.println("Serial comm init OK");

  myWire.onReceive(onReceive);
  myWire.begin((uint8_t)I2C_DEV_ADDR);
  //myWire.setClock(400000);
  //myWire.setWireTimeout();

  // Critical that these come *after* the TwoWire::begin() call above.
  // Map these pins to the alternate SERCOM (#4)
  pinPeripheral(DMX_INPUT_PIN_SDA, PIO_SERCOM_ALT);
  pinPeripheral(DMX_INPUT_PIN_SCK, PIO_SERCOM_ALT);
  
  InitializeDisplay();
  StartupMessage("Starting up...");

  if (!InitKeypad())
  {
    SystemPanic("Keypad init failed");
  }
  StartupMessage("Keypad OK");
  
  if (!sysConfig.InitializeStorage())
  {
    SystemPanic("Config load failed");
  }
  StartupMessage("Config load OK");

  strip.begin();
  //DisplayTestPattern();
  StartupMessage("LED init OK");
  //delay(1000);
}

void loop()
{
  unsigned long startTimeUsec = micros();

  uiController.Process();
  renderer.Render();

  unsigned long endTimeUsec = micros();
  unsigned long elapsedUsec = endTimeUsec - startTimeUsec;

  //Serial.println(elapsedUsec);
  fps = (double)1000000.0 / (double)elapsedUsec;

  //Serial.println(lastIsrUsec);
}

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
