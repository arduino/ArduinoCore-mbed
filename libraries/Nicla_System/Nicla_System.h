#ifndef Nicla_System_h_
#define Nicla_System_h_

#include "Arduino.h"
#include "BQ25120A.h"
#include "RGBled.h"

#include <mbed.h>
#include <I2C.h>

namespace nicla {

  bool begin();
  bool enable3V3LDO();
  bool enable1V8LDO();
  bool disableLDO();
  uint8_t readLDOreg();

  static RGBled leds;

}

#endif