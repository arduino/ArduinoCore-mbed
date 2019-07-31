/*
  macros.h - basic porting layer for mbed-enabled boards
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

#pragma once
#ifdef USE_ARDUINO_PINOUT

#define analogPinToPinName(P)			(P < A0 ? g_APinDescription[P+A0].name : g_APinDescription[P].name)
#define digitalPinToPinName(P)			(g_APinDescription[P].name)
#define digitalPinToInterrupt(P)		(P)
#define digitalPinToInterruptObj(P)		(g_APinDescription[P].irq)
#define digitalPinToPwmObj(P)		    (g_APinDescription[P].pwm)

#else

#define analogPinToPinName(P)			((PinName)P)
#define digitalPinToPinName(P)			((PinName)P)
#define digitalPinToInterrupt(P) 		((PinName)P)

#endif

#define REDIRECT_STDOUT_TO(stream) 			namespace mbed {										\
											FileHandle *mbed_override_console(int fd) { 			\
												return &stream;										\
											}														\
											FileHandle *mbed_target_override_console(int fd) { 		\
												return &stream;										\
											}														\
											}
