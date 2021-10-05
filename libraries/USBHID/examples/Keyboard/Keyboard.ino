#include "PluggableUSBHID.h"
#include "USBKeyboard.h"

USBKeyboard Keyboard;

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);
  Keyboard.printf("Hello world\n\r");
}
