#include "Portenta_lvgl.h"
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
}

void loop() {
  // Replace texture_raw with testData if using the INCBIN method
  // Also, replace 300x300 resolution with the actual one

  Display.fillScreen(0x0000FF00);
  Display.drawImage((void*)texture_raw, 300, 300, 0, 0);
  Display.updateDisplay();

  delay(1000);

  Display.fillScreen(0x0000FF00);
  Display.drawImage((void*)texture_raw, 300, 300, 0, (Display.getHeightSize() - 300));
  Display.updateDisplay();

  delay(1000);

  Display.fillScreen(0x0000FF00);
  Display.drawImage((void*)texture_raw, 300, 300, (Display.getWidthSize() - 300), (Display.getHeightSize() - 300));
  Display.updateDisplay();

  delay(1000);

  Display.fillScreen(0x0000FF00);
  Display.drawImage((void*)texture_raw, 300, 300, (Display.getWidthSize() - 300), 0);
  Display.updateDisplay();

  delay(1000);
}
