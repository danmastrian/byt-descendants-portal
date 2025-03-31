# LED Portal for Disney's _Descendants_ Musical

This repo contains the microcontroller code for a system that drives ~2000 WS2812 ("NeoPixel") LEDs
for a community theater production of [Disney's "Descendants" musical](https://www.mtishows.com/disneys-descendants-the-musical).

The LEDs are arranged in two identical concentric circles, positioned vertically with the two half-strips adjacent to each other, to form a "magical portal" set piece. The system can be controlled via
[DMX](https://en.wikipedia.org/wiki/DMX512), although the animations are generated procedurally by the device firmware.

The system includes a small [OLED display](https://www.amazon.com/dp/B08CDN5PSJ) and [4-button keypad](https://www.adafruit.com/product/1332) which is used to display status and set configuration values such as DMX channel range and maximum brightness.

The code was written for the [Adafruit Feather M4 Express (SAMD51)](https://www.adafruit.com/product/3857) board.

Keypad input is handled by an [Adafruit TCA8418](https://www.adafruit.com/product/4918), to reduce the number of GPIO pins used on the M4,
because specific pins are needed for the 8 LED data outputs.

NeoPixel data output is managed by an [Adafruit NeoPXL8 Friend](https://www.adafruit.com/product/3975), which drives 8 LED strips concurrently
and uses DMA to avoid blocking the M4's CPU.

The DMX interface uses a [SparkFun ESP32 Thing Plus DMX to LED Shield](https://www.sparkfun.com/sparkfun-esp32-thing-plus-dmx-to-led-shield.html),
which provides RS485 decoding as well as the electrical isolation that the DMX standard recommends. Unfortunately, this shield's library did not want
to compile for the Feather M4, so I added an [Adafruit ESP32](https://www.adafruit.com/product/3405) to the project, which connects to the
DMX shield and then forwards the DMX data to the Feather M4 via a secondary [i<sup>2</sup>c](https://en.wikipedia.org/wiki/I%C2%B2C) bus.
See https://github.com/danmastrian/byt-descendants-portal-dmx-peripheral for the code for the auxiliary MCU.
I did not use the main i<sup>2</sup>c bus because I had problems getting the M4 to work as both a controller (of the OLED and GPIO/keypad extender)
and peripheral (DMX data receiver from the ESP board) when using the default i<sup>2</sup>c bus/pins.
I had to add external pullup resistors to this bus, and initially I used 4.7Kohm resistors.
I saw a high rate of i2c packet truncation, and after some research I decided to try smaller resistors.
As I reduced the resistance, the rate of packet loss dropped. (TBD: 680ohm?)

Aside from the animations themselves, most of the system is designed to be a generic DMX-compatible driver for large NeoPixel installations.

Currently, DMX universe refresh latency is ~40 msec, and render loop frequency is ~80 fps.
