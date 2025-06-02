# LED Portal for Disney's _Descendants_ Musical

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;![IMG_0848](https://github.com/user-attachments/assets/0ce6afca-fe0a-4230-8061-792cdd0d43ad)

## Overview

This repo contains the microcontroller code for a system that drives ~2300 WS2812 ("[NeoPixel](https://learn.adafruit.com/adafruit-neopixel-uberguide/the-magic-of-neopixels)") LEDs
for a community theater production of [Disney's "Descendants" musical](https://www.mtishows.com/disneys-descendants-the-musical).

The LEDs are arranged in two identical concentric circles, positioned vertically with the two strips adjacent to each other, to form a "magical portal" set piece. The system can be controlled via
[DMX](https://en.wikipedia.org/wiki/DMX512), although the animations are generated procedurally by the device firmware.

The system includes a small [OLED display](https://www.amazon.com/dp/B08CDN5PSJ) and [4-button keypad](https://www.adafruit.com/product/1332) which is used to display status and set configuration values such as DMX channel range and maximum brightness.

Aside from the animations themselves, most of the system is designed to be a generic DMX-compatible driver for large NeoPixel installations.

Currently, DMX universe refresh latency is ~40 msec, and render loop frequency is ~80 fps.

## Goals

- 30-60+ frames/sec
- DMX latency < 50 msec
- As ridiculously bright as possible

## Build + Deploy

This project uses the Arduino CLI Sketch.yaml system. You may need to change COM ports in the yaml config.

On Windows, install the Arduino CLI via:

```cmd
winget install -e --id ArduinoSA.CLI
```

And then build the project and flash the MCU. You may need to double-click the Reset button to enter bootloader mode on the Feather M4 before uploading.

```cmd
arduino-cli build
arduino-cli upload
```

## LEDs + Power Budget

This was designed for 16 x 1 meter strips of [144 LEDs/meter strips of WS2812 LEDs](https://www.amazon.com/dp/B079ZRLMQR), so 2304 RGBW LEDs in total. Each pixel draws ~20 mA with a peak of 80 mA. Overall power draw at 5V should be 48-184A (230-920W), so this plan assumes that all pixels' RGBW channels will not be maxed out simultaneously.

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

## Physical Structure

The main structure of the portal is composed of two layers of 1/4" thick by 2" wide by 7 feet long [HDPE](https://en.wikipedia.org/wiki/High-density_polyethylene) black plastic, cut by our friends at [TAP Plastics](https://www.tapplastics.com/about/locations/detail/bellevue_wa). The 6 strips are bolted together in pairs, forming 3 two-layer sections, and the sections are bolted together with about 6 cm of overlap on the outer layer. At the bottom, the strips are secured with bolts to 2" hinges mounted on 2" x 6" boards to allow for a flexible angle.

My hope was that this plastic would be able to be bent without heat while still providing enough tension to naturally form a circle. This mostly worked, although the HDPE seemed to have a memory, and over time it lost its tendency to want to spring back. As a result, we had to redo the vertical tie lines that held it up, increasing the spacing of the eye bolts that anchored the tie lines to the ceiling grid.

Because HDPE is inert, I could not rely on adhesives. The HDPE strips were connected with #10 bolts and nylon lock nuts. Outer layer seams were reinforced by [steel mending plates](https://www.homedepot.com/p/Everbilt-4-Pack-2-in-Black-Mending-Plate-33766/327600351) to avoid the strips bowing outwards.

The LED strips came with 3M double-sided adhesive tape which adhered pretty well. But it was clear that the adhesion failed over time, especially as the strips were moved and bent. So to reliably secure the LED strips to the HDPE strips, I wrapped the entire ring with a clear stretchy monofilament, which worked very well.

## User Interface

### Home Screen

When the system has finished starting up, the home screen will be displayed.

```
[READY] - Mode 0 LOCK

 Bright   255 (100%)
 DMX Ch   374 - 381 *
 DMX Lag   42 ms     
 Render    83 fps
 Status    DMX ---
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

The `Status` line shows the currently-active animation or other rendering status.

If the system is unlocked, pressing `OK` will open the main menu, which allows you to select other screens described below.

If the system is locked, pressing `OK` will prompt you to enter the unlock code.

### Run Show

When the system is in manual mode, animations can be cued manually from the `RUN SHOW` screen.

On the `RUN SHOW` screen, the buttons behave as follows:

|Button|Description|
|------|-----------|
|BACK|If an animation is running, switch to the 'Close Portal' animation<br/>If no animation is running, hold to exit `RUN SHOW` mode.|
|LEFT|Auradon: start or switch-to animation|
|RIGHT|Isle: start or switch-to animation|
|OK|Finale: start or switch-to animation|

### DMX Dump

Displays DMX channel values. The first page shows the DMX channels currently in use by the system.
Pressing `LEFT` or `RIGHT` will cycle through all channels in the DMX universe.

A `*` character on the first line toggles on/off every time a packet of DMX data is received from the DMX receiver microcontroller.

### DMX Channel

Sets the first DMX channel used by the system. If the selected channel is C, the DMX footprint will be:

|Channel|Label|Notes|
|-------|-----------|------|
|C|Effect Trigger|[0-9] = Idle (Close Portal)<br/>[10-19] = Auradon<br/>[20-29] = Isle of the Lost<br/>[30-39] = Finale (rainbow)<br/>[250-255] = Manual RGB|
|C+1|Master Brightness|Intensity: 0-255|
|C+2|Rotation Speed|n * 10 pixels/sec (nominal: 29)|
|C+3|Flash Brightness|Intensity: 0-255|
|C+4|R|0-255|
|C+5|G|0-255|
|C+6|B|0-255|
|C+7|W|0-255|

### LED Brightness

Sets the maximum brightness of the LEDs. The dimmest value is 5 and the brightest is 255.
This value can be increased or decreased in increments of 5.

### Mode

Sets the operating mode of the system.

|Mode|Name|Description|
|-------|-----------|----|
|0|Idle|LEDs off|
|1|Test|Displays a test pattern|
|2|Manual Normal|Use 'Run Show' to manually trigger effects|
|3|Manual Sensory|Use 'Run Show' to manually trigger effects; output is limited to be sensory-friendly|
|4|DMX|System is controlled via DMX|

### Lock Interface

Press `OK` to lock the system to prevent settings from being changed.
When locked, only the home screen will be available, the word `LOCK` will appear in the upper-right corner, and a code will be required to unlock the system.
