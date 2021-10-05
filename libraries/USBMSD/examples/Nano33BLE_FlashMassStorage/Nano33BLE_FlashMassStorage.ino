/*
   This example exposes the second half (512KB) of Nano 33 BLE flash
   as a USB disk.
   The user can interact with this disk as a bidirectional communication with the board
   For example, the board could save data in a file to be retrieved later with a drag and drop
*/

#include "PluggableUSBMSD.h"

void setup() {
  Serial.begin(115200);
  MassStorage.begin();

  // write a file with some data
  // a+ means append, so every time the board is rebooted the file will grow by a new line
  FILE *f = fopen("/fs/data.txt", "a+");
  String hello = "Hello from Nano33BLE Filesystem\n";
  fwrite(hello.c_str(), hello.length(), 1, f);
  fclose(f);
}

void loop() {
  delay(1000);
}
