# 'develop' branch
forked from clashman/it8951

This branch is to compile it8951 arduino code on esp-idf environment.
modified by yooni.kim71@gmail.com

## Usage
 1. git clone --recursive https://github.com/andantesys/IT8951-for-esp32-with-arduino-code.git
 2. cd IT891-for-esp32-with-arduino-code
 3. git checkout develop
 4. make menuconfig
 5. make flash monitor

# Quick hack to port the IT8951 display driver to Arduino
These instructions are as simple as possible to allow for easy reproduction. I don't endorse using the Arduino IDE for anything serious. The Espressif/IDF environment provides a way better experience.

The pins need 3.3V! The original Arduino only has 5V pins. Don't use that board without voltage dividers.

Original code taken from: https://github.com/waveshare/IT8951/

# Usage
1. Get a microcontroller and a Waveshare display with IT8951 driver board
    * i.e. an ESP32: https://www.dfrobot.com/product-1590.html (find cheaper and faster shipping ESP32s on ebay etc.)
    * i.e. the 6" version: https://www.waveshare.com/6inch-e-paper-hat.htm
2. Wire your Arduino-compatible board with the following pins
    * Power -> 5V (USB)
    * Ground -> Ground
    * MISO (blue)   19
    * MOSI (yellow) 18
    * SCK (orange)   5
    * CS (green)    12
    * RESET (white) 16
    * HRDY (purple) 17
3. Download Arduino IDE
    * install IDE support for ESP32 ([Tutorial](https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/))
4. Flash this repo's code to the board
5. Enjoy your success

# Wiring
![Wiring](/wiring.jpg)

# Result
![Result](/result.jpg)

# Example Picture
The example picture that will show up on the epaper is [Obernberg am Inn Adlerwarte: Rotmilan](https://commons.wikimedia.org/wiki/File:Obernberg_am_Inn_Adlerwarte_Rotmilan-0136.jpg)
([CC-BY-SA-4.0](https://creativecommons.org/licenses/by-sa/4.0/)).
It was resized and encoded using 4 bit per pixel, little-endian-like.

To reproduce:
* ./convert_image.py 400 400 pic.jpg
* xxd -i pic > pic.ino
* manually insert width and height variable into pic.ino (unsigned int pic_width = 400; unsigned int pic_height = 400;)

The ESP's RAM is limited, so if you want full resolution images, you need to stream them to the device via WiFi or load them from flash storage.
