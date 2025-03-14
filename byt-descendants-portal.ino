#define SSD1306_NO_SPLASH 1 // doesn't work, needs to be global compiler option

#include <SPI.h>
#include <SdFat.h>
#include <Adafruit_SPIFlash.h>
#include "flash_config.h"

Adafruit_SPIFlash flash(&flashTransport);

FatVolume fatfs;

#define D_CONFIG "/config"
#define F_CONFIG_SYSCONFIG "/config/sysconfig.bin"


#include <Adafruit_Keypad.h>
#include <Adafruit_Keypad_Ringbuffer.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define KEYPAD_PID1332
#define R1 5
#define R2 99
#define R3 99
#define R4 99
#define C3 9
#define C4 10
#define C1 11
#define C2 12
#include "keypad_config.h"

Adafruit_Keypad customKeypad = Adafruit_Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#define LED_PIN A0
#define LED_COUNT (144 * 12)

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);

#include "UIController.h"

SystemConfiguration sysConfig;

typedef struct PersistentConfigData
{
  uint16_t dmxStartChannel;
  uint8_t brightness;
  uint8_t mode;  
} PersistentConfigData;

void StartupMessage(const char* msg)
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.setTextColor(SSD1306_WHITE);

  display.println(F("STARTING UP..."));
  display.println();
  display.println(msg);
  display.display();
  delay(100);
}

bool InitializeStorage()
{
  StartupMessage("InitializeStorage...");
  
  // Initialize flash library and check its chip ID.
  if (!flash.begin())
  {
    Serial.println(F("Error, failed to initialize flash chip!"));
    return false;
  }
  Serial.print(F("Flash chip JEDEC ID: 0x"));
  Serial.println(flash.getJEDECID(), HEX);
  StartupMessage("SD flash init OK");

  // First call begin to mount the filesystem.  Check that it returns true
  // to make sure the filesystem was mounted.
  if (!fatfs.begin(&flash))
  {
    Serial.println(F("Error, failed to mount newly formatted filesystem!"));
    return false;
  }
  StartupMessage("FAT FS init OK");

  if (!fatfs.exists(D_CONFIG))
  {
    Serial.println(F("Test directory not found, creating..."));
    fatfs.mkdir(D_CONFIG);

    if (!fatfs.exists(D_CONFIG))
    {
      Serial.println(F("Error, failed to create directory!"));
      return false;
    }
  }

  if (!fatfs.exists(F_CONFIG_SYSCONFIG))
  {
    File32 writeFile = fatfs.open(F_CONFIG_SYSCONFIG, FILE_WRITE);
    if (!writeFile)
    {
      Serial.println(F("Error, failed to open " F_CONFIG_SYSCONFIG " for writing!"));
      return false;
    }
    Serial.println(F("Opened file " F_CONFIG_SYSCONFIG " for writing/appending..."));

    // Default settings
    PersistentConfigData data { 0, 50, 0 };
  
    writeFile.write(&data, sizeof(data));
    writeFile.flush();
    writeFile.close();
    Serial.println(F("Wrote to file " F_CONFIG_SYSCONFIG "!"));
  }

  // Now open the same file but for reading.
  File32 readFile = fatfs.open(F_CONFIG_SYSCONFIG, FILE_READ);
  if (!readFile)
  {
    Serial.println(F("Error, failed to open " F_CONFIG_SYSCONFIG " for reading!"));
    return false;
  }

  StartupMessage("Reading config");

  // Read data using the same read, find, readString, etc. functions as when
  // using the serial class.  See SD library File class for more documentation:
  //   https://www.arduino.cc/en/reference/SD

  PersistentConfigData loadedConfig;
  if (readFile.read(&loadedConfig, sizeof(loadedConfig)))
  {
    sysConfig.dmxStartChannel = loadedConfig.dmxStartChannel;
    sysConfig.brightness = loadedConfig.brightness;
    StartupMessage("Config OK");  
  }
  readFile.close();

  return true;
}


UIStateMain uiStateMain;
UIController uiController(customKeypad, display, &uiStateMain);

void setup()
{
  Serial.begin(115200);
  Serial.println("hello serial");
  
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }

  display.display();
  //delay(1000); // skip splash screen

  StartupMessage("Starting up...");

  customKeypad.begin();
  StartupMessage("Keypad OK");
  
  InitializeStorage();

  StartupMessage("Init LED driver");

  strip.begin();
  strip.show();

  StartupMessage("Setup done.");
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
  delay(2);

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
