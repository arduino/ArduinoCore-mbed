#ifndef Nicla_System_h_
#define Nicla_System_h_

#include "Arduino.h"
#include "BQ25120A.h"
#include "RGBled.h"

#include <mbed.h>
#include <I2C.h>

#define USE_FASTCHG_TO_KICK_WATCHDOG 1

class nicla {

public:
  static bool begin();
  static bool enable3V3LDO();
  static bool enable1V8LDO();
  static bool disableLDO();
  static bool enterShipMode();
  static uint8_t readLDOreg();
  static bool enableCharge(uint8_t mA = 20);

  static RGBled leds;
  static BQ25120A _pmic;

  friend class RGBled;
  friend class BQ25120A;
  friend class Arduino_BHY2;

  static bool started;

private:
  static void pingI2CThd();
  static void checkChgReg();
  static rtos::Mutex i2c_mutex;
  static uint8_t _chg_reg;
};

#endif