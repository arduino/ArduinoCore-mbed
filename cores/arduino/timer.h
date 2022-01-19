#include "mbed.h"

enum TimerType {
  TIMER = 0x1,
  LPTIMER = 0x2
};

mbed::Timer* getTimer(TimerType t = TIMER);