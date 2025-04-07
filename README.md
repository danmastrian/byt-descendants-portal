# LED Portal for Disney's _Descendants_ Musical

## Overview

This repo contains the microcontroller code for a system that drives ~2300 WS2812 ("[NeoPixel](https://learn.adafruit.com/adafruit-neopixel-uberguide/the-magic-of-neopixels)") LEDs
for a community theater production of [Disney's "Descendants" musical](https://www.mtishows.com/disneys-descendants-the-musical).

The LEDs are arranged in two identical concentric circles, positioned vertically with the two half-strips adjacent to each other, to form a "magical portal" set piece. The system can be controlled via
[DMX](https://en.wikipedia.org/wiki/DMX512), although the animations are generated procedurally by the device firmware.

The system includes a small [OLED display](https://www.amazon.com/dp/B08CDN5PSJ) and [4-button keypad](https://www.adafruit.com/product/1332) which is used to display status and set configuration values such as DMX channel range and maximum brightness.

Aside from the animations themselves, most of the system is designed to be a generic DMX-compatible driver for large NeoPixel installations.

Currently, DMX universe refresh latency is ~40 msec, and render loop frequency is ~80 fps.

## Goals

- 30-60+ frames/sec
- DMX latency < 50 msec
- As ridiculously bright as possible

## LEDs and Power Budget

This was designed for 16 x 1 meter strips of [144 LEDs/meter strips of WS2812 LEDs](https://www.amazon.com/dp/B079ZRLMQR), 2304 RGBW LEDs in total. These draw ~20 mA/LED with a peak of 80 mA/pixel. Overall power draw at 5V should be 48-184A (230-920W), so this plan assumes that all pixels' RGBW channels will not be maxed out simultaneously.

## Control System

The code in this repo was written for the [Adafruit Feather M4 Express (SAMD51)](https://www.adafruit.com/product/3857) board.

Keypad input is handled by an [Adafruit TCA8418](https://www.adafruit.com/product/4918), to reduce the number of GPIO pins used on the M4,
because specific pins are needed for the 8 LED data outputs.

NeoPixel data output is managed by an [Adafruit NeoPXL8 Friend](https://www.adafruit.com/product/3975), which drives 8 LED strips concurrently
and uses DMA to avoid blocking the M4's CPU.

## DMX512 Interface

The DMX interface uses a [SparkFun ESP32 Thing Plus DMX to LED Shield](https://www.sparkfun.com/sparkfun-esp32-thing-plus-dmx-to-led-shield.html),
which provides RS485 decoding as well as the electrical isolation that the DMX standard recommends.

Unfortunately, this shield's library did not want
to compile for the Feather M4, so I added an [Adafruit ESP32](https://www.adafruit.com/product/3405) to the project, which connects to the
DMX shield and then forwards the DMX data to the Feather M4 via a secondary [i<sup>2</sup>c](https://en.wikipedia.org/wiki/I%C2%B2C) bus.
See https://github.com/danmastrian/byt-descendants-portal-dmx-peripheral for the code for the auxiliary MCU.

I did not use the main i<sup>2</sup>c bus because I had problems getting the M4 to work as both a controller (of the OLED and GPIO/keypad extender)
and peripheral (DMX data receiver from the ESP board) when using the default i<sup>2</sup>c bus/pins.

I had to add external pullup resistors to this bus, and initially I used 4.7Kohm resistors.
I saw a high rate of i<sup>2</sup>c packet truncation, and after some research I decided to try smaller resistors.
As I reduced the resistance, the rate of packet loss dropped. (TBD: 680ohm?)

## Power Supply

- [PSU: MeanWell 5V 100A](https://www.mouser.com/ProductDetail/MEAN-WELL/SE-600-5?qs=%252B6mEGs9UJHz4R8iFicJasg%3D%3D&countryCode=US&currencyCode=USD)
- [Bus Bars](https://www.amazon.com/dp/B07KVW7F5X)
- [VA Meter (contactless)](https://www.amazon.com/dp/B0BFJ5NV5L)
  - This lets me verify voltage and current draw without needing to shunt 100A through the meter.
- [Diode](https://www.amazon.com/dp/B0C1V6Y8ND)
  - This prevents the MCUs from trying to back-power the LED strips when the main PSU is off and the MCUs are being powered via USB.

## User Interface

### Home Screen

When the system has finished starting up, the home screen will be displayed.

```
[READY] - Mode 0 LOCK

 Bright   255 (100%)
 DMX Ch   374 - 381 *
 DMX Lag   42 ms     
 Render    83 fps

Press [OK] for menu
```

The first line has a spinning indicator to show that the main event loop is running.
It also shows the current operating mode of the system, and the word `LOCK` if the system is locked.

The `Bright` line shows the configured LED brightness level (0-255 and 0%-100%).

The `DMX Ch` line shows the DMX channel footprint of the system. Start and end channels are inclusive.
The `*` character after the channel range will toggle on/off every time a complete set of DMX universe data is received,
which indicates whether the system is properly connected to the DMX bus.

The `DMX Lag` line shows the average duration between receiving updates of the DMX universe data.
This shows how responsive the system should be to DMX control. This should be < 50 msec.

The `Render` line shows how many frames per second the system is currently generating.
This indicates how frequently the LED display can be updated. This should be no less than 30 fps and ideally > 60 fps.

If the system is unlocked, pressing `OK` will open the main menu, which allows you to select other screens described below.

If the system is locked, pressing `OK` will prompt you to enter the unlock code.

### DMX Dump

Displays DMX channel values. The first page shows the DMX channels currently in use by the system.
Pressing `LEFT` or `RIGHT` will cycle through all channels in the DMX universe.

A `*` character on the first line toggles on/off every time a packet of DMX data is received from the DMX receiver microcontroller.

### DMX Channel

Sets the first DMX channel used by the system. If the selected channel is C, the DMX footprint will be:

|Channel|Description|
|-------|-----------|
|C|blah|
|C+1|blah|

### LED Brightness

Sets the maximum brightness of the LEDs. The dimmest value is 5 and the brightest is 255.
This value can be increased or decreased in increments of 5.

### Mode

Sets the operating mode of the system.

|Mode|Description|
|-------|-----------|
|0|Normal mode. System is controlled by DMX.|
|1|Test mode. Displays various patterns to test the LED strands.|

### Lock Interface

Press `OK` to lock the system to prevent settings from being changed.
When locked, only the home screen will be available, the word `LOCK` will appear in the upper-right corner, and a code will be required to unlock the system.
