#include "PDM.h"

//mbed::DigitalOut led((PinName)40);

// Temporary patch to get unbuffered writes
mbed::UARTSerial serial(SERIAL1_TX, SERIAL1_RX, 1000000);

const int led = 40;
int led_status = HIGH;

#if 0
/*
   This snippet allows to redirect stdout/stderr on a Stream at your choice
   Attention: it must be in mbed namespace to override the weak core definition
*/
namespace mbed {
FileHandle *mbed_override_console(int fd) {
  return &serial;
}

FileHandle *mbed_target_override_console(int fd) {
  return &serial;
}
}
#endif

uint8_t buffer[1024];
volatile int idx = 0;

void toggle() {
  if (led_status == HIGH) {
    led_status = LOW;
  } else {
    led_status = HIGH;
  }
  digitalWrite(led, led_status);
}

void send(void* buf, size_t size) {
  memcpy(buffer, buf, size);
  idx = 1;
}

void setup() {
  // Start the PDM as MONO @ 16KHz : gain @20
  // At this frequency you have 15ms in the callcack to use the returned buffer
  PDM.begin(1, 16000, 20);
  // The IRQ can call a naked function or one with buffer and size
  PDM.onReceive(send);
  PDM.onReceive(toggle);

  pinMode(led, OUTPUT);
}

void loop() {
  if (idx == 1) {
    serial.write(buffer, DEFAULT_PDM_BUFFER_SIZE);
    idx = 0;
  }
}
