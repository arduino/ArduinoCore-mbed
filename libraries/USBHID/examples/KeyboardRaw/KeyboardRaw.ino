#include "PluggableUSBHID.h"
#include "USBKeyboard.h"

USBKeyboard Keyboard;

void setup() {
  // put your setup code here, to run once:
}

// raw keycodes can be imported from here https://gist.github.com/MightyPork/6da26e382a7ad91b5496ee55fdc73db2#file-usb_hid_keys-h
#define KEY_ESC     0x29 // Keyboard ESCAPE
#define KEY_I       0x0c // Keyboard i and I

void loop() {
  // in vim, jump between INSERT and COMMAND mode
  delay(1000);
  Keyboard.key_code_raw(KEY_ESC);
  delay(1000);
  Keyboard.key_code_raw(KEY_I);
}
