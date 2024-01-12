#include "SerialRPC.h"
#include "RPC.h"

/*
   This sketch demonstrates how to interact with the Portenta X8 Serial port (over USB)
   On the board, launch both 'proxy' and 'example' binaries (from https://github.com/arduino/portentax8-m4-proxy)
   The M4 provides the 'subtract' API (which will be invoked by 'example'
   It also provides a full duplex Serial-like interface that is proxies through the serial monitor
   Last but not leas, when you write 'echo' the corresponding function in 'example' will be triggered
*/

int subtract(int a, int b) {
  return a - b;
}

int led_status = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  RPC.bind("subtract", subtract);
  delay(1000);
}

int i = 0;
void loop() {

  if (millis() % 1000 == 0) {
    Serial.println("loop");
    delay(2);
  }

  String str = "";
  while (Serial.available()) {
    str += (char)Serial.read();
  }
  if (str != "") {
    //Serial.print(str);
  }

  if (str.startsWith("whoami")) {
    digitalWrite(LED_BUILTIN, HIGH);
    auto res = RPC.call("whoami").as<std::string>();
    Serial.println(res.c_str());
    digitalWrite(LED_BUILTIN, LOW);
  }

  if (str.startsWith("divide")) {
    float a = random() % 15000;
    float b = random() % 15000;
    Serial.println(String(a) + " / " + String(b));
    auto res = RPC.call("divide", a, b).as<float>();
    Serial.println(String(a) + " / " + String(b) + " = " + String(res));
  }

  if (str.startsWith("add")) {
    int a = random() % 15000;
    int b = random() % 15000;
    Serial.println(String(a) + " + " + String(b));
    auto res = RPC.call("add", a, b).as<int>();
    Serial.println(String(a) + " + " + String(b) + " = " + String(res));
  }

  if (str.startsWith("echo")) {
    auto res = RPC.call("echo", "X8").as<std::string>();
    Serial.println(res.c_str());
  }
}