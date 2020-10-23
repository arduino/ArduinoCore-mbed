//** just starting suggestion at this point --- jcw - 9/10/20
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

// #include <Arduino.h>
// #include "gpio_object.h"
// #include "Wire.h"
// #include "api/Common.h"
#include <wiring_private.h>
// #include "device.h"
// #include "pinmap.h"
// #include "WVariant.h"
// #include "pinmode_arduino.h"
#include "pins_arduino.h"
#include "mbed.h"


static uint8_t pin = 4;
// static gpio_t gpio;
// gpio_t gpio;

// extern PinDescription g_APinDescription[];
#if 0
PinDescription g_APinDescription[] = {
  // D0 - D7
	{ PH_15,        NULL, NULL, NULL },    // D0
	{ PK_1,         NULL, NULL, NULL },    // D1
	{ PJ_11,        NULL, NULL, NULL },    // D2
	{ PG_7,         NULL, NULL, NULL },    // D3
	{ PC_7,         NULL, NULL, NULL },    // D4
	{ PC_6,         NULL, NULL, NULL },    // D5
	{ PA_8,         NULL, NULL, NULL },    // D6
	{ PI_0,         NULL, NULL, NULL },    // D7

  // D8 - D14
	{ PC_3,         NULL, NULL, NULL },    // D8
	{ PI_1,         NULL, NULL, NULL },    // D9
	{ PC_2,         NULL, NULL, NULL },    // D10
	{ PH_8,         NULL, NULL, NULL },    // D11
	{ PH_7,         NULL, NULL, NULL },    // D12
	{ PA_10,        NULL, NULL, NULL },    // D13
	{ PA_9,         NULL, NULL, NULL },    // D14

  // A0 - A6
	{ PA_0C,        NULL, NULL, NULL },    // A0    ADC2_INP0
	{ PA_1C,        NULL, NULL, NULL },    // A1    ADC2_INP1
	{ PC_2C,        NULL, NULL, NULL },    // A2    ADC3_INP0
	{ PC_3C,        NULL, NULL, NULL },    // A3    ADC3_INP1
	{ PC_2_ALT0,    NULL, NULL, NULL },    // A4    ADC1_INP12
	{ PC_3_ALT0,    NULL, NULL, NULL },    // A5    ADC1_INP13
	{ PA_4,         NULL, NULL, NULL },    // A6    ADC1_INP18

  // LEDS
	{ PK_5,         NULL, NULL, NULL },    // LEDR
	{ PK_6,         NULL, NULL, NULL },    // LEDG
	{ PK_7,         NULL, NULL, NULL },    // LEDB
};
#endif


extern struct PinDescription g_APinDescription;
// extern void pinMode(pin_size_t pinNumber, PinMode pinMode);

