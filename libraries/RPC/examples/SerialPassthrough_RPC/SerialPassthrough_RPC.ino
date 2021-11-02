#include "Arduino.h"
#include "RPC.h"

void setup() {
  Serial.begin(115200);
  RPC.begin();
}

void loop() {
  String data = "";
  while (RPC.available()) {
    data += (char)RPC.read();
  }
  if (data != "") {
    Serial.write(data.c_str(), data.length());
  }
  data = "";
  while (Serial.available()) {
    data += (char)Serial.read();
  }
  if (data != "") {
    RPC.write(data.c_str(), data.length());
  }
}
