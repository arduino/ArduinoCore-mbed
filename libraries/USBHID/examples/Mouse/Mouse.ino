#include "PluggableUSBHID.h"
#include "USBMouse.h"

USBMouse Mouse;

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);
  Mouse.move(100,100);
  delay(1000);
  Mouse.move(-100,-100);
}
