#include "Nicla_System.h"

void setup() {
  nicla::begin();
  nicla::leds.begin();
}

void loop() {
  nicla::leds.setColor(green);
  delay(1000);
  nicla::leds.setColor(off);
  delay(1000);
}