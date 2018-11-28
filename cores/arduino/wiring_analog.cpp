/*
  wiring_analog.c - analog input and output functions
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2018 Arduino SA

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

static int resolution = 10;

void analogWrite(pin_size_t pin, int val)
{
  float percent = (float)val/(float)(1 << resolution);
	mbed::PwmOut((PinName)pin).write(percent);
}

int analogRead(pin_size_t pin)
{
	return mbed::AnalogIn((PinName)pin).read_u16();
}