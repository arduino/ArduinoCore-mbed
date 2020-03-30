/*
  wiring_digital.cpp - digital input and output functions
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
#include "mbed.h"

void pinMode(PinName pin, PinMode mode)
{
  pin_size_t idx = PinNameToIndex(pin);
  if (idx != NOT_A_PIN) {
    pinMode(idx, mode);
  } else {
    switch (mode) {
      case INPUT:
        mbed::DigitalIn(pin, PullNone);
        break;
      case OUTPUT:
        mbed::DigitalOut(pin, 0);
        break;
      case INPUT_PULLUP:
        mbed::DigitalIn(pin, PullUp);
        break;
      case INPUT_PULLDOWN:
        mbed::DigitalIn(pin, PullDown);
        break;
    }
  }
}

void pinMode(pin_size_t pin, PinMode mode)
{
  gpio_t* gpio = digitalPinToGpio(pin);
  if (gpio == NULL) {
    gpio = new gpio_t();
    digitalPinToGpio(pin) = gpio;
  }

  switch (mode) {
    case INPUT:
      gpio_init_in_ex(gpio, digitalPinToPinName(pin), PullNone);
      break;
    case OUTPUT:
      gpio_init_out(gpio, digitalPinToPinName(pin));
      break;
    case INPUT_PULLUP:
      gpio_init_in_ex(gpio, digitalPinToPinName(pin), PullUp);
      break;
    case INPUT_PULLDOWN:
      gpio_init_in_ex(gpio, digitalPinToPinName(pin), PullDown);
      break;
  }
}

void digitalWrite(PinName pin, PinStatus val)
{
  pin_size_t idx = PinNameToIndex(pin);
  if (idx != NOT_A_PIN) {
    digitalWrite(idx, val);
  } else {
    mbed::DigitalOut(pin).write((int)val);
  }
}

void digitalWrite(pin_size_t pin, PinStatus val)
{
  if (pin >= PINS_COUNT) {
    return;
  }
	gpio_write(digitalPinToGpio(pin), (int)val);
}

PinStatus digitalRead(PinName pin)
{
  pin_size_t idx = PinNameToIndex(pin);
  if (idx != NOT_A_PIN) {
    return digitalRead(idx);
  } else {
    return (PinStatus)mbed::DigitalIn(pin).read();
  }
}

PinStatus digitalRead(pin_size_t pin)
{
  if (pin >= PINS_COUNT) {
    return LOW;
  }
	return (PinStatus)gpio_read(digitalPinToGpio(pin));
}