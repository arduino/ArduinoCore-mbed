#include "Arduino.h"
#include "RPC_internal.h"

#ifdef CORE_CM7
int add(int a, int b) {
  printf("calling add on M7\n");
  delay(1000);
  return a + b;
}
int subtract(int a, int b) {
  printf("calling subtract on M7\n");
  return a - b;
}
#endif

rtos::Thread t;

void call_substract() {
  while (1) {
    delay(700);
    RPC1.print("Calling subtract ");
    auto res = RPC1.call("sub", 12, 45).as<int>();;
    RPC1.println(res);
  }
}

void setup() {
  // put your setup code here, to run once:
  RPC1.begin();
  Serial.begin(115200);
  //while (!Serial) {}
  //Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
#ifdef CORE_CM7
  RPC1.bind("add", add);
  RPC1.bind("sub", subtract);
#else
  t.start(call_substract);
#endif
}

void loop() {
  // put your main code here, to run repeatedly:
#ifndef CORE_CM7
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);

  RPC1.print("Calling add ");
  auto res = RPC1.call("add", 12, 45).as<int>();;
  RPC1.println(res);
#else
  while (RPC1.available()) {
    Serial.write(RPC1.read());
  }
#endif
}
