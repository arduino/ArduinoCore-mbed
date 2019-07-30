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

#ifdef digitalPinToInterruptObj
static mbed::InterruptIn* PinNameToInterruptObj(PinName P) {
  // reverse search for pinName in g_APinDescription[P].name fields
  for (pin_size_t i=0; i < PINS_COUNT; i++) {
    if (g_APinDescription[i].name == P) {
      return g_APinDescription[i].irq;
    }
  }
  return NULL;
}
#endif

void detachInterrupt(PinName interruptNum) {
#ifdef digitalPinToInterruptObj
  if (PinNameToInterruptObj(interruptNum) != NULL) {
    delete PinNameToInterruptObj(interruptNum);
  }
#endif
}

void detachInterrupt(pin_size_t interruptNum) {
#ifdef digitalPinToInterruptObj
  if (digitalPinToInterruptObj(interruptNum) != NULL) {
    delete digitalPinToInterruptObj(interruptNum);
  }
#endif
}

void attachInterruptParam(PinName interruptNum, voidFuncPtrParam func, PinStatus mode, void* param) {
  detachInterrupt(interruptNum);
  mbed::InterruptIn* irq = new mbed::InterruptIn(interruptNum);
  if (mode == FALLING) {
    irq->fall(mbed::callback(func, param));
  } else {
    irq->rise(mbed::callback(func, param));
  }
#ifdef digitalPinToInterruptObj
  digitalPinToInterruptObj(interruptNum) = irq;
#endif
}

void attachInterrupt(PinName interruptNum, voidFuncPtr func, PinStatus mode) {
  attachInterruptParam(interruptNum, (voidFuncPtrParam)func, mode, NULL);
}

void attachInterruptParam(pin_size_t interruptNum, voidFuncPtrParam func, PinStatus mode, void* param) {
  detachInterrupt(interruptNum);
  mbed::InterruptIn* irq = new mbed::InterruptIn(digitalPinToPinName(interruptNum));
  if (mode == FALLING) {
    irq->fall(mbed::callback(func, param));
  } else {
    irq->rise(mbed::callback(func, param));
  }
#ifdef digitalPinToInterruptObj
  digitalPinToInterruptObj(interruptNum) = irq;
#endif
}

void attachInterrupt(pin_size_t interruptNum, voidFuncPtr func, PinStatus mode) {
  attachInterruptParam(interruptNum, (voidFuncPtrParam)func, mode, NULL);
}