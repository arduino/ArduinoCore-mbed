/*
  ArduinoLogoDrawing

  created DD MMM YYYY
  by Leonardo Cavagnis
*/

#include "ArduinoGraphics.h"
#include "H7_Video.h" // H7_Video depends on ArduinoGraphics

H7_Video Display(480, 800, GIGA_DISPLAY_SHIELD);
//H7_Video Display(720, 480);

void setup() {
  Display.begin();
  
  Display.beginDraw();
  Display.background(255, 255, 255);
  Display.clear();
  Display.fill(0x008184);
  Display.circle(Display.height()/2, Display.width()/2, 300);
  Display.stroke(255, 255, 255);
  Display.noFill();
  for (int i=0; i<30; i++) {
    Display.circle((Display.height()/2)-55+5, Display.width()/2, 110-i);
    Display.circle((Display.height()/2)+55-5, Display.width()/2, 110-i);
  }
  Display.fill(255, 255, 255);
  Display.rect((Display.height()/2)-55-16+5, (Display.width()/2)-5, 32, 10);
  Display.fill(255, 255, 255);
  Display.rect((Display.height()/2)+55-16-5, (Display.width()/2)-5, 32, 10);
  Display.fill(255, 255, 255);
  Display.rect((Display.height()/2)+55-5-5, (Display.width()/2)-16, 10, 32);
  Display.endDraw();
}

void loop() { }