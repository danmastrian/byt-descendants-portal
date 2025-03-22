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

#define DMX_INPUT_PIN_SDA A2
#define DMX_INPUT_PIN_SCK A3

TwoWire myWire(&sercom4, DMX_INPUT_PIN_SDA, DMX_INPUT_PIN_SCK);

double fps = 0.0;

#define ROWS 1
#define COLS 4

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
  const int headerBytes = 3;
  //Serial.printf("onReceive[%d]: ", len);

  uint8_t recvBuf[len];
  for (int i = 0; i < len; ++i)
  {
    recvBuf[i] = myWire.read();
  }

  // Read header
  uint16_t startCh = (((uint16_t)recvBuf[0]) << 8) | recvBuf[1];
  uint8_t chCount = recvBuf[2];

  if (len != chCount + headerBytes)
  {
    Serial.printf("MALFORMED I2C PACKET: len = %d bytes, expected chCount = %d\n", len, chCount);
    return;
  }

  lastDmxPacketReceivedMsec = millis();
  
  for (uint8_t i = 0; i < chCount; ++i)
  {
    dmxData[startCh + i] = recvBuf[headerBytes + i];
    //Serial.printf("[%03d:%03d]", startCh + i, dmxData[startCh + i]);
  }
  //Serial.println();
}

class DisplayProcessor
{
public:

  virtual void Render() const;
};

class DummyDisplayProcessor : public DisplayProcessor
{
public:

  virtual void Render() const
  {
    rainbow(0);
  }
};

class DisplayController
{
private:

  const DisplayProcessor* processor;

public:

  DisplayController(const DisplayProcessor* processor)
  {
    this->processor = processor;
  }

  void Render() const;
  void SetProcessor(const DisplayProcessor* processor);
};

void DisplayController::Render() const
{
  strip.setBrightness(sysConfig.brightness);
  strip.clear();

  processor->Render();

  strip.show();
}

void DisplayController::SetProcessor(const DisplayProcessor* processor)
{
  this->processor = processor;
}

DummyDisplayProcessor ddp;
DisplayController dc(&ddp);

void setup()
{
  Serial.begin(115200);
  Serial.println("Serial comms OK");

  myWire.onReceive(onReceive);
  myWire.begin((uint8_t)I2C_DEV_ADDR);
  //myWire.setClock(400000);
  //myWire.setWireTimeout();

  // Critical that these come *after* the TwoWire::begin() call above
  pinPeripheral(DMX_INPUT_PIN_SDA, PIO_SERCOM_ALT);
  pinPeripheral(DMX_INPUT_PIN_SCK, PIO_SERCOM_ALT);
  
  InitializeDisplay();
  StartupMessage("Starting up...");

  if (!InitKeypad())
  {
    SystemPanic("Failed to init keypad");
  }
  StartupMessage("Keypad OK");
  
  if (!sysConfig.InitializeStorage())
  {
    SystemPanic("Config load failed");
  }
  StartupMessage("Flash config OK");

  strip.begin();
  DisplayTestPattern();
  StartupMessage("LED init OK");
  delay(1000);
}

void loop()
{
  unsigned long startTimeUsec = micros();

  uiController.Process();
  dc.Render();

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
