/*
  wiring_digital.c - digital input and output functions
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2005-2006 David A. Mellis

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

#include "Arduino.h"
#include "pins_arduino.h"

void pinMode(uint8_t pin, PinMode mode)
{
  switch (mode) {
    case INPUT:
      mbed::DigitalIn((PinName)pin).mode(PullNone);
      break;
    case OUTPUT:
      mbed::DigitalOut((PinName)pin);
      break;
    case INPUT_PULLUP:
      mbed::DigitalIn((PinName)pin).mode(PullUp);
      break;
    case INPUT_PULLDOWN:
      mbed::DigitalIn((PinName)pin).mode(PullDown);
      break;
  }
}


void digitalWrite(uint8_t pin, PinStatus val)
{
	mbed::DigitalOut((PinName)pin).write((int)val);
}

PinStatus digitalRead(uint8_t pin)
{
	return (PinStatus)mbed::DigitalIn((PinName)pin).read();
}