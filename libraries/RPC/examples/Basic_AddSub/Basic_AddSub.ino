#include "RPC.h"

int add(int a, int b) {
  return a + b;
}

int sub(int a, int b) {
  return a - b;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {

  }

  RPC.begin();
  RPC.bind("add", add);
  RPC.bind("sub", sub);
  if (RPC.cpu_id() == CM7_CPUID) {
    // Introduce a brief delay to allow the M4 sufficient time
    // to bind remote functions before invoking them.
    delay(100);
  }
  pinMode(LEDG, OUTPUT);
}

void loop() {
  static size_t loop_count = 0;

  // Blink every 512 iterations
  if (RPC.cpu_id() == CM4_CPUID && (loop_count++ % 512) == 0) {
    digitalWrite(LEDG, LOW);
    delay(10);
    digitalWrite(LEDG, HIGH);
    delay(10);
  }

  int res = RPC.call("add", 1, 2).as<int>();
  if (RPC.cpu_id() == CM7_CPUID) {
    Serial.println("add(1, 2) = " + String(res));
  }

  res = RPC.call("sub", res, 1).as<int>();
  if (RPC.cpu_id() == CM7_CPUID) {
    Serial.println("sub(3, 1) = " + String(res));
  }
  delay(250);
}
