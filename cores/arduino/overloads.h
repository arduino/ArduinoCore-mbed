/*
  overloads.h - C++ overloads to handle both Arduino and mbed pin numbering methods
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2018-2019 Arduino SA

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA
*/

void pinMode(PinName pinNumber, PinMode pinMode);
void digitalWrite(PinName pinNumber, PinStatus status);
PinStatus digitalRead(PinName pinNumber);
int analogRead(PinName pinNumber);
void analogWrite(PinName pinNumber, int value);

unsigned long pulseIn(PinName pin, uint8_t state, unsigned long timeout);
unsigned long pulseInLong(PinName pin, uint8_t state, unsigned long timeout);

void shiftOut(PinName dataPin, PinName clockPin, BitOrder bitOrder, uint8_t val);
uint8_t shiftIn(PinName dataPin, PinName clockPin, BitOrder bitOrder);

void attachInterrupt(PinName interruptNumber, voidFuncPtr callback, PinStatus mode);
void attachInterruptParam(PinName interruptNumber, voidFuncPtrParam callback, PinStatus mode, void* param);
void detachInterrupt(PinName interruptNumber);