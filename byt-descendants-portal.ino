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
#include "AnimationContext.h"
#include <Arduino.h>
#include "IdleRenderProcessor.h"
#include "TestRenderProcessor.h"
#include "DmxRenderProcessor.h"
#include "ShowRenderProcessor.h"

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

void CommonFaultHandler(const char* handlerName)
{
  Serial.printf("CRITICAL FAULT: %s\n", handlerName);
  Serial.flush();

  NVIC_SystemReset();
  
  while (true);
}

extern "C" void HardFault_Handler(void)
{
  CommonFaultHandler("HardFault");
}

extern "C" void MemManage_Handler(void)
{
  CommonFaultHandler("MemManage");
}

extern "C" void BusFault_Handler(void)
{
  CommonFaultHandler("BusFault");
}

extern "C" void UsageFault_Handler(void)
{
  CommonFaultHandler("UsageFault");
}

void EnableWatchdogTimer()
{
  WDT->CONFIG.bit.PER = 8; // ~2 second reset period
  WDT->CTRLA.bit.ENABLE = 1;
  while (WDT->SYNCBUSY.bit.ENABLE);

  Serial.println("WatchdogTimer enabled");
}

void ResetWatchdogTimer()
{
  WDT->CLEAR.reg = 0xA5;
  while (WDT->SYNCBUSY.bit.CLEAR);
}

RenderProcessor* renderProcessors[] = {
  new IdleRenderProcessor(),
  new TestRenderProcessor(),
  new ShowRenderProcessor(false), // DMX off; normal
  new ShowRenderProcessor(true),  // DMX off; sensory-friendly
  new DmxRenderProcessor(),       // DMX on
};

const int RenderProcessorCount = sizeof(renderProcessors) / sizeof(renderProcessors[0]);

bool manualRenderMode = false;

class RootRenderProcessor : public RenderProcessor
{
public:

  RootRenderProcessor()
    : RenderProcessor("ROOT")
  {
  }

  virtual void Render() const
  {
    // Gross hack; need to fix this later
    if ((sysConfig.mode == 4) && manualRenderMode)
    {
      // Ignore DMX and use manual controls
      renderProcessors[2]->Render();
      return;
    }

    if (sysConfig.mode >= RenderProcessorCount)
      return;

    renderProcessors[sysConfig.mode]->Render();
  }

  virtual void WriteStatusString(Print& output) const
  {
    if (sysConfig.mode >= RenderProcessorCount)
      return;

    renderProcessors[sysConfig.mode]->WriteStatusString(output);
  }
};

// Global data
double fps = 0.0;
RootRenderProcessor rrp;
RenderController renderer(&rrp);
volatile unsigned long lastIsrUsec = 0;
CRC32 crc;
unsigned long lastLoopStatusReportUsec = 0;

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
  delay(2000);
  Serial.println("Serial comm init OK");

  Serial1.begin(460800, SERIAL_8N1);

  //myWire.onReceive(onReceive);
  //myWire.begin((uint8_t)I2C_DEV_ADDR);
  //myWire.setClock(400000);
  //myWire.setWireTimeout();

  // Critical that these come *after* the TwoWire::begin() call above.
  // Map these pins to the alternate SERCOM (#4)
  pinPeripheral(DMX_INPUT_PIN_SDA, PIO_SERCOM_ALT);
  pinPeripheral(DMX_INPUT_PIN_SCK, PIO_SERCOM_ALT);
  
  //InitializeDisplay();
  //StartupMessage("STARTING UP...");

  //if (!InitKeypad())
  {
    //SystemPanic("Keypad init failed");
  }
  //StartupMessage("Keypad OK");

  //if (!sysConfig.InitializeStorage())
  {
    //SystemPanic("Config load failed");
  }
  //StartupMessage("Config load OK");

  //if (!strip.begin())
  {
    //SystemPanic("LED init failed");
  }
  //StartupMessage("LED init OK");

  //EnableWatchdogTimer();
  //StartupMessage("WDT init OK");
}

void loop()
{
  unsigned long startTimeUsec = micros();

  while (Serial1.available())
  {
    char c = Serial1.read();
    Serial.print(c);
  }

  if (startTimeUsec - lastLoopStatusReportUsec > 1000000)
  {
    Serial.printf("System loop alive @ %u us\n", startTimeUsec);
    lastLoopStatusReportUsec = startTimeUsec;
  }

  //uiController.Process();
  //renderer.Render();

  unsigned long endTimeUsec = micros();
  unsigned long elapsedUsec = endTimeUsec - startTimeUsec;

  //Serial.println(elapsedUsec);
  fps = (double)1000000.0 / (double)elapsedUsec;

  //Serial.println(lastIsrUsec);

  //ResetWatchdogTimer();
}
