# Arduino Camera Library

[![License](https://img.shields.io/badge/License-LGPLv3-blue.svg)](https://github.com/arduino/ArduinoCore-mbed/blob/master/libraries/Camera/LICENSE)

The Arduino camera library is a C++ library designed to capture pixels from cameras on supported Arduino products. It is currently compatible with three camera models: 

- OmniVision OV7670 (On the [Arduino Giga R1](https://docs.arduino.cc/hardware/giga-r1-wifi))
- Himax HM01B0 & HM0360 (On the [Arduino Portenta Vision Shield](https://docs.arduino.cc/hardware/portenta-vision-shield))
- Galaxy Core GC2145 (On the [Arduino Nicla Vision](https://docs.arduino.cc/hardware/nicla-vision))

This library captures pixels and stores them in a frame buffer. The frames can then be retrieved continuously for processing.

The library provides methods to initialize the camera, capture frames, and access the pixels in the frame buffer. It supports various configuration options, such as setting the resolution and format of the captured frames. Please note that not all configurations are available for all cameras. The Himax camera for example only supports grayscale.

## Features

- Captures pixels and stores them in a frame buffer
- Store frame buffer on external RAM
- Frames can be retrieved continuously for processing
- Supports configuration options such as resolution and format
- Motion detection callback on supported camera (Himax HM0360)
- Simulated optical zoom on supported camera (GC2145)


## Usage

To use this library, you must have a supported Arduino board and camera module. Once you have connected the camera to the board, you can include the camera library in your Arduino sketch and use its functions to capture frames and access the pixels. Here is a minimal example for the Arduino Nicla Vision:

```cpp
#include "camera.h"
#include "gc2145.h"

GC2145 galaxyCore;
Camera cam(galaxyCore);
FrameBuffer fb;

void setup() {
  cam.begin(CAMERA_R320x240, CAMERA_RGB565, 30);
}

void loop() {  
  if (cam.grabFrame(fb, 3000) == 0) {
    // Do something with the frame, e.g. send it over serial 
    Serial.write(fb.getBuffer(), cam.frameSize());
  } 
}
```
## Examples

- **CameraRawBytes:** This example demonstrates how to capture raw bytes from the camera and display them on the computer by using a [Processing](https://processing.org/download) sketch. It uses UART to communicate with Processing and the `Camera` library to capture and retrieve the raw bytes.
The Processing sketch can be found [here](../extras/CameraRawBytesVisualizer/CameraRawBytesVisualizer.pde).
- **MotionDetection:** This example shows how to use the camera to detect motion in the captured frames on the camera. If motion is detected, a callback function is executed in an interrupt context.
- **GigaCamera:** This example demonstrates how to use the camera on the Arduino Giga R1 to capture images and display them on an attached LCD display that is driven by a ST7701 controller.

## API

The API documentation can be found [here](./api.md).

## License

This library is released under the [LGPLv3 license](https://github.com/arduino/ArduinoCore-mbed/blob/master/libraries/Camera/LICENSE).
