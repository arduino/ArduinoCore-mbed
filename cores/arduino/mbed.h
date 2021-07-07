#ifndef _MBED_WRAP_H_
#define _MBED_WRAP_H_

#include "Arduino.h"

#if defined(__cplusplus)
#if !defined(ARDUINO_AS_MBED_LIBRARY)
#include "mbed/mbed.h"
#else
#include_next "mbed.h"
#endif // !ARDUINO_AS_MBED_LIBRARY
#endif //__cplusplus

#endif //_MBED_WRAP_H_
