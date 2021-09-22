/*
 Multiple Blinks

 Demonstrates the use of the Scheduler library for the boards:
 
 - Arduino Nano 33 BLE, or
 - Arduino Portenta H7, or
 - Arduino Nano RP2040 Connect

 Hardware required :
 * None (LEDs are already conencted to RGB LED)

 ATTENTION: LEDs polarity is reversed (so loop3 will turn the LED off by writing 1)

 created 8 Oct 2012
 by Cristian Maglie
 Modified by
 Scott Fitzgerald 19 Oct 2012

 This example code is in the public domain

 http://www.arduino.cc/en/Tutorial/MultipleBlinks
*/

// Include Scheduler since we want to manage multiple tasks.
#include <Scheduler.h>

// On Nano RP2040 Connect, RGB leds are connected to the wifi module
// The user APIs are the same, but we can't convert to int, so use defines
#if defined(ARDUINO_NANO_RP2040_CONNECT)

#include "WiFiNINA.h"
#define led1  LEDR
#define led2  LEDG
#define led3  LEDB

// On Nicla Sense ME, RGB leds are connected via an I2C module
// The user APIs are the same, but we can't convert to int, so use defines
#elif defined(ARDUINO_NICLA)

#include "Nicla_System.h"
#define led1  LEDR
#define led2  LEDG
#define led3  LEDB

#else

int led1 = LEDR;
int led2 = LEDG;
int led3 = LEDB;

#endif

void setup() {
  Serial.begin(9600);

  // Setup the 3 pins as OUTPUT
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);

  // Add "loop2" and "loop3" to scheduling.
  // "loop" is always started by default.
  Scheduler.startLoop(loop2);
  Scheduler.startLoop(loop3);
}

// Task no.1: blink LED with 1 second delay.
void loop() {
  digitalWrite(led1, HIGH);

  // IMPORTANT:
  // When multiple tasks are running 'delay' passes control to
  // other tasks while waiting and guarantees they get executed.
  delay(1000);

  digitalWrite(led1, LOW);
  delay(1000);
}

// Task no.2: blink LED with 0.1 second delay.
void loop2() {
  digitalWrite(led2, HIGH);
  delay(100);
  digitalWrite(led2, LOW);
  delay(100);
}

// Task no.3: accept commands from Serial port
// '0' turns off LED
// '1' turns on LED
void loop3() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == '0') {
      digitalWrite(led3, LOW);
      Serial.println("Led turned off!");
    }
    if (c == '1') {
      digitalWrite(led3, HIGH);
      Serial.println("Led turned on!");
    }
  }

  // IMPORTANT:
  // We must call 'yield' at a regular basis to pass
  // control to other tasks.
  yield();
}
