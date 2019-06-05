/*
  Arduino.h - Main include file for the Arduino SDK
  Copyright (c) 2005-2013 Arduino Team.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef Arduino_h
#define Arduino_h

#if defined(__cplusplus)
#if !defined(ARDUINO_AS_MBED_LIBRARY)
#define PinMode MbedPinMode
#endif
#include "mbed.h"
#undef PinMode
#endif

#if defined(ARDUINO_AS_MBED_LIBRARY)
#define PinMode ArduinoPinMode
#endif

#include "api/ArduinoAPI.h"

#if defined(ARDUINO_AS_MBED_LIBRARY)
#undef PinMode
#endif

#if defined(__cplusplus)
#if !defined(ARDUINO_AS_MBED_LIBRARY)
using namespace arduino;
#endif
extern "C"{
#endif

// avr-libc defines _NOP() since 1.6.2
#ifndef _NOP
#define _NOP() do { __asm__ volatile ("nop"); } while (0)
#endif

// Get the bit location within the hardware port of the given virtual pin.
// This comes from the pins_*.c file for the active board configuration.
//
// These perform slightly better as macros compared to inline functions
//
#define NOT_A_PIN 255
#define NOT_A_PORT 255
#define NOT_AN_INTERRUPT 255

// undefine stdlib's abs if encountered
#ifdef abs
#undef abs
#endif // abs
#define abs(x) ((x)>0?(x):-(x))

#ifdef __cplusplus
} // extern "C"
#endif

#include "pins_arduino.h"

#include "Serial.h"
#if defined(SERIAL_CDC)
#define Serial SerialUSB
#define Serial1 UART1
#define Serial2 UART2
#define Serial3 UART3
#else
#define Serial UART1
#define Serial1 UART2
#define Serial2 UART3

#endif

#endif
