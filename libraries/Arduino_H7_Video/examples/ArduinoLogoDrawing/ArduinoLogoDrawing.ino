/*
  ArduinoLogoDrawing

  created 17 Apr 2023
  by Leonardo Cavagnis
*/

#include "Arduino_H7_Video.h"
#include "ArduinoGraphics.h"

Arduino_H7_Video Display(800, 480, GigaDisplayShield);
//Arduino_H7_Video Display(1024, 768, USBCVideo);

void setup() {
  Display.begin();
  
  Display.beginDraw();
  Display.background(255, 255, 255);
  Display.clear();
  Display.fill(0x008184);
  Display.circle(Display.width()/2, Display.height()/2, 300);
  Display.stroke(255, 255, 255);
  Display.noFill();
  for (int i=0; i<30; i++) {
    Display.circle((Display.width()/2)-55+5, Display.height()/2, 110-i);
    Display.circle((Display.width()/2)+55-5, Display.height()/2, 110-i);
  }
  Display.fill(255, 255, 255);
  Display.rect((Display.width()/2)-55-16+5, (Display.height()/2)-5, 32, 10);
  Display.fill(255, 255, 255);
  Display.rect((Display.width()/2)+55-16-5, (Display.height()/2)-5, 32, 10);
  Display.fill(255, 255, 255);
  Display.rect((Display.width()/2)+55-5-5, (Display.height()/2)-16, 10, 32);
  Display.endDraw();
}

void loop() { }