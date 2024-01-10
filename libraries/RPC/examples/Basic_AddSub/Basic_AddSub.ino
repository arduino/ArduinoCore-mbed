#include "RPC.h"

int add(int a, int b) {
  return a + b;
}

void setup() {  
  RPC.begin();
  RPC.bind("add", add);
  pinMode(LEDG, OUTPUT);
}

void loop() {
  static size_t loop_count = 0;
  // Blink every 512 iterations
  if ((loop_count++ % 512) == 0) {
    digitalWrite(LEDG, LOW);
    delay(10);
    digitalWrite(LEDG, HIGH);
    delay(10);
  }
  int res = RPC.call("add", 1, 2).as<int>();
  RPC.call("sub", res, 1).as<int>();
  delay(250);
}
