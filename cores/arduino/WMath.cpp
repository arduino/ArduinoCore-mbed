/* -*- mode: jde; c-basic-offset: 2; indent-tabs-mode: nil -*- */

/*
  Part of the Wiring project - http://wiring.org.co
  Copyright (c) 2004-06 Hernando Barragan
  Modified 13 August 2006, David A. Mellis for Arduino - http://www.arduino.cc/
  
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

extern "C" {
  #include "stdlib.h"
  #include "hal/trng_api.h"
}
#if defined(ARDUINO_NANO_RP2040_CONNECT) || \
    defined(ARDUINO_PORTENTA_H7_M7) || \
    defined(ARDUINO_NICLA_VISION) || \
    defined(ARDUINO_OPTA) || \
    defined(ARDUINO_GIGA)
#define MBED_TRNG_SUPPORT 1
static long trng()
{
  trng_t trng_obj;
  trng_init(&trng_obj);
  long value;
  size_t olen;
  if (trng_get_bytes(&trng_obj, (uint8_t*)&value, sizeof(value), &olen) != 0)
    return -1;
  trng_free(&trng_obj);
  return value >= 0 ? value : -value;
}
#endif

#if (MBED_TRNG_SUPPORT == 1)
static bool useTRNG = true;
#endif

void randomSeed(unsigned long seed)
{
#if (MBED_TRNG_SUPPORT == 1)
  useTRNG = false;
#endif
  if (seed != 0) {
    srandom(seed);
  }
}

long random(long howbig)
{
  if (howbig == 0) {
    return 0;
  }
#if (MBED_TRNG_SUPPORT == 1)
  if (useTRNG == true) {
    return trng() % howbig;
  }
#endif
  return random() % howbig;
}

long random(long howsmall, long howbig)
{
  if (howsmall >= howbig) {
    return howsmall;
  }
  long diff = howbig - howsmall;
  return random(diff) + howsmall;
}

#undef MBED_TRNG_SUPPORT
