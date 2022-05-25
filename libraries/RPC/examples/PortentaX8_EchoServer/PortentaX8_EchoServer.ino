#include "RPC.h"
#include "SerialRPC.h"

/*
 * This sketch demonstrates how to interact with the Portenta X8 Serial port (over USB)
 * On the board, launch both 'proxy' and 'example' binaries (from https://github.com/arduino/portentax8-m4-proxy)
 * The M4 provides the 'subtract' API (which will be invoked by 'example'
 * It also provides a full duplex Serial-like interface that is proxies through the serial monitor
 * Last but not leas, when you write 'echo' the corresponding function in 'example' will be triggered
 */

int subtract(int a, int b) {
  return a-b;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  RPC.bind("subtract", subtract);
  delay(1000);
}

int i = 0;
void loop() {

  //RPC.print("hello");
  //RPC.send("echo", "test");
  //auto res = RPC.call("add", 5, 8).as<int>();
  //RPC.send("echo", String(res).c_str());

  String str = "";
  while (Serial.available()) {
    str += (char)Serial.read();
  }
  if (str != "") {
    Serial.print(str);
  }
  if (str.startsWith("echo")) {
    delay(100);
    RPC.send("echo", "test");
  }
}
