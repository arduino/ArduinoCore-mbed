#include "Portenta_Video.h"
#include "lvgl.h"

Portenta_Video Display;

void setup() {
  Display.begin();

  Display.fillScreen(PV_COLOR_RED);
  Display.update();
}

void loop() { 
}