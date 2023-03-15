/*
  This example shows how to enable/disable bootloader debug
  using MCUboot library. The used debug output is Serial1 

  Circuit:
  - Arduino Portenta H7 board

  This example code is in the public domain.
*/

#include <MCUboot.h>

// the setup function runs once when you press reset or power the board
void setup() {
  // set RTC register DR7
  MCUboot::bootDebug(true);
}

// the loop function runs over and over again forever
void loop() {


}
