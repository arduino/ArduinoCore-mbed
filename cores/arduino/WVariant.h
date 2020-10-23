/*
Copyright (c) 2015 Arduino LLC.  All right reserved.

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

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _EPioType
{
	PIO_NOT_A_PIN=-1,     /* Not under control of a peripheral. */
	PIO_EXTINT=0,         /* The pin is controlled by the associated signal of peripheral A. */
	PIO_ANALOG,           /* The pin is controlled by the associated signal of peripheral B. */
	PIO_SERCOM,           /* The pin is controlled by the associated signal of peripheral C. */
	PIO_SERCOM_ALT,       /* The pin is controlled by the associated signal of peripheral D. */
	PIO_TIMER,            /* The pin is controlled by the associated signal of peripheral E. */
	PIO_TIMER_ALT,        /* The pin is controlled by the associated signal of peripheral F. */
	PIO_COM,              /* The pin is controlled by the associated signal of peripheral G. */
	PIO_AC_CLK,           /* The pin is controlled by the associated signal of peripheral H. */
	PIO_DIGITAL,          /* The pin is controlled by PORT. */
	PIO_INPUT,            /* The pin is controlled by PORT and is an input. */
	PIO_INPUT_PULLUP,     /* The pin is controlled by PORT and is an input with internal pull-up resistor enabled. */
	PIO_OUTPUT,           /* The pin is controlled by PORT and is an output. */

	PIO_PWM=PIO_TIMER,
	PIO_PWM_ALT=PIO_TIMER_ALT,
} EPioType ;

/**
 * Pin Attributes to be OR-ed
 */
#define PIN_ATTR_NONE          (0UL<<0)
#define PIN_ATTR_COMBO         (1UL<<0)
#define PIN_ATTR_ANALOG        (1UL<<1)
#define PIN_ATTR_DIGITAL       (1UL<<2)
#define PIN_ATTR_PWM           (1UL<<3)
#define PIN_ATTR_TIMER         (1UL<<4)
#define PIN_ATTR_TIMER_ALT     (1UL<<5)
#define PIN_ATTR_EXTINT        (1UL<<6)


#ifdef __cplusplus
} // extern "C"
#endif

