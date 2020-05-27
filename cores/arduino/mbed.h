#ifndef _MBED_WRAP_H_
#define _MBED_WRAP_H_

#include "Arduino.h"

#if defined(__cplusplus)
#if !defined(ARDUINO_AS_MBED_LIBRARY)
#ifdef F
#define Arduino_F F
#undef F
#endif // F (mbed included after arduino.h)
#define F Mbed_F
#endif // !ARDUINO_AS_MBED_LIBRARY
#include "mbed/mbed.h"
#undef F
#endif //__cplusplus

#endif //_MBED_WRAP_H_
