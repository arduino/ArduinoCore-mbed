#include "Arduino.h"

#ifndef __ARDUINO_TIMER_H__
#define __ARDUINO_TIMER_H__

enum TimerType {
  TIMER = 0x1,
  LPTIMER = 0x2
};

typedef struct _mbed_timer mbed_timer;

namespace arduino {

  class ArduinoTimer {
    public:
      ArduinoTimer(void* _timer);
      ~ArduinoTimer();
      void start();
      void stop();
      void reset();

    private:
      mbed_timer* timer = NULL;
  };

}

arduino::ArduinoTimer getTimer(TimerType t = TIMER);

#endif //__ARDUINO_TIMER_H__


