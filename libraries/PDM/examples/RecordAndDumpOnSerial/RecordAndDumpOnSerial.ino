#include "PDM.h"
#include "USBSerial.h"
#include "USBAudio.h"

// After defining USE_USB_AUDIO the board will behave as an USB sound card
// You will, however, lose the chance to retrigger the bootloader via CDC Serial.
// To program a new sketch, double click the RESET button and wait for the serial port to be enumerated again.
// If USE_USB_AUDIO is not defined the stream will be sent via CDC Serial

#ifdef USE_USB_AUDIO
USBAudio audio(true, 16000, 1, 16000, 1);
#endif

const int led = 41;
int led_status = HIGH;

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
  // At this frequency you have 15ms in the callback to use the returned buffer
  PDM.begin(1, 16000, 20);
  // The IRQ can call a naked function or one with buffer and size
  PDM.onReceive(send);
  PDM.onReceive(toggle);

  pinMode(led, OUTPUT);
}

void loop() {
  if (idx == 1) {
#ifdef USE_USB_AUDIO
    audio.write(buffer, DEFAULT_PDM_BUFFER_SIZE);
#else
    SerialUSB.send(buffer, DEFAULT_PDM_BUFFER_SIZE);
#endif
    idx = 0;
  }
}
