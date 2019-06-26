#include <rpclib.h>
//#include <msgpack.h>
//#include <stdio.h>
#include "rpc/dispatcher.h"
#include "client.h"
#include "Arduino.h"

volatile uint8_t* rpc_data;
volatile size_t rpc_len;

/*
  mbed::UARTSerial serial(SERIAL1_TX, SERIAL1_RX, 115200);

  namespace mbed {
  FileHandle *mbed_override_console(int fd) {
  return &serial;
  }

  FileHandle *mbed_target_override_console(int fd) {
  return &serial;
  }
  }
*/

int signal_rpc_available(void *data, size_t len) {
  rpc_data = (uint8_t*)data;
  rpc_len = len;
}

RPCLIB_MSGPACK::unpacker pac_;
RPCLIB_MSGPACK::sbuffer output_buf_;
rpc::detail::dispatcher srv;
rpc::myclient client;

rtos::Thread thread;

// Blink function toggles the led in a long running loop
void add_th() {
  while (1) {
    int a = random() % 50;
    int b = random() % 50;
    printf("call add on %d %d\n", a, b);
    //client.call("add", a, b);
    client.fake_call("add", a, b);
    wait(1.0f);
    client.fake_call("foo");
    wait(1.0f);
  }
}

int foo() {
  printf("foo called\n");
  return 44;
}

void setup(void)
{
  RPC1.begin();
  // Binding the name "foo" to free function foo.
  // note: the signature is automatically captured
  //srv.bind("foo", &foo);

  delay(1000);

  // Binding a lambda function to the name "add".
  srv.bind("add", [](int a, int b) {
    printf("add called\n");
    return a + b;
  });

  srv.bind("foo", foo);

  thread.start(add_th);

  pac_.reserve_buffer(1024);
}

#include "mbed_memory_status.h"

void loop() {

  // read data from RPC channel into pac_.buffer()
  if (rpc_len > 0) {

    print_heap_and_isr_stack_info();
    printf("\n");

    memcpy(pac_.buffer(), (const void*)rpc_data, rpc_len);

    for (int i = 0; i<rpc_len; i++) {
      printf("[%02x] ",  rpc_data[i]);
    }
    printf("\n");
    pac_.buffer_consumed(rpc_len);
    RPCLIB_MSGPACK::unpacked result;
    while (pac_.next(result)) {
      auto msg = result.get();
      auto resp = srv.dispatch(msg, true);
      auto data = resp.get_data();
      if (resp.is_empty()) {
        printf("no response\n");
      } else {
        printf("result: %d\n", resp.get_result()->as<int>());
      }
      // post resp.get_data() to RPC channel
      //RPC1.write(ENDPOINT_SERVICE, (const uint8_t*)resp.get_data().data(), resp.get_data().size());
    }
    rpc_len = 0;
  }
}
