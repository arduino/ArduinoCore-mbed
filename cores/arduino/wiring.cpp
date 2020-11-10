/*
  wiring.cpp
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2005-2006 David A. Mellis

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

#include "wiring_private.h"
#include "drivers/LowPowerTimer.h"
#include "drivers/Timer.h"
#include "rtos/rtos.h"
#include "platform/mbed_wait_api.h"

using namespace std::chrono_literals;
using namespace std::chrono;

static mbed::Timer timer;

#if DEVICE_LPTICKER
  static mbed::LowPowerTimer lowPowerTimer;
#else
  // Fallback for devices which don't support
  // a low power ticker.
  static mbed::Timer& lowPowerTimer = timer;
#endif

unsigned long millis()
{
  return duration_cast<milliseconds>(lowPowerTimer.elapsed_time()).count();
}

unsigned long micros() {
  return timer.elapsed_time().count();
}

void delay(unsigned long ms)
{
#ifndef NO_RTOS
  rtos::ThisThread::sleep_for(ms * 1ms);
#else
  wait_us(ms * 1000);
#endif
}

void delayMicroseconds(unsigned int us)
{
  wait_us(us);
}

void init()
{
  timer.start();
  lowPowerTimer.start();
}

void yield() {
#ifndef NO_RTOS
  rtos::ThisThread::yield();
#endif
}
