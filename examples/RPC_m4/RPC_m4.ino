#include "Arduino.h"
#include "RPC_internal.h"

#define LED_BUILTIN PK_5

void setup() {
  // put your setup code here, to run once:
  RPC1.begin();
  //Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
  RPC1.println("test");
  //auto res = RPC1.client.call("add", 12, 45).as<int>();;
  //RPC1.println(res);
}
