// Use this file to produce an archive in {build_dir}/libraries/openamp/openamp.a, then move it to variants/ENVIE_M4/libs/libopenamp.a

#include "arduino_openamp.h"
#include "RPC.h"
#include <iostream>
#include "rpc/server.h"

void setup() {
  RPC1.begin();
  Serial.begin(115200);

  rpc::server srv(8080);

  // Binding the name "foo" to free function foo.
  // note: the signature is automatically captured
  srv.bind("foo", &foo);

  // Binding a lambda function to the name "add".
  srv.bind("add", [](int a, int b) {
    return a + b;
  });

  // Run the server loop.
  srv.run();

}

void loop() {
  if (RPC1.available() > 10) {
    Serial.print("I have new stuff ");
    Serial.println(RPC1.available());
    while (RPC1.available()) {
      Serial.write(RPC1.read());
    }
  }
}
