/*
  ArduinoLogoDrawing

  created DD MMM YYYY
  by Leonardo Cavagnis
*/

#include "ArduinoGraphics.h"
#include "H7_Video.h" // H7_Video depends on ArduinoGraphics

H7_Video Display(480, 800);

void setup() {
  Display.begin();
  
  Display.beginDraw();
  Display.background(255, 255, 255);
  Display.clear();
  Display.endDraw();
}

void loop() { }