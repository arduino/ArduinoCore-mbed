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

  Modified 28 September 2010 by Mark Sproul
*/

#include "Arduino.h"
#include "pins_arduino.h"

static gpio_t gpio[100];

void pinMode(uint8_t pin, PinMode mode)
{
  switch (mode) {
    case INPUT:
      gpio_init_inout(&gpio[pin], (PinName)pin, PIN_INPUT, PullNone, 0);
      break;
    case OUTPUT:
      gpio_init_inout(&gpio[pin], (PinName)pin, PIN_OUTPUT, PullNone, 0);
      break;
    case INPUT_PULLUP:
      gpio_init_inout(&gpio[pin], (PinName)pin, PIN_INPUT, PullUp, 0);
      break;
  }
}


void digitalWrite(uint8_t pin, PinStatus val)
{
	gpio_write(&gpio[pin], (int)val);
}

PinStatus digitalRead(uint8_t pin)
{
	return (PinStatus)gpio_read(&gpio[pin]);
}
