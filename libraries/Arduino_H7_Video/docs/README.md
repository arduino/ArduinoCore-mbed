# Arduino H7 Video Library

[![License](https://img.shields.io/badge/License-LGPLv3-blue.svg)]()

The Arduino H7 Video library is a C++ library designed to handle the video output of Arduino boards based on the STM32H7 microcontroller with DSI video interface. DSI stands for Display Serial Interface, which is a serial interface used to connect a display to the microcontroller.

This library is based on the graphics primitives of the [ArduinoGraphics](https://github.com/arduino-libraries/ArduinoGraphics) library and currently supports the Arduino Portenta H7 and Arduino Giga R1 WiFi boards. The library offers two modes of operation for the Portenta H7 board: you can connect the display to the video output via a USB Type-C connection or use the Giga Display Shield. For the Giga R1 board, only the Giga Display Shield is supported.

The library allows you to draw graphics elements on the screen using simple graphics primitives such as lines, circles, images, etc. Additionally, you can integrate third-party graphic libraries like [LVGL](https://lvgl.io/) and [emWin](https://www.segger.com/products/user-interface/emwin/) to achieve more complex GUI.

The library provides methods for initializing the video controller, clearing the screen, and drawing basic graphics elements.

## Features

- Handles video output of Arduino boards based on the STM32H7 microcontroller with DSI video interface
- Allows drawing graphics elements using simple primitives like lines, circles, images, etc.
- Integration of third-party graphic libraries like LVGL and emWin for more complex GUI

## Usage

To use this library, you must have a supported Arduino board and a display. Once you have connected the display to the board, you can include the display library in your Arduino sketch and use its functions to draw graphic elements on the screen. 
Here is a minimal example for the Arduino GIGA R1 WiFi with Giga Display Shield:

```cpp
#include "Arduino_H7_Video.h"
#include "ArduinoGraphics.h"

Arduino_H7_Video Display(800, 480, GigaDisplayShield);

void setup() {
  Display.begin();
  
  // Draw a green rectangle that covers the entire display
  Display.beginDraw();
  Display.clear();
  Display.noStroke();
  Display.fill(0, 255, 0);
  Display.rect(0, 0, Display.width(), Display.height());
  Display.endDraw();
}

void loop() { }
```
## Examples

- **[ArduinoLogo](../examples/ArduinoLogo):** This example demonstrates how to display an Arduino logo image on the screen.
- **[ArduinoLogoDrawing](../examples/ArduinoLogoDrawing):** This example demonstrates how to draw an Arduino logo image using graphics primitives (line, circle, rect, etc.).
- **[LVGLDemo](../examples/LVGLDemo):** This example demonstrates how to create a graphical user interface (GUI) using the LVGL library. It includes the [Arduino_GigaDisplayTouch](https://github.com/arduino-libraries/Arduino_GigaDisplayTouch/) library to handle touch events.
- **[emWinDemo](../examples/emWinDemo):** This example demonstrates how to create a graphical user interface (GUI) using the SEGGER emWin library. It includes the [emWin-Arduino-Library](https://github.com/SEGGERMicro/emWin-Arduino-Library) library.

## Guides

To learn more about usage of this library, you can check out the following guides:
- [GIGA Display Shield LVGL Guide](https://docs.arduino.cc/tutorials/giga-display-shield/lvgl-guide).
- [GIGA Display Shield Image Orientation Guide](https://docs.arduino.cc/tutorials/giga-display-shield/image-orientation)
- [GIGA Display Shield Image Draw Guide](https://docs.arduino.cc/tutorials/giga-display-shield/basic-draw-and-image)


You can also check out the following guide available in the Segger Wiki:
- using the SEGGER emWin library on the GIGA Display Shield, check out the [SEGGER emWin on Arduino Wiki](https://wiki.segger.com/emWin_on_Arduino).

## API

The API documentation can be found [here](./api.md).

## License

This library is released under the [LGPLv3 license]().