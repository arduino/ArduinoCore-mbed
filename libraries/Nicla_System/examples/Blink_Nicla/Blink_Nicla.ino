/*
  Blink for Nicla Sense ME

  Turns green LED on for one second, then turn it off.

  Most Arduino boards have a single LED connected from D13 to ground
  (with a current limiting resistor). In the Nicla Sense ME, the common
  anode RGB LED (DL1) is controlled by a RGB LED Driver (U8). 
  The RGB LED Driver state is set via I2C by the ANNA-B112 Module (MD1).

 ┌────────────┐      ┌────────────┐
 │  ANNA-B112 │      │ RGB-Driver │                 VPMID
 │    MD-1    │      │     U8     │          Red     ─┬─
 │            │      │        OUT3├────────◄──────┐   │
 │            │ I2C  │            │          Green│   │
 │            ├──────┤        OUT2├────────◄──────┼───┘
 │            │      │            │          Blue │
 │            │      │        OUT1├────────◄──────┘
 │            │      │            │
 │            │      │            │
 │            │      │            │
 └────────────┘      └────────┬───┘
                              │
                              ▼

  All of this is abstracted via the Nicla_System.h header file. Details
  on use for controling RGB LED can be found at:
  https://docs.arduino.cc/tutorials/nicla-sense-me/cheat-sheet#rgb-led

  More advanced users can look at the source code at:
  https://github.com/arduino/ArduinoCore-mbed/blob/master/libraries/Nicla_System/src/Nicla_System.h 

  Authors: Giulia Cioffi, Martino Facchin & Ali Jahangiri
  
  This example code is in the public domain.

  Last edit: 2nd February 2023
*/

//This sketch is only for the Nicla Sense ME, not the Nicla Vision
#ifdef ARDUINO_NICLA_VISION
  #error "Run the standard Blink.ino sketch for the Nicla Vision"
#endif

// Intialise library which communicates with RGB driver
// Functions accessible under 'nicla' namespace
#include "Nicla_System.h"       


void setup() {
  //run this code once when Nicla Sense ME board turns on
  nicla::begin();               // initialise library
  nicla::leds.begin();          // Start I2C connection
}

void loop() {
  //run this code in a loop
  nicla::leds.setColor(green);  //turn green LED on
  delay(1000);                  //wait 1 second
  nicla::leds.setColor(off);    //turn all LEDs off
  delay(1000);                  //wait 1 second
}


