/**************************************************************
** Brief introduction to a few HID Keyboard commands.        **
** Unlike the other Keyboard library, this one doesn't use   **
** the "press", "release", and "send" commands.              **
**************************************************************/

#include "PluggableUSBHID.h"
#include "USBKeyboard.h"

USBKeyboard Keyboard;

// Arbitrary pin.
const int CONTROL_PIN = 2;

void setup() {
  // We will use this to start/end the prints.
  pinMode(CONTROL_PIN, INPUT_PULLUP);
}

void loop() {
  // This will run only if the control pin is connected to ground.
  if( digitalRead(CONTROL_PIN) == LOW){
    delay(1000);
    Keyboard.printf("Hello world\n\r");
  }
}
