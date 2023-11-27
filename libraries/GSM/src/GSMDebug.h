/*
  GSMDebug.h
  Copyright (c) 2021 Arduino SA.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef GSMDEBUG_H
#define GSMDEBUG_H

#if defined __has_include
  #if __has_include ("Arduino_DebugUtils.h")
    #include "Arduino_DebugUtils.h"
    #define GSM_DEBUG_ENABLE 1
  #else
    #define DEBUG_ERROR(fmt, ...)
    #define DEBUG_WARNING(fmt, ...)
    #define DEBUG_INFO(fmt, ...)
    #define DEBUG_DEBUG(fmt, ...)
    #define DEBUG_VERBOSE(fmt, ...)
    #define GSM_DEBUG_ENABLE 0
  #endif
#else
    #define DEBUG_ERROR(fmt, ...)
    #define DEBUG_WARNING(fmt, ...)
    #define DEBUG_INFO(fmt, ...)
    #define DEBUG_DEBUG(fmt, ...)
    #define DEBUG_VERBOSE(fmt, ...)
    #define GSM_DEBUG_ENABLE 0
#endif

#endif /* GSMDEBUG_H*/
