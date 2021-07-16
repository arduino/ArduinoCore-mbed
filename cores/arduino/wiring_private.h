//** just starting suggestion at this point --- jcw - 9/10/20
/*
  wiring_private.h
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

#ifndef WiringPrivate_h
#define WiringPrivate_h

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

// #include "gpio_object.h"

#include "Arduino.h"
#include "WVariant.h"
#ifdef __cplusplus
extern "C"{
#endif
// #include "wiring_constants.h"
/// #include "api/Common.h"			// H7 equivalent
typedef void (*voidFuncPtr)(void);
#if 0
typedef struct {
	uint32_t mask;
	__IO uint32_t *reg_in;
	__IO uint32_t *reg_set;
	__IO uint32_t *reg_clr;
	PinName  pin;
	GPIO_TypeDef *gpio;
	uint32_t ll_pin;
} gpio_t;
#endif
// gpio_t gpio;

int pinPeripheral( uint32_t ulPin, EPioType ulPeripheral );
void shiftOutMatrix(pin_size_t dataPin, uint8_t clockPin, BitOrder bitOrder, uint32_t val);
#ifdef __cplusplus
} // extern "C"
#endif

#endif
