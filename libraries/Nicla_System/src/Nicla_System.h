#ifndef Nicla_System_h_
#define Nicla_System_h_

#include "Arduino.h"
#include "BQ25120A.h"
#include "RGBled.h"

#include <mbed.h>
#include <I2C.h>

class nicla {

public:
  static bool begin();
  static bool enable3V3LDO();
  static bool enable1V8LDO();
  static bool disableLDO();
  static bool enterShipMode();

  static RGBled leds;
  static BQ25120A _pmic;

  friend class RGBled;
  friend class BQ25120A;
  friend class Arduino_BHY2;

  static bool started;

private:
  static void pingI2CThd();
  static uint8_t readLDOreg();
  static rtos::Mutex i2c_mutex;
};

#endif