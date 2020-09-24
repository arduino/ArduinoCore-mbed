#include "Arduino.h"
#include "RPC_internal.h"

void setup() {
  Serial.begin(115200);
  RPC1.begin();
}

void loop() {
  String data = "";
  while (RPC1.available()) {
    data += (char)RPC1.read();
  }
  if (data != "") {
    Serial.write(data.c_str(), data.length());
  }
  data = "";
  while (Serial.available()) {
    data += (char)Serial.read();
  }
  if (data != "") {
    RPC1.write(data.c_str(), data.length());
  }
}
