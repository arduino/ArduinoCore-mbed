/*
  KeyboardModifiers

  Select one word each second while extending the current text selection.
   
  The purpose of this demo is to demonstrate how to use the modifier keys.
  Some keys such as the arrow and function keys are mapped in a list so you 
  don't have to know the key codes. You can find them in the file USBKeyboard.h.
  
  For these keys you can use the function key_code(). 
  For other keys such as character keys you need to look up the key codes 
  and use the key_code_raw() function.
  
  Author: Sebastian Romero @sebromero
  
  This example code is in the public domain.
*/

#include "PluggableUSBHID.h"
#include "USBKeyboard.h"

USBKeyboard Keyboard;

void setup() {}

void loop() {  
  delay(1000);
  Keyboard.key_code(RIGHT_ARROW, KEY_SHIFT | KEY_ALT);

  delay(1000);
  // Alternatively you can use the raw key code for RIGHT_ARROW 0x4f
  Keyboard.key_code_raw(0x4f, KEY_SHIFT | KEY_ALT);
}
