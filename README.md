# LED Portal for Disney's _Descendants_ Musical

This repo contains the microcontroller code for a system that drives ~2000 WS2812 ("NeoPixel") LEDs
for a community theater production of [Disney's "Descendants" musical](https://www.mtishows.com/disneys-descendants-the-musical).

The LEDs are arranged in two adjacent vertical circles to form a "magical portal" set piece. The system can be controlled via
DMX, although the animations are generated procedurally by the device firmware.

The system includes a small OLED display and 4-button keypad which is used to display status and set
configuration values such as DMX channel range and maximum brightness.

The code was written for the Adafruit Feather M4 Express (SAMD51) board.

Keypad input is handled by an Adafruit TCA8418, to reduce the number of GPIO pins used on the M4.

NeoPixel data output is managed by an Adafruit NeoPXL8 Friend, which drives 8 LED strips concurrently
and uses DMA to avoid blocking the M4's CPU.

The DMX interface uses a SparkFun ESP32 Thing Plus DMX to LED Shield, which provides RS485 decoding
as well as the electrical isolation that the DMX standard recommends. Unfortunately, this shield did
not seem to work well with the Feather M4, so I added an ESP MCU to the project, which connects to the
DMX shield and then forwards the DMX data to the Feather M4 via an i2c link.
