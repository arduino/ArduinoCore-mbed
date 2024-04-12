#include "Arduino.h"
#include "RPC.h"

void setup() {
  Serial.begin(115200);
  while (!Serial) {
  }
  RPC.begin();
}

void loop() {
  if (RPC.cpu_id() == CM4_CPUID) {
    RPC.println("Printed from M4 core");
    delay(1000);
  } else {
    while (RPC.available()) {
      Serial.print((char) RPC.read());
    }
  }
}
