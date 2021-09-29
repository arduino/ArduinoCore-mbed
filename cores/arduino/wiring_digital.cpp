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
#include "pinDefinitions.h"

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
      default:
        mbed::DigitalIn(pin, PullDown);
        break;
    }
  }
}

void pinMode(pin_size_t pin, PinMode mode)
{
  mbed::DigitalInOut* gpio = digitalPinToGpio(pin);
  if (gpio != NULL) {
    delete gpio;
  }
  gpio = new mbed::DigitalInOut(digitalPinToPinName(pin));
  digitalPinToGpio(pin) = gpio;

  switch (mode) {
    case INPUT:
      gpio->input();
      gpio->mode(PullNone);
      break;
    case OUTPUT:
      gpio->output();
      break;
    case INPUT_PULLUP:
      gpio->input();
      gpio->mode(PullUp);
      break;
    case INPUT_PULLDOWN:
    default:
      gpio->input();
      gpio->mode(PullDown);
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
  mbed::DigitalInOut* gpio = digitalPinToGpio(pin);
  if (gpio == NULL) {
    gpio = new mbed::DigitalInOut(digitalPinToPinName(pin), PIN_OUTPUT, PullNone, val);
    digitalPinToGpio(pin) = gpio;
  }
  gpio->write(val);
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
  mbed::DigitalInOut* gpio = digitalPinToGpio(pin);
  if (gpio == NULL) {
    gpio = new mbed::DigitalInOut(digitalPinToPinName(pin), PIN_INPUT, PullNone, 0);
    digitalPinToGpio(pin) = gpio;
  }
  return (PinStatus) gpio->read();
}