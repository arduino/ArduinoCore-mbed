// write() is blocking, so you need to open the midi interface on the "host" side
// on Linux, follow https://github.com/arduino-libraries/MIDIUSB#test-procedure-linux

#include "PluggableUSBMIDI.h"

USBMIDI midi;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  midi.write(MIDIMessage::NoteOn(6));
  delay(1000);
  midi.write(MIDIMessage::NoteOff(6));
  delay(1000);
}
