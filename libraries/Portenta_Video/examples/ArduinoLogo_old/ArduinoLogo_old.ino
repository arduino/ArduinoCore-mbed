#include "Portenta_Video.h"
#include "image.h"

// Alternatively, any raw RGB565 image can be included on demand using this macro
// Online image converter: https://lvgl.io/tools/imageconverter (Output format: Binary RGB565)
/*
#define INCBIN_PREFIX
#include "incbin.h"
INCBIN(test, "/home/user/Downloads/test.bin");
*/

Portenta_Video Display;

void setup() {
  Display.begin();

  // Replace texture_raw with testData if using the INCBIN method
  // Also, replace 300x300 resolution with the actual one
  Display.drawImage((Display.getWidth() - 300)/2, (Display.getHeight() - 300)/2, (void*)texture_raw, 300, 300);
  Display.update();
}

void loop() { }