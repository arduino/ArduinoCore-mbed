/*
  wiring_analog.cpp - analog input and output functions
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

#include "Arduino.h"
#include "pins_arduino.h"

static int write_resolution = 8;
static int read_resolution = 10;

#ifdef digitalPinToPwmObj
static mbed::PwmOut* PinNameToPwmObj(PinName P) {
  // reverse search for pinName in g_APinDescription[P].name fields
  for (pin_size_t i=0; i < PINS_COUNT; i++) {
    if (g_APinDescription[i].name == P) {
      return g_APinDescription[i].pwm;
    }
  }
  return NULL;
}
#endif

void analogWrite(PinName pin, int val)
{
  float percent = (float)val/(float)(1 << write_resolution);
#ifdef digitalPinToPwmObj
  mbed::PwmOut* pwm = PinNameToPwmObj(pin);
  if (pwm == NULL) {
    pwm = new mbed::PwmOut(pin);
    digitalPinToPwmObj(pin) = pwm;
    pwm->period_ms(2); //500Hz
  }
#else
  // attention: this leaks badly
  mbed::PwmOut* pwm = new mbed::PwmOut(digitalPinToPinName(pin));
#endif
  pwm->write(percent);
}

void analogWrite(pin_size_t pin, int val)
{
  float percent = (float)val/(float)(1 << write_resolution);
#ifdef digitalPinToPwmObj
  mbed::PwmOut* pwm = digitalPinToPwmObj(pin);
  if (pwm == NULL) {
    pwm = new mbed::PwmOut(digitalPinToPinName(pin));
    digitalPinToPwmObj(pin) = pwm;
    pwm->period_ms(2); //500Hz
  }
  pwm->write(percent);
#endif
}

void analogWriteResolution(int bits)
{
  write_resolution = bits;
}

int analogRead(PinName pin)
{
  int multiply_factor = 1;
#ifdef ANALOG_BUG_MBED
  multiply_factor = 4;
#endif
  return (mbed::AnalogIn(pin).read_u16() >> (16 - read_resolution)) * multiply_factor;
}

int analogRead(pin_size_t pin)
{
  int multiply_factor = 1;
#ifdef ANALOG_BUG_MBED
  multiply_factor = 4;
#endif
  return (mbed::AnalogIn(analogPinToPinName(pin)).read_u16() >> (16 - read_resolution)) * multiply_factor;
}

void analogReadResolution(int bits)
{
  read_resolution = bits;
}