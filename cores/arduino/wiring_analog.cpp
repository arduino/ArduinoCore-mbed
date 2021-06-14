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
#include "pinDefinitions.h"

static int write_resolution = 8;
static int read_resolution = 10;

#if DEVICE_ANALOGOUT
#include "drivers/AnalogOut.h"
mbed::AnalogOut* dac = NULL;
void analogWriteDAC(PinName pin, int val) {
  if (dac == NULL) {
    dac = new mbed::AnalogOut(pin);
  }
  float percent = (float)val/(float)((1 << write_resolution)-1);
  if (percent > 1.0f) {
    percent = 1.0f;
  }
  dac->write(percent);
}
#endif

void analogWrite(PinName pin, int val)
{
  pin_size_t idx = PinNameToIndex(pin);
  if (idx != NOT_A_PIN) {
    analogWrite(idx, val);
  } else {
    mbed::PwmOut* pwm = new mbed::PwmOut(pin);
    pwm->period_ms(2); //500Hz
    float percent = (float)val/(float)((1 << write_resolution)-1);
    pwm->write(percent);
  }
}

void analogWrite(pin_size_t pin, int val)
{
  if (pin >= PINS_COUNT) {
    return;
  }
#ifdef DAC
    if (pin == DAC) {
      analogWriteDAC(digitalPinToPinName(pin), val);
      return;
    }
#endif
  float percent = (float)val/(float)((1 << write_resolution)-1);
  mbed::PwmOut* pwm = digitalPinToPwm(pin);
  if (pwm == NULL) {
    pwm = new mbed::PwmOut(digitalPinToPinName(pin));
    digitalPinToPwm(pin) = pwm;
    pwm->period_ms(2); //500Hz
  }
  if (percent < 0) {
    delete pwm;
    digitalPinToPwm(pin) = NULL;
  } else {
    pwm->write(percent);
  }
}

void analogWriteResolution(int bits)
{
  write_resolution = bits;
}

int analogRead(PinName pin)
{
  for (pin_size_t i = 0; i < NUM_ANALOG_INPUTS; i++) {
    if (analogPinToPinName(i) == pin) {
      return analogRead(i + A0);
    }
  }
  return -1;
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
  mbed::AnalogIn* adc = analogPinToAdcObj(pin);
  if (adc == NULL) {
    adc = new mbed::AnalogIn(name);
    analogPinToAdcObj(pin) = adc;
#ifdef ANALOG_CONFIG
    if (isAdcConfigChanged) {
      adc->configure(adcCurrentConfig);
    }
#endif
  }
  return (adc->read_u16() >> (16 - read_resolution));
}

#ifdef ANALOG_CONFIG
/* Spot all active ADCs to reconfigure them */
void analogUpdate() 
{
  isAdcConfigChanged = true;
  //for (pin_size_t i = A0; i < A0 + NUM_ANALOG_INPUTS; i++) {  //also the other works
  for (pin_size_t i = 0; i < NUM_ANALOG_INPUTS; i++) {
    if (analogPinToAdcObj(i) != NULL) {
      analogPinToAdcObj(i)->configure(adcCurrentConfig);
    }
  }
}
#endif

void analogReadResolution(int bits)
{
  read_resolution = bits;
}

int getAnalogReadResolution()
{
  return read_resolution;
}