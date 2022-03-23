/*
  This example shows how to confirm an update Sketch after a swap
  using MCUboot library.

  Circuit:
  - Arduino Portenta H7 board

  This example code is in the public domain.
*/

#include <MCUboot.h>

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  // set confirmed flag to avoid MCUboot reverts to previous application at next reset
  MCUboot::confirmSketch();
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(100);                        // wait 100ms
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(100);                        // wait 100ms
}