int pinPeripheral( uint32_t ulPin, EPioType ulPeripheral )
{

	
	
  // Handle the case the pin isn't usable as PIO
//	if ( g_APinDescription[ulPin].ulPinType == PIO_NOT_A_PIN )
//	if ( g_APinDescription[ulPin].gpio == PIO_NOT_A_PIN )
//	if ( g_APinDescription[pin].gpio == PIO_NOT_A_PIN )
//		if ( g_APinDescription[pin] == PIO_NOT_A_PIN )
//  {
//    return -1 ;
//  }

  switch ( ulPeripheral )
  {
    case PIO_DIGITAL:
    case PIO_INPUT:
    case PIO_INPUT_PULLUP:
    case PIO_OUTPUT:
      // Disable peripheral muxing, done in pinMode
//                      PORT->Group[g_APinDescription[ulPin].ulPort].PINCFG[g_APinDescription[ulPin].ulPin].bit.PMUXEN = 0 ;

      // Configure pin mode, if requested
      if ( ulPeripheral == PIO_INPUT )
      {
	      pin_mode( ulPin, INPUT ) ;	// pinMode( ulPin, INPUT ) ;
	      
//	      pinMode( ulPin, INPUT ) ;	// pinMode( ulPin, INPUT ) ;
      }
      else
      {
        if ( ulPeripheral == PIO_INPUT_PULLUP )
        {
		pin_mode( ulPin, INPUT_PULLUP ) ;   // pinMode( ulPin, INPUT_PULLUP ) ;
//		pinMode( ulPin, INPUT_PULLUP ) ;   // pinMode( ulPin, INPUT_PULLUP ) ;
        }
        else
        {
          if ( ulPeripheral == PIO_OUTPUT )
          {
		  pin_mode( ulPin, OUTPUT ) ;         // pinMode( ulPin, OUTPUT ) ;
//		  gpio_init_out(&gpio, ulPin);
//		  pinMode( ulPin, OUTPUT ) ;         // pinMode( ulPin, OUTPUT ) ;
          }
          else
          {
            // PIO_DIGITAL, do we have to do something as all cases are covered?
          }
        }
      }
    break ;

    case PIO_ANALOG:
    case PIO_SERCOM:
    case PIO_SERCOM_ALT:
    case PIO_TIMER:
    case PIO_TIMER_ALT:
    case PIO_EXTINT:
    case PIO_COM:
    case PIO_AC_CLK:
#if 0
      // Is the pio pin in the lower 16 ones?
      // The WRCONFIG register allows update of only 16 pin max out of 32
      if ( g_APinDescription[ulPin].ulPin < 16 )
      {
        PORT->Group[g_APinDescription[ulPin].ulPort].WRCONFIG.reg = PORT_WRCONFIG_WRPMUX | PORT_WRCONFIG_PMUXEN | PORT_WRCONFIG_PMUX( ulPeripheral ) |
                                                                    PORT_WRCONFIG_WRPINCFG |
                                                                    PORT_WRCONFIG_PINMASK( g_APinDescription[ulPin].ulPin ) ;
      }
      else
      {
        PORT->Group[g_APinDescription[ulPin].ulPort].WRCONFIG.reg = PORT_WRCONFIG_HWSEL |
                                                                    PORT_WRCONFIG_WRPMUX | PORT_WRCONFIG_PMUXEN | PORT_WRCONFIG_PMUX( ulPeripheral ) |
                                                                    PORT_WRCONFIG_WRPINCFG |
                                                                    PORT_WRCONFIG_PINMASK( g_APinDescription[ulPin].ulPin - 16 ) ;
      }
#else
#if 0       
      if ( g_APinDescription[ulPin].ulPin & 1 ) // is pin odd?
      {
        uint32_t temp ;

        // Get whole current setup for both odd and even pins and remove odd one
        temp = (PORT->Group[g_APinDescription[ulPin].ulPort].PMUX[g_APinDescription[ulPin].ulPin >> 1].reg) & PORT_PMUX_PMUXE( 0xF ) ;
        // Set new muxing
        PORT->Group[g_APinDescription[ulPin].ulPort].PMUX[g_APinDescription[ulPin].ulPin >> 1].reg = temp|PORT_PMUX_PMUXO( ulPeripheral ) ;
        // Enable port mux
        PORT->Group[g_APinDescription[ulPin].ulPort].PINCFG[g_APinDescription[ulPin].ulPin].reg |= PORT_PINCFG_PMUXEN ;
      }
      else // even pin
      {
        uint32_t temp ;

        temp = (PORT->Group[g_APinDescription[ulPin].ulPort].PMUX[g_APinDescription[ulPin].ulPin >> 1].reg) & PORT_PMUX_PMUXO( 0xF ) ;
        PORT->Group[g_APinDescription[ulPin].ulPort].PMUX[g_APinDescription[ulPin].ulPin >> 1].reg = temp|PORT_PMUX_PMUXE( ulPeripheral ) ;
        PORT->Group[g_APinDescription[ulPin].ulPort].PINCFG[g_APinDescription[ulPin].ulPin].reg |= PORT_PINCFG_PMUXEN ; // Enable port mux
      }
#endif      
#endif
    break ;

    case PIO_NOT_A_PIN:
      return -1l ;
    break ;
  }

  return 0l ;
}



void shiftOutMatrix(pin_size_t dataPin, uint8_t clockPin, BitOrder bitOrder, uint32_t val)
{
	uint32_t i;

	for (i = 0; i < 32; i++)  {
		if (bitOrder == LSBFIRST)
			digitalWrite(dataPin, !!(val & (1 << i)) ? HIGH : LOW);
		else	
			digitalWrite(dataPin, !!(val & (1 << (31 - i))) ? HIGH : LOW);

		digitalWrite(clockPin, HIGH);
		digitalWrite(clockPin, LOW);		
	}
}

