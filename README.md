# LED Portal for Disney's _Descendants_ Musical

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;![IMG_0848](https://github.com/user-attachments/assets/0ce6afca-fe0a-4230-8061-792cdd0d43ad) ![Screenshot 2025-06-01 192745](https://github.com/user-attachments/assets/24c56cc4-0dbe-4a2b-95bd-d9916ad9d7aa)

## Overview

This repo contains the microcontroller code for a system that drives ~2300 WS2812 ("[NeoPixel](https://learn.adafruit.com/adafruit-neopixel-uberguide/the-magic-of-neopixels)") LEDs
for a community theater production of [Disney's "Descendants" musical](https://www.mtishows.com/disneys-descendants-the-musical).

The LEDs are arranged in two identical concentric circles, positioned vertically with the two strips adjacent to each other, to form a "magical portal" set piece. The system can be controlled via
[DMX](https://en.wikipedia.org/wiki/DMX512), although the animations are generated procedurally by the device firmware.

The system includes a small [OLED display](https://www.amazon.com/dp/B08CDN5PSJ) and [4-button keypad](https://www.adafruit.com/product/1332) which is used to display status and set configuration values such as DMX channel range and maximum brightness.

Aside from the animations themselves, most of the system is designed to be a generic DMX-compatible driver for large NeoPixel installations.

Refer to the [wiki](https://github.com/danmastrian/byt-descendants-portal/wiki) for more details.

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
