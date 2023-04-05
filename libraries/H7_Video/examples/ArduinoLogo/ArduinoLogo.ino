/*
  ArduinoLogo

  created DD MMM YYYY
  by Leonardo Cavagnis
*/

#include "ArduinoGraphics.h"
#include "H7_Video.h" // H7_Video depends on ArduinoGraphics

#include "img_arduinologo.h"
// Alternatively, any raw RGB565 image can be included on demand using this macro
// Online image converter: https://lvgl.io/tools/imageconverter (Output format: Binary RGB565)
/*
#define INCBIN_PREFIX
#include "incbin.h"
INCBIN(test, "/home/user/Downloads/test.bin");
*/

H7_Video Display(480, 800, GIGA_DISPLAY_SHIELD);
//H7_Video Display(720, 480);

void setup() {
  Display.begin();

  Image img_arduinologo(ENCODING_RGB16, (uint8_t *) texture_raw, 300, 300);
  Display.beginDraw();
  Display.image(img_arduinologo, (Display.height() - img_arduinologo.width())/2, (Display.width() - img_arduinologo.height())/2);
  Display.endDraw();
}

void loop() { }