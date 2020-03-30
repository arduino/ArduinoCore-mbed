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
#include "mbed/drivers/AnalogIn.h"

static int write_resolution = 8;
static int read_resolution = 10;

void analogWrite(PinName pin, int val)
{
  pin_size_t idx = PinNameToIndex(pin);
  if (idx != NOT_A_PIN) {
    analogWrite(idx, val);
  } else {
    mbed::PwmOut* pwm = new mbed::PwmOut(pin);
    pwm->period_ms(2); //500Hz
    float percent = (float)val/(float)(1 << write_resolution);
    pwm->write(percent);
  }
}

void analogWrite(pin_size_t pin, int val)
{
  if (pin >= PINS_COUNT) {
    return;
  }
  float percent = (float)val/(float)(1 << write_resolution);
  mbed::PwmOut* pwm = digitalPinToPwm(pin);
  if (pwm == NULL) {
    pwm = new mbed::PwmOut(digitalPinToPinName(pin));
    digitalPinToPwm(pin) = pwm;
    pwm->period_ms(2); //500Hz
  }
  pwm->write(percent);
}

void analogWriteResolution(int bits)
{
  write_resolution = bits;
}

int analogRead(PinName pin)
{
  return (mbed::AnalogIn(pin).read_u16() >> (16 - read_resolution));
}

int analogRead(pin_size_t pin)
{
  if (pin >= PINS_COUNT) {
    return -1;
  }
  PinName name = analogPinToPinName(pin);
  if (name == NC) {
    return -1;
  }
  mbed::AnalogIn* obj = analogPinToAnalogObj(pin);
  if (obj == NULL) {
    obj = new mbed::AnalogIn(name);
    analogPinToAnalogObj(pin) = obj;
  }
  return (obj->read_u16() >> (16 - read_resolution));
}

void analogReadResolution(int bits)
{
  read_resolution = bits;
}