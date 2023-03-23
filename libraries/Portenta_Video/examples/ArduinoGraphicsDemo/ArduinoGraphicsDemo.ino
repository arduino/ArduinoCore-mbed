#include "ArduinoGraphics.h"
#include "H7_Video.h"

#include "img_arduino.h"

H7_Video Display(480, 800);

void setup() {
  Display.begin();

  Image img_arduino(ENCODING_RGB16, (uint8_t *) texture_raw, 300, 300);
  Display.beginDraw();
  Display.image(img_arduino, (Display.width() - 300)/2, (Display.height() - 300)/2);
  Display.endDraw();
}

void loop() { }