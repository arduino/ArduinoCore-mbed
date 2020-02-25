#include "Arduino.h"
#include "RPC_internal.h"

#define LED_BUILTIN PK_5

#ifdef CORE_CM7
int add(int a, int b) {
  return a * b;
}
#endif

void setup() {
  // put your setup code here, to run once:
  RPC1.begin();
  Serial.begin(115200);
  while (!Serial) {}
  //Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
#ifdef CORE_CM7
  RPC1.server.bind("add", add);
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
  auto res = RPC1.client.call("add", 12, 45).as<int>();;
  RPC1.println(res);
#else
  while (RPC1.available()) {
    Serial.write(RPC1.read());
  }
#endif
}
