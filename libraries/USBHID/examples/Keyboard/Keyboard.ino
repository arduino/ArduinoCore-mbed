#include "PluggableUSBHID.h"
#include "USBKeyboard.h"

USBKeyboard Keyboard;

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // Every 30 seconds, write `Hello world\n\r` to the connected computer as though the arduino were a keyboard.
  // WARNING: Decreasing this delay could render the arduino useless, as the frequent newlines will make uploading new code 
  //          nearly impossible. See [this issue](https://github.com/arduino/ArduinoCore-mbed/issues/424) for details
  delay(30000);
  Keyboard.printf("Hello world\n\r");
}
