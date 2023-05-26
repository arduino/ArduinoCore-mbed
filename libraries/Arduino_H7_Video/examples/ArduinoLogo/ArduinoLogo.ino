/*
  ArduinoLogo

  created 17 Apr 2023
  by Leonardo Cavagnis
*/

#include "Arduino_H7_Video.h"
#include "ArduinoGraphics.h"

#include "img_arduinologo.h"
// Alternatively, any raw RGB565 image can be included on demand using this macro
// Online image converter: https://lvgl.io/tools/imageconverter (Output format: Binary RGB565)
/*
#define INCBIN_PREFIX
#include "incbin.h"
INCBIN(test, "/home/user/Downloads/test.bin");
*/

Arduino_H7_Video Display(800, 480, GigaDisplayShield);
//Arduino_H7_Video Display(1024, 768, USBCVideo);

Image img_arduinologo(ENCODING_RGB16, (uint8_t *) texture_raw, 300, 300);

void setup() {
  Display.begin();

  Display.beginDraw();
  Display.image(img_arduinologo, (Display.width() - img_arduinologo.width())/2, (Display.height() - img_arduinologo.height())/2);
  Display.endDraw();
}

void loop() { }