/*
  GSM.h - Library for GSM on mbed platforms.
  Copyright (c) 2011-2023 Arduino LLC.  All right reserved.
  
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

#include <mbed.h>
#include <GSM.h>

#if MBED_CONF_MBED_TRACE_ENABLE

static Stream* trace_stream = nullptr;
static PlatformMutex trace_mutex;
static char trace_timestamp[8];

static void trace_wait() {
  trace_mutex.lock();
}

static void trace_release() {
  trace_mutex.unlock();
}

static char* trace_time(size_t ss) {
  auto ms = std::chrono::time_point_cast<std::chrono::milliseconds>(rtos::Kernel::Clock::now()).time_since_epoch().count();
  snprintf(trace_timestamp, 8, "[%08llu]", ms);
  return trace_timestamp;
}

static void trace_println(const char* c) {
  if (trace_stream) {
    trace_stream->println(c);
  }
}
#endif

void arduino::GSMClass::setTraceLevel(int trace_level, bool timestamp) {
#if MBED_CONF_MBED_TRACE_ENABLE
  switch(trace_level) {
    case 0:  mbed_trace_config_set(TRACE_ACTIVE_LEVEL_NONE);  break;
    case 1:  mbed_trace_config_set(TRACE_ACTIVE_LEVEL_CMD);   break;
    case 2:  mbed_trace_config_set(TRACE_ACTIVE_LEVEL_ERROR); break;
    case 3:  mbed_trace_config_set(TRACE_ACTIVE_LEVEL_WARN);  break;
    case 4:  mbed_trace_config_set(TRACE_ACTIVE_LEVEL_INFO);  break;
    case 5:  mbed_trace_config_set(TRACE_ACTIVE_LEVEL_DEBUG); break;
    case 6:  mbed_trace_config_set(TRACE_ACTIVE_LEVEL_ALL);   break;
    default: mbed_trace_config_set(TRACE_ACTIVE_LEVEL_ALL);   break;
  }

  if (timestamp) {
    mbed_trace_prefix_function_set( &trace_time );
  }
#endif
}

void arduino::GSMClass::trace(Stream& stream) {
#if MBED_CONF_MBED_TRACE_ENABLE
  trace_stream = &stream;

  mbed_trace_init();
  mbed_trace_config_set(TRACE_ACTIVE_LEVEL_ALL);
  mbed_trace_print_function_set(trace_println);
  mbed_trace_mutex_wait_function_set(trace_wait);
  mbed_trace_mutex_release_function_set(trace_release);
#endif
}
