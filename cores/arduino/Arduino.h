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

#if ARDUINO_LIBRARY_DISCOVERY_PHASE == 1
#define ARDUINO_LIB_DISCOVERY_PHASE
#endif
#if !defined(Arduino_h) && !defined(ARDUINO_LIB_DISCOVERY_PHASE)
#define Arduino_h

#include "pinmode_arduino.h"
#include "api/ArduinoAPI.h"

#if defined(__cplusplus)

#undef F
// C++11 F replacement declaration
template <typename T1>
auto F(T1&& A)
  -> const arduino::__FlashStringHelper*
{
  return (const arduino::__FlashStringHelper*)A;
}

#if !defined(ARDUINO_AS_MBED_LIBRARY)
using namespace arduino;
#endif
extern "C"{
#endif

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

#define interrupts()        __enable_irq()
#define noInterrupts()      __disable_irq()

// We provide analogReadResolution and analogWriteResolution APIs
void analogReadResolution(int bits);
void analogWriteResolution(int bits);

#ifdef __cplusplus
} // extern "C"
#endif

#include "pins_arduino.h"

#ifdef __cplusplus
// Types used for the table below
typedef struct _PinDescription PinDescription;
typedef struct _AnalogPinDescription AnalogPinDescription;

// Pins table to be instantiated into variant.cpp
extern PinDescription g_APinDescription[];
extern AnalogPinDescription g_AAnalogPinDescription[];

#ifdef ANALOG_CONFIG
#include "hal/analogin_api.h"
typedef enum _AnalogReferenceMode AnalogReferenceMode;
void analogReference(uint8_t mode);
/* nRF specific function to change analog acquisition time */
typedef enum _AnalogAcquisitionTime AnalogAcquisitionTime;
void analogAcquisitionTime(uint8_t time);

/* Function to reconfigure already active ADC channels */
void analogUpdate();
extern bool isAdcConfigChanged;
extern analogin_config_t adcCurrentConfig;

#endif

#include "Serial.h"
#if defined(SERIAL_CDC)
#define Serial _UART_USB_
#define SerialUSB _UART_USB_
#else
#define Serial _UART1_
#endif
#define Serial1 _UART1_
#define Serial2 _UART2_
#define Serial3 _UART3_
#define Serial4 _UART4_

#include "overloads.h"
#endif

#include "macros.h"

#endif
