/*
  Copyright (c) 2012 Arduino.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "Arduino.h"
#include "pinDefinitions.h"

void detachInterrupt(PinName interruptNum) {
  pin_size_t idx = PinNameToIndex(interruptNum);
  if (idx != NOT_A_PIN) {
    detachInterrupt(idx);
  }
}

void detachInterrupt(pin_size_t interruptNum) {
  if ((interruptNum < PINS_COUNT) && (digitalPinToInterruptObj(interruptNum) != NULL)) {
    delete digitalPinToInterruptObj(interruptNum);
  }
}

void attachInterruptParam(PinName interruptNum, voidFuncPtrParam func, PinStatus mode, void* param) {
  pin_size_t idx = PinNameToIndex(interruptNum);
  if (idx != NOT_A_PIN) {
    attachInterruptParam(PinNameToIndex(interruptNum), func, mode, param);
  } else {
    mbed::InterruptIn* irq = new mbed::InterruptIn(interruptNum);
    if (mode == CHANGE) {
      irq->rise(mbed::callback(func, param));
      irq->fall(mbed::callback(func, param));
    } else if (mode == FALLING) {
      irq->fall(mbed::callback(func, param));
    } else {
      irq->rise(mbed::callback(func, param));
    }
  }
}

void attachInterrupt(PinName interruptNum, voidFuncPtr func, PinStatus mode) {
  attachInterruptParam(interruptNum, (voidFuncPtrParam)func, mode, NULL);
}

void attachInterruptParam(pin_size_t interruptNum, voidFuncPtrParam func, PinStatus mode, void* param) {
  if (interruptNum >= PINS_COUNT) {
    return;
  }
  detachInterrupt(interruptNum);
  mbed::InterruptIn* irq = new mbed::InterruptIn(digitalPinToPinName(interruptNum));
  if (mode == CHANGE) {
    irq->rise(mbed::callback(func, param));
    irq->fall(mbed::callback(func, param));
  } else if (mode == FALLING) {
    irq->fall(mbed::callback(func, param));
  } else {
    irq->rise(mbed::callback(func, param));
  }
  digitalPinToInterruptObj(interruptNum) = irq;
  // Give a default pullup for the pin, since calling InterruptIn with PinMode is impossible
  if (digitalPinToGpio(interruptNum) == NULL) {
    if (mode == FALLING) {
      pinMode(interruptNum, INPUT_PULLUP);
    } else if (mode == RISING) {
      pinMode(interruptNum, INPUT_PULLDOWN);
    } else {
      pinMode(interruptNum, INPUT);
    }
  }
}

void attachInterrupt(pin_size_t interruptNum, voidFuncPtr func, PinStatus mode) {
  attachInterruptParam(interruptNum, (voidFuncPtrParam)func, mode, NULL);
}
